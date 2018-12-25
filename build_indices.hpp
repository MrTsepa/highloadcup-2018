#pragma once

#include "types.hpp"
#include "utils.hpp"

void build_indices(
        unordered_map<i, Account> &account_map,
        unordered_map<i, Like> &like_map,
        Index &index
) {
    const t CURRENT_TIME = read_time();
    for (const auto &pair : account_map) {
        const Account &account = pair.second;

        index.all.emplace(account.id);
        if (account.sex) {
            index.is_f.emplace(account.id);
        } else {
            index.is_m.emplace(account.id);
        }
        index.email_cmp[account.email] = account.id;
        index.birth_cmp[account.birth] = account.id;

        string domain = account.email.substr(account.email.find('@') + 1);
        index.email_domain_index[domain].emplace(account.id);

        index.status_indexes[account.status].emplace(account.id);

        if (!account.fname.empty()) {
            index.fname_index[account.fname].emplace(account.id);
        } else {
            index.fname_null.emplace(account.id);
        }
        if (!account.sname.empty()) {
            index.sname_index[account.sname].emplace(account.id);
        } else {
            index.sname_null.emplace(account.id);
        }
        if (!account.phone.empty()) {
            auto c1 = account.phone.find('(');
            auto c2 = account.phone.find(')');
            string code = account.phone.substr(c1 + 1, c2 - c1 - 1);
            index.phone_index[code].emplace(account.id);
        } else {
            index.phone_null.emplace(account.id);
        }
        if (!account.country.empty()) {
            index.country_index[account.country].emplace(account.id);
        } else {
            index.country_null.emplace(account.id);
        }
        if (!account.city.empty()) {
            index.city_index[account.city].emplace(account.id);
        } else {
            index.city_null.emplace(account.id);
        }
        year_t year = get_year(account.birth);
        index.year_index[year].emplace(account.id);

        for (const auto &interest : account.interests) {
            index.interests_index[interest].emplace(account.id);
        }
        if (account.has_premium) {
            if (account.premium_start <= CURRENT_TIME and account.premium_finish >= CURRENT_TIME) {
                index.has_active_premium.emplace(account.id);
            }
        } else {
            index.premium_null.emplace(account.id);
        }
        index.like_index[account.id] = unordered_set<uint>();
    }

    for (const auto &it : like_map) {
        index.like_index[it.second.to].emplace(it.first);
    }
}