#pragma once

#include <string>
#include <sstream>

#include <map>
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
    CITY, BIRTH, INTERESTS, LIKES, PREMIUM, QUERY_ID, LIMIT
};

map<Field, string> field_string = {
        {ID, "id"}, {SEX, "sex"}, {EMAIL, "email"}, {STATUS, "status"},
        {FNAME, "fname"}, {SNAME, "sname"}, {PHONE, "phone"},
        {COUNTRY, "country"}, {CITY, "city"}, {BIRTH, "birth"},
        {INTERESTS, "interests"}, {LIKES, "likes"},
        {PREMIUM, "premium"}
};

struct Index {
    unordered_set<i> all;
    unordered_set<i> is_f;
    unordered_set<i> is_m;
    unordered_set<i> has_active_premium;

    map<string, i> email_cmp;
    map<t, i> birth_cmp;

    unordered_set<i> fname_null;
    unordered_set<i> sname_null;
    unordered_set<i> phone_null;
    unordered_set<i> country_null;
    unordered_set<i> city_null;
    unordered_set<i> premium_null;

    unordered_map<string, unordered_set<i> > email_domain_index;
    unordered_set<i> status_indexes[3];
    unordered_map<string, unordered_set<i> > fname_index;
    unordered_map<string, unordered_set<i> > sname_index;
    unordered_map<string, unordered_set<i> > phone_index;
    unordered_map<string, unordered_set<i> > country_index;
    unordered_map<string, unordered_set<i> > city_index;
    unordered_map<year_t, unordered_set<i> > year_index;
    unordered_map<string, unordered_set<i> > interests_index;
    unordered_map<i, unordered_set<i> > like_index;

    TrieNode<i> sname_prefix_trie;
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
