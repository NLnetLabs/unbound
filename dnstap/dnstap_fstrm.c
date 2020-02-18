/*
 * dnstap/dnstap_fstrm.c - Frame Streams protocol for dnstap
 *
 * Copyright (c) 2020, NLnet Labs. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 *
 * Definitions for the Frame Streams data transport protocol for
 * dnstap message logs.
 */

#include "config.h"
#include "dnstap/dnstap_fstrm.h"
#include "sldns/sbuffer.h"

void* fstrm_create_control_frame_start(char* contenttype, size_t* len)
{
	uint32_t* control;
	size_t n;
	/* start framestream message:
	 * 4byte 0: control indicator.
	 * 4byte bigendian: length of control frame
	 * 4byte bigendian: type START
	 * 4byte bigendian: option: content-type
	 * 4byte bigendian: length of string
	 * string of content type (dnstap)
	 */
	n = 4+4+4+4+4+strlen(contenttype);
	control = malloc(n);
	if(!control)
		return NULL;
	control[0] = 0;
	control[1] = htonl(4+4+4+strlen(contenttype));
	control[2] = htonl(FSTRM_CONTROL_FRAME_START);
	control[3] = htonl(FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE);
	control[4] = htonl(strlen(contenttype));
	memmove(&control[5], contenttype, strlen(contenttype));
	*len = n;
	return control;
}

void* fstrm_create_control_frame_stop(size_t* len)
{
	uint32_t* control;
	size_t n;
	/* stop framestream message:
	 * 4byte 0: control indicator.
	 * 4byte bigendian: length of control frame
	 * 4byte bigendian: type STOP
	 */
	n = 4+4+4;
	control = malloc(n);
	if(!control)
		return NULL;
	control[0] = 0;
	control[1] = htonl(4);
	control[2] = htonl(FSTRM_CONTROL_FRAME_STOP);
	*len = n;
	return control;
}

void* fstrm_create_control_frame_accept(char* contenttype, size_t* len)
{
	uint32_t* control;
	size_t n;
	/* control frame on reply:
	 * 4 bytes 0 escape
	 * 4 bytes bigendian length of frame
	 * 4 bytes bigendian type ACCEPT
	 * 4 bytes bigendian frame option content type
	 * 4 bytes bigendian length of string
	 * string of content type.
	 */
	/* len includes the escape and framelength */
	n = 4+4+4+4+4+strlen(contenttype);
	control = malloc(n);
	if(!control) {
		return NULL;
	}
	control[0] = 0;
	control[1] = htonl(4+4+4+strlen(contenttype));
	control[2] = htonl(FSTRM_CONTROL_FRAME_ACCEPT);
	control[3] = htonl(FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE);
	control[4] = htonl(strlen(contenttype));
	memmove(&control[5], contenttype, strlen(contenttype));
	*len = n;
	return control;
}

void* fstrm_create_control_frame_finish(size_t* len)
{
	uint32_t* control;
	size_t n;
	/* control frame on reply:
	 * 4 bytes 0 escape
	 * 4 bytes bigendian length of frame
	 * 4 bytes bigendian type FINISH
	 */
	/* len includes the escape and framelength */
	n = 4+4+4;
	control = malloc(n);
	if(!control) {
		return NULL;
	}
	control[0] = 0;
	control[1] = htonl(4);
	control[2] = htonl(FSTRM_CONTROL_FRAME_FINISH);
	*len = n;
	return control;
}

char* fstrm_describe_control(void* pkt, size_t len)
{
	uint32_t frametype = 0;
	char buf[512];
	size_t at = 0, remain;
	uint8_t* pos;

	buf[0]=0;
	if(len < 4) {
		snprintf(buf, sizeof(buf), "malformed control frame, "
			"too short, len=%u", (unsigned int)len);
		return strdup(buf);
	}
	frametype = sldns_read_uint32(pkt);
	if(frametype == FSTRM_CONTROL_FRAME_ACCEPT) {
		at+=snprintf(buf+at, sizeof(buf)-at, "accept");
	} else if(frametype == FSTRM_CONTROL_FRAME_START) {
		at+=snprintf(buf+at, sizeof(buf)-at, "start");
	} else if(frametype == FSTRM_CONTROL_FRAME_STOP) {
		at+=snprintf(buf+at, sizeof(buf)-at, "stop");
	} else if(frametype == FSTRM_CONTROL_FRAME_READY) {
		at+=snprintf(buf+at, sizeof(buf)-at, "ready");
	} else if(frametype == FSTRM_CONTROL_FRAME_FINISH) {
		at+=snprintf(buf+at, sizeof(buf)-at, "finish");
	} else {
		at+=snprintf(buf+at, sizeof(buf)-at, "type%d",
			(int)frametype);
	}

	/* show the content type options */
	pos = pkt + 4;
	remain = len - 4;
	while(remain >= 8) {
		uint32_t field_type = sldns_read_uint32(pos);
		uint32_t field_len = sldns_read_uint32(pos+4);
		if(remain < field_len) {
			at+=snprintf(buf+at, sizeof(buf)-at, "malformed_field");
			break;
		}
		if(field_type == FSTRM_CONTROL_FIELD_TYPE_CONTENT_TYPE) {
			at+=snprintf(buf+at, sizeof(buf)-at, " content-type(");
			if(at+field_len < sizeof(buf)) {
				memmove(buf+at, pos+8, field_len);
				at += field_len;
			}
			at+=snprintf(buf+at, sizeof(buf)-at, ")");
		} else {
			at+=snprintf(buf+at, sizeof(buf)-at,
				" field(type %u, length %u)",
				(unsigned int)field_type,
				(unsigned int)field_len);
		}
		pos += 8 + field_len;
		remain -= (8 + field_len);
	}
	if(remain > 0)
		at+=snprintf(buf+at, sizeof(buf)-at, " trailing-bytes"
			"(length %u)", (unsigned int)remain);
	return strdup(buf);
}
