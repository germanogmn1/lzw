#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
	uint8_t *data;
	int index;
	int bit;
} bitstream_t;

void write_bits(bitstream_t *s, int bits, int num) {
	fprintf(stderr, "WRITE %d bits: %d\n", bits, num);
	while (bits > 0) {
		s->data[s->index] |= num << s->bit;
		num >>= 8 - s->bit;
		bits -= 8 - s->bit;

		if (bits < 0) {
			s->bit = 8 + bits;
		} else {
			s->bit = 0;
			s->index++;
		}
	}
}

int extract_bits(uint8_t byte, int low_bit, int high_bit) {
	int mask = (~((1 << low_bit) - 1)) & ((1 << (high_bit)) - 1);
	// printf("extracting %d:%d from 0x%X = %d (mask=%d)\n", low_bit, high_bit, byte, (byte & mask) >> low_bit, mask);
	return (byte & mask) >> low_bit;
}

int read_bits(bitstream_t *s, int bits_to_read) {
	int result = 0;
	int result_bit = 0;
 int bits = bits_to_read; // debug
	while (bits_to_read) {
		int high_bit = s->bit + bits_to_read;
		int high_bit_clamped = (high_bit < 8) ? high_bit : 8;
		int extracted = extract_bits(s->data[s->index], s->bit, high_bit_clamped);

		result |= extracted << result_bit;
		int written = high_bit_clamped - s->bit;
		result_bit += written;
		bits_to_read -= written;

		// printf("result_bit = %d, bits = %d\n", result_bit, bits);

		if (high_bit_clamped == 8) {
			s->index++;
			s->bit = 0;
		} else {
			s->bit = high_bit_clamped;
		}
	}

	fprintf(stderr, "READ %d bits: %d\n", bits, result);
	return result;
}
