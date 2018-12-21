#include <fstream>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

#include <cstdint>

#include <unordered_map>

#define i int32_t
#define t int32_t

using namespace std;

struct Account {
    i id;
    t birth;
    t joined;
    t premium_start;
    t premium_finish;

    string * interests;
    string email;
    string fname;
    string sname;
    string phone;
    string country;
    string city;

    char sex;
    bool status1;
    bool status2;
    bool has_premium;
};

struct Like {
    i from;
    i to;
    t time;
};

void prepare() {
    ifstream ifs("../accounts_1.json");
    if (!ifs.is_open()) {
        std::cout << "Failed to load file" << std::endl;
    }

    unordered_map<i, Account> map;
    unordered_map<i, Like> like_map;

    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream(isw);
    auto json_accounts = d["accounts"].GetArray();
    for (auto &json_account : json_accounts) {
        Account account{};
        account.id = json_account["id"].GetInt();
        account.sex = json_account["sex"].GetString()[0];
        account.birth = json_account["birth"].GetInt();
        account.joined = json_account["joined"].GetInt();
        account.email = string(
                json_account["email"].GetString(),
                json_account["email"].GetStringLength()
        );

        if (json_account["status"].GetString()[0] == -47) { // свободны
            account.status1 = false;
            account.status2 = false;
        } else {
            if (json_account["status"].GetString()[1] == -78) { // все сложно
                account.status1 = true;
                account.status2 = false;
            } else { // заняты
                account.status1 = false;
                account.status2 = true;
            }
        }

        if (json_account.HasMember("fname")) {
            account.fname = string(
                    json_account["fname"].GetString(),
                    json_account["fname"].GetStringLength()
            );
        }

        if (json_account.HasMember("sname")) {
            account.fname = string(
                    json_account["sname"].GetString(),
                    json_account["sname"].GetStringLength()
            );
        }

        if (json_account.HasMember("phone")) {
            account.phone = string(
                    json_account["phone"].GetString(),
                    json_account["phone"].GetStringLength()
            );
        }

        if (json_account.HasMember("country")) {
            account.country = string(
                    json_account["country"].GetString(),
                    json_account["country"].GetStringLength()
            );
        }

        if (json_account.HasMember("city")) {
            account.city = string(
                    json_account["city"].GetString(),
                    json_account["city"].GetStringLength()
            );
        }

        if (json_account.HasMember("premium")) {
            account.has_premium = true;
            account.premium_start = json_account["premium"]["start"].GetInt();
            account.premium_finish = json_account["premium"]["finish"].GetInt();
        }

        map[account.id] = account;
        if (json_account.HasMember("likes")) {
            for (auto &json_like : json_account["likes"].GetArray()) {
                Like like{};
                like.from = json_account["id"].GetInt();
                like.to = json_like["id"].GetInt();
                like.time = json_like["ts"].GetInt();
                like_map[like.from] = like;
            }
        }
    }

    cout << map.size() << endl;
    cout << like_map.size() << endl;
}

int main() {
    prepare();
    return 0;
}