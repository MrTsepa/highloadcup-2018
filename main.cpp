#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#include <string>
#include <chrono>

#include <map>
#include <set>
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
        } else {
            is_m.emplace(account.id);
        }
        email_cmp[account.email] = account.id;
        birth_cmp[account.birth] = account.id;

        string domain = account.email.substr(account.email.find('@') + 1);
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


void filter_query_parse(
        const char *query,
        unordered_set<i> **sets,
        unordered_set<i> **neg_sets,
        size_t *sets_size,
        size_t *neg_sets_size
) {
    enum State {
        FIELD, PRED, VALUE, DONE
    };
    enum Field {
        SEX, EMAIL, STATUS, FNAME, SNAME, PHONE, COUNTRY,
        CITY, BIRTH, INTERESTS, LIKES, PREMIUM
    };
    enum Pred {
        LT, GT, EQ, STARTS, NULL_, DOMAIN_, NEQ, ANY, CODE, YEAR, CONTAINS, NOW
    };

    const char *cur = query;
    size_t k = 0, g = 0;
    string val;
    State state = FIELD;
    Field field;
    Pred pred;
    while (state != FIELD or cur[0] != '\0') {
        switch (state) {
            case FIELD: {
                if (cur[0] == 's' and cur[1] == 'e') {
                    field = SEX;
                    cur += 4;
                } else if (cur[0] == 'e') {
                    field = EMAIL;
                    cur += 6;
                } else if (cur[0] == 's' and cur[1] == 't') {
                    field = STATUS;
                    cur += 7;
                } else if (cur[0] == 'f') {
                    field = FNAME;
                    cur += 6;
                } else if (cur[0] == 's' and cur[1] == 'n') {
                    field = SNAME;
                    cur += 6;
                } else if (cur[0] == 'c' and cur[1] == 'o') {
                    field = COUNTRY;
                    cur += 8;
                } else if (cur[0] == 'c' and cur[1] == 'i') {
                    field = CITY;
                    cur += 5;
                } else if (cur[0] == 'b') {
                    field = BIRTH;
                    cur += 6;
                } else if (cur[0] == 'i') {
                    field = INTERESTS;
                    cur += 10;
                } else if (cur[0] == 'l') {
                    field = LIKES;
                    cur += 6;
                } else if (cur[0] == 'p') {
                    field = PREMIUM;
                    cur += 8;
                }
                state = PRED;
                break;
            }
            case PRED: {
                if (cur[0] == 'g' and cur[1] == 't') {
                    pred = GT;
                    cur += 3;
                } else if (cur[0] == 'l') {
                    pred = LT;
                    cur += 3;
                } else if (cur[0] == 'e') {
                    pred = EQ;
                    cur += 3;
                } else if (cur[0] == 's') {
                    pred = STARTS;
                    cur += 6;
                } else if (cur[0] == 'n' and cur[1] == 'u') {
                    pred = NULL_;
                    cur += 5;
                } else if (cur[0] == 'd') {
                    pred = DOMAIN_;
                    cur += 7;
                } else if (cur[0] == 'n' and cur[1] == 'e') {
                    pred = NEQ;
                    cur += 4;
                } else if (cur[0] == 'a') {
                    pred = ANY;
                    cur += 4;
                } else if (cur[0] == 'c' and cur[1] == 'o') {
                    pred = CODE;
                    cur += 5;
                } else if (cur[0] == 'y') {
                    pred = YEAR;
                    cur += 5;
                } else if (cur[0] == 'c' and cur[1] == 'o') {
                    pred = CONTAINS;
                    cur += 9;
                } else if (cur[0] == 'n' and cur[1] == 'o') {
                    pred = NOW;
                    cur += 4;
                }
                state = VALUE;
                break;
            }
            case VALUE: {
                const char *amp = strchr(cur, '&');
                if (amp != nullptr) {
                    val = string(cur, amp - cur);
                    cur = amp + 1;
                } else {
                    val = string(cur);
                    cur = cur + val.size();
                }
                state = DONE;
                break;
            }
            case DONE: {
                switch (field) {
                    case SEX :
                        switch (pred) {
                            case EQ:
                                sets[k] = val[0] == 'm' ? &is_m : &is_f;
                                k++;
                        }
                        break;
                    case EMAIL: {
                        switch (pred) {
                            case DOMAIN_: {
                                sets[k] = &email_domain_index[val];
                                k++;
                                break;
                            }
                            case LT: {
                                auto lower = email_cmp.lower_bound(val);
                                auto *s = new unordered_set<i>;
                                for (auto it = email_cmp.begin(); it != lower; it++) {
                                    s->emplace(it->second);
                                }
                                sets[k] = s;
                                k++;
                                break;
                            }
                            case GT: {
                                auto upper = email_cmp.upper_bound(val);
                                auto *s = new unordered_set<i>;
                                for (auto it = upper; it != email_cmp.end(); it++) {
                                    s->emplace(it->second);
                                }
                                sets[k] = s;
                                k++;

                                break;
                            }
                        }
                        break;
                    }
                    case STATUS: {
                        int status = 0; // свободны
                        if (val[0] != -47) {
                            if (val[1] == -78) { // все сложно
                                status = 1;
                            } else { // заняты
                                status = 2;
                            }
                        }
                        switch (pred) {
                            case EQ: {
                                sets[k] = &status_indexes[status];
                                k++;
                                break;
                            }
                            case NEQ: {
                                neg_sets[k] = &status_indexes[status];
                                g++;
                                break;
                            }
                        }
                        break;
                    }
                    case FNAME: {
                        switch (pred) {
                            case EQ: {
                                sets[k] = &fname_index[val];
                                k++;

                                break;
                            }
                            case ANY: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    sets[k] = &fname_index[val.substr(start, at - start)];
                                    k++;
                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            case NULL_: {
                                sets[k] = &fname_null;
                                k++;
                                break;
                            }
                        }
                        break;
                    }
                    case SNAME: {
                        switch (pred) {
                            case EQ: {
                                sets[k] = &sname_index[val];
                                k++;
                                break;
                            }
                            case ANY: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    sets[k] = &sname_index[val.substr(start, at - start)];
                                    k++;
                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            case NULL_: {
                                sets[k] = &sname_null;
                                k++;
                                break;
                            }
                        }
                        break;
                    }
                    case PHONE: {
                        switch (pred) {
                            case CODE: {
                                sets[k] = &phone_index[val];
                                k++;
                                break;
                            }
                            case NULL_: {
                                sets[k] = &phone_null;
                                k++;
                                break;
                            }
                        }
                        break;
                    }
                }
                state = FIELD;
                break;
            }
        }
    }
    *sets_size = k;
    *neg_sets_size = g;
}

