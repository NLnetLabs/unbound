#!/usr/bin/env python
from __future__ import print_function
from unbound import ub_ctx, RR_TYPE_A, RR_TYPE_RRSIG, RR_TYPE_NSEC, RR_TYPE_NSEC3
import ldns

def dnssecParse(domain, rrType=RR_TYPE_A):
    print("Resolving domain", domain)
    s, r = resolver.resolve(domain)
    print("status: %s, secure: %s, rcode: %s, havedata: %s, answer_len; %s" % (s, r.secure, r.rcode_str, r.havedata, r.answer_len))
    
    s, pkt = ldns.ldns_wire2pkt(r.packet)
    if s != 0:
        raise RuntimeError("Error parsing DNS packet")

    rrsigs = pkt.rr_list_by_type(RR_TYPE_RRSIG, ldns.LDNS_SECTION_ANSWER)
    print("RRSIGs from answer:", sorted(rrsigs))
    
    rrsigs = pkt.rr_list_by_type(RR_TYPE_RRSIG, ldns.LDNS_SECTION_AUTHORITY)
    print("RRSIGs from authority:", sorted(rrsigs))
    
    nsecs = pkt.rr_list_by_type(RR_TYPE_NSEC, ldns.LDNS_SECTION_AUTHORITY)
    print("NSECs:", sorted(nsecs))
    
    nsec3s = pkt.rr_list_by_type(RR_TYPE_NSEC3, ldns.LDNS_SECTION_AUTHORITY)
    print("NSEC3s:", sorted(nsec3s))
    
    print("---")


resolver = ub_ctx()
resolver.add_ta(".   IN DS   19036 8 2 49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5")
resolver.add_ta(".   IN DS   20326 8 2 E06D44B80B8F1D39A95C0B0D7C65D08458E880409BBC683457104237C7F8EC8D")
resolver.add_ta(".   IN DS   38696 8 2 683D2D0ACB8C9B712A1948B27F741219298D0A450D612C483AF444A4C0FB2B16")

dnssecParse("nic.cz")
dnssecParse("nonexistent-domain-blablabla.cz")
dnssecParse("nonexistent-domain-blablabla.root.cz")

