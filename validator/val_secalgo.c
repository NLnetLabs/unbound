/*
 * validator/val_secalgo.c - validator security algorithm functions.
 *
 * Copyright (c) 2012, NLnet Labs. All rights reserved.
 *
 * This software is open source.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * Neither the name of the NLNET LABS nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *
 * This file contains helper functions for the validator module.
 * These functions take raw data buffers, formatted for crypto verification,
 * and do the library calls (for the crypto library in use).
 */
#include "config.h"
#include <ldns/ldns.h>
#include "validator/val_secalgo.h"
#include "util/data/packed_rrset.h"
#include "util/log.h"

#if !defined(HAVE_SSL) && !defined(HAVE_NSS)
#error "Need crypto library to do digital signature cryptography"
#endif

/* OpenSSL implementation */
#ifdef HAVE_SSL
#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL_RAND_H
#include <openssl/rand.h>
#endif

#ifdef HAVE_OPENSSL_CONF_H
#include <openssl/conf.h>
#endif

#ifdef HAVE_OPENSSL_ENGINE_H
#include <openssl/engine.h>
#endif

/**
 * Return size of DS digest according to its hash algorithm.
 * @param algo: DS digest algo.
 * @return size in bytes of digest, or 0 if not supported. 
 */
size_t
ds_digest_size_supported(int algo)
{
	switch(algo) {
#ifdef HAVE_EVP_SHA1
		case LDNS_SHA1:
			return SHA_DIGEST_LENGTH;
#endif
#ifdef HAVE_EVP_SHA256
		case LDNS_SHA256:
			return SHA256_DIGEST_LENGTH;
#endif
#ifdef USE_GOST
		case LDNS_HASH_GOST:
			if(EVP_get_digestbyname("md_gost94"))
				return 32;
			else	return 0;
#endif
#ifdef USE_ECDSA
		case LDNS_SHA384:
			return SHA384_DIGEST_LENGTH;
#endif
		default: break;
	}
	return 0;
}

#ifdef USE_GOST
/** Perform GOST hash */
static int
do_gost94(unsigned char* data, size_t len, unsigned char* dest)
{
	const EVP_MD* md = EVP_get_digestbyname("md_gost94");
	if(!md) 
		return 0;
	return ldns_digest_evp(data, (unsigned int)len, dest, md);
}
#endif

int
secalgo_ds_digest(int algo, unsigned char* buf, size_t len,
	unsigned char* res)
{
	switch(algo) {
#ifdef HAVE_EVP_SHA1
		case LDNS_SHA1:
			(void)SHA1(buf, len, res);
			return 1;
#endif
#ifdef HAVE_EVP_SHA256
		case LDNS_SHA256:
			(void)SHA256(buf, len, res);
			return 1;
#endif
#ifdef USE_GOST
		case LDNS_HASH_GOST:
			if(do_gost94(buf, len, res))
				return 1;
#endif
#ifdef USE_ECDSA
		case LDNS_SHA384:
			(void)SHA384(buf, len, res);
			return 1;
#endif
		default: 
			verbose(VERB_QUERY, "unknown DS digest algorithm %d", 
				algo);
			break;
	}
	return 0;
}

/** return true if DNSKEY algorithm id is supported */
int
dnskey_algo_id_is_supported(int id)
{
	switch(id) {
	case LDNS_DSA:
	case LDNS_DSA_NSEC3:
	case LDNS_RSASHA1:
	case LDNS_RSASHA1_NSEC3:
	case LDNS_RSAMD5:
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
	case LDNS_RSASHA256:
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
	case LDNS_RSASHA512:
#endif
#ifdef USE_ECDSA
	case LDNS_ECDSAP256SHA256:
	case LDNS_ECDSAP384SHA384:
#endif
		return 1;
#ifdef USE_GOST
	case LDNS_ECC_GOST:
		/* we support GOST if it can be loaded */
		return ldns_key_EVP_load_gost_id();
#endif
	default:
		return 0;
	}
}

