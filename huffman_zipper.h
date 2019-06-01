#ifndef HUFFMAN_ZIPPER
#define HUFFMAN_ZIPPER

#include <vector>
#include <array>
#include <queue>
#include <functional>
#include <algorithm>

class huffman_zipper {
    struct huffman_tree {
        huffman_tree *zero;

        huffman_tree *one;

        unsigned char c;

        explicit huffman_tree(unsigned char c = 0) : zero(nullptr), one(nullptr), c(c) {}

        ~huffman_tree() {
            delete zero, delete one;
        }
    };

    static size_t const ALPHA_SIZE = 256;

    std::array<std::vector<bool>, ALPHA_SIZE> tree_encode;

    huffman_tree *tree_decode;

    huffman_tree *tree_position;

    std::array<std::pair<size_t, unsigned char>, ALPHA_SIZE> stats;

    unsigned char buffer_char;

    size_t buffer_position;

    bool last;

    std::queue<bool> buffer_bool;

    size_t amount_of_bytes;

    void add_byte(unsigned char byte, size_t max = 0);

    void inc_word(unsigned char byte);

 public:
    huffman_zipper();

    ~huffman_zipper();

    void rebuild_tree();

    void encode(unsigned char const *words, size_t length, std::vector<unsigned char> &encoded);

    unsigned char final();

    size_t decode(unsigned char const *words, size_t length, std::vector<unsigned char> &decoded, int eos);

    bool decoded();
};

#endif