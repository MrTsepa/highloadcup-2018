#pragma once

#include "types.hpp"
#include "utils.hpp"

void build_indices(
        unordered_map<i, Account> &account_map,
        unordered_map<i, unordered_map<i, t> > &like_map,
        Index &index
) {
    const t CURRENT_TIME = read_time();
    for (auto &pair : account_map) {
        Account &account = pair.second;

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
            trie_insert(index.sname_prefix_trie, account.sname, 0, account.id);
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
    }

    size_t j = 0;
    for (auto &item : index.email_cmp) {
        index.email_cmp_split[j / SPLIT_LEN].emplace_no_sset(item.second);
        j++;
    }

    j = 0;
    for (auto &item : index.birth_cmp) {
        index.birth_cmp_split[j / SPLIT_LEN].emplace_no_sset(item.second);
        j++;
    }

    for (auto &item : like_map) {
        for (auto &item1 : item.second) {
            index.like_index[item1.first].emplace(item.first);
        }
    }
}