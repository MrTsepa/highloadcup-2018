#pragma once

#include <string>
#include <sstream>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "trie.hpp"

#define i uint32_t
#define t int32_t
#define year_t int

using namespace std;

struct Account {
    i id = 0;
    t birth = 0;
    t joined = 0;
    t premium_start = 0;
    t premium_finish = 0;

    unordered_set<string> interests = unordered_set<string>();
    string email;
    string fname;
    string sname;
    string phone;
    string country;
    string city;

    char status = 0;

    bool sex = false;
    bool has_premium = false;
};

enum Field {
    ID, SEX, EMAIL, STATUS, FNAME, SNAME, PHONE, COUNTRY,
    CITY, BIRTH, INTERESTS, LIKES, PREMIUM, QUERY_ID, LIMIT,
    ORDER, KEYS, JOINED
};

map<Field, string> field_string = {
        {ID, "id"}, {SEX, "sex"}, {EMAIL, "email"}, {STATUS, "status"},
        {FNAME, "fname"}, {SNAME, "sname"}, {PHONE, "phone"},
        {COUNTRY, "country"}, {CITY, "city"}, {BIRTH, "birth"},
        {INTERESTS, "interests"}, {LIKES, "likes"},
        {PREMIUM, "premium"}
};

template <typename T>
struct IndexSet {
    unordered_set<T> uset;
    set<T> sset;

    bool no_sset = false;

    void emplace(T &arg) {
        uset.emplace(arg);
        sset.emplace(arg);
    }

    void emplace(const T &arg) {
        uset.emplace(arg);
        sset.emplace(arg);
    }

    void emplace_no_sset(T &arg) {
        no_sset = true;
        uset.emplace(arg);
    }

    typename set<T>::reverse_iterator srbegin() {
        return sset.rbegin();
    }

    typename set<T>::reverse_iterator srend() {
        return sset.rend();
    }

    bool has(const T &arg) {
        return uset.find(arg) != uset.end();
    }

    size_t size() {
        return uset.size();
    }

    void clear() {
        uset.clear();
        sset.clear();
    }
};

const size_t SPLIT_LEN = 600;
const size_t SPLIT_COUNT = 50;

struct Index {
    IndexSet<i> all;
    IndexSet<i> is_f;
    IndexSet<i> is_m;
    IndexSet<i> has_active_premium;

    map<string, i> email_cmp;
    IndexSet<i> email_cmp_split[SPLIT_COUNT];
    map<t, i> birth_cmp;
    IndexSet<i> birth_cmp_split[SPLIT_COUNT];


    IndexSet<i> fname_null;
    IndexSet<i> sname_null;
    IndexSet<i> phone_null;
    IndexSet<i> country_null;
    IndexSet<i> city_null;
    IndexSet<i> premium_null;

    unordered_map<string, IndexSet<i> > email_domain_index;
    IndexSet<i> status_indexes[3];
    unordered_map<string, IndexSet<i> > fname_index;
    unordered_map<string, IndexSet<i> > sname_index;
    unordered_map<string, IndexSet<i> > phone_index;
    unordered_map<string, IndexSet<i> > country_index;
    unordered_map<string, IndexSet<i> > city_index;
    unordered_map<year_t, IndexSet<i> > year_index;
    unordered_map<year_t, IndexSet<i> > joined_year_index;
    unordered_map<string, IndexSet<i> > interests_index;
    unordered_map<i, IndexSet<i> > like_index;

    TrieNode<IndexSet<i> > sname_prefix_trie;
};

string account_field_value(Account& account, Field field) {
    switch (field) {
        case ID: return to_string(account.id);
        case BIRTH: return to_string(account.birth);
        case SEX: return account.sex ? "f" : "m";
        case EMAIL: return account.email;
        case FNAME: return account.fname;
        case SNAME: return account.sname;
        case PHONE: return account.phone;
        case COUNTRY: return account.country;
        case CITY: return account.city;
        case STATUS: {
            switch (account.status) {
                case 0:
                    return "свободны";
                case 1:
                    return "всё сложно";
                case 2:
                    return "заняты";
                default:
                    return "";
            }
        }
        default: return "";
        case PREMIUM: {
            stringstream ss;
            ss  << "{\"start\":"
                << account.premium_start
                << ",\"finish\":"
                << account.premium_finish
                << "}";
            return ss.str();
        }
    }
}
