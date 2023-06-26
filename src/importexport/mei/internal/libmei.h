/////////////////////////////////////////////////////////////////////////////
// Name:        att.h
// Author:      Laurent Pugin
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_H__
#define __LIBMEI_H__

#define DEFINITION_FACTOR 10

#include "meiconverter.h"

namespace libmei {

#define STRING_FORMAT_MAX_LEN 2048

void LogWarning(const char *fmt, ...)
{
    std::string str(STRING_FORMAT_MAX_LEN, 0);
    va_list args;
    va_start(args, fmt);
    vsnprintf(&str[0], STRING_FORMAT_MAX_LEN, fmt, args);
    va_end(args);
    str.resize(strlen(str.data()));
    mu::iex::mei::Convert::logs.push_back(QString::fromStdString(str));
}

std::string StringFormat(const char *fmt, ...)
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
