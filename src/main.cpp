#include <iostream>
#include <set>
#include <chrono>
#include <vector>
#include <bitset>
#include <unordered_set>

#include <evhttp.h>

#include <boost/container/flat_set.hpp>
#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "utils.hpp"
#include "build_indices.hpp"
#include "parse_json.hpp"
#include "filter_query_parse.hpp"
#include "group_query_parse.hpp"

using namespace std;

Store store;
Index ind;
Strings strings;
Likes likes;


void filter(evhttp_request *request, void *params) {
    try {
        const char *query = evhttp_uridecode(strchr(request->uri, '?') + 1, 1, nullptr);
        Filter::Fields fields;
        vector<Field> fields_to_print {ID, EMAIL};
        long limit;

        fields = Filter::query_parse(query);
        bset result = bset().set();
        for (auto &item : fields) {
            Field field = item.first;
            Pred pred = item.second.first;
            string &val = item.second.second;
            bool print = true;
            switch (field) {
                case SEX: {
                    switch (pred) {
                        case EQ: {
                            result &= ind.sex_index[val == "f"];
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case EMAIL: {
                    switch (pred) {
                        case DOMAIN_: {
                            result &= ind.domain_index[val];
                            break;
                        }
                        case LT: {
                            break;
                        }
                        case GT: {
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case STATUS: {
                    unsigned char status = convert_status(val);
                    switch (pred) {
                        case EQ: {
                            result &= ind.status_index[status];
                            break;
                        }
                        case NEQ: {
                            result &= ~ind.status_index[status];
                            break;
                        }
                        default: return throw -1;
                    }
                    break;
                }
                case FNAME: {
                    switch (pred) {
                        case EQ: {
                            result &= ind.fname_index[strings.fnames.index_of(strings.fnames.find(val))];
                            break;
                        }
                        case ANY: {
                            size_t start = 0;
                            bset any = bset();
                            while (true) {
                                auto at = val.find(',', start);
                                any |= ind.fname_index[strings.fnames.index_of(strings.fnames.find(val.substr(start, at - start)))];
                                start = at + 1;
                                if (at == string::npos) break;
                            }
                            result &= any;
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.fname_index[strings.fnames.index_of(strings.fnames.find(""))];
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.fname_index[strings.fnames.index_of(strings.fnames.find(""))];
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case SNAME: {
                    switch (pred) {
                        case EQ: {
                            break;
                        }
                        case STARTS: {
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.sname_null;
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.sname_null;
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case PHONE: {
                    switch (pred) {
                        case CODE: {
                            result &= ind.code_index[val];
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.phone_null;
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.phone_null;
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case COUNTRY: {
                    switch (pred) {
                        case EQ: {
                            result &= ind.countries_index[strings.countries.index_of(strings.countries.find(val))];
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.countries_index[strings.countries.index_of(strings.countries.find(""))];
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.countries_index[strings.countries.index_of(strings.countries.find(""))];
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case CITY: {
                    switch (pred) {
                        case EQ: {
                            result &= ind.cities_index[strings.cities.index_of(strings.cities.find(val))];
                            break;
                        }
                        case ANY: {
                            size_t start = 0;
                            bset any = bset();
                            while (true) {
                                auto at = val.find(',', start);
                                any |= ind.cities_index[strings.cities.index_of(strings.cities.find(val.substr(start, at - start)))];
                                start = at + 1;
                                if (at == string::npos) break;
                            }
                            result &= any;
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.cities_index[strings.cities.index_of(strings.cities.find(""))];
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.cities_index[strings.cities.index_of(strings.cities.find(""))];
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case BIRTH: {
                    char *err;
                    const long long_val = strtol(val.c_str(), &err, 10);
                    if (*err != 0) throw -1;
                    switch (pred) {
                        case LT: {
                            break;
                        }
                        case GT: {
                            break;
                        }
                        case YEAR: {
                            result &= ind.year_index[long_val];
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case INTERESTS: {
                    switch (pred) {
                        case CONTAINS: {
                            size_t start = 0;
                            while (true) {
                                auto at = val.find(',', start);
                                result &= ind.interests_index[strings.interests.index_of(strings.interests.find(val.substr(start, at - start)))];
                                start = at + 1;
                                if (at == string::npos) break;
                            }
                            break;
                        }
                        case ANY: {
                            size_t start = 0;
                            bset any = bset();
                            while (true) {
                                auto at = val.find(',', start);
                                any |= ind.interests_index[strings.interests.index_of(strings.interests.find(val.substr(start, at - start)))];
                                start = at + 1;
                                if (at == string::npos) break;
                            }
                            result &= any;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case LIKES: {
                    switch (pred) {
                        case CONTAINS: {
                            size_t start = 0;
                            bset all = bset();
                            while (true) {
                                auto at = val.find(',', start);
                                char *err;
                                const long long_val = strtol(val.substr(start, at - start).c_str(), &err, 10);
                                if (*err != 0) throw -1;
                                for (auto &item : ind.like_index[long_val]) {
                                    all[ind.id_index[item]] = true;
                                }
                                result &= all;
                                start = at + 1;
                                if (at == string::npos) break;
                                all.reset();
                            }
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case PREMIUM: {
                    switch (pred) {
                        case NOW: {
                            if (val == "1") {
                                result &= ind.has_active_premium;
                            } else if (val == "0") {
                                result &= ~ind.has_active_premium;
                                print = false;
                            } else throw -1;
                            break;
                        }
                        case NULL_: {
                            if (val == "1") {
                                result &= ind.premium_null;
                                print = false;
                            } else if (val == "0") {
                                result &= ~ind.premium_null;
                            } else throw -1;
                            break;
                        }
                        default: throw -1;
                    }
                    break;
                }
                case LIMIT: {
                    char *err;
                    limit = strtol(val.c_str(), &err, 10);
                    if (*err != 0) {
                        throw -1;
                    }
                    print = false;
                    break;
                }
                case QUERY_ID: {
                    print = false;
                    break;
                }
                default: throw -1;
            }
            if (print) {
                fields_to_print.emplace_back(field);
            }
        }
        evbuffer *buffer;
        buffer = evbuffer_new();
        evbuffer_add_printf(buffer, "{\"accounts\": [");
        size_t k = 0;
        for (int i = store.size() - 1; i >= 0; i--) {
            if (!result[i]) {
                continue;
            }
            if (k >= limit) {
                break;
            }
            bool good = true;
            for (auto &item : fields) {
                Field field = item.first;
                Pred pred = item.second.first;
                string &val = item.second.second;
                switch (field) {
                    case EMAIL: {
                        switch (pred) {
                            case LT: {
                                if (store[i].email >= val) good = false;
                                break;
                            }
                            case GT: {
                                if (store[i].email <= val) good = false;
                                break;
                            }
                        }
                        break;
                    }
                    case SNAME: {
                        switch (pred) {
                            case EQ: {
                                if (store[i].sname != val) good = false;
                                break;
                            }
                            case STARTS: {
                                if (!boost::algorithm::starts_with(store[i].sname, val)) good = false;
                                break;
                            }
                        }
                        break;
                    }
                    case BIRTH: {
                        char *err;
                        const long long_val = strtol(val.c_str(), &err, 10);
                        if (*err != 0) throw -1;
                        switch (pred) {
                            case LT: {
                                if (store[i].birth >= long_val) good = false;
                                break;
                            }
                            case GT: {
                                if (store[i].birth <= long_val) good = false;
                                break;
                            }
                        }
                        break;
                    }
                }
                if (!good) break;
            }
            if (!good) continue;
            if (k > 0) {
                evbuffer_add_printf(buffer, ",");
            }
            evbuffer_add_printf(buffer, "{");
            size_t l = 0;
            for (Field field : fields_to_print) {
                if (field == LIMIT or field == QUERY_ID or field == INTERESTS or field == LIKES) {
                    continue;
                }
                if (l > 0) {
                    evbuffer_add_printf(buffer, ",");
                }
                switch (field) {
                    case ID:
                    case BIRTH:
                    case PREMIUM: {
                        evbuffer_add_printf(
                                buffer,
                                R"("%s":%s)",
                                field_string[field].data(),
                                account_field_value(store[i], field, strings).data()
                        );
                        break;
                    }
                    default: {
                        evbuffer_add_printf(
                                buffer,
                                R"("%s":"%s")",
                                field_string[field].data(),
                                account_field_value(store[i], field, strings).data()
                        );
                    }
                }
                l++;
            }
            evbuffer_add_printf(buffer, "}");
            k++;
        }
        evbuffer_add_printf(buffer, "]}");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "application/json");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_OK, nullptr, buffer);
        evbuffer_free(buffer);
        return;
    } catch (...) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        return;
    }
}

void group(evhttp_request *request, void *params) {
    try {
        const char *query = evhttp_uridecode(strchr(request->uri, '?') + 1, 1, nullptr);
        Group::Fields fields = Group::query_parse(query);

        evbuffer *buffer;
        buffer = evbuffer_new();
        evbuffer_add_printf(buffer, "{\"groups\": []}");

        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "application/json");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_OK, nullptr, buffer);
        evbuffer_free(buffer);
    } catch (...) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        return;
    }
}

void new_account(evhttp_request *request, void *params) {
    evbuffer *buffer;
    buffer = evbuffer_new();
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evbuffer_add_printf(buffer, "{}");
    evhttp_send_reply(request, 201, nullptr, buffer);
    evbuffer_free(buffer);
}

void update_likes(evhttp_request *request, void *params) {
    evbuffer *buffer;
    buffer = evbuffer_new();
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evbuffer_add_printf(buffer, "{}");
    evhttp_send_reply(request, 202, nullptr, buffer);
    evbuffer_free(buffer);
}

void notfound(evhttp_request *request, void *params) {
    evbuffer *buffer;
    buffer = evbuffer_new();
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    const char* cur = request->uri;
    cur += 10;
    while (*cur != '/') {
        if ('0' <= *cur && *cur <= '9') {

        } else {
            evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
            return;
        }
        cur++;
    }
    cur++;
    if (*cur == '\0' || *cur == '?') {
        evbuffer_add_printf(buffer, "{}");
        evhttp_send_reply(request, 202, nullptr, buffer);
    } else if (strncmp(cur, "recommend", 9) == 0) {
        evbuffer_add_printf(buffer, "{\"accounts\": []}");
        evhttp_send_reply(request, HTTP_OK, nullptr, buffer);
    } else if (strncmp(cur, "suggest", 7) == 0) {
        evbuffer_add_printf(buffer, "{\"accounts\": []}");
        evhttp_send_reply(request, HTTP_OK, nullptr, buffer);
    } else {
        evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
    }
    evbuffer_free(buffer);
}

void start_server(char *host, uint16_t port) {
    event_base *ebase;
    evhttp *server;
    ebase = event_base_new();
    server = evhttp_new(ebase);
    evhttp_set_cb(server, "/accounts/filter/", filter, nullptr);
    evhttp_set_cb(server, "/accounts/group/", group, nullptr);
    evhttp_set_cb(server, "/accounts/new/", new_account, nullptr);
    evhttp_set_cb(server, "/accounts/likes/", update_likes, nullptr);
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
    t = chrono::system_clock::now();
    if (getenv("REUSE_UNZIP")[0] == '0') {
        unzip();
    }
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    parse_json(store, likes, strings);
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    build_indices(store, likes, ind);
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