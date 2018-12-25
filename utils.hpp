#pragma once

#include <fstream>

#include "types.hpp"

using namespace std;

year_t get_year(t timestamp) {
    time_t a = timestamp;
    tm utc_tm = *gmtime(&a);
    return utc_tm.tm_year + 1900;
}

void unzip() {
    char command[255];
    sprintf(command, "unzip -q -o %s", getenv("DATA_PATH"));
    system(command);
}

t read_time() {
    char options_path[255];
    sprintf(options_path, "%s", getenv("OPTIONS_PATH"));
    auto ifs = ifstream(options_path);
    t time;
    ifs >> time;
    return time;
}