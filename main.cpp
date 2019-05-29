#include <vector>
#include <array>
#include <cstdio>

#include "haffman_tree.h"

std::vector<unsigned char> to_bytes(std::vector<bool> &bool_data, bool full = false) {
    std::vector<unsigned char> result;
    size_t num;
    for (num = 0; num + (full ? 7 : 8) < bool_data.size(); num += 8) {
        unsigned char c = 0;
        for (int i = 0; i < 8; ++i) {
            c <<= 1;
            c |= bool_data[num + i];
        }
        result.push_back(c);
    }
    bool_data.erase(bool_data.begin(), (bool_data.begin() + num));
    return result;
}

void code(FILE *input, FILE *output) {
    haffman_tree tree;
    std::array<std::pair<size_t, unsigned char>, 256> stats;
    for (size_t i = 0; i < 256; i++) {
        stats[i] = std::make_pair(0, i);
    }
    auto *bytes = static_cast<unsigned char *>(std::malloc(8192));
    std::vector<bool> new_bools; // not bytes, but...
    bool flag = true;
    while (!std::feof(input)) {
        size_t amount = std::fread(bytes, 1, 8192, input);
        for (size_t i = 0; i < amount; i++) {
            if (flag) { tree.rebuild_tree(stats); }
            std::vector<bool> symbol_code = tree.code(bytes[i]);
            for (size_t ch = 0; ch < 256; ch++) {
                if (stats[ch].second == bytes[i]) {
                    stats[ch].first++;
                    flag = false;
                    while (ch != 255 && stats[ch + 1].first < stats[ch].first) {
                        std::swap(stats[ch + 1], stats[ch]);
                        ch++;
                        flag = true;
                    }
                    break;
                }
            }
            new_bools.insert(new_bools.end(), symbol_code.begin(), symbol_code.end());
            if (new_bools.size() > 10000) {
                std::vector<unsigned char> tmp = to_bytes(new_bools);
                std::fwrite(tmp.data(), 1, tmp.size(), output);
            }
        }
    }
    std::free(bytes);
    if (new_bools.empty()) {
        return;
    } else {
        new_bools.push_back(new_bools.back() ^ true);
        while (new_bools.size() % 8) {
            new_bools.push_back(new_bools.back());
        }
        std::vector<unsigned char> tmp = to_bytes(new_bools, true);
        std::fwrite(tmp.data(), 1, tmp.size(), output);
    }
}

bool get_next(std::pair<int, int> &state, unsigned char &now, unsigned char &next, unsigned char &next_after_next, size_t &index, FILE *input) {
    if ((state.first == 0 || state.first == 1) && index < 8) {
        return static_cast<bool>(now & 1 << (7 - (index++)));
    }
    if (state.first == 0 && index == 8) {
        now = next;
        next = next_after_next;
        next_after_next = static_cast<unsigned char>(getc(input));
        if (std::feof(input)) {
            if (next == 255 || next == 0) {
                state.first = 2;
                state.second = 8;
            } else {
                state.first = 1;
                int max_index = 7;
                while (max_index && static_cast<bool>(next & (1 << (7 - max_index))) == static_cast<bool>(next & (1 << (8 - max_index)))) {
                    max_index--;
                }
                state.second = max_index;
            }
        }
        index = 0;
        return static_cast<bool>(now & 1 << (7 - (index++)));
    }
    if (state.first == 1 && index == 8) {
        now = next;
        state.first = 2;
        if (state.second == 1) {
            state.first = 3;
        }
        index = 0;
        return static_cast<bool>(now & 1 << (7 - (index++)));
    }

    if (state.first == 2 && index + 1 == state.second) {
        state.first = 3;
        return static_cast<bool>(now & 1 << (7 - (index++)));
    }
    if (state.first == 2 && index + 1 < state.second) {
        return static_cast<bool>(now & 1 << (7 - (index++)));
    }
}

void decode(FILE *input, FILE *output) {
    haffman_tree tree;
    std::array<std::pair<size_t, unsigned char>, 256> stats;
    for (size_t i = 0; i < 256; i++) {
        stats[i] = std::make_pair(0, i);
    }
    bool end_reached = false;
    std::pair<int, int> state(0, 0);
    auto now = static_cast<unsigned char>(std::getc(input));
    if (std::feof(input)) {
        return;
    }
    auto next = static_cast<unsigned char>(std::getc(input));
    if (std::feof(input)) {
        throw std::runtime_error("No such word :(");
    }
    auto next_after_next = static_cast<unsigned char>(std::getc(input));
    if (std::feof(input)) {
        if (next == 255 || next == 0) {
            state.first = 2;
            state.second = 8;
        } else {
            state.first = 1;
            int max_index = 7;
            while (max_index && ((next & (1 << max_index)) == (now & (1 << (max_index - 1))))) {
                max_index--;
            }
        }
    }
    size_t index(0);
    bool flag = true;
    while (state.first != 3) {
        if (flag) { tree.rebuild_tree(stats); }
        unsigned char symbol = tree.decode(&get_next, state, now, next, next_after_next, index, input);
        for (size_t ch = 0; ch < 256; ch++) {
            if (stats[ch].second == symbol) {
                stats[ch].first++;
                flag = false;
                while (ch != 255 && stats[ch + 1].first < stats[ch].first) {
                    std::swap(stats[ch + 1], stats[ch]);
                    ch++;
                    flag = true;
                }
                break;
            }
        }
        std::putc(symbol, output);
    }
}

int main() {
    FILE *input_file = std::fopen("1", "rb");
    FILE *output_file = std::fopen("Var2", "wb");

    decode(input_file, output_file);

    std::fclose(input_file);
    std::fclose(output_file);
}