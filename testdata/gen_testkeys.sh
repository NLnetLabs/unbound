#!/bin/sh
#
# gen_testkeys.sh - Generate all TLS/SSL test keys and certificates for
# Unbound integration tests (.tdir tests).
#
# This script generates key material on the fly so that .key and .pem files
# need not be stored in the source repository.
#
# Usage: ./gen_testkeys.sh [testdata_dir]
#   testdata_dir defaults to the directory containing this script.
#
# Requirements: openssl
#
# Key categories generated:
#   1. "standard" server/control keypair (3072-bit RSA, CN=unbound / CN=unbound-control)
#   2. "doh" server keypair (1024-bit RSA, CN=unbound) - for DoH downstream tests
#   3. "bad" server/control keypair (3072-bit RSA, separate CA) - for negative auth tests
#   4. "bad_small" server/control keypair (1024-bit RSA, separate CA) - ctrl_itr/ctrl_pipe
#   5. "petal" self-signed keypair (3072-bit RSA, CN=petal)
#   6. "nsd" self-signed keypair (3072-bit RSA, CN=nsd)
#   7. "test_cert" signing keypair (3072-bit RSA, CN=petal) - for 10-unbound-anchor
#   8. PKCS#7 signatures used by 10-unbound-anchor
#
# DNSSEC keys (.key/.private/.ds) are NOT generated here because they are
# cryptographic test vectors with pre-computed signatures embedded in .rpl
# files. Regenerating them would require re-signing all zone test data.
#
# DNSCrypt keys are NOT generated here because they require the
# dnscrypt-wrapper tool. See dnscrypt/testdata/gencert.sh for that.

set -eu

TESTDATA_DIR="${1:-$(cd "$(dirname "$0")" && pwd)}"
DAYS=7200
HASH=sha256

if ! command -v openssl >/dev/null 2>&1; then
    echo "openssl is required to generate the test certificates." >&2
    exit 1
fi

# Temporary working directory
WORK_DIR=$(mktemp -d)
trap 'rm -rf "$WORK_DIR"' EXIT INT TERM

######################################################################
# Helper: generate a self-signed CA key+cert pair
# Usage: gen_ca <outdir> <basename> <cn> <bits>
######################################################################
gen_ca() {
    _dir="$1"; _base="$2"; _cn="$3"; _bits="$4"

    openssl genrsa -out "$_dir/${_base}.key" "$_bits" 2>/dev/null

    cat > "$WORK_DIR/ca.cnf" <<EOF
[req]
default_bits=$_bits
default_md=$HASH
prompt=no
distinguished_name=dn
x509_extensions=v3_ca
[dn]
commonName=$_cn
[v3_ca]
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid:always,issuer:always
basicConstraints=critical,CA:TRUE,pathlen:0
subjectAltName=DNS:$_cn
EOF

    openssl req -new -x509 \
        -key "$_dir/${_base}.key" \
        -config "$WORK_DIR/ca.cnf" \
        -days "$DAYS" \
        -out "$_dir/${_base}.pem" 2>/dev/null
}

######################################################################
# Helper: generate a client key+cert signed by a CA
# Usage: gen_client <outdir> <basename> <cn> <bits> <ca_key> <ca_pem>
######################################################################
gen_client() {
    _dir="$1"; _base="$2"; _cn="$3"; _bits="$4"; _ca_key="$5"; _ca_pem="$6"

    openssl genrsa -out "$_dir/${_base}.key" "$_bits" 2>/dev/null

    cat > "$WORK_DIR/client.cnf" <<EOF
[req]
default_bits=$_bits
default_md=$HASH
prompt=no
distinguished_name=dn
req_extensions=v3_req
[dn]
commonName=$_cn
[v3_req]
basicConstraints=critical,CA:FALSE
subjectAltName=DNS:$_cn
EOF

    # Create trust anchor from CA cert
    openssl x509 -addtrust serverAuth \
        -in "$_ca_pem" \
        -out "$WORK_DIR/ca_trust.pem" 2>/dev/null

    openssl req -new \
        -config "$WORK_DIR/client.cnf" \
        -key "$_dir/${_base}.key" 2>/dev/null \
    | openssl x509 -req \
        -days "$DAYS" \
        -CA "$WORK_DIR/ca_trust.pem" \
        -CAkey "$_ca_key" \
        -CAcreateserial \
        -"$HASH" \
        -extfile "$WORK_DIR/client.cnf" \
        -extensions v3_req \
        -out "$_dir/${_base}.pem" 2>/dev/null
}

######################################################################
# Helper: generate a self-signed key+cert (non-CA)
# Usage: gen_selfsigned <outdir> <basename> <cn> <bits>
######################################################################
gen_selfsigned() {
    _dir="$1"; _base="$2"; _cn="$3"; _bits="$4"

    openssl genrsa -out "$_dir/${_base}.key" "$_bits" 2>/dev/null

    cat > "$WORK_DIR/self.cnf" <<EOF
[req]
default_bits=$_bits
default_md=$HASH
prompt=no
distinguished_name=dn
[dn]
commonName=$_cn
EOF

    openssl req -new -x509 \
        -key "$_dir/${_base}.key" \
        -config "$WORK_DIR/self.cnf" \
        -days "$DAYS" \
        -out "$_dir/${_base}.pem" 2>/dev/null
}

