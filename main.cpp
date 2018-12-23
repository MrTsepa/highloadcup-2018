#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>

#include <map>
#include <unordered_map>
#include <vector>

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
    i email_domain;
    t birth;
    t joined;
    t premium_start;
    t premium_finish;

    string * interests;
    string email_name;
    string fname;
    string sname;
    string phone;
    string country;
    string city;

    bool sex;
    bool status1 = false;
    bool status2 = false;
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
boost::bimap<boost::bimaps::unordered_set_of<i>,
        boost::bimaps::unordered_set_of<string> > domains;

map<string, i> emails_map;


void unzip() {
    system(string("unzip -q -o ").append(getenv("DATA_PATH")).data());
}

void prepare() {
    for (int i = 1; ; i++) {
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

            auto at = email.find('@');
            account.email_name = email.substr(0, at);
            string domain = email.substr(at + 1);
            auto domains_it = domains.right.find(domain);
            if (domains_it == domains.right.end()) {
                account.email_domain = static_cast<unsigned int>(domain.size());
                domains.insert({static_cast<unsigned int>(domains.size()), domain});
            } else {
                account.email_domain = domains_it->second;
            }

            if (json_account["status"].GetString()[0] != -47) { // свободны
                if (json_account["status"].GetString()[1] == -78) { // все сложно
                    account.status1 = true;
                } else { // заняты
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
            emails_map[email] = account.id;

        }
    }

    cout << "Accounts " << accounts_map.size() << endl;
    cout << "Likes " << like_map.size() << endl;
    cout << "Domains " << domains.size() << endl;
}

void notfound(evhttp_request *request, void *params) {
    evhttp_send_error(request, HTTP_NOTFOUND, nullptr);
}

void start_server(char * host, uint16_t port) {
    event_base* ebase;
    evhttp* server;
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
    unzip();
    prepare();
    char * host = getenv("HOST");
    uint16_t port = static_cast<uint16_t>(stoi(getenv("PORT")));
    start_server(host, port);
    return 0;
}