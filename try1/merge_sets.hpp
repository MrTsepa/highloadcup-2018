#pragma once

#include <vector>
#include <unordered_set>
#include <set>

#include "types.hpp"

using namespace std;
void intersection(
        vector<IndexSet<i> *> &sets,
        unordered_set<i> &result
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
            for (size_t k = 0; k < sets.size(); k++) {
                if (k == k_min) {
                    continue;
                }
                if (!sets[k]->has(*iter)) {
                    good = false;
                    break;
                }
            }
            if (good) {
                result.emplace(*iter);
            }
        }
    }
}

size_t intersection_size(
        unordered_set<i> &set1,
        unordered_set<i> &set2
) {
    auto& s1 = set1.size() < set2.size() ? set1 : set2;
    auto& s2 = set1.size() < set2.size() ? set2 : set1;
    size_t res = 0;
    for (auto &item : s1) {
        if (s2.find(item) != s2.end()) {
            ++res;
        }
    }
    return res;
}

size_t intersection_size3(
        unordered_set<i> &set1,
        unordered_set<i> &set2,
        unordered_set<i> &set3
) {
    auto& s01 = set1.size() < set2.size() ? set1 : set2;
    auto& s1 = set3.size() < s01.size() ? set3 : set1;
    auto& s2 = set3.size() < s01.size() ? set1 : set2;
    auto& s3 = set3.size() < s01.size() ? set2 : set3;
    size_t res = 0;
    for (auto &item : s1) {
        if (s2.find(item) != s2.end() && s3.find(item) != s3.end()) {
            ++res;
        }
    }
    return res;
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
                if (result.size() == limit) {
                    return;
                }
            }
        }
    }
}