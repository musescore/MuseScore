/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2013-2023  Xiph.Org Foundation
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

#include <io.h>
#include <windows.h>
#include "share/win_utf8_io.h"

#define UTF8_BUFFER_SIZE 32768

/* detect whether it is Windows APP (UWP) or standard Win32 envionment */
#ifdef WINAPI_FAMILY_PARTITION
	#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
		#define FLAC_WINDOWS_APP 1
	#else
		#define FLAC_WINDOWS_APP 0
	#endif
#else
	#define FLAC_WINDOWS_APP 0
#endif

static int local_vsnprintf(char *str, size_t size, const char *fmt, va_list va)
{
	int rc;

#if defined _MSC_VER
	if (size == 0)
		return 1024;
	rc = vsnprintf_s(str, size, _TRUNCATE, fmt, va);
	if (rc < 0)
		rc = size - 1;
#elif defined __MINGW32__
	rc = __mingw_vsnprintf(str, size, fmt, va);
#else
	rc = vsnprintf(str, size, fmt, va);
#endif

	return rc;
}

/* convert WCHAR stored Unicode string to UTF-8. Caller is responsible for freeing memory */
static char *utf8_from_wchar(const wchar_t *wstr)
{
	char *utf8str;
	int len;

	if (!wstr)
		return NULL;
	if ((len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL)) == 0)
		return NULL;
	if ((utf8str = (char *)malloc(len)) == NULL)
		return NULL;
	if (WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8str, len, NULL, NULL) == 0) {
		free(utf8str);
		utf8str = NULL;
	}

	return utf8str;
}

/* convert UTF-8 back to WCHAR. Caller is responsible for freeing memory */
static wchar_t *wchar_from_utf8(const char *str)
{
	wchar_t *widestr;
	int len;

	if (!str)
		return NULL;
	if ((len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0)) == 0)
		return NULL;
	if ((widestr = (wchar_t *)malloc(len*sizeof(wchar_t))) == NULL)
		return NULL;
	if (MultiByteToWideChar(CP_UTF8, 0, str, -1, widestr, len) == 0) {
		free(widestr);
		widestr = NULL;
	}

	return widestr;
}

/* retrieve WCHAR commandline, expand wildcards and convert everything to UTF-8 */
int get_utf8_argv(int *argc, char ***argv)
{
#if !FLAC_WINDOWS_APP
	typedef int (__cdecl *wgetmainargs_t)(int*, wchar_t***, wchar_t***, int, int*);
	wgetmainargs_t wgetmainargs;
	HMODULE handle;
#endif // !FLAC_WINDOWS_APP
	int wargc;
	wchar_t **wargv;
	wchar_t **wenv;
	char **utf8argv;
	int ret, i;

#if FLAC_WINDOWS_APP
	wargc = __argc;
	wargv = __wargv;
	wenv = _wenviron;
#else // !FLAC_WINDOWS_APP
	if ((handle = LoadLibraryW(L"msvcrt.dll")) == NULL) return 1;
	if ((wgetmainargs = (wgetmainargs_t)GetProcAddress(handle, "__wgetmainargs")) == NULL) {
		FreeLibrary(handle);
		return 1;
	}
	i = 0;
	/* when the 4th argument is 1,  __wgetmainargs expands wildcards but also erroneously converts \\?\c:\path\to\file.flac to \\file.flac */
	if (wgetmainargs(&wargc, &wargv, &wenv, 1, &i) != 0) {
		FreeLibrary(handle);
		return 1;
	}
#endif // !FLAC_WINDOWS_APP
	if ((utf8argv = (char **)calloc(wargc, sizeof(char*))) == NULL) {
	#if !FLAC_WINDOWS_APP
		FreeLibrary(handle);
	#endif // !FLAC_WINDOWS_APP
		return 1;
	}

	ret = 0;
	for (i=0; i<wargc; i++) {
		if ((utf8argv[i] = utf8_from_wchar(wargv[i])) == NULL) {
			ret = 1;
			break;
		}
	}

#if !FLAC_WINDOWS_APP
	FreeLibrary(handle); /* do not free it when wargv or wenv are still in use */
#endif // !FLAC_WINDOWS_APP

	if (ret == 0) {
		*argc = wargc;
		*argv = utf8argv;
	} else {
		for (i=0; i<wargc; i++)
			free(utf8argv[i]);
		free(utf8argv);
	}

	return ret;
}

/* similar to CreateFileW but accepts UTF-8 encoded lpFileName */
HANDLE WINAPI CreateFile_utf8(const char *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	wchar_t *wname;
	HANDLE handle = INVALID_HANDLE_VALUE;

	if ((wname = wchar_from_utf8(lpFileName)) != NULL) {
#if !FLAC_WINDOWS_APP
		handle = CreateFileW(wname, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
#else // FLAC_WINDOWS_APP
		CREATEFILE2_EXTENDED_PARAMETERS params;
		params.dwSize = sizeof(params);
		params.dwFileAttributes = dwFlagsAndAttributes & 0xFFFF;
		params.dwFileFlags = dwFlagsAndAttributes & 0xFFF00000;
		params.dwSecurityQosFlags = dwFlagsAndAttributes & 0x000F0000;
		params.lpSecurityAttributes = lpSecurityAttributes;
		params.hTemplateFile = hTemplateFile;
		handle = CreateFile2(wname, dwDesiredAccess, dwShareMode, dwCreationDisposition, &params);
#endif // FLAC_WINDOWS_APP
		free(wname);
	}

	return handle;
}

/* return number of characters in the UTF-8 string */
size_t strlen_utf8(const char *str)
{
	size_t len;
	len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0); /* includes terminating null */
	if (len != 0)
		return len-1;
	else
		return strlen(str);
}