/**
 * Output a libcrypto openssl error to the logfile.
 * @param str: string to add to it.
 * @param e: the error to output, error number from ERR_get_error().
 */
static void
log_crypto_error(const char* str, unsigned long e)
{
	char buf[128];
	/* or use ERR_error_string if ERR_error_string_n is not avail TODO */
	ERR_error_string_n(e, buf, sizeof(buf));
	/* buf now contains */
	/* error:[error code]:[library name]:[function name]:[reason string] */
	log_err("%s crypto %s", str, buf);
}

/**
 * Setup DSA key digest in DER encoding ... 
 * @param sig: input is signature output alloced ptr (unless failure).
 * 	caller must free alloced ptr if this routine returns true.
 * @param len: input is initial siglen, output is output len.
 * @return false on failure.
 */
static int
setup_dsa_sig(unsigned char** sig, unsigned int* len)
{
	unsigned char* orig = *sig;
	unsigned int origlen = *len;
	int newlen;
	BIGNUM *R, *S;
	DSA_SIG *dsasig;

	/* extract the R and S field from the sig buffer */
	if(origlen < 1 + 2*SHA_DIGEST_LENGTH)
		return 0;
	R = BN_new();
	if(!R) return 0;
	(void) BN_bin2bn(orig + 1, SHA_DIGEST_LENGTH, R);
	S = BN_new();
	if(!S) return 0;
	(void) BN_bin2bn(orig + 21, SHA_DIGEST_LENGTH, S);
	dsasig = DSA_SIG_new();
	if(!dsasig) return 0;

	dsasig->r = R;
	dsasig->s = S;
	*sig = NULL;
	newlen = i2d_DSA_SIG(dsasig, sig);
	if(newlen < 0) {
		DSA_SIG_free(dsasig);
		free(*sig);
		return 0;
	}
	*len = (unsigned int)newlen;
	DSA_SIG_free(dsasig);
	return 1;
}

#ifdef USE_ECDSA
/**
 * Setup the ECDSA signature in its encoding that the library wants.
 * Converts from plain numbers to ASN formatted.
 * @param sig: input is signature, output alloced ptr (unless failure).
 * 	caller must free alloced ptr if this routine returns true.
 * @param len: input is initial siglen, output is output len.
 * @return false on failure.
 */
static int
setup_ecdsa_sig(unsigned char** sig, unsigned int* len)
{
	ECDSA_SIG* ecdsa_sig;
	int newlen;
	int bnsize = (int)((*len)/2);
	/* if too short or not even length, fails */
	if(*len < 16 || bnsize*2 != (int)*len)
		return 0;
	/* use the raw data to parse two evenly long BIGNUMs, "r | s". */
	ecdsa_sig = ECDSA_SIG_new();
	if(!ecdsa_sig) return 0;
	ecdsa_sig->r = BN_bin2bn(*sig, bnsize, ecdsa_sig->r);
	ecdsa_sig->s = BN_bin2bn(*sig+bnsize, bnsize, ecdsa_sig->s);
	if(!ecdsa_sig->r || !ecdsa_sig->s) {
		ECDSA_SIG_free(ecdsa_sig);
		return 0;
	}

	/* spool it into ASN format */
	*sig = NULL;
	newlen = i2d_ECDSA_SIG(ecdsa_sig, sig);
	if(newlen <= 0) {
		ECDSA_SIG_free(ecdsa_sig);
		free(*sig);
		return 0;
	}
	*len = (unsigned int)newlen;
	ECDSA_SIG_free(ecdsa_sig);
	return 1;
}
#endif /* USE_ECDSA */

/**
 * Setup key and digest for verification. Adjust sig if necessary.
 *
 * @param algo: key algorithm
 * @param evp_key: EVP PKEY public key to create.
 * @param digest_type: digest type to use
 * @param key: key to setup for.
 * @param keylen: length of key.
 * @return false on failure.
 */
