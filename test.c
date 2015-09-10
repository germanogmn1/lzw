#include <assert.h>
#include "lzw.c"

char enc_buf[4096];
char res_buf[4096];
void assert_lzw(char *plain) {
	memset(enc_buf, 0, sizeof(enc_buf));
	memset(res_buf, 0, sizeof(res_buf));
	size_t esz = lzw_encode(plain, strlen(plain), enc_buf);
	lzw_decode(enc_buf, esz, res_buf);

	char *input = plain;
	char *output = res_buf;
	while (*input) {
		if (*input++ != *output++) {
			res_buf[4095] = 0;
			printf("\x1b[41mTEST FAILED: %s == %s\x1b[0m\n", plain, res_buf);
			return;
		}
	}
	printf("\x1b[42mTEST PASSED: \"%s\" == \"%s\"\x1b[0m\n", plain, res_buf);
}

bool failed = false;
void _assert_eq(int actual, int expected, char *actual_s, char *expected_s) {
	if (actual == expected) {
		printf("\x1b[42mTEST PASSED: %s == %s\x1b[0m\n", actual_s, expected_s);
	} else {
		failed = true;
		printf("\x1b[41mTEST FAILED: %s == %s (%d)\x1b[0m\n", actual_s, expected_s, actual);
	}
}

#define ASSERT_EQ(actual, expected) _assert_eq((actual), (expected), #actual, #expected)

int main(int argc, char **argv) {
#if 0
	bitstream_t bs = {};
	bs.data = malloc(1000000);
	memset(bs.data, 0, 1000000);

	{
		write_bits(&bs, 1, 0b1);
		ASSERT_EQ(bs.data[0], 0b00000001);
		ASSERT_EQ(bs.bit, 1);
		ASSERT_EQ(bs.index, 0);
		printf("\n");

		write_bits(&bs, 4, 0b1010);
		ASSERT_EQ(bs.data[0], 0b00010101);
		ASSERT_EQ(bs.bit, 5);
		ASSERT_EQ(bs.index, 0);
		printf("\n");

		write_bits(&bs, 3, 0b101);
		ASSERT_EQ(bs.data[0], 0b10110101);
		ASSERT_EQ(bs.bit, 0);
		ASSERT_EQ(bs.index, 1);
		printf("\n");

		write_bits(&bs, 5, 0b11011);
		ASSERT_EQ(bs.data[0], 0b10110101);
		ASSERT_EQ(bs.data[1], 0b00011011);
		ASSERT_EQ(bs.bit, 5);
		ASSERT_EQ(bs.index, 1);
		printf("\n");

		write_bits(&bs, 5, 0b10110);
		ASSERT_EQ(bs.data[0], 0b10110101);
		ASSERT_EQ(bs.data[1], 0b11011011);
		ASSERT_EQ(bs.data[2], 0b00000010);
		ASSERT_EQ(bs.bit, 2);
		ASSERT_EQ(bs.index, 2);
		printf("\n");

		write_bits(&bs, 20, 0xABCDE);
		ASSERT_EQ(bs.data[0], 0b10110101);
		ASSERT_EQ(bs.data[1], 0b11011011);
		ASSERT_EQ(bs.data[2], 0b01111010);
		ASSERT_EQ(bs.data[3], 0b11110011);
		ASSERT_EQ(bs.data[4], 0b00101010);
		ASSERT_EQ(bs.bit, 6);
		ASSERT_EQ(bs.index, 4);
		printf("\n");

		write_bits(&bs, 1, 0);
		ASSERT_EQ(bs.data[0], 0b10110101);
		ASSERT_EQ(bs.data[1], 0b11011011);
		ASSERT_EQ(bs.data[2], 0b01111010);
		ASSERT_EQ(bs.data[3], 0b11110011);
		ASSERT_EQ(bs.data[4], 0b00101010);
		ASSERT_EQ(bs.bit, 7);
		ASSERT_EQ(bs.index, 4);
		printf("\n");
	}

	{
		ASSERT_EQ(extract_bits(0b11011011, 0, 8), 0b11011011);
		ASSERT_EQ(extract_bits(0b00000001, 0, 1), 0b1);
		ASSERT_EQ(extract_bits(0b11111110, 0, 1), 0b0);
		ASSERT_EQ(extract_bits(0b01111111, 7, 8), 0b0);
		ASSERT_EQ(extract_bits(0b10000000, 7, 8), 0b1);
		ASSERT_EQ(extract_bits(0b11011011, 2, 6), 0b0110);
		printf("\n");
	}

	bs.index = 0;
	bs.bit = 0;

	{
		ASSERT_EQ(read_bits(&bs, 1), 0b1);
		ASSERT_EQ(read_bits(&bs, 4), 0b1010);
		ASSERT_EQ(read_bits(&bs, 3), 0b101);
		ASSERT_EQ(read_bits(&bs, 5), 0b11011);
		ASSERT_EQ(read_bits(&bs, 5), 0b10110);
		ASSERT_EQ(read_bits(&bs, 20), 0xABCDE);
		ASSERT_EQ(read_bits(&bs, 1), 0);
		printf("\n");
	}

	assert_lzw("ababcbababaaaaaaa");
	assert_lzw("A circular buffer, or ring buffer, is a FIFO container consisting of a fixed-size buffer and head & tail indices. The head index is incremented as items are added and the tail index when items are removed.");
#endif
#if 1
	void *ibuff = malloc(10000000);
	memset(ibuff, 0, 10000000);
	void *obuff = malloc(10000000);
	memset(obuff, 0, 10000000);
	int *ebuff = malloc(10000000);
	memset(ebuff, 0, 10000000);
	size_t rsz = read(STDIN_FILENO, ibuff, 10000000);
	size_t esz = lzw_encode(ibuff, rsz, ebuff);
	lzw_decode(ebuff, esz, obuff);
	write(STDOUT_FILENO, obuff, rsz);
	fprintf(stderr, "input size: %lu; compressed size: %lu; rate: %f\n", rsz, esz, (float)esz/(float)rsz);
#endif

	return failed;
}
