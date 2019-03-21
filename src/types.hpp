#pragma once

#include <string>
#include <sstream>

#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <bitset>

#define i_t uint32_t
#define t_t int32_t
#define year_t unsigned short

using namespace std;

typedef bitset<1400000> bset;

struct Account {
    i_t id;
    t_t birth;
    t_t premium_start = 0;
    t_t premium_finish = 0;
    unsigned short fname;
    unsigned short city;
    unsigned char status;
    unsigned char country;
    bool has_premium = false;
    bool sex;
    string email;
    string phone;
    string sname;
    boost::container::flat_set<unsigned char> interests;
};

struct Like {
    i_t from;
    i_t to;
    t_t time;
};

struct Strings {
    boost::container::flat_set<string> interests;
    boost::container::flat_set<string> cities;
    boost::container::flat_set<string> countries;
    boost::container::flat_set<string> fnames;
};

struct Index {
    unordered_map<i_t, size_t> id_index;
    unordered_map<bool, bset> sex_index;
    unordered_map<year_t, bset> year_index;
    unordered_map<string, bset> domain_index;
    unordered_map<unsigned char, bset> interests_index;
    unordered_map<unsigned char, bset> status_index;
    unordered_map<i_t, boost::container::flat_set<i_t>> like_index;
    unordered_map<unsigned short, bset> cities_index;
    unordered_map<unsigned char, bset> countries_index;
    unordered_map<unsigned short, bset> fname_index;
    unordered_map<string, bset> code_index;
    bset has_active_premium;
    bset premium_null;
    bset phone_null;
    bset sname_null;
};

typedef vector<Account> Store;
typedef vector<Like> Likes;

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

enum Pred {
    LT, GT, EQ, STARTS, NULL_, DOMAIN_, NEQ, ANY, CODE, YEAR, CONTAINS, NOW, Nan
};

string account_field_value(Account& account, Field field, Strings &strings) {
    switch (field) {
        case ID: return to_string(account.id);
        case BIRTH: return to_string(account.birth);
        case SEX: return account.sex ? "f" : "m";
        case EMAIL: return account.email;
        case FNAME: return *strings.fnames.nth(account.fname);
        case SNAME: return account.sname;
        case PHONE: return account.phone;
        case COUNTRY: return *strings.countries.nth(account.country);
        case CITY: return *strings.cities.nth(account.city);
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