######################################################################
# Helper: generate the signing certificate expected by unbound-anchor
######################################################################
gen_anchor_signer() {
    _dir="$1"; _base="$2"; _bits="$3"

    openssl genrsa -out "$_dir/${_base}.key" "$_bits" 2>/dev/null

    cat > "$WORK_DIR/anchor.cnf" <<EOF
[req]
default_bits=$_bits
default_md=$HASH
prompt=no
distinguished_name=dn
x509_extensions=v3_ca
[dn]
commonName=petal
emailAddress=petal
[v3_ca]
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid:always,issuer:always
basicConstraints=critical,CA:TRUE,pathlen:0
subjectAltName=DNS:petal
keyUsage=digitalSignature,keyCertSign
EOF

    openssl req -new -x509 \
        -key "$_dir/${_base}.key" \
        -config "$WORK_DIR/anchor.cnf" \
        -days "$DAYS" \
        -out "$_dir/${_base}.pem" 2>/dev/null
}

######################################################################
# Generate the "standard" keypairs (3072-bit) into a staging area
######################################################################
echo "Generating standard server/control keypairs (3072-bit)..."
STAGE_STD="$WORK_DIR/standard"
mkdir -p "$STAGE_STD"
gen_ca "$STAGE_STD" "unbound_server" "unbound" 3072
gen_client "$STAGE_STD" "unbound_control" "unbound-control" 3072 \
    "$STAGE_STD/unbound_server.key" "$STAGE_STD/unbound_server.pem"

######################################################################
# Generate the "doh" keypairs (1024-bit) into a staging area
######################################################################
echo "Generating DoH server keypair (1024-bit)..."
STAGE_DOH="$WORK_DIR/doh"
mkdir -p "$STAGE_DOH"
gen_ca "$STAGE_DOH" "unbound_server" "unbound" 1024

######################################################################
# Generate the "bad" keypairs (3072-bit, separate CA for mismatch tests)
######################################################################
echo "Generating bad server/control keypairs (3072-bit)..."
STAGE_BAD="$WORK_DIR/bad"
mkdir -p "$STAGE_BAD"
gen_ca "$STAGE_BAD" "bad_server" "unbound" 3072
gen_client "$STAGE_BAD" "bad_control" "unbound-control" 3072 \
    "$STAGE_BAD/bad_server.key" "$STAGE_BAD/bad_server.pem"

######################################################################
# Generate the "bad_small" keypairs (1024-bit, for ctrl_itr/ctrl_pipe)
######################################################################
echo "Generating bad_small server/control keypairs (1024-bit)..."
STAGE_BAD_SM="$WORK_DIR/bad_small"
mkdir -p "$STAGE_BAD_SM"
gen_ca "$STAGE_BAD_SM" "bad_server" "unbound" 1024
gen_client "$STAGE_BAD_SM" "bad_control" "unbound-control" 1024 \
    "$STAGE_BAD_SM/bad_server.key" "$STAGE_BAD_SM/bad_server.pem"

######################################################################
# Generate "petal" self-signed keypair (3072-bit)
######################################################################
echo "Generating petal keypair (3072-bit)..."
STAGE_PETAL="$WORK_DIR/petal"
mkdir -p "$STAGE_PETAL"
gen_selfsigned "$STAGE_PETAL" "petal" "petal" 3072

######################################################################
# Generate "nsd" self-signed keypair (3072-bit)
######################################################################
echo "Generating nsd keypair (3072-bit)..."
STAGE_NSD="$WORK_DIR/nsd"
mkdir -p "$STAGE_NSD"
gen_selfsigned "$STAGE_NSD" "nsd_server" "nsd" 3072

######################################################################
# Generate "test_cert" self-signed keypair (3072-bit)
######################################################################
echo "Generating test_cert keypair (3072-bit)..."
STAGE_TEST="$WORK_DIR/test_cert"
mkdir -p "$STAGE_TEST"
gen_anchor_signer "$STAGE_TEST" "test_cert" 3072

######################################################################
# Distribute generated keys to all .tdir directories that need them
######################################################################
echo "Distributing keys to test directories..."

# Directories that use the standard 3072-bit server AND control keys
STD_BOTH_DIRS="
09-unbound-control
ctrl_itr
ctrl_pipe
dnstap
dnstap_reconnect
dnstap_tcp
dnstap_tls
dnstap_tls_badcert
dnstap_tls_badname
dnstap_tls_clientauth
dnstap_tls_peername
dynlibmod
fwd_bogus
http_user_agent
ip_ratelimit
padding
ratelimit
redis_reconnect_interval
redis_replica
stat_values
stream_ssl
tls_reuse
tls_reuse_auth
"

for dir in $STD_BOTH_DIRS; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_STD/unbound_server.key" "$tdir/unbound_server.key"
        cp "$STAGE_STD/unbound_server.pem" "$tdir/unbound_server.pem"
        cp "$STAGE_STD/unbound_control.key" "$tdir/unbound_control.key"
        cp "$STAGE_STD/unbound_control.pem" "$tdir/unbound_control.pem"
    fi
