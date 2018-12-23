#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>
#include <chrono>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <bitset>

#include <boost/dynamic_bitset.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <evhttp.h>

#define i uint32_t
#define t int32_t

using namespace std;

struct Account {
    i id;
    t birth;
    t joined;
    t premium_start;
    t premium_finish;

    string *interests;
    string email;
    string fname;
    string sname;
    string phone;
    string country;
    string city;

    char status = 0;

    bool sex;
    bool has_premium = false;
};

struct Like {
    i from;
    i to;
    t time;
};

unordered_map<i, Account> accounts_map;
unordered_map<i, Like> like_map;

vector<i> order;
boost::dynamic_bitset<> sex;

map<string, i> email_comp;
unordered_map<string, unordered_set<i> > email_domain_index;
unordered_set<i> status_indexes[3];
unordered_map<string, unordered_set<i> > fname_index;
unordered_set<i> fname_null;
unordered_map<string, unordered_set<i> > sname_index;
unordered_set<i> sname_null;
unordered_map<string, unordered_set<i> > phone_index;
unordered_set<i> phone_null;
unordered_map<string, unordered_set<i> > country_index;
unordered_set<i> country_null;
unordered_map<string, unordered_set<i> > city_index;
unordered_set<i> city_null;

void unzip() {
    system(string("unzip -q -o ").append(getenv("DATA_PATH")).data());
}

void prepare() {
    for (int i = 1;; i++) {
        char path[20];
        sprintf(path, "accounts_%d.json", i);
        auto ifs = ifstream(path);
        if (!ifs.is_open()) {
            break;
        }
        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document d;
        d.ParseStream(isw);
        auto json_accounts = d["accounts"].GetArray();
        for (auto &json_account : json_accounts) {

            // CONSTRUCT ACCOUNT

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

            accounts_map[account.id] = account;

            // CONSTRUCT LIKES

            if (json_account.HasMember("likes")) {
                for (auto &json_like : json_account["likes"].GetArray()) {
                    Like like{};
                    like.from = static_cast<unsigned int>(json_account["id"].GetInt());
                    like.to = static_cast<unsigned int>(json_like["id"].GetInt());
                    like.time = json_like["ts"].GetInt();
                    like_map[like.from] = like;
                }
            }

            // BUILD INDEXES

            order.emplace_back(account.id);
            sex.push_back(account.sex);
            email_comp[email] = account.id;

            auto at = account.email.find('@');
            string domain = account.email.substr(at + 1);
            auto domain_index_it = email_domain_index.find(domain);
            if (domain_index_it == email_domain_index.end()) {
                email_domain_index[domain] = unordered_set<uint>();
            }
            email_domain_index[domain].emplace(account.id);
            status_indexes[account.status].emplace(account.id);
            if (!account.fname.empty()) {
                auto fname_index_it = fname_index.find(account.fname);
                if (fname_index_it == fname_index.end()) {
                    fname_index[account.fname] = unordered_set<uint>();
                }
                fname_index[account.fname].emplace(account.id);
            } else {
                fname_null.emplace(account.id);
            }
            if (!account.sname.empty()) {
                auto sname_index_it = sname_index.find(account.sname);
                if (sname_index_it == sname_index.end()) {
                    sname_index[account.sname] = unordered_set<uint>();
                }
                sname_index[account.sname].emplace(account.id);
            } else {
                sname_null.emplace(account.id);
            }
            if (!account.phone.empty()) {
                auto c1 = account.phone.find('(');
                auto c2 = account.phone.find(')');
                string code = account.phone.substr(c1 + 1, c2 - c1 - 1);
                auto phone_index_it = phone_index.find(code);
                if (phone_index_it == phone_index.end()) {
                    phone_index[code] = unordered_set<uint>();
                }
                phone_index[code].emplace(account.id);
            } else {
                phone_null.emplace(account.id);
            }
            if (!account.country.empty()) {
                auto country_index_it = country_index.find(account.country);
                if (country_index_it == country_index.end()) {
                    country_index[account.country] = unordered_set<uint>();
                }
                country_index[account.country].emplace(account.id);
            } else {
                country_null.emplace(account.id);
            }
            if (!account.city.empty()) {
                auto city_index_it = city_index.find(account.city);
                if (city_index_it == city_index.end()) {
                    city_index[account.city] = unordered_set<uint>();
                }
                city_index[account.city].emplace(account.id);
            } else {
                city_null.emplace(account.id);
            }
        }
    }

    cout << "Accounts " << accounts_map.size() << endl;
    cout << "Likes " << like_map.size() << endl;
//    for (auto it : country_index) {
//        cout << it.first << " " << it.second.size() << endl;
//    }
//    for (auto it : status_indexes) {
//        cout << it.size() << endl;
//    }

}

void notfound(evhttp_request *request, void *params) {
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "text/plain");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
    return;
}

void start_server(char *host, uint16_t port) {
    event_base *ebase;
    evhttp *server;
    ebase = event_base_new();
    server = evhttp_new(ebase);
    evhttp_set_gencb(server, notfound, nullptr);
    if (evhttp_bind_socket(server, host, port) != 0) {
        cout << "Could not bind to " << host << ":" << port << endl;
    } else {
        cout << "Binded to " << host << ":" << port << endl;
    }
    event_base_dispatch(ebase);
    evhttp_free(server);
    event_base_free(ebase);
}

int main() {
    chrono::time_point<chrono::system_clock> t;
    unzip();
    t = chrono::system_clock::now();
    prepare();
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    char * host = getenv("HOST");
    uint16_t port = static_cast<uint16_t>(stoi(getenv("PORT")));
    start_server(host, port);
    return 0;
}