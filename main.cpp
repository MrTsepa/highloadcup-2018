#include <fstream>
#include <iostream>
#include <sstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <cstdint>

#include <unordered_map>
#include <evhttp.h>

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
    bool status1 = false;
    bool status2 = false;
    bool has_premium = false;
};

struct Like {
    i from;
    i to;
    t time;
};

unordered_map<i, Account> map;
unordered_map<i, Like> like_map;

void unzip() {
    char * cpath = getenv("DATA_PATH");
    string path;
    if (cpath == nullptr) {
        path = "data.zip";
    } else {
        path = string(cpath);
    }
    system(string("unzip -q -o ").append(path).data());
}

void prepare() {
    for (int i = 1; ; i++) {
        char path[20];
        sprintf(path, "accounts_%d.json", i);
        cout << path;
        auto ifs = ifstream(path);
        if (!ifs.is_open()) {
            cout << " fail" << endl;
            break;
        }
        cout << " success" << endl;
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
    }

    cout << map.size() << endl;
    cout << like_map.size() << endl;
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