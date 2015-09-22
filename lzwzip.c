#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lzw.h"

void *read_file(char *path, int64_t *fsize) {
	FILE *file = fopen(path, "r");
	if (!file)
		return 0;
	if (fseek(file, 0, SEEK_END))
		return 0;
	int64_t size = ftell(file);
	if (size == -1)
		return 0;
	if (fseek(file, 0, SEEK_SET))
		return 0;

	void *buff = malloc(size);
	fread(buff, size, 1, file);

	*fsize = size;
	return buff;
}

void usage(char *bin) {
	printf("usage: %s -cd file\n", bin);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		usage(argv[0]);
		return 2;
	}

	int compress;
	if (strcmp(argv[1], "-c") == 0) {
		compress = 1;
	} else if (strcmp(argv[1], "-d") == 0) {
		compress = 0;
	} else {
		usage(argv[0]);
		return 2;
	}

	int64_t insize;
	void *input = read_file(argv[2], &insize);
	void *output = malloc(insize * 2);

	size_t outsize;
	if (compress) {
		outsize = lzw_encode(input, insize, output);
		fprintf(stderr, "input size: %lu; compressed size: %lu; rate: %f\n", (size_t)insize, outsize, (float)outsize/(float)insize);
	} else {
		outsize = lzw_decode(input, insize, output);
	}
	fwrite(output, outsize, 1, stdout);

	return 0;
}
