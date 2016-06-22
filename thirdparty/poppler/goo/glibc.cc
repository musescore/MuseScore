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

#include "glibc.h"

#ifndef HAVE_GMTIME_R
struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
  struct tm *gt;
  gt = gmtime(timep);
  if (gt)
    *result = *gt;
  return gt;
}
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
  struct tm *lt;
  lt = localtime(timep);
  *result = *lt;
  return lt;
}
#endif

#ifndef HAVE_TIMEGM
// Get offset of local time from UTC in seconds. DST is ignored.
static time_t getLocalTimeZoneOffset()
{
  time_t utc, local;
  struct tm tm_utc;
  time (&utc);
  gmtime_r(&utc, &tm_utc);
  local = mktime(&tm_utc);
  return difftime(utc, local);
}

time_t timegm(struct tm *tm)
{
  tm->tm_isdst = 0;
  time_t t = mktime(tm);
  if (t == -1)
    return t;

  t += getLocalTimeZoneOffset();
  return t;
}
#endif