static int
setup_key_digest(int algo, EVP_PKEY** evp_key, const EVP_MD** digest_type, 
	unsigned char* key, size_t keylen)
{
	DSA* dsa;
	RSA* rsa;

	switch(algo) {
		case LDNS_DSA:
		case LDNS_DSA_NSEC3:
			*evp_key = EVP_PKEY_new();
			if(!*evp_key) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			dsa = ldns_key_buf2dsa_raw(key, keylen);
			if(!dsa) {
				verbose(VERB_QUERY, "verify: "
					"ldns_key_buf2dsa_raw failed");
				return 0;
			}
			if(EVP_PKEY_assign_DSA(*evp_key, dsa) == 0) {
				verbose(VERB_QUERY, "verify: "
					"EVP_PKEY_assign_DSA failed");
				return 0;
			}
			*digest_type = EVP_dss1();

			break;
		case LDNS_RSASHA1:
		case LDNS_RSASHA1_NSEC3:
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
		case LDNS_RSASHA256:
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
		case LDNS_RSASHA512:
#endif
			*evp_key = EVP_PKEY_new();
			if(!*evp_key) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			rsa = ldns_key_buf2rsa_raw(key, keylen);
			if(!rsa) {
				verbose(VERB_QUERY, "verify: "
					"ldns_key_buf2rsa_raw SHA failed");
				return 0;
			}
			if(EVP_PKEY_assign_RSA(*evp_key, rsa) == 0) {
				verbose(VERB_QUERY, "verify: "
					"EVP_PKEY_assign_RSA SHA failed");
				return 0;
			}

			/* select SHA version */
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
			if(algo == LDNS_RSASHA256)
				*digest_type = EVP_sha256();
			else
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
				if(algo == LDNS_RSASHA512)
				*digest_type = EVP_sha512();
			else
#endif
				*digest_type = EVP_sha1();

			break;
		case LDNS_RSAMD5:
			*evp_key = EVP_PKEY_new();
			if(!*evp_key) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			rsa = ldns_key_buf2rsa_raw(key, keylen);
			if(!rsa) {
				verbose(VERB_QUERY, "verify: "
					"ldns_key_buf2rsa_raw MD5 failed");
				return 0;
			}
			if(EVP_PKEY_assign_RSA(*evp_key, rsa) == 0) {
				verbose(VERB_QUERY, "verify: "
					"EVP_PKEY_assign_RSA MD5 failed");
				return 0;
			}
			*digest_type = EVP_md5();

			break;
#ifdef USE_GOST
		case LDNS_ECC_GOST:
			*evp_key = ldns_gost2pkey_raw(key, keylen);
			if(!*evp_key) {
				verbose(VERB_QUERY, "verify: "
					"ldns_gost2pkey_raw failed");
				return 0;
			}
			*digest_type = EVP_get_digestbyname("md_gost94");
			if(!*digest_type) {
				verbose(VERB_QUERY, "verify: "
					"EVP_getdigest md_gost94 failed");
				return 0;
			}
			break;
#endif
#ifdef USE_ECDSA
		case LDNS_ECDSAP256SHA256:
			*evp_key = ldns_ecdsa2pkey_raw(key, keylen,
				LDNS_ECDSAP256SHA256);
			if(!*evp_key) {
				verbose(VERB_QUERY, "verify: "
					"ldns_ecdsa2pkey_raw failed");
				return 0;
			}
#ifdef USE_ECDSA_EVP_WORKAROUND
			/* openssl before 1.0.0 fixes RSA with the SHA256
			 * hash in EVP.  We create one for ecdsa_sha256 */
			{
				static int md_ecdsa_256_done = 0;
				static EVP_MD md;
				if(!md_ecdsa_256_done) {
					EVP_MD m = *EVP_sha256();
					md_ecdsa_256_done = 1;
					m.required_pkey_type[0] = (*evp_key)->type;
					m.verify = (void*)ECDSA_verify;
					md = m;
				}
				*digest_type = &md;
			}
#else
			*digest_type = EVP_sha256();
#endif
			break;
		case LDNS_ECDSAP384SHA384:
			*evp_key = ldns_ecdsa2pkey_raw(key, keylen,
				LDNS_ECDSAP384SHA384);
			if(!*evp_key) {
				verbose(VERB_QUERY, "verify: "
					"ldns_ecdsa2pkey_raw failed");
				return 0;
			}
#ifdef USE_ECDSA_EVP_WORKAROUND
			/* openssl before 1.0.0 fixes RSA with the SHA384
			 * hash in EVP.  We create one for ecdsa_sha384 */
			{
				static int md_ecdsa_384_done = 0;
				static EVP_MD md;
				if(!md_ecdsa_384_done) {
					EVP_MD m = *EVP_sha384();
					md_ecdsa_384_done = 1;
					m.required_pkey_type[0] = (*evp_key)->type;
					m.verify = (void*)ECDSA_verify;
					md = m;
				}
				*digest_type = &md;
			}
#else
			*digest_type = EVP_sha384();
#endif
			break;
#endif /* USE_ECDSA */
		default:
			verbose(VERB_QUERY, "verify: unknown algorithm %d", 
				algo);
			return 0;
	}
	return 1;
}

