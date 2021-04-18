#ifndef HAW_HELPFUL_H
#define HAW_HELPFUL_H

#include <string>

namespace haw::logger {
struct Helpful {
    static std::string className(const std::string& sig);
    static std::string methodName(const std::string& sig);
};
}

#endif // HAW_HELPFUL_H
