#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <vector>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>

/*
remove blank spaces before/after the string
*/
std::string strip(std::string str) {
    int start = str.find_first_not_of(" \t\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    int end = str.find_last_not_of(" \t\r\f\v");
    return str.substr(start, end - start + 1);
}

#endif // STRINGUTILS_H