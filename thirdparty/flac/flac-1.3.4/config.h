/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Target processor is big endian. */
#define CPU_IS_BIG_ENDIAN 0

/* Target processor is little endian. */
#define CPU_IS_LITTLE_ENDIAN 0

/* Set FLAC__BYTES_PER_WORD to 8 (4 is the default) */
#define ENABLE_64_BIT_WORDS 0

/* define to align allocated memory on 32-byte boundaries */
/* #undef FLAC__ALIGN_MALLOC_DATA */

/* define if you have docbook-to-man or docbook2man */
/* #undef FLAC__HAS_DOCBOOK_TO_MAN */

/* define if you are compiling for x86 and have the NASM assembler */
/* #undef FLAC__HAS_NASM */

/* define if you have the ogg library */
#define OGG_FOUND 0
#define FLAC__HAS_OGG OGG_FOUND

/* define if compiler has __attribute__((target("cpu=power8"))) support */
/* #undef FLAC__HAS_TARGET_POWER8 */

/* define if compiler has __attribute__((target("cpu=power9"))) support */
/* #undef FLAC__HAS_TARGET_POWER9 */

/* Set to 1 if <x86intrin.h> is available. */
#define FLAC__HAS_X86INTRIN 1

/* define if building for Darwin / MacOS X */
/* #undef FLAC__SYS_DARWIN */

/* define if building for Linux */
/* #undef FLAC__SYS_LINUX */

/* define to enable use of Altivec instructions */
/* #undef FLAC__USE_ALTIVEC */

/* define to enable use of AVX instructions */
#define WITH_AVX 1
#define FLAC__USE_AVX WITH_AVX

/* define to enable use of VSX instructions */
/* #undef FLAC__USE_VSX */

/* Compiler has the __builtin_bswap16 intrinsic */
#define HAVE_BSWAP16 1

/* Compiler has the __builtin_bswap32 intrinsic */
#define HAVE_BSWAP32 1

/* Define to 1 if you have the <byteswap.h> header file. */
#define HAVE_BYTESWAP_H

/* define if you have clock_gettime */
#define HAVE_CLOCK_GETTIME

/* Define to 1 if you have the <cpuid.h> header file. */
#define HAVE_CPUID_H

/* Define to 1 if C++ supports variable-length arrays. */
#define HAVE_CXX_VARARRAYS

/* Define to 1 if C supports variable-length arrays. */
/* #undef HAVE_C_VARARRAYS */

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO

/* Define to 1 if you have the `getopt_long' function. */
/* #undef HAVE_GETOPT_LONG */

/* Define if you have the iconv() function and it works. */
/* #undef HAVE_ICONV */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
/* #undef HAVE_LANGINFO_CODESET */

/* lround support */
#define HAVE_LROUND 1

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define to 1 if the system has the type `socklen_t'. */
/* #undef HAVE_SOCKLEN_T */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
/* #undef HAVE_STDLIB_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#define HAVE_SYS_IOCTL_H

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H

/* Define to 1 if typeof works with your compiler. */
/* #undef HAVE_TYPEOF */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H

/* Define to 1 if you have the <x86intrin.h> header file. */
/* #undef HAVE_X86INTRIN_H */

/* Define as const if the declaration of iconv() needs const. */
/* #undef ICONV_CONST */

/* Define if debugging is disabled */
/* #undef NDEBUG */

/* Name of package */
/* #undef PACKAGE */

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Define to the full name of this package. */
/* #undef PACKAGE_NAME */

/* Define to the full name and version of this package. */
/* #undef PACKAGE_STRING */

/* Define to the one symbol short name of this package. */
/* #undef PACKAGE_TARNAME */

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* The size of `off_t', as computed by sizeof. */
/* #undef SIZEOF_OFF_T */

/* The size of `void*', as computed by sizeof. */
/* #undef SIZEOF_VOIDP */

/* Define to 1 if you have the ANSI C header files. */
/* #undef STDC_HEADERS */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
#define _ALL_SOURCE
#endif

/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _FORTIFY_SOURCE
#define DODEFINE_FORTIFY_SOURCE 2
#define _FORTIFY_SOURCE DODEFINE_FORTIFY_SOURCE
#endif

#ifndef _XOPEN_SOURCE
/* #undef DODEFINE_XOPEN_SOURCE */
#define _XOPEN_SOURCE DODEFINE_XOPEN_SOURCE
#endif

/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
/* #undef _POSIX_PTHREAD_SEMANTICS */
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
/* #undef _TANDEM_SOURCE */
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
#define DODEFINE_EXTENSIONS
#define __EXTENSIONS__ DODEFINE_EXTENSIONS
#endif


/* Target processor is big endian. */
#define WORDS_BIGENDIAN CPU_IS_BIG_ENDIAN

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#ifndef _FILE_OFFSET_BITS
# define _FILE_OFFSET_BITS 64
#endif

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
#ifndef _LARGEFILE_SOURCE
# define _LARGEFILE_SOURCE
#endif

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to __typeof__ if your compiler spells it that way. */
/* #undef typeof */