/* get the console width in characters */
int win_get_console_width(void)
{
	int width = 80;
#if !FLAC_WINDOWS_APP
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(hOut != INVALID_HANDLE_VALUE && hOut != NULL)
		if (GetConsoleScreenBufferInfo(hOut, &csbi) != 0)
			width = csbi.dwSize.X;
#endif // !FLAC_WINDOWS_APP
	return width;
}

/* print functions */

#if !FLAC_WINDOWS_APP
static int wprint_console(FILE *stream, const wchar_t *text, size_t len)
{
	DWORD out;
	int ret;

	do {
		if (stream == stdout) {
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut == INVALID_HANDLE_VALUE || hOut == NULL || GetFileType(hOut) != FILE_TYPE_CHAR)
				break;
			if (WriteConsoleW(hOut, text, len, &out, NULL) == 0)
				return -1;
			return out;
		}
		if (stream == stderr) {
			HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
			if (hErr == INVALID_HANDLE_VALUE || hErr == NULL || GetFileType(hErr) != FILE_TYPE_CHAR)
				break;
			if (WriteConsoleW(hErr, text, len, &out, NULL) == 0)
				return -1;
			return out;
		}
	} while(0);

	ret = fputws(text, stream);
	if (ret < 0)
		return ret;
	return len;
}
#endif // !FLAC_WINDOWS_APP

int printf_utf8(const char *format, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, format);

	ret = vfprintf_utf8(stdout, format, argptr);

	va_end(argptr);

	return ret;
}

int fprintf_utf8(FILE *stream, const char *format, ...)
{
	int ret;
	va_list argptr;
	va_start(argptr, format);

	ret = vfprintf_utf8(stream, format, argptr);

	va_end(argptr);

	return ret;
}

int vfprintf_utf8(FILE *stream, const char *format, va_list argptr)
{
	char *utmp = NULL;
	wchar_t *wout = NULL;
	int ret = -1;

	do {
		if (!(utmp = (char *)malloc(UTF8_BUFFER_SIZE))) break;
		if ((ret = local_vsnprintf(utmp, UTF8_BUFFER_SIZE, format, argptr)) <= 0) break;
		if (!(wout = wchar_from_utf8(utmp))) {
			ret = -1;
			break;
		}
#if !FLAC_WINDOWS_APP
		ret = wprint_console(stream, wout, wcslen(wout));
#else // FLAC_WINDOWS_APP
		OutputDebugStringW(wout);
		ret = 0;
#endif // FLAC_WINDOWS_APP
	} while(0);

	free(utmp);
	free(wout);

	return ret;
}

/* file functions */

FILE* fopen_utf8(const char *filename, const char *mode)
{
	wchar_t *wname = NULL;
	wchar_t *wmode = NULL;
	FILE *f = NULL;

	do {
		if (!(wname = wchar_from_utf8(filename))) break;
		if (!(wmode = wchar_from_utf8(mode))) break;
		f = _wfopen(wname, wmode);
	} while(0);

	free(wname);
	free(wmode);

	return f;
}

int stat64_utf8(const char *path, struct __stat64 *buffer)
{
	wchar_t *wpath;
	int ret;

	if (!(wpath = wchar_from_utf8(path))) return -1;
	ret = _wstat64(wpath, buffer);
	free(wpath);

	return ret;
}

int chmod_utf8(const char *filename, int pmode)
{
	wchar_t *wname;
	int ret;

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ret = _wchmod(wname, pmode);
	free(wname);

	return ret;
}

int utime_utf8(const char *filename, struct utimbuf *times)
{
	wchar_t *wname;
	struct __utimbuf64 ut;
	int ret;

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ut.actime = times->actime;
	ut.modtime = times->modtime;
	ret = _wutime64(wname, &ut);
	free(wname);

	return ret;
}

int unlink_utf8(const char *filename)
{
	wchar_t *wname;
	int ret;

	if (!(wname = wchar_from_utf8(filename))) return -1;
	ret = _wunlink(wname);
	free(wname);

	return ret;
}

int rename_utf8(const char *oldname, const char *newname)
{
	wchar_t *wold = NULL;
	wchar_t *wnew = NULL;
	int ret = -1;

	do {
		if (!(wold = wchar_from_utf8(oldname))) break;
		if (!(wnew = wchar_from_utf8(newname))) break;
		ret = _wrename(wold, wnew);
	} while(0);

	free(wold);
	free(wnew);

	return ret;
}
