/* grabbag - Convenience lib for various routines common to several tools
 * Copyright (C) 2013-2016  Xiph.org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "share/compat.h"

/*
 * FLAC needs to compile and work correctly on systems with a normal ISO C99
 * snprintf as well as Microsoft Visual Studio which has an non-standards
 * conformant snprint_s function.
 *
 * The important difference occurs when the resultant string (plus string
 * terminator) would have been longer than the supplied size parameter. When
 * this happens, ISO C's snprintf returns the length of resultant string, but
 * does not over-write the end of the buffer. MS's snprintf_s in this case
 * returns -1.
 *
 * The _MSC_VER code below attempts to modify the return code for vsnprintf_s
 * to something that is more compatible with the behaviour of the ISO C version.
 */

int
flac_snprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list va;
	int rc;

#if defined _MSC_VER
	if (size == 0)
		return 1024;
#endif

	va_start (va, fmt);

#if defined _MSC_VER
	rc = vsnprintf_s (str, size, _TRUNCATE, fmt, va);
	if (rc < 0)
		rc = size - 1;
#elif defined __MINGW32__
	rc = __mingw_vsnprintf (str, size, fmt, va);
#else
	rc = vsnprintf (str, size, fmt, va);
#endif
	va_end (va);

	return rc;
}

int
flac_vsnprintf(char *str, size_t size, const char *fmt, va_list va)
{
	int rc;

#if defined _MSC_VER
	if (size == 0)
		return 1024;
	rc = vsnprintf_s (str, size, _TRUNCATE, fmt, va);
	if (rc < 0)
		rc = size - 1;
#elif defined __MINGW32__
	rc = __mingw_vsnprintf (str, size, fmt, va);
#else
	rc = vsnprintf (str, size, fmt, va);
#endif

	return rc;
}
