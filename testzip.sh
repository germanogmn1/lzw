#!/usr/bin/env bash

clang --std=c99 lzw.c lzwzip.c -o lzwzip || exit 1

./lzwzip -c lzwzip > comp
./lzwzip -d comp > decomp
md5sum < lzwzip
md5sum < decomp
rm comp decomp
