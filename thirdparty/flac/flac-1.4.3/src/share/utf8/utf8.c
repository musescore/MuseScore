/*
 * Copyright (C) 2001 Peter Harris <peter.harris@hummingbird.com>
 * Copyright (C) 2001 Edmund Grimley Evans <edmundo@rano.org>
 *
 * Buffer overflow checking added: Josh Coalson, 9/9/2007
 *
 * Win32 part rewritten: lvqcl, 2/2/2016
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Convert a string between UTF-8 and the locale's charset.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "share/alloc.h"
#include "share/utf8.h"

#ifdef _WIN32

#include <windows.h>

int utf8_encode(const char *from, char **to)
{
	wchar_t *unicode = NULL;
	char *utf8 = NULL;
	int ret = -1;

	do {
		int len;

		len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from, -1, NULL, 0);
		if(len == 0) break;
		unicode = (wchar_t*) safe_malloc_mul_2op_((size_t)len, sizeof(wchar_t));
		if(unicode == NULL) break;
		len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, from, -1, unicode, len);
		if(len == 0) break;

		len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
		if(len == 0) break;
		utf8 = (char*) safe_malloc_mul_2op_((size_t)len, sizeof(char));
		if(utf8 == NULL) break;
		len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, len, NULL, NULL);
		if(len == 0) break;

		ret = 0;

	} while(0);

	free(unicode);

	if(ret == 0) {
		*to = utf8;
	} else {
		free(utf8);
		*to = NULL;
	}

	return ret;
}

int utf8_decode(const char *from, char **to)
{
	wchar_t *unicode = NULL;
	char *acp = NULL;
	int ret = -1;

	do {
		int len;

		len = MultiByteToWideChar(CP_UTF8, 0, from, -1, NULL, 0);
		if(len == 0) break;
		unicode = (wchar_t*) safe_malloc_mul_2op_((size_t)len, sizeof(wchar_t));
		if(unicode == NULL) break;
		len = MultiByteToWideChar(CP_UTF8, 0, from, -1, unicode, len);
		if(len == 0) break;

		len = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, NULL, 0, NULL, NULL);
		if(len == 0) break;
		acp = (char*) safe_malloc_mul_2op_((size_t)len, sizeof(char));
		if(acp == NULL) break;
		len = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, acp, len, NULL, NULL);
		if(len == 0) break;

		ret = 0;

	} while(0);

	free(unicode);

	if(ret == 0) {
		*to = acp;
	} else {
		free(acp);
		*to = NULL;
	}

	return ret;
}

#else /* End win32. Rest is for real operating systems */


#ifdef HAVE_LANGINFO_CODESET
#include <langinfo.h>
#endif

#include <string.h>

#include "share/safe_str.h"
#include "iconvert.h"
#include "charset.h"

static const char *current_charset(void)
{
  const char *c = 0;
#ifdef HAVE_LANGINFO_CODESET
  c = nl_langinfo(CODESET);
#endif

  if (!c)
    c = getenv("CHARSET");

  return c? c : "US-ASCII";
}

static int convert_buffer(const char *fromcode, const char *tocode,
			  const char *from, size_t fromlen,
			  char **to, size_t *tolen)
{
  int ret = -1;

#ifdef HAVE_ICONV
  ret = iconvert(fromcode, tocode, from, fromlen, to, tolen);
  if (ret != -1)
    return ret;
#endif

#ifndef HAVE_ICONV /* should be ifdef USE_CHARSET_CONVERT */
  ret = charset_convert(fromcode, tocode, from, fromlen, to, tolen);
  if (ret != -1)
    return ret;
#endif

  return ret;
}

static int convert_string(const char *fromcode, const char *tocode,
			  const char *from, char **to, char replace)
{
  int ret;
  size_t fromlen;
  char *s;

  fromlen = strlen(from);
  ret = convert_buffer(fromcode, tocode, from, fromlen, to, 0);
  if (ret == -2)
    return -1;
  if (ret != -1)
    return ret;

  s = safe_malloc_add_2op_(fromlen, /*+*/1);
  if (!s)
    return -1;
  snprintf(s, fromlen + 1, "%s", from);
  *to = s;
  for (; *s; s++)
    if (*s & ~0x7f)
      *s = replace;
  return 3;
}

int utf8_encode(const char *from, char **to)
{
  return convert_string(current_charset(), "UTF-8", from, to, '#');
}

int utf8_decode(const char *from, char **to)
{
  return convert_string("UTF-8", current_charset(), from, to, '?');
}

#endif
