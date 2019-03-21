#pragma once

#include <vector>
#include <unordered_map>
#include <set>

#include "types.hpp"

using namespace std;


namespace Group {
    typedef map<Field, string> Fields;

    Fields query_parse(const char *query) {
        Fields result;
        const char *cur = query;
        string val;
        enum State {
            FIELD, VALUE
        };
        State state = FIELD;
        Field field;
        while (state != FIELD or cur[0] != '\0') {
            switch (state) {
                case FIELD: {
                    if (strncmp(cur, "order", 5) == 0) {
                        field = ORDER;
                        cur += 6;
                    } else if (strncmp(cur, "sex", 3) == 0) {
                        field = SEX;
                        cur += 4;
                    } else if (strncmp(cur, "keys", 4) == 0) {
                        field = KEYS;
                        cur += 5;
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
                        break;
                    } else if (strncmp(cur, "limit", 5) == 0) {
                        field = LIMIT;
                        cur += 6;
                        break;
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
                    result[field] = val;
                    state = FIELD;
                    break;
                }
            }
        }
        return result;
    }
}