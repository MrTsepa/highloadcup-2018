#pragma once

#include <fstream>

#include "types.hpp"

using namespace std;

year_t get_year(t_t timestamp) {
    time_t a = timestamp;
    tm utc_tm = *gmtime(&a);
    return utc_tm.tm_year + 1900;
}

void unzip() {
    char command[255];
    sprintf(command, "unzip -q -o %s", getenv("DATA_PATH"));
    system(command);
}

t_t read_time() {
    char options_path[255];
    sprintf(options_path, "%s", getenv("OPTIONS_PATH"));
    auto ifs = ifstream(options_path);
    t_t time;
    ifs >> time;
    return time;
}

bool convert_sex(const char c) {
    return c == 'f';
}

unsigned char convert_status(const char* s) {
    unsigned char status = 0; // свободны
    if (s[0] != -47) {
        if (s[1] == -78) { // все сложно
            status = 1;
        } else { // заняты
            status = 2;
        }
    }
    return status;
}

unsigned char convert_status(string &s) {
    unsigned char status = 0; // свободны
    if (s[0] != -47) {
        if (s[1] == -78) { // все сложно
            status = 1;
        } else { // заняты
            status = 2;
        }
    }
    return status;
}