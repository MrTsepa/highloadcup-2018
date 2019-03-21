#pragma once

#include "types.hpp"
#include "utils.hpp"

void build_indices(Store &store, Likes &likes, Index &index) {
    const t_t CURRENT_TIME = read_time();
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.id_index[account.id] = i;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.sex_index[account.sex][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.year_index[get_year(account.birth)][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        string domain = account.email.substr(account.email.find('@') + 1);
        index.domain_index[domain][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.has_premium) {
            if (account.premium_start <= CURRENT_TIME and account.premium_finish >= CURRENT_TIME) {
                index.has_active_premium[i] = true;
            }
        } else {
            index.premium_null[i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        for (unsigned char j : account.interests) {
            index.interests_index[j][i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.status_index[account.status][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.cities_index[account.city][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.countries_index[account.country][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        index.fname_index[account.fname][i] = true;
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.phone.empty()) {
            index.phone_null[i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (account.sname.empty()) {
            index.sname_null[i] = true;
        }
    }
    for (size_t i = 0; i < store.size(); i++) {
        Account &account = store[i];
        if (!account.phone.empty()) {
            auto c1 = account.phone.find('(');
            auto c2 = account.phone.find(')');
            string code = account.phone.substr(c1 + 1, c2 - c1 - 1);
            index.code_index[code][i] = true;
        }
    }
    for (size_t i = 0; i < likes.size(); i++) {
        Like &like = likes[i];
        index.like_index[like.to].emplace(like.from);
    }
}
