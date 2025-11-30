// utils.h
#pragma once
#include <string>
#include <ctime>
using namespace std;


inline string now_string() {
    time_t t = time(nullptr);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return string(buf);
}
