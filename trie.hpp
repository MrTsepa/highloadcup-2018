#pragma once

#include <unordered_map>

using namespace std;

template <typename T>
unordered_set<T> empty_set;

template <typename T>
struct TrieNode {
    unordered_map<char, TrieNode<T>* > children;
    unordered_set<T> prefix_set;
};

template <typename T>
void trie_insert(TrieNode<T> &node, string &s, size_t pos, T &val) {
    if (pos > s.size()) {
        return;
    }
    node.prefix_set.emplace(val);
    if (node.children[s[pos]] == nullptr) {
        node.children[s[pos]] = new TrieNode<T>;
    }
    trie_insert(*node.children[s[pos]], s, pos + 1, val);
}

template <typename T>
unordered_set<T>* trie_prefix_set_ptr(TrieNode<T> &node, string s, size_t pos) {
    if (pos == s.size()) {
        return &node.prefix_set;
    }
    if (node.children.find(s[pos]) == node.children.end()) {
        return &empty_set<T>;
    }
    return trie_prefix_set_ptr(*node.children[s[pos]], s, pos + 1);
}