done

# Directories that use the standard 3072-bit server key only (no control key)
STD_SERVER_ONLY_DIRS="
auth_tls
auth_tls_failcert
doq_downstream
proxy_protocol
ssl_req_order
ssl_req_timeout
"

for dir in $STD_SERVER_ONLY_DIRS; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_STD/unbound_server.key" "$tdir/unbound_server.key"
        cp "$STAGE_STD/unbound_server.pem" "$tdir/unbound_server.pem"
    fi
done

# Directories that use the DoH 1024-bit server key (no control key)
DOH_DIRS="
doh_downstream
doh_downstream_buffer_size
doh_downstream_endpoint
doh_downstream_notls
doh_downstream_post
"

for dir in $DOH_DIRS; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_DOH/unbound_server.key" "$tdir/unbound_server.key"
        cp "$STAGE_DOH/unbound_server.pem" "$tdir/unbound_server.pem"
    fi
done

# 09-unbound-control gets the 3072-bit bad keys
tdir="$TESTDATA_DIR/09-unbound-control.tdir"
if [ -d "$tdir" ]; then
    cp "$STAGE_BAD/bad_server.key" "$tdir/bad_server.key"
    cp "$STAGE_BAD/bad_server.pem" "$tdir/bad_server.pem"
    cp "$STAGE_BAD/bad_control.key" "$tdir/bad_control.key"
    cp "$STAGE_BAD/bad_control.pem" "$tdir/bad_control.pem"
fi

# ctrl_itr and ctrl_pipe get the 1024-bit bad keys
for dir in ctrl_itr ctrl_pipe; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_BAD_SM/bad_server.key" "$tdir/bad_server.key"
        cp "$STAGE_BAD_SM/bad_server.pem" "$tdir/bad_server.pem"
        cp "$STAGE_BAD_SM/bad_control.key" "$tdir/bad_control.key"
        cp "$STAGE_BAD_SM/bad_control.pem" "$tdir/bad_control.pem"
    fi
done

# Petal keys
PETAL_DIRS="
http_user_agent
blanks_https
auth_https_origin
auth_https
"

for dir in $PETAL_DIRS; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_PETAL/petal.key" "$tdir/petal.key"
        cp "$STAGE_PETAL/petal.pem" "$tdir/petal.pem"
    fi
done

# 10-unbound-anchor gets petal + test_cert + server/control keys in keys/ subdir
tdir="$TESTDATA_DIR/10-unbound-anchor.tdir"
if [ -d "$tdir" ]; then
    cp "$STAGE_PETAL/petal.key" "$tdir/petal.key"
    cp "$STAGE_PETAL/petal.pem" "$tdir/petal.pem"
    cp "$STAGE_TEST/test_cert.key" "$tdir/test_cert.key"
    cp "$STAGE_TEST/test_cert.pem" "$tdir/test_cert.pem"
    mkdir -p "$tdir/keys"
    cp "$STAGE_STD/unbound_server.key" "$tdir/keys/unbound_server.key"
    cp "$STAGE_STD/unbound_server.pem" "$tdir/keys/unbound_server.pem"
    cp "$STAGE_STD/unbound_control.key" "$tdir/keys/unbound_control.key"
    cp "$STAGE_STD/unbound_control.pem" "$tdir/keys/unbound_control.pem"
    # Also copy test_cert into keys/ for the signit script
    cp "$STAGE_TEST/test_cert.key" "$tdir/keys/test_cert.key"
    cp "$STAGE_TEST/test_cert.pem" "$tdir/keys/test_cert.pem"

    openssl smime -sign -binary -outform DER \
        -in "$tdir/127.0.0.1/root.xml" \
        -inkey "$tdir/test_cert.key" \
        -signer "$tdir/test_cert.pem" \
        -out "$tdir/127.0.0.1/root.p7s" 2>/dev/null
    openssl smime -sign -binary -outform DER \
        -in "$tdir/127.0.0.1/no_more_keys.xml" \
        -inkey "$tdir/test_cert.key" \
        -signer "$tdir/test_cert.pem" \
        -out "$tdir/127.0.0.1/no_more_keys.p7s" 2>/dev/null
    # bad.p7s deliberately signs different content from bad.xml.
    openssl smime -sign -binary -outform DER \
        -in "$tdir/127.0.0.1/root.xml" \
        -inkey "$tdir/test_cert.key" \
        -signer "$tdir/test_cert.pem" \
        -out "$tdir/127.0.0.1/bad.p7s" 2>/dev/null
fi

# NSD keys for auth_tls tests
for dir in auth_tls auth_tls_failcert; do
    tdir="$TESTDATA_DIR/${dir}.tdir"
    if [ -d "$tdir" ]; then
        cp "$STAGE_NSD/nsd_server.key" "$tdir/nsd_server.key"
        cp "$STAGE_NSD/nsd_server.pem" "$tdir/nsd_server.pem"
    fi
done

echo "Done. All TLS test keys and certificates generated."