/**
 * Check a canonical sig+rrset and signature against a dnskey
 * @param buf: buffer with data to verify, the first rrsig part and the
 *	canonicalized rrset.
 * @param algo: DNSKEY algorithm.
 * @param sigblock: signature rdata field from RRSIG
 * @param sigblock_len: length of sigblock data.
 * @param key: public key data from DNSKEY RR.
 * @param keylen: length of keydata.
 * @param reason: bogus reason in more detail.
 * @return secure if verification succeeded, bogus on crypto failure,
 *	unchecked on format errors and alloc failures.
 */
enum sec_status
verify_canonrrset(ldns_buffer* buf, int algo, unsigned char* sigblock, 
	unsigned int sigblock_len, unsigned char* key, unsigned int keylen,
	char** reason)
{
	const EVP_MD *digest_type;
	EVP_MD_CTX ctx;
	int res, dofree = 0;
	EVP_PKEY *evp_key = NULL;
	
	if(!setup_key_digest(algo, &evp_key, &digest_type, key, keylen)) {
		verbose(VERB_QUERY, "verify: failed to setup key");
		*reason = "use of key for crypto failed";
		EVP_PKEY_free(evp_key);
		return sec_status_bogus;
	}
	/* if it is a DSA signature in bind format, convert to DER format */
	if((algo == LDNS_DSA || algo == LDNS_DSA_NSEC3) && 
		sigblock_len == 1+2*SHA_DIGEST_LENGTH) {
		if(!setup_dsa_sig(&sigblock, &sigblock_len)) {
			verbose(VERB_QUERY, "verify: failed to setup DSA sig");
			*reason = "use of key for DSA crypto failed";
			EVP_PKEY_free(evp_key);
			return sec_status_bogus;
		}
		dofree = 1;
	}
#ifdef USE_ECDSA
	else if(algo == LDNS_ECDSAP256SHA256 || algo == LDNS_ECDSAP384SHA384) {
		/* EVP uses ASN prefix on sig, which is not in the wire data */
		if(!setup_ecdsa_sig(&sigblock, &sigblock_len)) {
			verbose(VERB_QUERY, "verify: failed to setup ECDSA sig");
			*reason = "use of signature for ECDSA crypto failed";
			EVP_PKEY_free(evp_key);
			return sec_status_bogus;
		}
		dofree = 1;
	}
#endif /* USE_ECDSA */

	/* do the signature cryptography work */
	EVP_MD_CTX_init(&ctx);
	if(EVP_VerifyInit(&ctx, digest_type) == 0) {
		verbose(VERB_QUERY, "verify: EVP_VerifyInit failed");
		EVP_PKEY_free(evp_key);
		if(dofree) free(sigblock);
		return sec_status_unchecked;
	}
	if(EVP_VerifyUpdate(&ctx, (unsigned char*)ldns_buffer_begin(buf), 
		(unsigned int)ldns_buffer_limit(buf)) == 0) {
		verbose(VERB_QUERY, "verify: EVP_VerifyUpdate failed");
		EVP_PKEY_free(evp_key);
		if(dofree) free(sigblock);
		return sec_status_unchecked;
	}

	res = EVP_VerifyFinal(&ctx, sigblock, sigblock_len, evp_key);
	if(EVP_MD_CTX_cleanup(&ctx) == 0) {
		verbose(VERB_QUERY, "verify: EVP_MD_CTX_cleanup failed");
		EVP_PKEY_free(evp_key);
		if(dofree) free(sigblock);
		return sec_status_unchecked;
	}
	EVP_PKEY_free(evp_key);

	if(dofree)
		free(sigblock);

	if(res == 1) {
		return sec_status_secure;
	} else if(res == 0) {
		verbose(VERB_QUERY, "verify: signature mismatch");
		*reason = "signature crypto failed";
		return sec_status_bogus;
	}

	log_crypto_error("verify:", ERR_get_error());
	return sec_status_unchecked;
}

