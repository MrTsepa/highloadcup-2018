#pragma once

#include <vector>
#include <unordered_set>
#include <set>

#include "types.hpp"

using namespace std;

void add_to_limited_set(set<i> &s, const i &item, long limit) {
    for (auto &it : s) {
        cout << it << ' ';
    }
    cout << endl;
    if (s.size() < limit) {
        s.emplace(item);
    } else {
        if (*s.begin() < item) {
            s.emplace(item);
            s.erase(s.begin());
        }
    }
    for (auto &it : s) {
        cout << it << ' ';
    }
    cout << endl;
    cout << endl;
}

void merge_sets(
        vector<unordered_set<i> *> &sets,
        vector<unordered_set<i> *> &neg_sets,
        long limit,
        set<i> &result
) {
    if (!sets.empty()) {
        size_t min_size = sets[0]->size();
        size_t k_min = 0;
        for (size_t k = 1; k < sets.size(); k++) {
            if (min_size > sets[k]->size()) {
                min_size = sets[k]->size();
                k_min = k;
            }
        }
        for (const auto &item : *sets[k_min]) {
            bool good = true;
            for (size_t k = 0; k < sets.size(); k++) {
                if (k == k_min) {
                    continue;
                }
                if (sets[k]->find(item) == sets[k]->end()) {
                    good = false;
                    break;
                }
            }
            if (good) {
                for (const auto &neg_set : neg_sets) {
                    if (neg_set->find(item) != neg_set->end()) {
                        good = false;
                        break;
                    }
                }
            }
            if (good) {
//                result.emplace(item);
                add_to_limited_set(result, item, limit);
            }
        }
    }
}