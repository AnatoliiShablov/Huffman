#ifndef SH_ZIP_LIBRARY_H
#define SH_ZIP_LIBRARY_H

#include <vector>
#include <array>
#include <functional>
#include <algorithm>

class haffman_tree {
    static size_t const ALPHA_SIZE = 256;

    std::array<std::vector<bool>, ALPHA_SIZE> tree_data;

 public:
    void rebuild_tree(std::array<std::pair<size_t, unsigned char>, ALPHA_SIZE> const &stats) {
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
            for (size_t i = 0; i < it.size() / 2; i++) {
                std::swap(it[i], it[it.size() - i - 1]);
            }
        }
    }

    std::vector<bool> const &code(unsigned char word) {
        return tree_data[word];
    }

    unsigned char
    decode(std::function<bool(std::pair<int, int> &, unsigned char &, unsigned char &, unsigned char &, size_t &, FILE *)> const &get_next,
           std::pair<int, int> &state, unsigned char &now, unsigned char &next, unsigned char &next_after_next, size_t &index, FILE *input) {
        std::vector<bool> tmp;
        for (unsigned char i = 0;; i++) {
            size_t checked = 0;
            for (size_t j = 0; j < tree_data[i].size(); j++) {
                if (tmp.size() == j) {
                    if (state.first == 3) {
                        for (size_t trur = 0; trur < 256; trur++) {
                            for (size_t trtrt = 0; trtrt < tree_data[trur].size(); trtrt++)
                                std::printf(tree_data[trur][trtrt] ? "1" : "0");
                            std::printf("---- %zu\n",trur);
                        }
                        throw std::runtime_error("No such word :(");
                    }
                    tmp.push_back(get_next(state, now, next, next_after_next, index, input));
                }
                if (tmp[j] == tree_data[i][j]) {
                    checked++;
                } else {
                    break;
                }
            }
            if (checked == tree_data[i].size()) {
                return i;
            }
            if (i == 255) {
                throw std::runtime_error("No such word :(");
            }
        }
    }
};

#endif