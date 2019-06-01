#include <vector>
#include <cstdio>
#include <chrono>
#include <cstring>
#include "huffman_file_api.h"

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 3) {
        std::fprintf(stdout, "Huffman_zip  [options] source destination\n--mode=encode|decode - mode of zipper [encode]\n");
        std::fprintf(stderr, "Amount of args = %d\n", argc);
        return 0;
    }
    if (argc == 3) {
        FILE *input = std::fopen(argv[1], "rb");
        if (!input) {
            std::fprintf(stdout, "Wrong name of file\n");
            std::fprintf(stderr, "File with name %s doesn't exists\n", argv[1]);
            return 0;
        }
        FILE *output = std::fopen(argv[2], "wb");
        encode(input, output,true);
        std::fclose(input);
        std::fclose(output);
    } else if (strcmp(argv[1], "--mode=encode") == 0) {
        FILE *input = std::fopen(argv[2], "rb");
        if (!input) {
            std::fprintf(stdout, "Wrong name of file\n");
            std::fprintf(stderr, "File with name %s doesn't exists\n", argv[2]);
            return 0;
        }
        FILE *output = std::fopen(argv[3], "wb");
        encode(input, output,true);
        std::fclose(input);
        std::fclose(output);
    } else if (strcmp(argv[1], "--mode=decode") == 0) {
        FILE *input = std::fopen(argv[2], "rb");
        if (!input) {
            std::fprintf(stdout, "Wrong name of file\n");
            std::fprintf(stderr, "File with name %s doesn't exists\n", argv[2]);
            return 0;
        }
        FILE *output = std::fopen(argv[3], "wb");
        decode(input, output, true);
        std::fclose(input);
        std::fclose(output);
    } else {
        std::fprintf(stdout, "Wrong option\n");
        std::fprintf(stderr, "Option %s doesn't exists\n", argv[1]);
        return 0;
    }
    return 0;
}
