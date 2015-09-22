#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "bitstream.c"

#define MEGABYTE 1000000

typedef struct {
	int prefix;
	uint8_t byte;
} dict_entry_t;

typedef struct {
	int size;
	dict_entry_t *data;
} dict_t;

static void init_dict(dict_t *d) {
	d->data = malloc(100 * MEGABYTE);
	for (int b = 0; b < 256; b++) {
		dict_entry_t *entry = d->data + b;
		entry->prefix = -1;
		entry->byte = b;
	}
	d->size = 256;
}

static void dict_put(dict_t *d, int prefix, uint8_t byte) {
	dict_entry_t *e = d->data + d->size++;
	e->prefix = prefix;
	e->byte = byte;
}

static int dict_find(dict_t *d, int prefix, uint8_t byte) {
	for (int i = 0; i < d->size; i++) {
		dict_entry_t entry = d->data[i];
		if (entry.prefix == prefix && entry.byte == byte) {
			return i;
		}
	}
	return -1;
}

static uint8_t dict_first_byte(dict_t* d, int index) {
	dict_entry_t e = d->data[index];
	while (e.prefix >= 0)
		e = d->data[e.prefix];
	return e.byte;
}

static int dict_copy_val(dict_t *d, int index, char *dest) {
	int count = 0;
	int i = index;
	while (i >= 0) {
		dict_entry_t e = d->data[i];
		dest[count++] = e.byte;
		i = e.prefix;
	}

	for (
		int start = 0, end = count - 1;
		start < end;
		start++, end--
	) {
		char t = dest[start];
		dest[start] = dest[end];
		dest[end] = t;
	}
	return count;
}

size_t lzw_encode(char *in, size_t in_size, void *out) {
	char *src = in; // save for debug

	bitstream_t stream = {};
	stream.data = out;
	int index_bits = 9;

	dict_t dict;
	init_dict(&dict);

	int index = -1;
	for (size_t i = 0; i < in_size; i++) {
		uint8_t b = in[i];
		int index_b = dict_find(&dict, index, b);
		// fprintf(stderr, "FIND\t'%c'(%d)\t'%c'(%d) = '%c'(%d)\n", index,index, b,b, index_b,index_b);
		if (index_b >= 0) {
			index = index_b;
		} else {
			dict_put(&dict, index, b);
			// fprintf(stderr, "PUT\t'%c'(%d)\t'%c'(%d)\n", index,index, b,b);
			if (dict.size >= ((1 << index_bits) - 1)) {
				index_bits++;
				fprintf(stderr, "WRITE index_bits=%d\n", index_bits);
			}
			write_bits(&stream, index_bits, index);
			index = b;
		}
	}
	if (index >= 0) {
		if (index > ((1 << index_bits) - 1)) {
			index_bits++;
			fprintf(stderr, "WRITE index_bits=%d\n", index_bits);
		}
		write_bits(&stream, index_bits, index);
	}

#if 0
	fprintf(stderr, "in: \"%s\"(%lu)\n", src, strlen(src));
	char buf[1024];
	memset(buf, 'z', 1024);
	for (int i = 256; i < dict.size; i++) {
		dict_entry_t e = dict.data[i];
		int end = dict_copy_val(&dict, i, buf);
		buf[end] = 0;
		fprintf(stderr, "- @%d\t%d%c\t%s\n", i - 255, e.prefix, e.byte, buf);
	}

#if 0
	fprintf(stderr, "out: ");
	for (int i = 0; i < outc; i++)
		fprintf(stderr, "%d ", out[i]);
	fprintf(stderr, "\n");
#endif
#endif

	return stream.index + 1;
}

size_t lzw_decode(void *data, int datas, char *out) {
	char *res = out;
	size_t rc = 0;

	dict_t dict = {};
	init_dict(&dict);

	bitstream_t stream = {};
	stream.data = data;
	int index_bits = 9;

	int index = read_bits(&stream, index_bits);
	res[rc++] = index;
	int old = index;
	while (stream.index < datas) {
		index = read_bits(&stream, index_bits);
		if (index >= 0 && index < dict.size) {
			rc += dict_copy_val(&dict, index, res + rc);
			uint8_t b = dict_first_byte(&dict, index);
			dict_put(&dict, old, b);
			if (dict.size >= ((1 << index_bits) - 3)) {
				index_bits++;
				fprintf(stderr, "READ index_bits=%d\n", index_bits);
			}
		} else {
			uint8_t b = dict_first_byte(&dict, old);
			dict_put(&dict, old, b);
			if (dict.size >= ((1 << index_bits) - 3)) {
				index_bits++;
				fprintf(stderr, "READ index_bits=%d\n", index_bits);
			}
			rc += dict_copy_val(&dict, dict.size - 1, res + rc);
		}
		old = index;
	}
	res[rc] = '\0';

#if 0
	fprintf(stderr, "decoded: \"%s\"(%lu)\n", res, strlen(res));
	char buf[1024];
	memset(buf, 'z', 1024);
	for (int i = 256; i < dict.size; i++) {
		dict_entry_t e = dict.data[i];
		int end = dict_copy_val(&dict, i, buf);
		buf[end] = 0;
		fprintf(stderr, "- @%d\t%d%c\t%s\n", i - 255, e.prefix, e.byte, buf);
	}
#endif

	return rc;
}
