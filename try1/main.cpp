#include <iostream>
#include <set>
#include <chrono>

#include <evhttp.h>

#include "types.hpp"
#include "utils.hpp"
#include "parse_json.hpp"
#include "build_indices.hpp"
#include "filter_query_parse.hpp"
#include "group_query_parse.hpp"
#include "merge_sets.hpp"

using namespace std;

unordered_map<i, Account> account_map;
unordered_map<i, unordered_map<i, t> > like_map;

Index ind;

vector<IndexSet<i> *> sets;
vector<IndexSet<i> *> neg_sets;
vector<vector<IndexSet<i> *> *> any_sets;
set<i> merge_result;
unordered_set<i> intersection_result;

set<pair<size_t, string>> groups;
vector<pair<unordered_set<i> *, string>> aaa;

void group(evhttp_request *request, void *params) {
    const char *query = evhttp_uridecode(strchr(request->uri, '?') + 1, 1, nullptr);
    long limit = 0;
    bool order = true;
    set<Field> fields;
    vector<Field> keys;
    sets.emplace_back(&ind.all);
    if (group_query_parse(
            query,
            ind,
            sets,
            &limit,
            &order,
            fields,
            keys
        ) != 0) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        sets.clear();
        return;
    }

    intersection(sets, intersection_result);

    evbuffer *buffer;
    buffer = evbuffer_new();
    evbuffer_add_printf(buffer, "{\"groups\": [");
    if (keys.size() == 1) {
        switch (keys[0]) {
            case SEX: {
                size_t m = intersection_size(intersection_result, ind.is_m.uset);
                size_t f = intersection_result.size() - m;
                groups.emplace(m, "\"sex\":\"m\",");
                groups.emplace(f, "\"sex\":\"f\",");
                break;
            }
            case STATUS: {
                size_t s0 = intersection_size(intersection_result, ind.status_indices[0].uset);
                size_t s1 = intersection_size(intersection_result, ind.status_indices[1].uset);
                size_t s2 = intersection_result.size() - s0 - s1;
                groups.emplace(s0, "\"status\":\"свободны\",");
                groups.emplace(s1, "\"status\":\"всё сложно\",");
                groups.emplace(s2, "\"status\":\"заняты\",");
                break;
            }
            case COUNTRY: {
                for (auto &item : ind.country_index) {
                    size_t size = intersection_size(intersection_result, item.second.uset);
                    if (size != 0) {
                        groups.emplace(size, "\"country\":\"" + item.first + "\",");
                    }
                }
                groups.emplace(intersection_size(intersection_result, ind.country_null.uset), "");
                break;
            }
            case CITY: {
                for (auto &item : ind.city_index) {
                    size_t size = intersection_size(intersection_result, item.second.uset);
                    if (size != 0) {
                        groups.emplace(size, "\"city\":\"" + item.first + "\",");
                    }
                }
                groups.emplace(intersection_size(intersection_result, ind.city_null.uset), "");
                break;
            }
            case INTERESTS: {
                for (auto &item : ind.interests_index) {
                    size_t size = intersection_size(intersection_result, item.second.uset);
                    if (size != 0) {
                        groups.emplace(size, "\"interests\":\"" + item.first + "\",");
                    }
                }
                break;
            }
        }
    } else {
        switch (keys[0]) {
            case SEX: {
                aaa.emplace_back(&ind.is_m.uset, "\"sex\":\"m\",");
                aaa.emplace_back(&ind.is_f.uset, "\"sex\":\"f\",");
                break;
            }
            case STATUS: {
                aaa.emplace_back(&ind.status_indices[0].uset, "\"status\":\"свободны\",");
                aaa.emplace_back(&ind.status_indices[1].uset, "\"status\":\"всё сложно\",");
                aaa.emplace_back(&ind.status_indices[2].uset, "\"status\":\"заняты\",");
                break;
            }
            case COUNTRY: {
                for (auto &item : ind.country_index) {
                    aaa.emplace_back(&item.second.uset, "\"country\":\"" + item.first + "\",");
                }
                aaa.emplace_back(&ind.country_null.uset, "");
                break;
            }
            case CITY: {
                for (auto &item : ind.city_index) {
                    aaa.emplace_back(&item.second.uset, "\"city\":\"" + item.first + "\",");
                }
                aaa.emplace_back(&ind.city_null.uset, "");
                break;
            }
            case INTERESTS: {
                for (auto &item : ind.interests_index) {
                    aaa.emplace_back(&item.second.uset, "\"country\":\"" + item.first + "\",");
                }
                break;
            }
            default:
                return;
        }
        for (auto &aitem : aaa) {
            switch (keys[1]) {
                case SEX: {
                    size_t m = intersection_size3(intersection_result, ind.is_m.uset, *aitem.first);
                    size_t f = intersection_result.size() - m;
                    groups.emplace(m, aitem.second + "\"sex\":\"m\",");
                    groups.emplace(f, aitem.second + "\"sex\":\"f\",");
                    break;
                }
                case STATUS: {
                    size_t s0 = intersection_size3(intersection_result, ind.status_indices[0].uset, *aitem.first);
                    size_t s1 = intersection_size3(intersection_result, ind.status_indices[1].uset, *aitem.first);
                    size_t s2 = intersection_result.size() - s0 - s1;
                    groups.emplace(s0, aitem.second + "\"status\":\"свободны\",");
                    groups.emplace(s1, aitem.second + "\"status\":\"всё сложно\",");
                    groups.emplace(s2, aitem.second + "\"status\":\"заняты\",");
                    break;
                }
                case COUNTRY: {
                    for (auto &item : ind.country_index) {
                        size_t size = intersection_size3(intersection_result, item.second.uset, *aitem.first);
                        if (size != 0) {
                            groups.emplace(size, aitem.second + "\"country\":\"" + item.first + "\",");
                        }
                    }
                    groups.emplace(intersection_size3(intersection_result, ind.country_null.uset, *aitem.first), "");
                    break;
                }
                case CITY: {
                    for (auto &item : ind.city_index) {
                        size_t size = intersection_size3(intersection_result, item.second.uset, *aitem.first);
                        if (size != 0) {
                            groups.emplace(size, aitem.second + "\"city\":\"" + item.first + "\",");
                        }
                    }
                    groups.emplace(intersection_size3(intersection_result, ind.city_null.uset, *aitem.first), "");
                    break;
                }
                case INTERESTS: {
                    for (auto &item : ind.interests_index) {
                        size_t size = intersection_size3(intersection_result, item.second.uset, *aitem.first);
                        if (size != 0) {
                            groups.emplace(size, aitem.second + "\"interests\":\"" + item.first + "\",");
                        }
                    }
                    break;
                }
                default: return;
            }
        }
    }
    if (order) {
        size_t k = 0;
        for (auto it = groups.begin(); it != groups.end(); it++) {
            if (k == limit) {
                break;
            }
            if (it->first == 0) {
                continue;
            }
            if (k > 0) {
                evbuffer_add_printf(buffer, ",");
            }
            evbuffer_add_printf(
                    buffer,
                    "{%s\"count\":%zu}",
                    it->second.c_str(),
                    it->first
            );
            k++;
        }
    } else {
        size_t k = 0;
        for (auto it = groups.rbegin(); it != groups.rend(); it++) {
            if (k == limit) {
                break;
            }
            if (it->first == 0) {
                break;
            }
            if (k > 0) {
                evbuffer_add_printf(buffer, ",");
            }
            evbuffer_add_printf(
                    buffer,
                    "{%s\"count\":%zu}",
                    it->second.c_str(),
                    it->first
            );
            k++;
        }
    }
    evbuffer_add_printf(buffer, "]}");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Content-Type", "application/json");
    evhttp_add_header(evhttp_request_get_output_headers(request),
                      "Connection", "Keep-Alive");
    evhttp_send_reply(request, HTTP_OK, "OK", buffer);
    evbuffer_free(buffer);
    sets.clear();
    intersection_result.clear();
    groups.clear();
    aaa.clear();
}

void filter(evhttp_request *request, void *params) {
    long limit = 0;
    const char *query = evhttp_uridecode(strchr(request->uri, '?') + 1, 1, nullptr);
    sets.emplace_back(&ind.all);
    set<Field> fields = set<Field> {ID, EMAIL};
    if (filter_query_parse(
            query,
            ind,
            sets,
            neg_sets,
            any_sets,
            &limit,
            fields
        ) != 0) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        sets.clear();
        neg_sets.clear();
        any_sets.clear();
        return;
    }
    merge_sets(sets, neg_sets, any_sets, limit, merge_result);

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
                            account_field_value(account_map[*it], field).data()
                    );
                    break;
                }
                default: {
                    evbuffer_add_printf(
                            buffer,
                            R"("%s":"%s")",
                            field_string[field].data(),
                            account_field_value(account_map[*it], field).data()
                    );
                }
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
    merge_result.clear();
    sets.clear();
    neg_sets.clear();
    any_sets.clear();
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
    evhttp_set_cb(server, "/accounts/group/", group, nullptr);
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
    parse_json(account_map, like_map);
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    cout << account_map.size() << ' ' << like_map.size() << endl;
    t = chrono::system_clock::now();
    build_indices(account_map, like_map, ind);
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