void merge_sets(unordered_set<i> **sets,
                unordered_set<i> **neg_sets,
                size_t sets_size,
                size_t neg_sets_size,
                set<i>* result) {
    if (sets_size > 0) {
        size_t min_size = sets[0]->size();
        size_t k_min = 0;
        for (size_t k = 1; k < sets_size; k++) {
            if (min_size < sets[k]->size()) {
                min_size = sets[k]->size();
                k_min = k;
            }
        }
        for (const auto& item : *sets[k_min]) {
            bool good = true;
            for (int k = 0; k < sets_size; k++) {
                if (k == k_min) {
                    continue;
                }
                if (sets[k]->find(item) == sets[k]->end()) {
                    good = false;
                    break;
                }
            }
            if (good) {
                result->emplace(item);
            }
        }
    }
}

void filter(evhttp_request *request, void *params) {
    unordered_set<i> *sets[100];
    unordered_set<i> **neg_sets[100];
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
    evhttp_set_cb(server, "/accounts/filter", filter, nullptr);
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

    unordered_set<i> *sets[100];
    unordered_set<i> *neg_sets[100];
    size_t sets_size = 0;
    size_t neg_sets_size = 0;

    filter_query_parse("email_lt=f&status_eq=всё+сложно&sex_eq=m", sets, neg_sets, &sets_size, &neg_sets_size);
    set<i> result;
    merge_sets(sets, neg_sets, sets_size, neg_sets_size, &result);

    cout << sets_size << ' ' << neg_sets_size << endl;
    for (auto item : result) {
        cout << accounts_map[item].id << ' ' << accounts_map[item].sex << ' ' << accounts_map[item].email << endl;
    }

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