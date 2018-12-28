#pragma once

#include <vector>

#include "types.hpp"

using namespace std;



int group_query_parse(
        const char *query,
        Index &index,
        vector<IndexSet<i> *> &sets,
        long *limit,
        bool *order,
        set<Field> &fields,
        vector<Field> &keys
) {
    enum State {
        FIELD, VALUE, DONE
    };

    const char *cur = query;
    string val;
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
                } else if (strncmp(cur, "status", 6) == 0) {
                    field = STATUS;
                    cur += 7;
                } else if (strncmp(cur, "joined", 6) == 0) {
                    field = JOINED;
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
                    case KEYS: {
                        size_t start = 0;
                        while (true) {
                            auto at = val.find(',', start);
                            string key = val.substr(start, at - start);
                            if (key == "sex") keys.emplace_back(SEX);
                            else if (key == "interests") keys.emplace_back(INTERESTS);
                            else if (key == "status") keys.emplace_back(STATUS);
                            else if (key == "country") keys.emplace_back(COUNTRY);
                            else if (key == "city") keys.emplace_back(CITY);
                            else return -1;
                            start = at + 1;
                            if (at == string::npos) break;
                        }
                        break;
                    }
                    case SEX: {
                        sets.emplace_back(val[0] == 'm' ? &index.is_m : &index.is_f);
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
                        sets.emplace_back(&index.status_indexes[status]);
                        break;
                    }
                    case FNAME: {
                        sets.emplace_back(&index.fname_index[val]);
                        break;
                    }
                    case SNAME: {
                        sets.emplace_back(&index.sname_index[val]);
                        break;
                    }
                    case COUNTRY: {
                        sets.emplace_back(&index.country_index[val]);
                        break;
                    }
                    case CITY: {
                        sets.emplace_back(&index.city_index[val]);
                        break;
                    }
                    case BIRTH: {
                        char *err;
                        const long long_val = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        }
                        sets.emplace_back(&index.year_index[long_val]);
                        break;
                    }
                    case JOINED: {
                        char *err;
                        const long long_val = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        }
                        sets.emplace_back(&index.joined_year_index[long_val]);
                        break;
                    }
                    case INTERESTS: {
                        sets.emplace_back(&index.interests_index[val]);
                        break;
                    }
                    case LIKES: {
                        char *err;
                        long long_val = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        }
                        sets.emplace_back(&index.like_index[long_val]);
                        break;
                    }
                    case LIMIT: {
                        char *err;
                        long converted = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        } else {
                            *limit = converted;
                        }
                        break;
                    }
                    case ORDER: {
                        if (val == "1") {
                            *order = true;
                        } else if (val == "-1") {
                            *order = false;
                        } else {
                            return -1;
                        }
                        break;
                    }
                    case QUERY_ID: {
                        break;
                    }
                    default:
                        return -1;
                }
                state = FIELD;
                break;
            }
        }
    }
    return 0;
}

