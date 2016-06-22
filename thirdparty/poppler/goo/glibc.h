//========================================================================
//
// glibc.h
//
// Emulate various non-portable glibc functions.
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2016 Adrian Johnson <ajohnson@redneon.com>
//
//========================================================================

#ifndef GLIBC_H
#define GLIBC_H

#include "config.h"

#include <time.h>

extern "C" {

#ifndef HAVE_GMTIME_R
struct tm *gmtime_r(const time_t *timep, struct tm *result);
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r(const time_t *timep, struct tm *result);
#endif

#ifndef HAVE_TIMEGM
time_t timegm(struct tm *tm);
#endif

};

#endif // GLIBC_H