/**************************************************/
#elif defined(HAVE_NSS)
/* libnss implementation */
#include <nss3/sechash.h>
#include <nss3/pk11pub.h>
#include <nss3/keyhi.h>
#include <nss3/secerr.h>
#include <nspr4/prerror.h>

size_t
ds_digest_size_supported(int algo)
{
	/* uses libNSS */
	switch(algo) {
#ifdef HAVE_EVP_SHA1
		case LDNS_SHA1:
			return SHA1_LENGTH;
#endif
#ifdef HAVE_EVP_SHA256
		case LDNS_SHA256:
			return SHA256_LENGTH;
#endif
#ifdef USE_ECDSA
		case LDNS_SHA384:
			return SHA384_LENGTH;
#endif
		/* GOST not supported in NSS */
		case LDNS_HASH_GOST:
		default: break;
	}
	return 0;
}

int
secalgo_ds_digest(int algo, unsigned char* buf, size_t len,
	unsigned char* res)
{
	/* uses libNSS */
	switch(algo) {
#ifdef HAVE_EVP_SHA1
		case LDNS_SHA1:
			return HASH_HashBuf(HASH_AlgSHA1, res, buf, len)
				== SECSuccess;
#endif
#ifdef HAVE_EVP_SHA256
		case LDNS_SHA256:
			return HASH_HashBuf(HASH_AlgSHA256, res, buf, len)
				== SECSuccess;
#endif
#ifdef USE_ECDSA
		case LDNS_SHA384:
			return HASH_HashBuf(HASH_AlgSHA384, res, buf, len)
				== SECSuccess;
#endif
		case LDNS_HASH_GOST:
		default: 
			verbose(VERB_QUERY, "unknown DS digest algorithm %d", 
				algo);
			break;
	}
	return 0;
}

int
dnskey_algo_id_is_supported(int id)
{
	/* uses libNSS */
	switch(id) {
	case LDNS_DSA:
	case LDNS_DSA_NSEC3:
	case LDNS_RSASHA1:
	case LDNS_RSASHA1_NSEC3:
	case LDNS_RSAMD5:
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
	case LDNS_RSASHA256:
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
	case LDNS_RSASHA512:
#endif
#ifdef USE_ECDSA
	case LDNS_ECDSAP256SHA256:
	case LDNS_ECDSAP384SHA384:
#endif
		return 1;
	case LDNS_ECC_GOST:
	default:
		return 0;
	}
}

/* return a new public key for NSS */
static SECKEYPublicKey* nss_key_create(KeyType ktype)
{
	SECKEYPublicKey* key;
	PLArenaPool* arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
	if(!arena) {
		log_err("out of memory, PORT_NewArena failed");
		return NULL;
	}
	key = PORT_ArenaZNew(arena, SECKEYPublicKey);
	if(!key) {
		log_err("out of memory, PORT_ArenaZNew failed");
		PORT_FreeArena(arena, PR_FALSE);
		return NULL;
	}
	key->arena = arena;
	key->keyType = ktype;
	key->pkcs11Slot = NULL;
	key->pkcs11ID = CK_INVALID_HANDLE;
	return key;
}

