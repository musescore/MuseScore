/////////////////////////////////////////////////////////////////////////////
// Name:        libmei.h
// Author:      Laurent Pugin
// Created:     2023
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_H__
#define __LIBMEI_H__

#include "meiconverter.h"

#include "framework/global/types/string.h"

#define DEFINITION_FACTOR 1

namespace libmei {
#define STRING_FORMAT_MAX_LEN 2048

/**
 * Implement the namespace level libmei function.
 */

inline void LogWarning(const char* fmt, ...)
{
    std::string str(STRING_FORMAT_MAX_LEN, 0);
    va_list args;
    va_start(args, fmt);
    vsnprintf(&str[0], STRING_FORMAT_MAX_LEN, fmt, args);
    va_end(args);
    str.resize(strlen(str.data()));
    mu::iex::mei::Convert::logs.push_back(muse::String::fromStdString(str));
}

/**
 * Implement the namespace level libmei function.
 */

inline std::string StringFormat(const char* fmt, ...)
{
    std::string str(STRING_FORMAT_MAX_LEN, 0);
    va_list args;
    va_start(args, fmt);
    vsnprintf(&str[0], STRING_FORMAT_MAX_LEN, fmt, args);
    va_end(args);
    str.resize(strlen(str.data()));
    return str;
}
} // namespace

#endif
