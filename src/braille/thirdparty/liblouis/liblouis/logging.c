/* liblouis Braille Translation and Back-Translation  Library

   Copyright (C) 2004, 2005, 2006 ViewPlus Technologies, Inc. www.viewplus.com
   Copyright (C) 2004, 2005, 2006 JJB Software, Inc. www.jjb-software.com

   This file is part of liblouis.

   liblouis is free software: you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published
   by the Free Software Foundation, either version 2.1 of the License, or
   (at your option) any later version.

   liblouis is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with liblouis. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 * @brief Logging
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "internal.h"

void EXPORT_CALL
_lou_logWidecharBuf(logLevels level, const char *msg, const widechar *wbuf, int wlen) {
	/* When calculating output size:
	 * Each wdiechar is represented in hex, thus needing two bytes for each
	 * byte in the widechar (sizeof(widechar) * 2)
	 * Allow space for the "0x%X " formatting (+ 3)
	 * Number of characters in widechar buffer (wlen * )
	 * Give space for additional message (+ strlen(msg))
	 * Remember the null terminator (+ 1)
	 */
	int logBufSize = (wlen * ((sizeof(widechar) * 3) + 3)) + 3 + (int)strlen(msg);
	char *logMsg = malloc(logBufSize);
	char *p = logMsg;
	const char *formatString;
	int i = 0;
	if (sizeof(widechar) == 2)
		formatString = "0x%04X ";
	else
		formatString = "0x%08X ";
	for (i = 0; i < (int)strlen(msg); i++) logMsg[i] = msg[i];
	p += strlen(msg);
	for (i = 0; i < wlen; i++) {
		p += sprintf(p, formatString, wbuf[i]);
	}
	*p = '~';
	p++;
	*p = ' ';
	p++;
	for (i = 0; i < wlen; i++) {
		if (wbuf[i] & 0xff00)
			*p = ' ';
		else
			*p = (char)wbuf[i];
		p++;
	}
	*p = '\0';
	_lou_logMessage(level, "%s", logMsg);
	free(logMsg);
}

static void EXPORT_CALL
defaultLogCallback(logLevels level, const char *message) {
	lou_logPrint("%s",
			message);  // lou_logPrint takes formatting, protect against % in message
}

static logcallback logCallbackFunction = defaultLogCallback;
void EXPORT_CALL
lou_registerLogCallback(logcallback callback) {
	if (callback == NULL)
		logCallbackFunction = defaultLogCallback;
	else
		logCallbackFunction = callback;
}

static logLevels logLevel = LOU_LOG_INFO;
void EXPORT_CALL
lou_setLogLevel(logLevels level) {
	logLevel = level;
}

void EXPORT_CALL
_lou_logMessage(logLevels level, const char *format, ...) {
	if (format == NULL) return;
	if (level < logLevel) return;
	if (logCallbackFunction != NULL) {
#ifdef _WIN32
		double f = 2.3;	 // Needed to force VC++ runtime floating point support
#endif
		char *s;
		size_t len;
		va_list argp;
		va_start(argp, format);
		len = vsnprintf(0, 0, format, argp);
		va_end(argp);
		if ((s = malloc(len + 1)) != 0) {
			va_start(argp, format);
			vsnprintf(s, len + 1, format, argp);
			va_end(argp);
			logCallbackFunction(level, s);
			free(s);
		}
	}
}

static FILE *logFile = NULL;
static char initialLogFileName[256] = "";

void EXPORT_CALL
lou_logFile(const char *fileName) {
	if (logFile) {
		fclose(logFile);
		logFile = NULL;
	}
	if (fileName == NULL || fileName[0] == 0) return;
	if (initialLogFileName[0] == 0) strcpy(initialLogFileName, fileName);
	logFile = fopen(fileName, "a");
	if (logFile == NULL && initialLogFileName[0] != 0)
		logFile = fopen(initialLogFileName, "a");
	if (logFile == NULL) {
		fprintf(stderr, "Cannot open log file %s\n", fileName);
		logFile = stderr;
	}
}

void EXPORT_CALL
lou_logPrint(const char *format, ...) {
#ifndef __SYMBIAN32__
	va_list argp;
	if (format == NULL) return;
	if (logFile == NULL) logFile = fopen(initialLogFileName, "a");
	if (logFile == NULL) logFile = stderr;
	va_start(argp, format);
	vfprintf(logFile, format, argp);
	fprintf(logFile, "\n");
	fflush(logFile);
	va_end(argp);
#endif
}

/* Close the log file */
void EXPORT_CALL
lou_logEnd(void) {
	if (logFile != NULL && logFile != stderr) fclose(logFile);
	logFile = NULL;
}
