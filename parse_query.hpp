#pragma once

#include <vector>
#include <unordered_map>
#include <set>

#include "types.hpp"

using namespace std;

unordered_set<i> email_lt_set;
unordered_set<i> email_gt_set;
unordered_set<i> birth_lt_set;
unordered_set<i> birth_gt_set;
unordered_set<i> fname_any_set;
unordered_set<i> city_any_set;
unordered_set<i> interests_any_set;


int filter_query_parse(
        const char *query,
        Index &index,
        vector<unordered_set<i> *> &sets,
        vector<unordered_set<i> *> &neg_sets,
        long *limit,
        set<Field> &fields
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
                                sets.emplace_back(val[0] == 'm' ? &index.is_m : &index.is_f);
                                break;
                            default:
                                return -1;
                        }
                        break;
                    case EMAIL: {
                        switch (pred) {
                            case DOMAIN_: {
                                sets.emplace_back(&index.email_domain_index[val]);
                                break;
                            }
                            case LT: {
                                auto lower = index.email_cmp.lower_bound(val);
                                email_lt_set.clear();
                                for (auto it = index.email_cmp.begin(); it != lower; it++) {
                                    email_lt_set.emplace(it->second);
                                }
                                sets.emplace_back(&email_lt_set);
                                break;
                            }
                            case GT: {
                                auto upper = index.email_cmp.upper_bound(val);
                                email_gt_set.clear();
                                for (auto it = upper; it != index.email_cmp.end(); it++) {
                                    email_gt_set.emplace(it->second);
                                }
                                sets.emplace_back(&email_gt_set);
                                break;
                            }
                            default:
                                return -1;
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
                                sets.emplace_back(&index.status_indexes[status]);
                                break;
                            }
                            case NEQ: {
                                neg_sets.emplace_back(&index.status_indexes[status]);
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case FNAME: {
                        switch (pred) {
                            case EQ: {
                                sets.emplace_back(&index.fname_index[val]);
                                break;
                            }
                            case ANY: {
                                fname_any_set.clear();
                                size_t start = 0;
                                while (true) {
                                    auto at = val.find(',', start);
                                    const unordered_set<i>* s = &index.fname_index[val.substr(start, at - start)];
                                    fname_any_set.insert(s->begin(), s->end());
                                    start = at + 1;
                                    if (at == string::npos) break;
                                }
                                sets.emplace_back(&fname_any_set);
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.fname_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.fname_null);
                                } else return -1;
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case SNAME: {
                        switch (pred) {
                            case EQ: {
                                sets.emplace_back(&index.sname_index[val]);
                                break;
                            }
                            case STARTS: {
                                // TODO !!!
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.sname_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.sname_null);
                                } else return -1;
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case PHONE: {
                        switch (pred) {
                            case CODE: {
                                sets.emplace_back(&index.phone_index[val]);
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.phone_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.phone_null);
                                } else return -1;
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case COUNTRY: {
                        switch (pred) {
                            case EQ: {
                                auto it = index.country_index.find(val);
                                if (it != index.country_index.end()) {
                                    sets.emplace_back(&it->second);
                                }
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.country_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.country_null);
                                } else return -1;
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case CITY: {
                        switch (pred) {
                            case EQ: {
                                auto it = index.city_index.find(val);
                                if (it != index.city_index.end()) {
                                    sets.emplace_back(&it->second);
                                }
                                break;
                            }
                            case ANY: {
                                city_any_set.clear();
                                size_t start = 0;
                                while (true) {
                                    auto at = val.find(',', start);
                                    const unordered_set<i>* s = &index.city_index[val.substr(start, at - start)];
                                    city_any_set.insert(s->begin(), s->end());
                                    start = at + 1;
                                    if (at == string::npos) break;
                                }
                                sets.emplace_back(&city_any_set);
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.city_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.city_null);
                                } else return -1;
                                break;
                            }
                            default:
                                return -1;
                        }
                        break;
                    }
                    case BIRTH: {
                        char *err;
                        const long long_val = strtol(val.c_str(), &err, 10);
                        if (*err != 0) {
                            return -1;
                        }
                        switch (pred) {
                            case LT: {
                                auto lower = index.birth_cmp.lower_bound(long_val);
                                birth_lt_set.clear();
                                for (auto it = index.birth_cmp.begin(); it != lower; it++) {
                                    birth_lt_set.emplace(it->second);
                                }
                                sets.emplace_back(&birth_lt_set);
                                break;
                            }
                            case GT: {
                                auto upper = index.birth_cmp.upper_bound(long_val);
                                birth_gt_set.clear();
                                for (auto it = upper; it != index.birth_cmp.end(); it++) {
                                    birth_gt_set.emplace(it->second);
                                }
                                sets.emplace_back(&birth_gt_set);
                                break;
                            }
                            case YEAR: {
                                sets.emplace_back(&index.year_index[long_val]);
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case INTERESTS: {
                        switch (pred) {
                            case CONTAINS: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    sets.emplace_back(&index.interests_index[val.substr(start, at - start)]);
                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            case ANY: {
                                interests_any_set.clear();
                                size_t start = 0;
                                while (true) {
                                    auto at = val.find(',', start);
                                    const unordered_set<i>* s = &index.interests_index[val.substr(start, at - start)];
                                    interests_any_set.insert(s->begin(), s->end());
                                    start = at + 1;
                                    if (at == string::npos) break;
                                }
                                sets.emplace_back(&interests_any_set);
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case LIKES: {
                        switch (pred) {
                            case CONTAINS: {
                                size_t start = 0;
                                auto at = val.find(',');
                                while (at != string::npos) {
                                    char *err;
                                    const long like = strtol(val.substr(start, at - start).c_str(), &err, 10);
                                    if (*err != 0) {
                                        return -1;
                                    }
                                    sets.emplace_back(&index.like_index[like]);
                                    start = at + 1;
                                    at = val.find(',', start);
                                }
                                break;
                            }
                            default: return -1;
                        }
                        break;
                    }
                    case PREMIUM: {
                        switch (pred) {
                            case NOW: {
                                if (val == "1") {
                                    sets.emplace_back(&index.has_active_premium);
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.has_active_premium);
                                } else return -1;
                                break;
                            }
                            case NULL_: {
                                if (val == "1") {
                                    sets.emplace_back(&index.premium_null);
                                    fields.erase(fields.find(field));
                                } else if (val == "0") {
                                    neg_sets.emplace_back(&index.premium_null);
                                } else return -1;
                                break;
                            }
                            default: return -1;
                        }
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
                }
                state = FIELD;
                break;
            }
        }
    }
    return 0;
}