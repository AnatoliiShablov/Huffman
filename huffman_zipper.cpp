#include "huffman_zipper.h"

void huffman_zipper::add_byte(unsigned char byte, size_t max) {
    for (size_t i = 8; i-- != max;) {
        buffer_bool.emplace(byte & (1 << i));
    }
}

void huffman_zipper::inc_word(unsigned char byte) {
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
    } else {
        tree_position = tree_decode;
    }
}

huffman_zipper::huffman_zipper() : tree_decode(nullptr), tree_position(nullptr), buffer_char(0), buffer_position(7), last(false), amount_of_bytes(0) {
    for (size_t i = 0; i < ALPHA_SIZE; i++) {
        stats[i].first = 0;
        stats[i].second = static_cast<unsigned char>(i);
    }
    rebuild_tree();
}

void huffman_zipper::rebuild_tree() {
    delete tree_decode;
    for (auto &it:tree_encode) {
        it.clear();
    }

    std::vector<std::pair<size_t, std::vector<unsigned char>>> new_stats;
    std::vector<huffman_tree *> new_trees;
    size_t i = 0, j = 0;
    std::pair<size_t, std::vector<unsigned char>> next;
    huffman_tree *next_tree;
    while (i != ALPHA_SIZE || j != ALPHA_SIZE - 2) {
        next_tree = new huffman_tree();
        if (i < ALPHA_SIZE && (j >= new_stats.size() || new_stats[j].first >= stats[i].first)) {
            tree_encode[stats[i].second].push_back(false);
            next.first = stats[i].first;
            next.second.assign(1, stats[i].second);
            next_tree->zero = new huffman_tree(stats[i].second);
            i++;
        } else {
            for (auto it : new_stats[j].second) {
                tree_encode[it].push_back(false);
            }
            next.first = new_stats[j].first;
            next.second = new_stats[j].second;
            next_tree->zero = new_trees[j];
            j++;
        }
        if (i < ALPHA_SIZE && (j >= new_stats.size() || new_stats[j].first >= stats[i].first)) {
            tree_encode[stats[i].second].push_back(true);
            next.first += stats[i].first;
            next.second.push_back(stats[i].second);
            next_tree->one = new huffman_tree(stats[i].second);
            i++;
        } else {
            for (auto it : new_stats[j].second) {
                tree_encode[it].push_back(true);
            }
            next.first += new_stats[j].first;
            next.second.insert(next.second.end(), new_stats[j].second.begin(), new_stats[j].second.end());
            next_tree->one = new_trees[j];
            j++;
        }
        new_stats.emplace_back(next);
        new_trees.push_back(next_tree);
    }
    for (auto &it : tree_encode) {
        for (i = 0; i < it.size() / 2; i++) {
            std::swap(it[i], it[it.size() - i - 1]);
        }
    }
    tree_position = tree_decode = new_trees.back();
}

void huffman_zipper::encode(unsigned char const *words, size_t length, std::vector<unsigned char> &encoded) {
    for (size_t i = 0; i < length; i++) {
        for (bool &&j : tree_encode[words[i]]) {
            buffer_char |= j << buffer_position;
            if (buffer_position-- == 0) {
                encoded.push_back(buffer_char);
                buffer_char = 0;
                buffer_position = 7;
            }
        }
        last = tree_encode[words[i]].back();
        inc_word(words[i]);
    }
}

unsigned char huffman_zipper::final() {
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

size_t huffman_zipper::decode(unsigned char const *words, size_t length, std::vector<unsigned char> &decoded, int eos) {
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
        if (buffer_bool.empty()) {
            if (last_uncoded + 1 == length) {
                add_byte(words[last_uncoded++], max_index);
            } else {
                add_byte(words[last_uncoded++]);
            }
        }
        bool now = buffer_bool.front();
        buffer_bool.pop();
        if (now) {
            tree_position = tree_position->one;
        } else {
            tree_position = tree_position->zero;
        }
        if (!tree_position->one) {
            decoded.emplace_back(tree_position->c);
            inc_word(decoded.back());
        }
    }
    if (eos) {
        while (!buffer_bool.empty()) {
            bool now = buffer_bool.front();
            buffer_bool.pop();
            if (now) {
                tree_position = tree_position->one;
            } else {
                tree_position = tree_position->zero;
            }
            if (!tree_position->one) {
                decoded.emplace_back(tree_position->c);
                inc_word(decoded.back());
            }
        }
        return flag ? length + 1 : last_uncoded;
    }
    return last_uncoded;
}

bool huffman_zipper::decoded() {
    return tree_position == tree_decode;
}

huffman_zipper::~huffman_zipper() {
    delete tree_decode;
}