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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <evhttp.h>

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

struct Like {
    i to;
    t time;
};

t CURRENT_TIME;

unordered_map<i, Account> accounts_map;
unordered_map<i, Like> like_map;

vector<i> order;
unordered_set<i> is_f;
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


year_t get_year(t timestamp) {
    time_t a = timestamp;
    tm utc_tm = *gmtime(&a);
    return utc_tm.tm_year + 1900;
}

void unzip() {
    char command[255];
    sprintf(command, "unzip -q -o %s", getenv("DATA_PATH"));
    system(command);
}

void read_time() {
    char options_path[255];
    sprintf(options_path, "%s", getenv("OPTIONS_PATH"));
    auto ifs = ifstream(options_path);
    ifs >> CURRENT_TIME;
}

void parse_json() {
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
                    Like like{};
                    auto from = static_cast<unsigned int>(json_account["id"].GetInt());
                    like.to = static_cast<unsigned int>(json_like["id"].GetInt());
                    like.time = json_like["ts"].GetInt();
                    like_map[from] = like;
                }
            }
        }
    }
}

void build_indices() {
    for (const auto& pair : accounts_map) {
        Account account = pair.second;

        order.emplace_back(account.id);
        if (account.sex) {
            is_f.emplace(account.id);
        }
        email_cmp[account.email] = account.id;
        birth_cmp[account.birth] = account.id;

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
        year_t year = get_year(account.birth);
        auto year_index_it = year_index.find(year);
        if (year_index_it == year_index.end()) {
            year_index[year] = unordered_set<uint>();
        }
        year_index[year].emplace(account.id);

        for (const auto &interest : account.interests) {
            if (interests_index.find(interest) == interests_index.end()) {
                interests_index[interest] = unordered_set<uint>();
            }
            interests_index[interest].emplace(account.id);
        }
        if (account.has_premium) {
            if (account.premium_start <= CURRENT_TIME and account.premium_finish >= CURRENT_TIME) {
                has_active_premium.emplace(account.id);
            }
        } else {
            premium_null.emplace(account.id);
        }
        like_index[account.id] = unordered_set<uint>();
    }

    for (const auto& it : like_map) {
        like_index[it.second.to].emplace(it.first);
    }
}

void notfound(evhttp_request *request, void *params) {
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "text/plain");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
}

void start_server(char *host, uint16_t port) {
    event_base *ebase;
    evhttp *server;
    ebase = event_base_new();
    server = evhttp_new(ebase);
    evhttp_set_gencb(server, notfound, nullptr);
    if (evhttp_bind_socket(server, host, port) != 0) {
        cout << "Could not bind to " << host << ":" << port << endl;
    }
    event_base_dispatch(ebase);
    evhttp_free(server);
    event_base_free(ebase);
}

int main() {
    chrono::time_point<chrono::system_clock> t;
    unzip();
    read_time();
    t = chrono::system_clock::now();
    parse_json();
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    build_indices();
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    if (getenv("START_SERVER")[0] == '1') {
        char *host = getenv("HOST");
        uint16_t port = static_cast<uint16_t>(stoi(getenv("PORT")));
        start_server(host, port);
    } else {
        cout << "START_SERVER is 0, quiting." << endl;
        return 0;
    }
    return 0;
}