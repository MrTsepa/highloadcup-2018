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

enum Field {
    ID, SEX, EMAIL, STATUS, FNAME, SNAME, PHONE, COUNTRY,
    CITY, BIRTH, INTERESTS, LIKES, PREMIUM, QUERY_ID, LIMIT
};

map<Field, string> field_string = {
        {ID, "id"}, {SEX, "sex"}, {EMAIL, "email"}, {STATUS, "status"},
        {FNAME, "fname"}, {SNAME, "sname"}, {PHONE, "phone"},
        {COUNTRY, "country"}, {CITY, "city"}, {BIRTH, "birth"},
        {INTERESTS, "interests"}, {LIKES, "likes"},
        {PREMIUM, "premium"}
};

string account_field_value(Account& account, Field field) {
    switch (field) {
        case ID: return to_string(account.id);
        case BIRTH: return to_string(account.birth);
        case SEX: return account.sex ? "f" : "m";
        case EMAIL: return account.email;
        case FNAME: return account.fname;
        case SNAME: return account.sname;
        case PHONE: return account.phone;
        case COUNTRY: return account.country;
        case CITY: return account.city;
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
    }
}

t CURRENT_TIME;

unordered_map<i, Account> accounts_map;
unordered_map<i, Like> like_map;

unordered_set<i> all;
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

        all.emplace(account.id);
        if (account.sex) {
            is_f.emplace(account.id);
        } else {
            is_m.emplace(account.id);
        }
        email_cmp[account.email] = account.id;
        birth_cmp[account.birth] = account.id;

        string domain = account.email.substr(account.email.find('@') + 1);
        email_domain_index[domain].emplace(account.id);

        status_indexes[account.status].emplace(account.id);

        if (!account.fname.empty()) {
            fname_index[account.fname].emplace(account.id);
        } else {
            fname_null.emplace(account.id);
        }
        if (!account.sname.empty()) {
            sname_index[account.sname].emplace(account.id);
        } else {
            sname_null.emplace(account.id);
        }
        if (!account.phone.empty()) {
            auto c1 = account.phone.find('(');
            auto c2 = account.phone.find(')');
            string code = account.phone.substr(c1 + 1, c2 - c1 - 1);
            phone_index[code].emplace(account.id);
        } else {
            phone_null.emplace(account.id);
        }
        if (!account.country.empty()) {
            country_index[account.country].emplace(account.id);
        } else {
            country_null.emplace(account.id);
        }
        if (!account.city.empty()) {
            city_index[account.city].emplace(account.id);
        } else {
            city_null.emplace(account.id);
        }
        year_t year = get_year(account.birth);
        year_index[year].emplace(account.id);

        for (const auto &interest : account.interests) {
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

int filter_query_parse(
        const char *query,
        vector<unordered_set<i>* >& sets,
        vector<unordered_set<i>* >& neg_sets,
        long* limit,
        set<Field>& fields
) {
    enum State {
        FIELD, PRED, VALUE, DONE
    };

    enum Pred {
        LT, GT, EQ, STARTS, NULL_, DOMAIN_, NEQ, ANY, CODE, YEAR, CONTAINS, NOW, Nan
    };

    const char *cur = query;
    string val;
    State state = FIELD;
    Field field;
    Pred pred = Nan;
    while (state != FIELD or cur[0] != '\0') {
        switch (state) {
            case FIELD: {
                if (strncmp(cur, "sex", 3) == 0) {
                    field = SEX;
                    cur += 4;
                } else if (strncmp(cur, "email", 5) == 0) {
                    field = EMAIL;
                    cur += 6;
                } else if (strncmp(cur, "status", 6) == 0) {
                    field = STATUS;
                    cur += 7;
                } else if (strncmp(cur, "fname", 5) == 0) {
                    field = FNAME;
                    cur += 6;
                } else if (strncmp(cur, "sname", 5) == 0) {
                    field = SNAME;
                    cur += 6;
                } else if (strncmp(cur, "country", 7) == 0) {
                    field = COUNTRY;
                    cur += 8;
                } else if (strncmp(cur, "city", 4) == 0) {
                    field = CITY;
                    cur += 5;
                } else if (strncmp(cur, "birth", 5) == 0) {
                    field = BIRTH;
                    cur += 6;
                } else if (strncmp(cur, "interests", 9) == 0) {
                    field = INTERESTS;
                    cur += 10;
                } else if (strncmp(cur, "likes", 5) == 0) {
                    field = LIKES;
                    cur += 6;
                } else if (strncmp(cur, "premium", 7) == 0) {
                    field = PREMIUM;
                    cur += 8;
                } else if (strncmp(cur, "query_id", 8) == 0) {
                    field = QUERY_ID;
                    cur += 9;
                    state = VALUE;
                    break;
                } else if (strncmp(cur, "limit", 5) == 0) {
                    field = LIMIT;
                    cur += 6;
                    state = VALUE;
                    break;
                } else {
                    return -1;
                }
                state = PRED;
                break;
            }
            case PRED: {
                if (strncmp(cur, "gt", 2) == 0) {
                    pred = GT;
                    cur += 3;
                } else if (strncmp(cur, "lt", 2) == 0) {
                    pred = LT;
                    cur += 3;
                } else if (strncmp(cur, "eq", 2) == 0) {
                    pred = EQ;
                    cur += 3;
                } else if (strncmp(cur, "starts", 6) == 0) {
                    pred = STARTS;
                    cur += 7;
                } else if (strncmp(cur, "null", 4) == 0) {
                    pred = NULL_;
                    cur += 5;
                } else if (strncmp(cur, "domain", 6) == 0) {
                    pred = DOMAIN_;
                    cur += 7;
                } else if (strncmp(cur, "neq", 3) == 0) {
                    pred = NEQ;
                    cur += 4;
                } else if (strncmp(cur, "any", 3) == 0) {
                    pred = ANY;
                    cur += 4;
                } else if (strncmp(cur, "code", 4) == 0) {
                    pred = CODE;
                    cur += 5;
                } else if (strncmp(cur, "year", 4) == 0) {
                    pred = YEAR;
                    cur += 5;
                } else if (strncmp(cur, "contains", 8) == 0) {
                    pred = CONTAINS;
                    cur += 9;
                } else if (strncmp(cur, "now", 3) == 0) {
                    pred = NOW;
                    cur += 4;
                } else {
                    return -1;
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
                fields.emplace(field);
                switch (field) {
                    case SEX:
                        switch (pred) {
                            case EQ:
                                sets.emplace_back(val[0] == 'm' ? &is_m : &is_f);
                                break;
                            default: return -1;
                        }
                        break;
                    case EMAIL: {
                        switch (pred) {
                            case DOMAIN_: {
                                sets.emplace_back(&email_domain_index[val]);
                                break;
                            }
                            case LT: {
                                auto lower = email_cmp.lower_bound(val);
                                auto *s = new unordered_set<i>;
                                for (auto it = email_cmp.begin(); it != lower; it++) {
                                    s->emplace(it->second);
                                }
                                sets.emplace_back(s);
                                break;
                            }
                            case GT: {
                                auto upper = email_cmp.upper_bound(val);
                                auto *s = new unordered_set<i>;
                                for (auto it = upper; it != email_cmp.end(); it++) {
                                    s->emplace(it->second);
                                }
                                sets.emplace_back(s);
                                break;
                            }
                            default: return -1;
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
                                sets.emplace_back(&status_indexes[status]);

                                break;
                            }
                            case NEQ: {
                                neg_sets.emplace_back(&status_indexes[status]);
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case FNAME: {
                        switch (pred) {
                            case EQ: {
                                sets.emplace_back(&fname_index[val]);


                                break;
                            }
                            case ANY: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    sets.emplace_back(&fname_index[val.substr(start, at - start)]);

                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            case NULL_: {
                                sets.emplace_back(&fname_null);

                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case SNAME: {
                        switch (pred) {
                            case EQ: {
                                sets.emplace_back(&sname_index[val]);
                                break;
                            }
                            case ANY: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    sets.emplace_back(&sname_index[val.substr(start, at - start)]);

                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            case NULL_: {
                                sets.emplace_back(&sname_null);
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case PHONE: {
                        switch (pred) {
                            case CODE: {
                                sets.emplace_back(&phone_index[val]);
                                break;
                            }
                            case NULL_: {
                                sets.emplace_back(&phone_null);
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case COUNTRY: {
                        switch (pred) {
                            case EQ: {
                                auto it = country_index.find(val);
                                if (it != country_index.end()) {
                                    sets.emplace_back(&it->second);
                                }
                                break;
                            }
                            case NULL_: {
                                sets.emplace_back(&country_null);
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case LIMIT: {
                        char* err;
                        long converted = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        } else {
                            *limit = converted;
                        }
                        break;
                    }
                }
                state = FIELD;
                break;
            }
        }
    }
    return 0;
}

void merge_sets(vector<unordered_set<i>* >& sets,
                vector<unordered_set<i>* >& neg_sets,
                set<i>* result) {
    if (!sets.empty()) {
        size_t min_size = sets[0]->size();
        size_t k_min = 0;
        for (size_t k = 1; k < sets.size(); k++) {
            if (min_size < sets[k]->size()) {
                min_size = sets[k]->size();
                k_min = k;
            }
        }
        for (const auto& item : *sets[k_min]) {
            bool good = true;
            for (size_t k = 0; k < sets.size(); k++) {
                if (k == k_min) {
                    continue;
                }
                if (sets[k]->find(item) == sets[k]->end()) {
                    good = false;
                    break;
                }
            }
            if (good) {
                for (size_t k = 0; k < neg_sets.size(); k++) {
                    if (neg_sets[k]->find(item) != neg_sets[k]->end()) {
                        good = false;
                        break;
                    }
                }
            }
            if (good) {
                result->emplace(item);
            }
        }
    }
}

set<i> merge_result;
vector<unordered_set<i> *> sets;
vector<unordered_set<i> *> neg_sets;

void filter(evhttp_request *request, void *params) {
    long limit = 0;
    const char *query = strchr(request->uri, '?') + 1;
    merge_result.clear();
    sets.clear();
    neg_sets.clear();
    sets.emplace_back(&all);
    set<Field> fields = set<Field> {ID, EMAIL};
    if (filter_query_parse(query, sets, neg_sets, &limit, fields) != 0) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        return;
    }
    merge_sets(sets, neg_sets, &merge_result);

    evbuffer *buffer;
    buffer = evbuffer_new();
    evbuffer_add_printf(buffer, "{\"accounts\": [");
    int k = 0;
    for (auto it = merge_result.rbegin(); it != merge_result.rend(); it++) {
        if (k >= limit) {
            break;
        }
        if (k > 0) {
            evbuffer_add_printf(buffer, ",");
        }
        evbuffer_add_printf(buffer, "{");
        size_t l = 0;
        for (auto fit = fields.begin(); fit != fields.end(); fit++, l++) {
            const Field field = *fit;
            if (field == LIMIT or field == QUERY_ID) {
                continue;
            }
            if (l > 0) {
                evbuffer_add_printf(buffer, ",");
            }
            if (field == ID or field == BIRTH) {
                evbuffer_add_printf(
                        buffer,
                        R"("%s":%s)",
                        field_string[field].data(),
                        account_field_value(accounts_map[*it], field).data()
                );
            } else {
                evbuffer_add_printf(
                        buffer,
                        R"("%s":"%s")",
                        field_string[field].data(),
                        account_field_value(accounts_map[*it], field).data()
                );
            }
        }
        evbuffer_add_printf(buffer, "}");
        k++;
    }
    evbuffer_add_printf(buffer, "]}");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                       "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evhttp_send_reply(request, HTTP_OK, "OK", buffer);
    evbuffer_free(buffer);
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
    evhttp_set_cb(server, "/accounts/filter/", filter, nullptr);
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