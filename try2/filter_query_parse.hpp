#pragma once

#include <vector>
#include <unordered_map>
#include <set>

#include "types.hpp"

using namespace std;

enum State {
    FIELD, PRED, VALUE
};

typedef map<Field, pair<Pred, string>> Fields;

Fields filter_query_parse(const char *query) {
    Fields result;

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
                } else if (strncmp(cur, "phone", 5) == 0) {
                    field = PHONE;
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
                    throw -1;
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
                    throw -1;
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
                result[field] = make_pair(pred, val);
                state = FIELD;
                break;
            }
        }
    }
    return result;
}