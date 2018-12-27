#pragma once

#include <vector>
#include <unordered_set>
#include <set>

#include "types.hpp"

using namespace std;

template <typename T>
void print_set(set<T> &s) {
    for (auto &item : s) {
        cout << item << ' ';
    }
    cout << endl;
}

void add_to_limited_set(set<i> &s, const i &item, long limit) {
    if (s.size() < limit) {
        s.emplace(item);
    } else {
        if (*s.begin() < item) {
            s.emplace(item);
            s.erase(s.begin());
        }
    }
}

void merge_sets(
        vector<IndexSet<i> *> &sets,
        vector<IndexSet<i> *> &neg_sets,
        vector<vector<IndexSet<i> *> *> &any_sets,
        long limit,
        set<i> &result
) {
    if (!sets.empty()) {
        size_t min_size = sets[0]->size();
        size_t k_min = 0;
        for (size_t k = 1; k < sets.size(); k++) {
            if (min_size > sets[k]->size() and !sets[k]->no_sset) {
                min_size = sets[k]->size();
                k_min = k;
            }
        }
        for (auto iter = sets[k_min]->srbegin(); iter != sets[k_min]->srend(); iter++) {
            bool good = true;
            for (size_t k = 1; k < sets.size(); k++) {
                if (k == k_min) {
                    continue;
                }
                if (!sets[k]->has(*iter)) {
                    good = false;
                    break;
                }
            }
            if (good) {
                for (const auto &neg_set : neg_sets) {
                    if (neg_set->has(*iter)) {
                        good = false;
                        break;
                    }
                }
            }
            if (good) {
                for (const auto &any_set_vec: any_sets) {
                    bool flag = false;
                    for (const auto &any_set : *any_set_vec) {
                        if (any_set->has(*iter)) {
                            flag = true;
                            break;
                        }
                    }
                    if (!flag) {
                        good = false;
                        break;
                    }
                }
            }
            if (good) {
                result.emplace(*iter);
//                print_set(result);
                if (result.size() == limit) {
                    return;
                }
//                add_to_limited_set(result, *iter, limit);
            }
        }
    }
}