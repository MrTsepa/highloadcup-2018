#include <iostream>
#include <vector>
#include <unordered_set>

#include "trie.hpp"

using namespace std;

template <typename T>
void print_set(unordered_set<T> &s) {
    for (auto &item : s) {
        cout << item << ' ';
    }
    cout << endl;
}

int main() {
    vector<string> strings {"a", "aa", "ab", "aac", "b"};
    vector<int> is {1, 2, 3, 4, 5};
    TrieNode<int> root;
    for (int i = 0; i < 5; i++) {
        trie_insert(root, strings[i], 0, is[i]);
    }
    print_set(*trie_prefix_set_ptr(root, ""s, 0)); // 5 4 3 2 1
    print_set(*trie_prefix_set_ptr(root, "a"s, 0)); // 4 3 2 1
    print_set(*trie_prefix_set_ptr(root, "aa"s, 0)); // 4 2
    print_set(*trie_prefix_set_ptr(root, "b"s, 0)); // 5
    print_set(*trie_prefix_set_ptr(root, "c"s, 0)); //
}