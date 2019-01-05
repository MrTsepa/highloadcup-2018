#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "types.hpp"
#include "utils.hpp"

using namespace std;

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
            Account account;
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
                account.city = static_cast<unsigned short>(strings.cities.index_of(it));
            }
            if (json_account.HasMember("country")) {
                string country = json_account["country"].GetString();
                auto it = strings.countries.emplace(country).first;
                account.country = static_cast<unsigned char>(strings.countries.index_of(it));
            }
            if (json_account.HasMember("fname")) {
                string fname = json_account["fname"].GetString();
                auto it = strings.fnames.emplace(fname).first;
                account.fname = static_cast<unsigned short>(strings.fnames.index_of(it));
            }
            if (json_account.HasMember("sname")) {
                account.sname = json_account["sname"].GetString();
            }
            account.status = convert_status(json_account["status"].GetString());
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
