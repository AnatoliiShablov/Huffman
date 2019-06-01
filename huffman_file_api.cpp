#include <cstring>
#include <vector>
#include "huffman_zipper.h"

bool empty(FILE *file) {
    int c = getc(file);
    if (c != EOF) {
        ungetc(c, file);
        return false;
    }
    return true;
}

void encode(FILE *source, FILE *destination, bool print) {
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
        encoding_system.encode(bytes, length, coded);
        if (print) {
            std::printf("%zu - blocks were loaded\n", i++);
        };
        if (coded.size() > 1024 * 512) {
            std::fwrite(coded.data(), 1, coded.size(), destination);
            coded.clear();
        }
    }
    std::free(bytes);
    std::fwrite(coded.data(), 1, coded.size(), destination);
    std::fputc(encoding_system.final(), destination);
}

void decode(FILE *source, FILE *destination, bool print) {
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
        if (print) {
            std::printf("%zu - blocks were loaded\n", i++);
        };
        if (decoded.size() > 1024 * 512) {
            std::fwrite(decoded.data(), 1, decoded.size(), destination);
            decoded.clear();
        }
    }
    std::free(bytes);
    std::fwrite(decoded.data(), 1, decoded.size(), destination);
    if (!decoding_system.decoded()) {
        throw std::runtime_error("Not enough bits :(");
    }
}
