#include <iostream>
#include <set>
#include <chrono>

#include <evhttp.h>

#include "types.hpp"
#include "utils.hpp"
#include "parse_json.hpp"
#include "build_indices.hpp"
#include "parse_query.hpp"
#include "merge_sets.hpp"

using namespace std;

unordered_map<i, Account> account_map;
unordered_map<i, Like> like_map;

Index ind;

vector<unordered_set<i> *> sets;
vector<unordered_set<i> *> neg_sets;
set<i> merge_result;

void filter(evhttp_request *request, void *params) {
    long limit = 0;
    const char *query = strchr(request->uri, '?') + 1;
    merge_result.clear();
    sets.clear();
    neg_sets.clear();
//    sets.emplace_back(&all);
    set<Field> fields = set<Field> {ID, EMAIL};
    if (filter_query_parse(
            query,
            ind,
            sets,
            neg_sets,
            &limit,
            fields
        ) != 0) {
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Content-Type", "text/plain");
        evhttp_add_header(evhttp_request_get_output_headers(request),
                          "Connection", "Keep-Alive");
        evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
        return;
    }
    merge_sets(sets, neg_sets, limit, merge_result);

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
                        account_field_value(account_map[*it], field).data()
                );
            } else {
                evbuffer_add_printf(
                        buffer,
                        R"("%s":"%s")",
                        field_string[field].data(),
                        account_field_value(account_map[*it], field).data()
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
    t = chrono::system_clock::now();
    unzip();
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
    t = chrono::system_clock::now();
    parse_json(account_map, like_map);
    cout << static_cast<chrono::duration<double>>(chrono::system_clock::now() - t).count() << endl;
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