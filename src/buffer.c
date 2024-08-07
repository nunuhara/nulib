/* Copyright (C) 2019 Nunuhara Cabbage <nunuhara@haniwa.technology>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>
#include "nulib.h"
#include "nulib/buffer.h"
#include "nulib/little_endian.h"
#include "nulib/string.h"

void buffer_init(struct buffer *r, uint8_t *buf, size_t size)
{
	r->buf = buf;
	r->size = size;
	r->index = 0;
}

string buffer_read_string(struct buffer *r)
{
	int len = strlen(buffer_strdata(r));
	string s = string_new_len(buffer_strdata, len);
	buffer_skip(r, len + 1);
	return s;
}

char *buffer_skip_string(struct buffer *r)
{
	char *s = buffer_strdata(r);
	buffer_skip(r, strlen(s) + 1);
	return s;
}

string buffer_read_pascal_string(struct buffer *r)
{
	uint32_t len = buffer_read_u32(r);
	if (len > r->size - (r->index + 4))
		ERROR("Invalid string length: %u", len);
	string s = string_new_len(r->buf + r->index, len);
	r->index += len;
	return s;
}

void buffer_read_bytes(struct buffer *r, uint8_t *dst, size_t n)
{
	if (buffer_remaining(r) < n)
		ERROR("Out of bounds buffer read");
	memcpy(dst, r->buf+r->index, n);
	r->index += n;
}

void buffer_skip(struct buffer *r, size_t off)
{
	r->index = min(r->index + off, r->size);
}

bool buffer_check_bytes(struct buffer *r, const char *data, size_t n)
{
	bool eq = buffer_remaining(r) >= n && !strncmp(buffer_strdata(r), data, n);
	buffer_skip(r, n);
	return eq;
}

void _buffer_reserve(struct buffer *b, size_t size)
{
	size_t new_size = b->index + size;
	if (unlikely(!b->size))
		b->size = 64;
	while (b->size <= new_size)
		b->size *= 2;
	b->buf = xrealloc(b->buf, b->size);
}

void buffer_write_bytes(struct buffer *b, const uint8_t *bytes, size_t len)
{
	buffer_reserve(b, len);
	memcpy(b->buf + b->index, bytes, len);
	b->index += len;
}

void buffer_write_string(struct buffer *b, string s)
{
	buffer_write_bytes(b, (uint8_t*)s, string_length(s) + 1);
}

void buffer_write_cstring(struct buffer *b, const char *s)
{
	buffer_write_bytes(b, (uint8_t*)s, strlen(s));
}

void buffer_write_cstringz(struct buffer *b, const char *s)
{
	buffer_write_bytes(b, (uint8_t*)s, strlen(s)+1);
}

void buffer_write_pascal_string(struct buffer *b, string s)
{
	buffer_write_u32(b, string_length(s));
	buffer_write_bytes(b, (uint8_t*)s, string_length(s));
}

void buffer_write_pascal_cstring(struct buffer *b, const char *s)
{
	size_t len = strlen(s);
	buffer_write_u32(b, len);
	buffer_write_bytes(b, (uint8_t*)s, len);
}
