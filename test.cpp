#include <cstdio>
#include <chrono>
#include <random>
#include "huffman_file_api.h"

class uchar_generator {
    std::array<size_t, 256> stats;

    std::mt19937 machine;

 public:
    uchar_generator() : machine(static_cast<unsigned long>(clock())) {
        for (size_t i = 0; i < 256; i++) {
            stats[i] = machine() % machine();
            // std::printf("%zu ", stats[i]);
            if (i) {
                stats[i] += stats[i - 1];
            }
        }
        //   std::printf("\n");
    }

    unsigned char operator()() {
        auto tmp = static_cast<uint32_t>(static_cast<uint32_t>(machine()) % stats[255]);
        for (size_t i = 0; i < 256; i++) {
            if (tmp < stats[i]) {
                return static_cast<unsigned char>(i);
            }
        }
        return 255;
    }
};

void create_file(char const *file_name, size_t amount_of_bytes) {
    FILE *file = std::fopen(file_name, "wb");
    static uchar_generator generator;
    for (size_t i = 0; i < amount_of_bytes; i++) {
        auto c = generator();
        std::fwrite(&c, 1, 1, file);
    }
    std::fclose(file);
}

void remove_file(char const *file_name) {
    std::remove(file_name);
}

bool cmp_file(char const *lhs, char const *rhs) {
    FILE *lhs_file = std::fopen(lhs, "rb");
    FILE *rhs_file = std::fopen(rhs, "rb");

    if (!lhs_file || !rhs_file) {
        return false;
    }
    while (true) {
        auto c1 = static_cast<unsigned char>(std::getc(lhs_file));
        auto c2 = static_cast<unsigned char>(std::getc(rhs_file));
        if (std::feof(lhs_file) && std::feof(rhs_file)) {
            std::fclose(lhs_file);
            std::fclose(rhs_file);
            return true;
        }
        if (static_cast<bool>(std::feof(lhs_file)) != static_cast<bool>(std::feof(rhs_file))) {
            std::fclose(lhs_file);
            std::fclose(rhs_file);
            return false;
        }
        if (c1 != c2) {
            std::fclose(lhs_file);
            std::fclose(rhs_file);
            return false;
        }
    }
    return false;
}

size_t sum_file(char const *file_name) {
    FILE *file = std::fopen(file_name, "rb");
    if (!file) {
        return 0;
    }
    std::fseek(file, 0, SEEK_END);
    return static_cast<size_t>(std::ftell(file));
}

int main() {
    char const *source = "1";
    char const *dest = "2";
    char const *buf = "coded";
    for (size_t i = 0; i < 32; i++) {
        std::printf("%2zu|",i+1);
        if(i == 10) {
            create_file(buf, 1);
            FILE *src2 = std::fopen(buf, "rb");
            FILE *dst2 = std::fopen(dest, "wb");
            try {
                auto decode_start = std::chrono::high_resolution_clock::now();
                decode(src2, dst2);
                auto decode_end = std::chrono::high_resolution_clock::now();
                std::fclose(src2);
                std::fclose(dst2);
                if (!cmp_file(source, dest)) {
                    std::printf("Error handled, source file != dest file");
                } else {
                    size_t new_size = sum_file(buf);
                    std::printf(
                            "%10zu - size in bytes (source) | %10zu - size in bytes (compressed)| %10.3f - factor | %10.3f - decode time\n",
                            UINT64_C(1) << 0, new_size,
                            static_cast<float>(UINT64_C(1) << 0) / new_size,
                            std::chrono::duration<float>(decode_end - decode_start).count());
                }
            } catch (std::runtime_error &a) {
                std::printf("Erroe handled - %s\n", a.what());
                std::fclose(src2);
                std::fclose(dst2);
            }
            remove_file(buf);
            remove_file(dest);
            continue;
        }
        create_file(source, UINT64_C(1) << i);

        FILE *src1 = std::fopen(source, "rb");
        FILE *dst1 = std::fopen(buf, "wb");
        auto encode_start = std::chrono::high_resolution_clock::now();
        encode(src1, dst1);
        auto encode_end = std::chrono::high_resolution_clock::now();
        std::fclose(src1);
        std::fclose(dst1);

        FILE *src2 = std::fopen(buf, "rb");
        FILE *dst2 = std::fopen(dest, "wb");
        try {
            auto decode_start = std::chrono::high_resolution_clock::now();
            decode(src2, dst2);
            auto decode_end = std::chrono::high_resolution_clock::now();
            std::fclose(src2);
            std::fclose(dst2);
            if (!cmp_file(source, dest)) {
                std::printf("Error handled, source file != dest file");
            } else {
                size_t new_size = sum_file(buf);
                std::printf(
                        "%10zu - size in bytes (source) | %10zu - size in bytes (compressed)| %10.3f - factor | %10.3f - encode time| %10.3f - decode time\n",
                        UINT64_C(1) << i, new_size,
                        static_cast<float>(UINT64_C(1) << i) / new_size,
                        std::chrono::duration<float>(encode_end - encode_start).count(),
                        std::chrono::duration<float>(decode_end - decode_start).count());
            }
        } catch (std::runtime_error &a) {
            std::printf("Erroe handled - %s\n", a.what());
            std::fclose(src2);
            std::fclose(dst2);
        }
        remove_file(source);
        remove_file(dest);
        remove_file(buf);
    }
}