static SECKEYPublicKey* nss_buf2rsa(unsigned char* key, size_t len)
{
	SECKEYPublicKey* pk;
	uint16_t exp;
	uint16_t offset;
	uint16_t int16;
	SECItem modulus = {siBuffer, NULL, 0};
	SECItem exponent = {siBuffer, NULL, 0};
	if(len == 0)
		return NULL;
	if(key[0] == 0) {
		if(len < 3)
			return NULL;
		/* the exponent is too large so it's places further */
		memmove(&int16, key+1, 2);
		exp = ntohs(int16);
		offset = 3;
	} else {
		exp = key[0];
		offset = 1;
	}

	/* key length at least one */
	if(len < (size_t)offset + exp + 1)
		return NULL;
	
	exponent.data = key+offset;
	exponent.len = exp;
	offset += exp;
	modulus.data = key+offset;
	modulus.len = (len - offset);

	pk = nss_key_create(rsaKey);
	if(SECITEM_CopyItem(pk->arena, &pk->u.rsa.modulus, &modulus)) {
		SECKEY_DestroyPublicKey(pk);
		return NULL;
	}
	if(SECITEM_CopyItem(pk->arena, &pk->u.rsa.publicExponent, &exponent)) {
		SECKEY_DestroyPublicKey(pk);
		return NULL;
	}
	return pk;
}

/**
 * Setup key and digest for verification. Adjust sig if necessary.
 *
 * @param algo: key algorithm
 * @param evp_key: EVP PKEY public key to create.
 * @param digest_type: digest type to use
 * @param key: key to setup for.
 * @param keylen: length of key.
 * @return false on failure.
 */
static int
nss_setup_key_digest(int algo, SECKEYPublicKey** pubkey, HASH_HashType* htype,
	unsigned char* key, size_t keylen)
{
	/* TODO uses libNSS */

	switch(algo) {
		case LDNS_DSA:
		case LDNS_DSA_NSEC3:
			/* TODO */
			/*
			*evp_key = EVP_PKEY_new();
			if(!*evp_key) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			dsa = ldns_key_buf2dsa_raw(key, keylen);
			if(!dsa) {
				verbose(VERB_QUERY, "verify: "
					"ldns_key_buf2dsa_raw failed");
				return 0;
			}
			if(EVP_PKEY_assign_DSA(*evp_key, dsa) == 0) {
				verbose(VERB_QUERY, "verify: "
					"EVP_PKEY_assign_DSA failed");
				return 0;
			}
			*digest_type = EVP_dss1();
			*/
			*htype = HASH_AlgSHA1;

			break;
		case LDNS_RSASHA1:
		case LDNS_RSASHA1_NSEC3:
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
		case LDNS_RSASHA256:
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
		case LDNS_RSASHA512:
#endif
			*pubkey = nss_buf2rsa(key, keylen);
			if(!*pubkey) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			/* select SHA version */
#if defined(HAVE_EVP_SHA256) && defined(USE_SHA2)
			if(algo == LDNS_RSASHA256)
				*htype = HASH_AlgSHA256;
			else
#endif
#if defined(HAVE_EVP_SHA512) && defined(USE_SHA2)
				if(algo == LDNS_RSASHA512)
				*htype = HASH_AlgSHA512;
			else
#endif
				*htype = HASH_AlgSHA1;

			break;
		case LDNS_RSAMD5:
			*pubkey = nss_buf2rsa(key, keylen);
			if(!*pubkey) {
				log_err("verify: malloc failure in crypto");
				return 0;
			}
			*htype = HASH_AlgMD5;

			break;
#ifdef USE_ECDSA
		case LDNS_ECDSAP256SHA256:
			/* TODO
			*evp_key = ldns_ecdsa2pkey_raw(key, keylen,
				LDNS_ECDSAP256SHA256);
			if(!*evp_key) {
				verbose(VERB_QUERY, "verify: "
					"ldns_ecdsa2pkey_raw failed");
				return 0;
			}
			*/
			*htype = HASH_AlgSHA256;
			break;
		case LDNS_ECDSAP384SHA384:
			/* TODO
			*evp_key = ldns_ecdsa2pkey_raw(key, keylen,
				LDNS_ECDSAP384SHA384);
			if(!*evp_key) {
				verbose(VERB_QUERY, "verify: "
					"ldns_ecdsa2pkey_raw failed");
				return 0;
			}
			*/
			*htype = HASH_AlgSHA384;
			break;
#endif /* USE_ECDSA */
		case LDNS_ECC_GOST:
		default:
			verbose(VERB_QUERY, "verify: unknown algorithm %d", 
				algo);
			return 0;
	}
	return 1;
}

