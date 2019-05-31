#include <vector>
#include <array>
#include <cstdio>
#include <chrono>
#include <cstring>

#include "huffman_zipper.h"

bool empty(FILE *file) {
    int c = getc(file);
    if (c != EOF) {
        ungetc(c, file);
        return false;
    }
    return true;
}

void encode(FILE *source, FILE *destination) {
    if (empty(source)) {
        return;
    }
    huffman_zipper encoding_system;
    auto *bytes = static_cast<unsigned char *>(std::malloc(1024 * 1024));
    std::vector<unsigned char> coded;
    coded.reserve(1024 * 1024);
    size_t i = 1;
    while (!std::feof(source)) {
        size_t length = std::fread(bytes, 1, 1024 * 1024, source);
        encoding_system.code(bytes, length, coded);
        std::printf("%zu - blocks were loaded\n", i++);
        if (coded.size() > 1024 * 512) {
            std::fwrite(coded.data(), 1, coded.size(), destination);
            coded.clear();
        }
    }
    std::free(bytes);
    std::fwrite(coded.data(), 1, coded.size(), destination);
    std::fputc(encoding_system.final(), destination);
}

void decode(FILE *source, FILE *destination) {
    if (empty(source)) {
        return;
    }
    huffman_zipper decoding_system;
    auto *bytes = static_cast<unsigned char *>(std::malloc(1024 * 1024));
    size_t last_uncoded = 0;
    size_t length = 0;
    std::vector<unsigned char> decoded;
    decoded.reserve(1024 * 1024);
    size_t i = 1;
    while (!std::feof(source)) {
        std::memmove(bytes, bytes + last_uncoded, length - last_uncoded);
        length -= last_uncoded;
        length += std::fread(bytes + length, 1, 1024 * 1024 - length, source);
        last_uncoded = decoding_system.decode(bytes, length, decoded, std::feof(source));
        std::printf("%zu - blocks were loaded\n", i++);
        if (decoded.size() > 1024 * 512) {
            std::fwrite(decoded.data(), 1, decoded.size(), destination);
            decoded.clear();
        }
    }
    std::free(bytes);
    std::fwrite(decoded.data(), 1, decoded.size(), destination);
    if (!decoding_system.decoded()) {
        std::fclose(destination);
        throw std::runtime_error("Not enough bits :(");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 3) {
        std::fprintf(stdout, "Huffman_zip  [options] source destination\n--mode=encode|decode mode od zipper [encode]\n");
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
        encode(input, output);
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
        encode(input, output);
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
        decode(input, output);
        std::fclose(input);
        std::fclose(output);
    } else {
        std::fprintf(stdout, "Wrong option\n");
        std::fprintf(stderr, "Option %s doesn't exists\n", argv[1]);
        return 0;
    }
    return 0;
}
