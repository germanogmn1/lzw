#ifndef LZW_H
#define LZW_H

size_t lzw_encode(void *input, size_t input_size, void **output);
size_t lzw_decode(void *input, size_t input_size, void **output);

#endif
