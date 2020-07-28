/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */


#include "fluid_sys.h"

static char fluid_errbuf[512];  /* buffer for error message */

static fluid_log_function_t fluid_log_function[LAST_LOG_LEVEL];
static void* fluid_log_user_data[LAST_LOG_LEVEL];
static int fluid_log_initialized = 0;

static char* fluid_libname = "fluidsynth";


void fluid_sys_config()
{
  fluid_log_config();
}


unsigned int fluid_debug_flags = 0;

#if DEBUG
/*
 * fluid_debug
 */
int fluid_debug(int level, char * fmt, ...)
{
  if (fluid_debug_flags & level) {
    fluid_log_function_t fun;
    va_list args;

    va_start (args, fmt);
    vsnprintf(fluid_errbuf, sizeof (fluid_errbuf), fmt, args);
    va_end (args);

    fun = fluid_log_function[FLUID_DBG];
    if (fun != NULL) {
      (*fun)(level, fluid_errbuf, fluid_log_user_data[FLUID_DBG]);
    }
  }
  return 0;
}
#endif

/**
 * Installs a new log function for a specified log level.
 * @param level Log level to install handler for.
 * @param fun Callback function handler to call for logged messages
 * @param data User supplied data pointer to pass to log function
 * @return The previously installed function.
 */
fluid_log_function_t
fluid_set_log_function(int level, fluid_log_function_t fun, void* data)
{
  fluid_log_function_t old = NULL;

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    old = fluid_log_function[level];
    fluid_log_function[level] = fun;
    fluid_log_user_data[level] = data;
  }
  return old;
}

/**
 * Default log function which prints to the stderr.
 * @param level Log level
 * @param message Log message
 * @param data User supplied data (not used)
 */
void
fluid_default_log_function(int level, char* message, void* data)
{
  FILE* out;

#if defined(WIN32)
  out = stdout;
#else
  out = stderr;
#endif

  if (fluid_log_initialized == 0) {
    fluid_log_config();
  }

  switch (level) {
  case FLUID_PANIC:
    FLUID_FPRINTF(out, "%s: panic: %s\n", fluid_libname, message);
    break;
  case FLUID_ERR:
    FLUID_FPRINTF(out, "%s: error: %s\n", fluid_libname, message);
    break;
  case FLUID_WARN:
    FLUID_FPRINTF(out, "%s: warning: %s\n", fluid_libname, message);
    break;
  case FLUID_INFO:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  case FLUID_DBG:
#if DEBUG
    FLUID_FPRINTF(out, "%s: debug: %s\n", fluid_libname, message);
#endif
    break;
  default:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  }
  fflush(out);
}

/*
 * fluid_init_log
 */
void
fluid_log_config(void)
{
  if (fluid_log_initialized == 0) {

    fluid_log_initialized = 1;

    if (fluid_log_function[FLUID_PANIC] == NULL) {
      fluid_set_log_function(FLUID_PANIC, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_ERR] == NULL) {
      fluid_set_log_function(FLUID_ERR, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_WARN] == NULL) {
      fluid_set_log_function(FLUID_WARN, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_INFO] == NULL) {
      fluid_set_log_function(FLUID_INFO, fluid_default_log_function, NULL);
    }

    if (fluid_log_function[FLUID_DBG] == NULL) {
      fluid_set_log_function(FLUID_DBG, fluid_default_log_function, NULL);
    }
  }
}

/**
 * Print a message to the log.
 * @param level Log level (#fluid_log_level).
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return Always returns -1
 */
int
fluid_log(int level, char* fmt, ...)
{
  fluid_log_function_t fun = NULL;

  va_list args;
  va_start (args, fmt);
  vsnprintf(fluid_errbuf, sizeof (fluid_errbuf), fmt, args);
  va_end (args);

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    fun = fluid_log_function[level];
    if (fun != NULL) {
      (*fun)(level, fluid_errbuf, fluid_log_user_data[level]);
    }
  }
  return FLUID_FAILED;
}

/**
 * An improved strtok, still trashes the input string, but is portable and
 * thread safe.  Also skips token chars at beginning of token string and never
 * returns an empty token (will return NULL if source ends in token chars though).
 * NOTE: NOT part of public API
 * @internal
 * @param str Pointer to a string pointer of source to tokenize.  Pointer gets
 *   updated on each invocation to point to beginning of next token.  Note that
 *   token char get's overwritten with a 0 byte.  String pointer is set to NULL
 *   when final token is returned.
 * @param delim String of delimiter chars.
 * @return Pointer to the next token or NULL if no more tokens.
 */
char *fluid_strtok (char **str, char *delim)
{
  char *s, *d, *token;
  char c;

  if (str == NULL || delim == NULL || !*delim)
  {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return NULL;
  }

  s = *str;
  if (!s) return NULL;	/* str points to a NULL pointer? (tokenize already ended) */

  /* skip delimiter chars at beginning of token */
  do
  {
    c = *s;
    if (!c)	/* end of source string? */
    {
      *str = NULL;
      return NULL;
    }

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	s++;		/* advance to next source char */
	break;
      }
    }
  } while (*d);		/* while token char match */

  token = s;		/* start of token found */

  /* search for next token char or end of source string */
  for (s = s+1; *s; s++)
  {
    c = *s;

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	*s = '\0';	/* overwrite token char with zero byte to terminate token */
	*str = s+1;	/* update str to point to beginning of next token */
	return token;
      }
    }
  }

  /* we get here only if source string ended */
  *str = NULL;
  return token;
}

/*
 * fluid_error
 */
char*
fluid_error()
{
  return fluid_errbuf;
}


/*
 *
 *  fluid_is_midifile
 */
int
fluid_is_midifile(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == NULL) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "MThd", 4) == 0;
}

/*
 *  fluid_is_soundfont
 *
 */
int
fluid_is_soundfont(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == NULL) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "RIFF", 4) == 0;
}

/*=============================================================*/
/*                                                             */
/*                           Win32                             */
/*                                                             */
/*=============================================================*/

/***************************************************************
 *
 *               Timer
 *
 */

//timer disabled


/***************************************************************
 *
 *               Floating point exceptions
 *
 *  The floating point exception functions were taken from Ircam's
 *  jMax source code. http://www.ircam.fr/jmax
 *
 *  FIXME: check in config for i386 machine
 *
 *  Currently not used. I leave the code here in case we want to pick
 *  this up again some time later.
 */



/***************************************************************
 *
 *               Profiling (Linux, i586 only)
 *
 */


/***************************************************************
 *
 *               Threads
 *
 */

//thread disabled


/***************************************************************
 *
 *               Sockets
 *
 */

//socket disabled
