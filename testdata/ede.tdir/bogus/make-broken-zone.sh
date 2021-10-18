#!/usr/bin/env bash

CSK=`ldns-keygen -a ECDSAP256SHA256 -k -r /dev/urandom dnssec-failures.test`
echo $CSK

echo ". IN DS 20326 8 2 e06d44b80b8f1d39a95c0b0d7c65d08458e880409bbc683457104237c7f8ec8d" | \
        cat $CSK.ds - > bogus/trust-anchors

ONEMONTHAGO=`date -d 'now - 1 month' +%Y%m%d`
YESTERDAY=`date -d 'now - 2 days' +%Y%m%d`
TOMORROW=`date -d 'now + 2 days' +%Y%m%d`
ldns-signzone -i $YESTERDAY bogus/dnssec-failures.test $CSK -f - | \
        grep -v '^missingrrsigs\.dnssec-failures\.test\..*IN.*RRSIG.*TXT' | \
        sed 's/Signatures invalid/Signatures INVALID/g' | \
        grep -v '^notyetincepted\.dnssec-failures\.test\..*IN.*TXT' | \
        grep -v '^notyetincepted\.dnssec-failures\.test\..*IN.*RRSIG.*TXT' | \
        grep -v '^expired\.dnssec-failures\.test\..*IN.*TXT' | \
        grep -v '^expired\.dnssec-failures\.test\..*IN.*RRSIG.*TXT' > base
ldns-signzone -i $ONEMONTHAGO -e $YESTERDAY bogus/dnssec-failures.test $CSK -f - | \
        grep -v '[      ]NSEC[  ]' | \
        grep '^expired\.dnssec-failures\.test\..*IN.*TXT' > expired
ldns-signzone -i $TOMORROW bogus/dnssec-failures.test $CSK -f - | \
        grep -v '[      ]NSEC[  ]' | \
        grep '^notyetincepted\.dnssec-failures\.test\..*IN.*TXT' > notyetincepted
cat base expired notyetincepted > bogus/dnssec-failures.test.signed && rm -f base expired notyetincepted $CSK.*
