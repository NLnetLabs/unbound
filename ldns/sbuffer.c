/*
 * buffer.c -- generic memory buffer .
 *
 * Copyright (c) 2001-2008, NLnet Labs. All rights reserved.
 *
 * See LICENSE for the license.
 *
 */
/**
 * \file
 *
 * This file contains the definition of ldns_buffer, and functions to manipulate those.
 */
#include "config.h"
#include "ldns/sbuffer.h"
#include <stdarg.h>

ldns_buffer *
ldns_buffer_new(size_t capacity)
{
	ldns_buffer *buffer = (ldns_buffer*)malloc(sizeof(ldns_buffer));

	if (!buffer) {
		return NULL;
	}
	
	buffer->_data = (uint8_t *) malloc(capacity);
	if (!buffer->_data) {
		free(buffer);
		return NULL;
	}
	
	buffer->_position = 0;
	buffer->_limit = buffer->_capacity = capacity;
	buffer->_fixed = 0;
	buffer->_status_err = 0;
	
	ldns_buffer_invariant(buffer);
	
	return buffer;
}

void
ldns_buffer_new_frm_data(ldns_buffer *buffer, void *data, size_t size)
{
	assert(data != NULL);

	buffer->_position = 0; 
	buffer->_limit = buffer->_capacity = size;
	buffer->_fixed = 0;
	buffer->_data = malloc(size);
	if(!buffer->_data) {
		buffer->_status_err = 1;
		return;
	}
	memcpy(buffer->_data, data, size);
	buffer->_status_err = 0;
	
	ldns_buffer_invariant(buffer);
}

void
ldns_buffer_init_frm_data(ldns_buffer *buffer, void *data, size_t size)
{
	memset(buffer, 0, sizeof(*buffer));
	buffer->_data = data;
	buffer->_capacity = buffer->_limit = size;
	buffer->_fixed = 1;
}

int
ldns_buffer_set_capacity(ldns_buffer *buffer, size_t capacity)
{
	void *data;
	
	ldns_buffer_invariant(buffer);
	assert(buffer->_position <= capacity);

	data = (uint8_t *) realloc(buffer->_data, capacity);
	if (!data) {
		buffer->_status_err = 1;
		return 0;
	} else {
		buffer->_data = data;
		buffer->_limit = buffer->_capacity = capacity;
		return 1;
	}
}

int
ldns_buffer_reserve(ldns_buffer *buffer, size_t amount)
{
	ldns_buffer_invariant(buffer);
	assert(!buffer->_fixed);
	if (buffer->_capacity < buffer->_position + amount) {
		size_t new_capacity = buffer->_capacity * 3 / 2;

		if (new_capacity < buffer->_position + amount) {
			new_capacity = buffer->_position + amount;
		}
		if (!ldns_buffer_set_capacity(buffer, new_capacity)) {
			buffer->_status_err = 1;
			return 0;
		}
	}
	buffer->_limit = buffer->_capacity;
	return 1;
}

int
ldns_buffer_printf(ldns_buffer *buffer, const char *format, ...)
{
	va_list args;
	int written = 0;
	size_t remaining;
	
	if (ldns_buffer_status_ok(buffer)) {
		ldns_buffer_invariant(buffer);
		assert(buffer->_limit == buffer->_capacity);

		remaining = ldns_buffer_remaining(buffer);
		va_start(args, format);
		written = vsnprintf((char *) ldns_buffer_current(buffer), remaining,
				    format, args);
		va_end(args);
		if (written == -1) {
			buffer->_status_err = 1;
			return -1;
		} else if ((size_t) written >= remaining) {
			if (!ldns_buffer_reserve(buffer, (size_t) written + 1)) {
				buffer->_status_err = 1;
				return -1;
			}
			va_start(args, format);
			written = vsnprintf((char *) ldns_buffer_current(buffer),
			    ldns_buffer_remaining(buffer), format, args);
			va_end(args);
			if (written == -1) {
				buffer->_status_err = 1;
				return -1;
			}
		}
		buffer->_position += written;
	}
	return written;
}

void
ldns_buffer_free(ldns_buffer *buffer)
{
	if (!buffer) {
		return;
	}

	if (!buffer->_fixed)
		free(buffer->_data);

	free(buffer);
}

void *
ldns_buffer_export(ldns_buffer *buffer)
{
	buffer->_fixed = 1;
	return buffer->_data;
}

int
ldns_bgetc(ldns_buffer *buffer)
{
	if (!ldns_buffer_available_at(buffer, buffer->_position, sizeof(uint8_t))) {
		ldns_buffer_set_position(buffer, ldns_buffer_limit(buffer));
		/* ldns_buffer_rewind(buffer);*/
		return EOF;
	}
	return (int)ldns_buffer_read_u8(buffer);
}

void 
ldns_buffer_copy(ldns_buffer* result, ldns_buffer* from)
{
	size_t tocopy = ldns_buffer_limit(from);

	if(tocopy > ldns_buffer_capacity(result))
		tocopy = ldns_buffer_capacity(result);
	ldns_buffer_clear(result);
	ldns_buffer_write(result, ldns_buffer_begin(from), tocopy);
	ldns_buffer_flip(result);
}
