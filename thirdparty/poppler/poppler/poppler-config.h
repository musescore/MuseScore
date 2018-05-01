/* poppler/poppler-config.h.  Generated from poppler-config.h.in by configure.  */
//================================================= -*- mode: c++ -*- ====
//
// poppler-config.h
//
// Copyright 1996-2011 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2014 Bogdan Cristea <cristeab@gmail.com>
// Copyright (C) 2014 Hib Eris <hib@hiberis.nl>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef POPPLER_CONFIG_H
#define POPPLER_CONFIG_H

#include <stdio.h>

// We duplicate some of the config.h #define's here since they are
// used in some of the header files we install.  The #ifndef/#endif
// around #undef look odd, but it's to silence warnings about
// redefining those symbols.

/* Defines the poppler version. */
#ifndef POPPLER_VERSION
#define POPPLER_VERSION "0.44.0"
#endif

/* Enable multithreading support. */
#ifndef MULTITHREADED
#define MULTITHREADED 1
#endif

/* Use fixedpoint. */
#ifndef USE_FIXEDPOINT
/* #undef USE_FIXEDPOINT */
#endif

/* Use single precision arithmetic in the Splash backend */
#ifndef USE_FLOAT
/* #undef USE_FLOAT */
#endif

/* Include support for OPI comments. */
#ifndef OPI_SUPPORT
#define OPI_SUPPORT 1
#endif

/* Enable word list support. */
#ifndef TEXTOUT_WORD_LIST
#define TEXTOUT_WORD_LIST 1
#endif

/* Support for curl is compiled in. */
/* #undef POPPLER_HAS_CURL_SUPPORT */

/* Use libjpeg instead of builtin jpeg decoder. */
/* #undef ENABLE_LIBJPEG */

/* Build against libtiff. */
/* #undef ENABLE_LIBTIFF */

/* Build against libpng. */
//#ifndef ENABLE_LIBPNG
//#define ENABLE_LIBPNG 1
//#endif

/* Use zlib instead of builtin zlib decoder for uncompressing flate streams. */
/* #undef ENABLE_ZLIB_UNCOMPRESS */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_DIRENT_H
#define HAVE_DIRENT_H 1
#endif

/* Defines if gettimeofday is available on your system */
#if (!defined (_MSCVER) && !defined (_MSC_VER))
   // MSVC does not have time of day
   #ifndef HAVE_GETTIMEOFDAY
   #define HAVE_GETTIMEOFDAY 1
   #endif
#else
   #ifdef HAVE_GETTIMEOFDAY
   #undef HAVE_GETTIMEOFDAY
   #endif
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#ifndef HAVE_NDIR_H
/* #undef HAVE_NDIR_H */
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_SYS_DIR_H
/* #undef HAVE_SYS_DIR_H */
#endif

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#ifndef HAVE_SYS_NDIR_H
/* #undef HAVE_SYS_NDIR_H */
#endif

/* Have FreeType2 include files */
#ifndef HAVE_FREETYPE_H
#define HAVE_FREETYPE_H 1
#endif

/* Defines if use cms */
//#ifndef USE_CMS
//#define USE_CMS 1
//#endif

// Also, there are preprocessor symbols in the header files
// that are used but never defined when building poppler using configure
// or cmake: DISABLE_OUTLINE, DEBUG_MEM, SPLASH_CMYK, HAVE_T1LIB_H,
// ENABLE_PLUGINS, DEBUG_FORMS, HAVE_FREETYPE_FREETYPE_H

//------------------------------------------------------------------------
// version
//------------------------------------------------------------------------

// copyright notice
#define popplerCopyright "Copyright 2005-2016 The Poppler Developers - http://poppler.freedesktop.org"
#define xpdfCopyright "Copyright 1996-2011 Glyph & Cog, LLC"

//------------------------------------------------------------------------
// popen
//------------------------------------------------------------------------

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define popen _popen
#define pclose _pclose
#endif

#if defined(VMS) || defined(VMCMS) || defined(DOS) || defined(OS2) || defined(__EMX__) || defined(_WIN32) || defined(__DJGPP__) || defined(MACOS)
#define POPEN_READ_MODE "rb"
#else
#define POPEN_READ_MODE "r"
#endif

//------------------------------------------------------------------------
// Win32 stuff
//------------------------------------------------------------------------

#if defined(_WIN32) && !defined(_MSC_VER)
#include <windef.h>
#else
#define CDECL
#endif

#if defined(_WIN32)
#ifdef _MSC_VER
#define strtok_r strtok_s
#elif __MINGW32__ && !defined(__WINPTHREADS_VERSION)
char * strtok_r (char *s, const char *delim, char **save_ptr);
#endif
#endif

//------------------------------------------------------------------------
// Compiler
//------------------------------------------------------------------------

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#ifdef __MINGW_PRINTF_FORMAT
#define GCC_PRINTF_FORMAT(fmt_index, va_index) \
	__attribute__((__format__(__MINGW_PRINTF_FORMAT, fmt_index, va_index)))
#else
#define GCC_PRINTF_FORMAT(fmt_index, va_index) \
	__attribute__((__format__(__printf__, fmt_index, va_index)))
#endif
#else
#define GCC_PRINTF_FORMAT(fmt_index, va_index)
#endif

// Causes compilation errors with Visual Studio 17, and fmax/fmin are available
//#if defined(_MSC_VER)
//#define fmax(a, b) std::max(a, b)
//#define fmin(a, b) std::min(a, b)
//#endif


#endif /* POPPLER_CONFIG_H */
