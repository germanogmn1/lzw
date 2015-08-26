#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	int prefix;
	uint8_t byte;
} dict_entry_t;

typedef struct {
	int size;
	dict_entry_t *data;
} dict_t;

void init_dict(dict_t *d) {
	d->data = malloc(0x10000);
	for (int b = 0; b < 256; b++) {
		dict_entry_t *entry = d->data + b;
		entry->prefix = -1;
		entry->byte = b;
	}
	d->size = 256;
}

void dict_put(dict_t *d, int prefix, uint8_t byte) {
	dict_entry_t *e = d->data + d->size++;
	e->prefix = prefix;
	e->byte = byte;
}

int dict_find(dict_t *d, int prefix, uint8_t byte) {
	for (int i = 0; i < d->size; i++) {
		dict_entry_t entry = d->data[i];
		if (entry.prefix == prefix && entry.byte == byte) {
			return i;
		}
	}
	return -1;
}

uint8_t dict_first_byte(dict_t* d, int index) {
	dict_entry_t e = d->data[index];
	while (e.prefix >= 0)
		e = d->data[e.prefix];
	return e.byte;
}

int dict_copy_val(dict_t *d, int index, char *dest) {
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

int lzw_encode(char *in, size_t in_size, int *out) {
	char *src = in; // save for debug

	dict_t dict;
	init_dict(&dict);

	int outc = 0;
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
			out[outc++] = index;
			index = b;
		}
	}
	if (index >= 0)
		out[outc++] = index;

	#if 1
	fprintf(stderr, "in: \"%s\"(%lu)\n", src, strlen(src));
	char buf[1024];
	memset(buf, 'z', 1024);
	for (int i = 256; i < dict.size; i++) {
		dict_entry_t e = dict.data[i];
		int end = dict_copy_val(&dict, i, buf);
		buf[end] = 0;
		fprintf(stderr, "- @%d\t%d%c\t%s\n", i - 255, e.prefix, e.byte, buf);
	}

	fprintf(stderr, "out: ");
	for (int i = 0; i < outc; i++)
		fprintf(stderr, "%d ", out[i]);
	fprintf(stderr, "\n");
	#endif

	return outc;
}

size_t lzw_decode(int *data, int datas, char *out) {
	char *res = out;
	size_t rc = 0;

	dict_t dict = {};
	init_dict(&dict);
	int index = data[0];
	res[rc++] = index;
	int old = index;
	for (int i = 1; i < datas; i++) {
		int index = data[i];
		if (index >= 0 && index < dict.size) {
			rc += dict_copy_val(&dict, index, res + rc);
			uint8_t b = dict_first_byte(&dict, index);
			dict_put(&dict, old, b);
		} else {
			uint8_t b = dict_first_byte(&dict, old);
			dict_put(&dict, old, b);
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

// end //

#include <assert.h>

int enc_buf[4096];
char res_buf[4096];
void assert_lzw(char *plain) {
	memset(enc_buf, '$', sizeof(enc_buf));
	memset(res_buf, '#', sizeof(res_buf));
	size_t esz = lzw_encode(plain, strlen(plain), enc_buf);
	lzw_decode(enc_buf, esz, res_buf);

	char *input = plain;
	char *output = &res_buf[0];
	while (*input) {
		if (*input++ != *output++) {
			res_buf[4095] = 0;
			return;
		}
	}
	fprintf(stderr, "PASSED TEST: \"%s\" == \"%s\"\n", plain, res_buf);
}

int main(int argc, char **argv) {
	assert_lzw("ababcbababaaaaaaa");
	assert_lzw("A circular buffer, or ring buffer, is a FIFO container consisting of a fixed-size buffer and head & tail indices. The head index is incremented as items are added and the tail index when items are removed.");

	void *ibuff = malloc(10000000);
	void *obuff = malloc(10000000);
	int *ebuff = malloc(10000000);
	size_t rsz = read(STDIN_FILENO, ibuff, 10000000);
	size_t esz = lzw_encode(ibuff, rsz, ebuff);
	lzw_decode(ebuff, esz, obuff);
	write(STDOUT_FILENO, obuff, rsz);
	fprintf(stderr, "input size: %lu; compressed size: %lu; rate: %f\n", rsz, esz, (float)esz/(float)rsz);
}
