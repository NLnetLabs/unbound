#!/bin/sh

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

usage() {
	printf -- "usage: %s [options] name type class ...\n" "$1" >&2
	printf -- "\tsends the name-type-class queries over DNS-over-CoAP.\n" >&2
	printf -- "-s server\tIP address to send the queries to, default [::1]\n" >&2
	printf -- "-p\t\tPort to connect to, default: 5684 if -C is set, 5683 otherwise\n" >&2
	printf -- "-r\t\tCoAP resource, default: /\n" >&2
	printf -- "-c\t\tContent-format in request, default: 553\n" >&2
	printf -- "-k\t\tCredentials file for OSCORE or CoAPS\n" >&2
	printf -- "-P\t\tUse PSK with CoAPS instead of PKI. Ignored when using OSCORE\n" >&2
	printf -- "-a\t\tCertificate file for CoAPS PKI\n" >&2
	printf -- "-C\t\tUse CoAPS\n" >&2
	printf -- "-h\t\tThis help text\n" >&2
	exit 1
}

COMMAND="$0"

if [ $# -eq 0 ]; then
	usage "${COMMAND}"
fi

SCHEMA="coap"
SERVER="[::1]"
RESOURCE="/"
CONTENT_FORMAT=553
PORT=""
KEYFILE=""
CERTFILE=""
USE_PSK=0

while getopts "a:c:Chk:s:p:Pr:" OPTION; do
	case "${OPTION}" in
		a)
			CERTFILE="${OPTARG}"
			;;
		c)
			CONTENT_FORMAT="${OPTARG}"
			;;
		C)
			SCHEMA="coaps"
			;;
		k)
			KEYFILE="${OPTARG}"
			;;
		p)
			PORT="${OPTARG}"
			;;
		P)
			USE_PSK=1
			;;
		r)
			RESOURCE="/"
			;;
		s)
			SERVER="${OPTARG}"
			;;
		h|?)
			usage $COMMAND
			;;
		*)
			usage $COMMAND
			;;
	esac
done

shift "$(( OPTIND - 1 ))"

if [ $(( $# % 3 )) -ne 0 ]; then
	echo "Invalid input. Specify qname, qtype, and qclass." >&2
	usage $COMMAND
fi

if [ -z "${PORT}" ]; then
	if [ "${SCHEMA}" = "coap" ]; then
		PORT=5683
	elif [ "${SCHEMA}" = "coaps" ]; then
		PORT=5684
	fi
fi

if [ -n "${KEYFILE}" ]; then
	if [ "${SCHEMA}" = "coap" ]; then
		KEYFILE_FLAGS=-E
		KEYFILE="${KEYFILE},client.seq"
	elif [ "${SCHEMA}" = "coaps" ] && [ "${USE_PSK}" -eq 1 ]; then
		KEYFILE_FLAGS="-u default_user -k theverysecretdefaultkey -h"
	elif [ "${SCHEMA}" = "coaps" ]; then
		if [ -n "${CERTFILE}" ]; then
			KEYFILE_FLAGS="-c ${CERTFILE}"
		fi
		KEYFILE_FLAGS="${KEYFILE_FLAGS} -j"
	fi
fi

COAP_CLIENT_COMMAND=coap-client

if command -v coap-client-openssl 2>&1 > /dev/null; then
	COAP_CLIENT_COMMAND=coap-client-openssl
fi
if ! command -v "${COAP_CLIENT_COMMAND}" 2>&1 > /dev/null; then
	echo "coap-client command missing. In Ubuntu it is part of package libcoap3-bin" >&2
	exit 1
fi

while [ $# -gt 0 ]; do
	${SCRIPT_DIR}/../shell-parse $1 $2 $3 | \
		"${COAP_CLIENT_COMMAND}" -m fetch -t "${CONTENT_FORMAT}" \
			${KEYFILE_FLAGS} "${KEYFILE}" \
			"${SCHEMA}://${SERVER}:${PORT}${RESOURCE}" -f - > client_outfile
	if grep -qI . client_outfile; then	# client outfile is ASCII
		cat client_outfile
	else								# client outfile is binary
		cat client_outfile | "${SCRIPT_DIR}"/../shell-parse
	fi
	shift 3
done
