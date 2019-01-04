#pragma once

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "types.hpp"

using namespace std;

void parse_json(unordered_map<i, Account>& accounts_map, unordered_map<i, unordered_map<i, t> >& like_map) {
    for (int j = 1;; j++) {
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
            account.joined = json_account["joined"].GetInt();
            string email = string(
                    json_account["email"].GetString(),
                    json_account["email"].GetStringLength()
            );
            account.email = email;
            if (json_account["status"].GetString()[0] != -47) { // свободны
                if (json_account["status"].GetString()[1] == -78) { // все сложно
                    account.status = 1;
                } else { // заняты
                    account.status = 2;
                }
            }

            if (json_account.HasMember("fname")) {
                account.fname = string(
                        json_account["fname"].GetString(),
                        json_account["fname"].GetStringLength()
                );
            }

            if (json_account.HasMember("sname")) {
                account.sname = string(
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

            if (json_account.HasMember("interests")) {
                for (const auto &s : json_account["interests"].GetArray()) {
                    account.interests.emplace(string(s.GetString(), s.GetStringLength()));
                }
            }

            accounts_map[account.id] = account;

            if (json_account.HasMember("likes")) {
                for (const auto &json_like : json_account["likes"].GetArray()) {

                    i from = static_cast<unsigned int>(json_account["id"].GetInt());
                    i to = static_cast<unsigned int>(json_like["id"].GetInt());
                    t time = json_like["ts"].GetInt();
                    like_map[from].emplace(to, time);
                }
            }
        }
    }
}

