#ifndef HUFFMAN_FILE_API
#define HUFFMAN_FILE_API

#include <cstdio>
#include "huffman_zipper.h"

bool empty(FILE *file);

void encode(FILE *source, FILE *destination, bool print = false);

void decode(FILE *source, FILE *destination, bool print = false);

#endif