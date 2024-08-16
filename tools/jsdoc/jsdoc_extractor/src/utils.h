#pragma once

#include <string>
#include <vector>
#include <algorithm>

template<typename T>
static bool contains(const std::vector<T>& vec, const T& val)
{
    auto it = std::find(vec.cbegin(), vec.cend(), val);
    return it != vec.cend();
}

static void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

static void split(const std::string& str, std::vector<std::string>& out, const std::string& delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    std::size_t delimLen = delim.length();

    while (current != std::string::npos) {
        out.push_back(str.substr(previous, current - previous));
        previous = current + delimLen;
        current = str.find(delim, previous);
    }
    out.push_back(str.substr(previous, current - previous));
}

static bool startsWith(const std::string& str, const std::string& start)
{
    if (str.size() < start.size()) {
        return false;
    }

    for (size_t i = 0; i < start.size(); ++i) {
        if (str.at(i) != start.at(i)) {
            return false;
        }
    }

    return true;
}
