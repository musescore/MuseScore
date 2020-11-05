#include "helpful.h"

using namespace haw::logger;

static const std::string Colon("::");
static const char ArgBegin('(');
static const char Space(' ');

std::string Helpful::className(const std::string& sig)
{
    std::size_t endFunc = sig.find_first_of(ArgBegin);
    if (endFunc == std::string::npos) {
        return sig;
    }

    std::size_t beginFunc = sig.find_last_of(Colon, endFunc);
    if (beginFunc == std::string::npos) {
        return std::string();
    }

    std::size_t beginClassColon = sig.find_last_of(Colon, beginFunc - 2);
    std::size_t beginClassSpace = sig.find_last_of(Space, beginFunc - 2);

    std::size_t beginClass = std::string::npos;
    if (beginClassColon == std::string::npos) {
        beginClass = beginClassSpace;
    } else if (beginClassSpace == std::string::npos) {
        beginClass = beginClassColon;
    } else {
        beginClass = std::max(beginClassColon, beginClassSpace);
    }

    if (beginClass == std::string::npos) {
        beginClass = 0;
    } else {
        beginClass += 1;
    }

    return sig.substr(beginClass, (beginFunc - 1 - beginClass));
}

std::string Helpful::methodName(const std::string& sig)
{
    std::size_t endFunc = sig.find_first_of(ArgBegin);
    if (endFunc == std::string::npos) {
        return sig;
    }

    std::size_t beginFunc = sig.find_last_of(Colon, endFunc);
    if (beginFunc == std::string::npos) {
        beginFunc = sig.find_last_of(Space, endFunc);
    }

    if (beginFunc == std::string::npos) {
        beginFunc = 0;
    } else {
        beginFunc += 1;
    }

    return sig.substr(beginFunc, (endFunc - beginFunc));
}
