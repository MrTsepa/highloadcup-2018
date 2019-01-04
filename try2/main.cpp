#include <iostream>
#include <set>
#include <chrono>
#include <vector>
#include <bitset>
#include <unordered_set>

#include <evhttp.h>

#include <boost/container/flat_set.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "utils.hpp"


using namespace std;

typedef bitset<1400000> bset;


struct Account {
    i_t id;
    t_t birth;
    t_t premium_start = 0;
    t_t premium_finish = 0;
    bool has_premium;
    bool sex;
    string email;
    boost::container::flat_set<unsigned char> interests;
    unsigned short city;
    unsigned char status;
    unsigned char country;
    string phone;
    unsigned short fname;
    string sname;
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
    bset has_active_premium;
    bset premium_null;
    bset phone_null;
    bset sname_null;
};

typedef vector<Account> Store;
typedef vector<Like> Likes;


void parse_json(Store &store, Likes &likes, Strings &strings) {
    for (int j = 1; j < 200; j++) {
        cout << j << endl;
        char path[20];
        sprintf(path, "accounts_%d.json", j);
        auto ifs = ifstream(path);
        if (!ifs.is_open()) {
            break;
        }
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document d;
        d.ParseStream(isw);
        auto json_accounts = d["accounts"].GetArray();
        for (const auto &json_account : json_accounts) {
            Account account{};
            account.id = static_cast<unsigned int>(json_account["id"].GetInt());
            account.sex = json_account["sex"].GetString()[0] == 'f';
            account.birth = json_account["birth"].GetInt();
            account.email = json_account["email"].GetString();
            if (json_account.HasMember("premium")) {
                account.has_premium = true;
                account.premium_start = json_account["premium"]["start"].GetInt();
                account.premium_finish = json_account["premium"]["finish"].GetInt();
            }
            if (json_account.HasMember("interests")) {
                for (const auto &s : json_account["interests"].GetArray()) {
                    string interest = s.GetString();
                    auto it = strings.interests.emplace(interest).first;
                    account.interests.emplace(strings.interests.index_of(it));
                }
            }
            if (json_account.HasMember("city")) {
                string city = json_account["city"].GetString();
                auto it = strings.cities.emplace(city).first;
                account.city = strings.cities.index_of(it);
            }
            if (json_account.HasMember("country")) {
                string country = json_account["country"].GetString();
                auto it = strings.countries.emplace(country).first;
                account.country = strings.countries.index_of(it);
            }
            if (json_account.HasMember("fname")) {
                string fname = json_account["fname"].GetString();
                auto it = strings.fnames.emplace(fname).first;
                account.fname = strings.fnames.index_of(it);
            }
            if (json_account.HasMember("sname")) {
                account.sname = json_account["sname"].GetString();
            }
            if (json_account["status"].GetString()[0] != -47) { // свободны
                if (json_account["status"].GetString()[1] == -78) { // все сложно
                    account.status = 1;
                } else { // заняты
                    account.status = 2;
                }
            }
            if (json_account.HasMember("phone")) {
                account.phone = string(json_account["phone"].GetString());
            }
            store.emplace_back(account);
            if (json_account.HasMember("likes")) {
                for (const auto &json_like : json_account["likes"].GetArray()) {
                    Like like {
                        like.from = static_cast<unsigned int>(json_account["id"].GetInt()),
                        like.to = static_cast<unsigned int>(json_like["id"].GetInt()),
                        like.time = json_like["ts"].GetInt()
                    };
                    likes.emplace_back(like);
                }
            }
        }
    }
}

void build_indices(Store &store, Likes &likes, Index &index) {
    const t_t CURRENT_TIME = read_time();
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.id_index[account.id] = i;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.sex_index[account.sex][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.year_index[get_year(account.birth)][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        string domain = account.email.substr(account.email.find('@') + 1);
        index.domain_index[domain][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.has_premium) {
            if (account.premium_start <= CURRENT_TIME and account.premium_finish >= CURRENT_TIME) {
                index.has_active_premium[i] = true;
            }
        } else {
            index.premium_null[i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        for (unsigned char j : account.interests) {
            index.interests_index[j][i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.status_index[account.status][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.cities_index[account.city][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.countries_index[account.country][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.fname_index[account.fname][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.phone.empty()) {
            index.phone_null[i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.sname.empty()) {
            index.sname_null[i] = true;
        }
    }
    for (size_t i = 0; i < likes.size(); i++) {
        Like &like = likes[i];
        index.like_index[like.from].emplace(like.to);
    }
}

int main() {
    Store store;
    Index index;
    Strings strings;
    Likes likes;

    chrono::time_point<chrono::system_clock> t;
    t = chrono::system_clock::now();
    if (getenv("REUSE_UNZIP")[0] == '0') {
        unzip();
    }
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    parse_json(store, likes, strings);
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    build_indices(store, likes, index);
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;

//    if (getenv("START_SERVER")[0] == '1') {
//        char *host = getenv("HOST");
//        uint16_t port = static_cast<uint16_t>(stoi(getenv("PORT")));
//        start_server(host, port);
//    } else {
//        cout << "START_SERVER is 0, quiting." << endl;
//        return 0;
//    }
    return 0;
}