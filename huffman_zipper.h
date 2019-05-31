#ifndef SH_ZIP_LIBRARY_H
#define SH_ZIP_LIBRARY_H

#include <vector>
#include <array>
#include <functional>
#include <algorithm>

class huffman_zipper {
    static size_t const ALPHA_SIZE = 256;

    std::array<std::vector<bool>, ALPHA_SIZE> tree_data;

    std::array<std::pair<size_t, unsigned char>, ALPHA_SIZE> stats;

    unsigned char buffer_char;

    size_t buffer_position;

    bool last;

    std::vector<bool> buffer_bool;

    size_t amount_of_bytes;

    void add_byte(unsigned char byte, size_t max = 0) {
        for (size_t i = 8; i-- != max;) {
            buffer_bool.emplace_back(byte & (1 << i));
        }
    }

    void inc_word(unsigned char byte) {
        for (size_t ch = 0; ch < ALPHA_SIZE; ch++) {
            if (stats[ch].second == byte) {
                stats[ch].first++;
                while (ch != ALPHA_SIZE - 1 && stats[ch + 1].first < stats[ch].first) {
                    std::swap(stats[ch + 1], stats[ch]);
                    ch++;
                }
                break;
            }
        }
        amount_of_bytes++;
        if ((amount_of_bytes & amount_of_bytes - 1) == 0) {
            rebuild_tree();
        }
    }

 public:
    huffman_zipper() : buffer_char(0), buffer_position(7), last(false), amount_of_bytes(0) {
        for (size_t i = 0; i < ALPHA_SIZE; i++) {
            stats[i].first = 0;
            stats[i].second = static_cast<unsigned char>(i);
        }
        rebuild_tree();
    }

    void rebuild_tree() {
        for (auto &it:tree_data) {
            it.clear();
        }
        std::vector<std::pair<size_t, std::vector<unsigned char>>> new_stats;
        size_t i = 0, j = 0;
        bool first, second;
        std::pair<size_t, std::vector<unsigned char>> next;
        while (i != ALPHA_SIZE || j != ALPHA_SIZE - 2) {
            if (i < ALPHA_SIZE && (j >= new_stats.size() || new_stats[j].first >= stats[i].first)) {
                tree_data[stats[i].second].push_back(false);
                next.first = stats[i].first;
                next.second.assign(1, stats[i].second);
                i++;
            } else {
                for (auto it : new_stats[j].second) {
                    tree_data[it].push_back(false);
                }
                next.first = new_stats[j].first;
                next.second = new_stats[j++].second;
            }
            if (i < ALPHA_SIZE && (j >= new_stats.size() || new_stats[j].first >= stats[i].first)) {
                tree_data[stats[i].second].push_back(true);
                next.first += stats[i].first;
                next.second.push_back(stats[i].second);
                i++;
            } else {
                for (auto it : new_stats[j].second) {
                    tree_data[it].push_back(true);
                }
                next.first += new_stats[j].first;
                next.second.insert(next.second.end(), new_stats[j].second.begin(), new_stats[j].second.end());
                j++;
            }
            new_stats.emplace_back(next);
        }
        for (auto &it : tree_data) {
            for (i = 0; i < it.size() / 2; i++) {
                std::swap(it[i], it[it.size() - i - 1]);
            }
        }
    }

    void code(unsigned char const *words, size_t length, std::vector<unsigned char> &encoded) {
        for (size_t i = 0; i < length; i++) {
            for (bool &&j : tree_data[words[i]]) {
                buffer_char |= j << buffer_position;
                if (buffer_position-- == 0) {
                    encoded.push_back(buffer_char);
                    buffer_char = 0;
                    buffer_position = 7;
                }
            }
            last = tree_data[words[i]].back();
            inc_word(words[i]);
        }
    }

    unsigned char final() {
        last ^= true;
        if (buffer_position == 7) {
            return static_cast<unsigned char>(last ? 255 : 0);
        }
        if (!last) {
            return buffer_char;
        }
        while (true) {
            buffer_char |= last << buffer_position;
            if (buffer_position-- == 0) {
                break;
            }
        }
        return buffer_char;
    }

    size_t decode(unsigned char const *words, size_t length, std::vector<unsigned char> &decoded, int eos) {
        if (length == 0 || (!eos && length < 3)) {
            return 0;
        }
        size_t max_index(0);
        bool flag = false;
        if (eos) {
            if (words[length - 1] == 255 || words[length - 1] == 0) {
                length--;
                flag = true;
            } else {
                bool tmp = static_cast<bool>(words[length - 1] & 1);
                max_index = 0;
                while (static_cast<bool>(words[length - 1] & (1 << max_index)) == tmp) {
                    max_index++;
                }
            }
        } else {
            length -= 2;
        }

        size_t last_uncoded = 0;
        while (last_uncoded != length) {
            for (size_t i = 0; i < 256; i++) {
                while (buffer_bool.size() < tree_data[i].size()) {
                    if (last_uncoded == length) {
                        break;
                    } else if (last_uncoded + 1 == length) {
                        add_byte(words[last_uncoded++], max_index);
                    } else {
                        add_byte(words[last_uncoded++]);
                    }
                }
                if (buffer_bool.size() < tree_data[i].size()) {
                    continue;
                }
                bool correctness_flag = true;
                for (size_t j = 0; j < tree_data[i].size(); j++) {
                    if (tree_data[i][j] != buffer_bool[j]) {
                        correctness_flag = false;
                        break;
                    }
                }
                if (correctness_flag) {
                    decoded.emplace_back(i);
                    buffer_bool.erase(buffer_bool.begin(), buffer_bool.begin() + tree_data[i].size());
                    inc_word(decoded.back());
                    break;
                }
            }
        }
        if (eos) {
            bool used = true;
            while (used) {
                used = false;
                for (size_t i = 0; i < 256; i++) {
                    if (buffer_bool.size() < tree_data[i].size()) {
                        continue;
                    }
                    bool correctness_flag = true;
                    for (size_t j = 0; j < tree_data[i].size(); j++) {
                        if (tree_data[i][j] != buffer_bool[j]) {
                            correctness_flag = false;
                            break;
                        }
                    }
                    if (correctness_flag) {
                        used = true;
                        decoded.emplace_back(i);
                        buffer_bool.erase(buffer_bool.begin(), buffer_bool.begin() + tree_data[i].size());
                        inc_word(decoded.back());
                        break;
                    }
                }
            }
            return flag ? length + 1 : last_uncoded;
        }
        return last_uncoded;
    }

    bool decoded() {
        return buffer_bool.empty();
    }
};

#endif