/**
 * Check a canonical sig+rrset and signature against a dnskey
 * @param buf: buffer with data to verify, the first rrsig part and the
 *	canonicalized rrset.
 * @param algo: DNSKEY algorithm.
 * @param sigblock: signature rdata field from RRSIG
 * @param sigblock_len: length of sigblock data.
 * @param key: public key data from DNSKEY RR.
 * @param keylen: length of keydata.
 * @param reason: bogus reason in more detail.
 * @return secure if verification succeeded, bogus on crypto failure,
 *	unchecked on format errors and alloc failures.
 */
enum sec_status
verify_canonrrset(ldns_buffer* buf, int algo, unsigned char* sigblock, 
	unsigned int sigblock_len, unsigned char* key, unsigned int keylen,
	char** reason)
{
	/* TODO uses libNSS */
	/* large enough for the different hashes */
	unsigned char hash[HASH_LENGTH_MAX];
	HASH_HashType htype = 0;
	SECKEYPublicKey* pubkey = NULL;
	SECItem secsig = {siBuffer, sigblock, sigblock_len};
	SECItem sechash = {siBuffer, hash, 0};
	SECStatus res;
	int err;

	// extern SECKEYPublicKey *SECKEY_DecodeDERPublicKey(SECItem *pubkder);
	// SECKEYPublicKey* SECKEY_ImportDERPublicKey(SECItem *derKey, CK_KEY_TYPE type);
	if(!nss_setup_key_digest(algo, &pubkey, &htype, key, keylen)) {
		verbose(VERB_QUERY, "verify: failed to setup key");
		*reason = "use of key for crypto failed";
		SECKEY_DestroyPublicKey(pubkey);
		return sec_status_bogus;
	}

	/* TODO: need to convert DSA, ECDSA signatures? */

	/* do the signature cryptography work */
	/* hash the data */
	sechash.len = HASH_ResultLen(htype);
	if(sechash.len > sizeof(hash)) {
		verbose(VERB_QUERY, "verify: hash too large for buffer");
		SECKEY_DestroyPublicKey(pubkey);
		return sec_status_unchecked;
	}
	if(HASH_HashBuf(htype, hash, (unsigned char*)ldns_buffer_begin(buf),
		(unsigned int)ldns_buffer_limit(buf)) != SECSuccess) {
		verbose(VERB_QUERY, "verify: HASH_HashBuf failed");
		SECKEY_DestroyPublicKey(pubkey);
		return sec_status_unchecked;
	}
	/* verify the signature */
	res = PK11_Verify(pubkey, &secsig, &sechash, NULL /*wincx*/);
	SECKEY_DestroyPublicKey(pubkey);

	if(res == SECSuccess) {
		return sec_status_secure;
	}
	err = PORT_GetError();
	if(err != SEC_ERROR_BAD_SIGNATURE) {
		/* failed to verify */
		verbose(VERB_QUERY, "verify: PK11_Verify failed: %s",
			PORT_ErrorToString(err));
		return sec_status_unchecked;
	}
	verbose(VERB_QUERY, "verify: signature mismatch: %s",
		PORT_ErrorToString(err));
	*reason = "signature crypto failed";
	return sec_status_bogus;
}


#endif /* HAVE_SSL or HAVE_NSS */
