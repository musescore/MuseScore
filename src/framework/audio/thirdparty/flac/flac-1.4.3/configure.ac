#  FLAC - Free Lossless Audio Codec
#  Copyright (C) 2001-2009  Josh Coalson
#  Copyright (C) 2011-2023  Xiph.Org Foundation
#
#  This file is part the FLAC project.  FLAC is comprised of several
#  components distributed under different licenses.  The codec libraries
#  are distributed under Xiph.Org's BSD-like license (see the file
#  COPYING.Xiph in this distribution).  All other programs, libraries, and
#  plugins are distributed under the GPL (see COPYING.GPL).  The documentation
#  is distributed under the Gnu FDL (see COPYING.FDL).  Each file in the
#  FLAC distribution contains at the top the terms under which it may be
#  distributed.
#
#  Since this particular file is relevant to all components of FLAC,
#  it may be distributed under the Xiph.Org license, which is the least
#  restrictive of those mentioned above.  See the file COPYING.Xiph in this
#  distribution.

# NOTE that for many of the AM_CONDITIONALs we use the prefix FLaC__
# instead of FLAC__ since autoconf triggers off 'AC_' in strings

AC_PREREQ(2.60)
AC_INIT([flac],[1.4.3],[flac-dev@xiph.org],[flac],[https://www.xiph.org/flac/])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/flac/main.c])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign 1.10 -Wall tar-pax no-dist-gzip dist-xz subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AX_CHECK_ENABLE_DEBUG
user_cflags=$CFLAGS

#Prefer whatever the current ISO standard is.
AC_PROG_CC
AC_USE_SYSTEM_EXTENSIONS
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT([win32-dll disable-static pic-only])
AM_PROG_AS
AC_PROG_CXX
XIPH_C_COMPILER_IS_CLANG
XIPH_GCC_REALLY_IS_GCC
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

AC_CHECK_SIZEOF(off_t,1)	# Fake default value.
AC_CHECK_SIZEOF([void*])
AC_SEARCH_LIBS([lround],[m], [AC_DEFINE(HAVE_LROUND,1,lround support)])

AM_PROG_CC_C_O
AC_C_INLINE
AC_C_TYPEOF

AC_CHECK_HEADERS([stdint.h stdbool.h inttypes.h byteswap.h sys/auxv.h sys/param.h sys/ioctl.h termios.h x86intrin.h cpuid.h arm_neon.h])

if test "x$ac_cv_header_stdint_h" != xyes -o "x$ac_cv_header_stdbool_h" != xyes; then
AC_MSG_ERROR("Header stdint.h and/or stdbool.h not found")
fi


XIPH_C_BSWAP32
XIPH_C_BSWAP16

ac_cv_c_big_endian=0
ac_cv_c_little_endian=0
AC_C_BIGENDIAN([ac_cv_c_big_endian=1], [ac_cv_c_little_endian=1], [
	AC_MSG_WARN([[*****************************************************************]])
	AC_MSG_WARN([[*** Not able to determine endian-ness of target processor.       ]])
	AC_MSG_WARN([[*** The constants CPU_IS_BIG_ENDIAN and CPU_IS_LITTLE_ENDIAN in  ]])
	AC_MSG_WARN([[*** config.h may need to be hand editied.                        ]])
	AC_MSG_WARN([[*****************************************************************]])
])
AC_DEFINE_UNQUOTED(CPU_IS_BIG_ENDIAN, ${ac_cv_c_big_endian},
					[Target processor is big endian.])
AC_DEFINE_UNQUOTED(CPU_IS_LITTLE_ENDIAN, ${ac_cv_c_little_endian},
					[Target processor is little endian.])
AC_DEFINE_UNQUOTED(WORDS_BIGENDIAN, ${ac_cv_c_big_endian},
					[Target processor is big endian.])

AC_ARG_ENABLE(asm-optimizations, AS_HELP_STRING([--disable-asm-optimizations],[Do not use any CPU specific optimization routines]), asm_opt=no, asm_opt=yes)
AM_CONDITIONAL(FLaC__NO_ASM, test "x$asm_opt" = xno)
if test "x$asm_opt" = xno ; then
AC_DEFINE(FLAC__NO_ASM)
AH_TEMPLATE(FLAC__NO_ASM, [define to disable use of assembly code])
fi

dnl check for getauxval in standard library
AC_CHECK_FUNCS(getauxval)

dnl check for getopt in standard library
dnl AC_CHECK_FUNCS(getopt_long , , [LIBOBJS="$LIBOBJS getopt.o getopt1.o"] )
AC_CHECK_FUNCS(getopt_long, [], [])

AC_CHECK_SIZEOF(void*,1)

asm_optimisation=no
case "$host_cpu" in
	amd64|x86_64)
		case "$host" in
			*gnux32)
				# x32 user space and 64 bit kernel.
				cpu_x86_64=true
				AC_DEFINE(FLAC__CPU_X86_64)
				AH_TEMPLATE(FLAC__CPU_X86_64, [define if building for x86_64])
				;;
			*)
				if test $ac_cv_sizeof_voidp = 4 ; then
					# This must be a 32 bit user space running on 64 bit kernel so treat
					# this as ia32.
					cpu_ia32=true
					AC_DEFINE(FLAC__CPU_IA32)
					AH_TEMPLATE(FLAC__CPU_IA32, [define if building for ia32/i386])
			else
					# x86_64 user space and kernel.
					cpu_x86_64=true
					AC_DEFINE(FLAC__CPU_X86_64)
					AH_TEMPLATE(FLAC__CPU_X86_64, [define if building for x86_64])
				fi
				;;
		esac
		;;
	i*86)
		cpu_ia32=true
		AC_DEFINE(FLAC__CPU_IA32)
		AH_TEMPLATE(FLAC__CPU_IA32, [define if building for ia32/i386])
		;;
	arm64|aarch64)
		cpu_arm64=true
		AC_DEFINE(FLAC__CPU_ARM64)
		AH_TEMPLATE(FLAC__CPU_ARM64, [define if building for ARM])
		;;
esac
AM_CONDITIONAL(FLAC__CPU_X86_64, test "x$cpu_x86_64" = xtrue)
AM_CONDITIONAL(FLaC__CPU_IA32, test "x$cpu_ia32" = xtrue)
AM_CONDITIONAL(FLAC__CPU_ARM64, test "x$cpu_arm64" = xtrue)

if test "x$ac_cv_header_x86intrin_h" = xyes -a "x$asm_opt" = xyes; then
  AC_DEFINE([FLAC__HAS_X86INTRIN], 1, [Set to 1 if <x86intrin.h> is available.])
  asm_optimisation=yes
else
  AC_DEFINE([FLAC__HAS_X86INTRIN], 0)
fi

if test "x$ac_cv_header_arm_neon_h" = xyes -a "x$asm_opt" = xyes; then
  AC_DEFINE([FLAC__HAS_NEONINTRIN], 1, [Set to 1 if <arm_neon.h> is available.])
  AC_MSG_CHECKING([whether arm_neon.h has A64 functions])
  AC_COMPILE_IFELSE(
	[AC_LANG_PROGRAM([[#include <arm_neon.h>]],
	[[float64x2_t sum5; sum5 = vdupq_n_f64(0.0f);]])],
	[AC_MSG_RESULT([yes])
	 has_a64neon=yes],
	[AC_MSG_RESULT([no])])
  if test "x$has_a64neon" = xyes; then
    AC_DEFINE([FLAC__HAS_A64NEONINTRIN], 1, [Set to 1 if <arm_neon.h> has A64 instructions.])
    asm_optimisation=yes
  else
    AC_DEFINE([FLAC__HAS_A64NEONINTRIN], 0)
  fi
else
  AC_DEFINE([FLAC__HAS_NEONINTRIN], 0)
fi

case "$host" in
	i386-*-openbsd3.[[0-3]]) OBJ_FORMAT=aoutb ;;
	*-*-cygwin|*mingw*) OBJ_FORMAT=win32 ;;
	*-*-darwin*) OBJ_FORMAT=macho ;;
	*emx*) OBJ_FORMAT=aout ;;
	*djgpp) OBJ_FORMAT=coff ;;
	*) OBJ_FORMAT=elf ;;
esac
AC_SUBST(OBJ_FORMAT)

os_is_windows=no
case "$host" in
	*mingw*)
		os_is_windows=yes
		AC_CHECK_TOOL(RC,[windres],[:])
		;;
esac

AM_CONDITIONAL(OS_IS_WINDOWS, test "x$os_is_windows" = xyes)
AM_CONDITIONAL(HAVE_WINDRES, test "x$RC" != "x:")

case "$host" in
	*-linux-*)
		sys_linux=true
		AC_DEFINE(FLAC__SYS_LINUX)
		AH_TEMPLATE(FLAC__SYS_LINUX, [define if building for Linux])
		;;
	*-*-darwin*)
		sys_darwin=true
		AC_DEFINE(FLAC__SYS_DARWIN)
		AH_TEMPLATE(FLAC__SYS_DARWIN, [define if building for Darwin / MacOS X])
		;;
esac
AM_CONDITIONAL(FLaC__SYS_DARWIN, test "x$sys_darwin" = xtrue)
AM_CONDITIONAL(FLaC__SYS_LINUX, test "x$sys_linux" = xtrue)

if test "x$cpu_ia32" = xtrue || test "x$cpu_x86_64" = xtrue ; then
AC_DEFINE(FLAC__ALIGN_MALLOC_DATA)
AH_TEMPLATE(FLAC__ALIGN_MALLOC_DATA, [define to align allocated memory on 32-byte boundaries])
fi

AM_CONDITIONAL([DEBUG], [test "x${ax_enable_debug}" = "xyes" || test "x${ax_enable_debug}" = "xinfo"])

AC_ARG_ENABLE(avx,
AS_HELP_STRING([--disable-avx],[Disable AVX, AVX2 optimizations. There is runtime detection of CPU features, so disabling is only necessary when a compiler does not know about them]),
[case "${enableval}" in
	yes) use_avx=true ;;
	no)  use_avx=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-avx) ;;
esac],[use_avx=true])
AM_CONDITIONAL(FLaC__USE_AVX, test "x$use_avx" = xtrue)
if test "x$use_avx" = xtrue ; then
AC_DEFINE(FLAC__USE_AVX)
AH_TEMPLATE(FLAC__USE_AVX, [define to enable use of AVX instructions])
fi

AC_ARG_ENABLE(thorough-tests,
AS_HELP_STRING([--disable-thorough-tests],[Disable thorough (long) testing, do only basic tests]),
[case "${enableval}" in
	yes) thorough_tests=true ;;
	no)  thorough_tests=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-thorough-tests) ;;
esac],[thorough_tests=true])
AC_ARG_ENABLE(exhaustive-tests,
AS_HELP_STRING([--enable-exhaustive-tests],[Enable exhaustive testing (VERY long)]),
[case "${enableval}" in
	yes) exhaustive_tests=true ;;
	no)  exhaustive_tests=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-exhaustive-tests) ;;
esac],[exhaustive_tests=false])
if test "x$thorough_tests" = xfalse ; then
FLAC__TEST_LEVEL=0
elif test "x$exhaustive_tests" = xfalse ; then
FLAC__TEST_LEVEL=1
else
FLAC__TEST_LEVEL=2
fi
AC_SUBST(FLAC__TEST_LEVEL)

AC_ARG_ENABLE(werror,
	AS_HELP_STRING([--enable-werror],[Enable -Werror in all Makefiles]))

AC_ARG_ENABLE([stack-smash-protection],
    [AS_HELP_STRING([--disable-stack-smash-protection],[Disable GNU GCC stack smash protection])],,
    [AS_IF([test "$ac_cv_c_compiler_gnu" = "yes"],
        [enable_stack_smash_protection=yes],[enable_stack_smash_protection=no])])

AC_ARG_ENABLE([fortify-source],
    [AS_HELP_STRING([--disable-fortify-source],[Disable _FORTIFY_SOURCE buffer overflow protection])],,
    [AS_IF([test "$ac_cv_c_compiler_gnu" = "yes"],
        [enable_fortify_source=yes],[enable_fortify_source=no])])

case "$host" in
	*mingw*)
		if test "$enable_fortify_source" = "yes"; then
			AC_SEARCH_LIBS(__memset_chk, ssp, , mingw_has_memset_chk=no)
		fi
		if test "$enable_stack_smash_protection" = "yes"; then
			AC_SEARCH_LIBS(__stack_chk_fail, ssp, , mingw_has_stack_chk_fail=no)
		fi
	;;
esac


AC_ARG_ENABLE(64-bit-words,
	AS_HELP_STRING([--disable-64-bit-words],[Set FLAC__BYTES_PER_WORD to 4 (8 is the default)]))
if test "x$enable_64_bit_words" = xno ; then
	AC_DEFINE_UNQUOTED([ENABLE_64_BIT_WORDS],0,[Set FLAC__BYTES_PER_WORD to 8 (this is the default)])
else
	AC_DEFINE_UNQUOTED([ENABLE_64_BIT_WORDS],1)
fi
AC_SUBST(ENABLE_64_BIT_WORDS)

AC_ARG_ENABLE(valgrind-testing,
AS_HELP_STRING([--enable-valgrind-testing],[Run all tests inside Valgrind]),
[case "${enableval}" in
	yes) FLAC__TEST_WITH_VALGRIND=yes ;;
	no)  FLAC__TEST_WITH_VALGRIND=no ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-valgrind-testing) ;;
esac],[FLAC__TEST_WITH_VALGRIND=no])
AC_SUBST(FLAC__TEST_WITH_VALGRIND)

AC_ARG_ENABLE(doxygen-docs,
AS_HELP_STRING([--disable-doxygen-docs],[Disable API documentation building via Doxygen]),
[case "${enableval}" in
	yes) enable_doxygen_docs=true ;;
	no)  enable_doxygen_docs=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-doxygen-docs) ;;
esac],[enable_doxygen_docs=true])
if test "x$enable_doxygen_docs" != xfalse ; then
	AC_CHECK_PROGS(DOXYGEN, doxygen)
fi
AM_CONDITIONAL(FLaC__HAS_DOXYGEN, test -n "$DOXYGEN")

if test ! -n "$DOXYGEN" && test -f "$srcdir/doc/FLAC.tag" && test -f "$srcdir/doc/api/modules.html" ; then
	HAS_PREBUILT_DOXYGEN=yes
fi
AM_CONDITIONAL(FLaC__HAS_PREBUILT_DOXYGEN, test "x$HAS_PREBUILT_DOXYGEN" = xyes)

dnl build FLAC++ or not
AC_ARG_ENABLE([cpplibs],
AS_HELP_STRING([--disable-cpplibs],[Do not build libFLAC++]),
[case "${enableval}" in
	yes) disable_cpplibs=false ;;
	no)  disable_cpplibs=true ;;
	*)   AC_MSG_ERROR(bad value ${enableval} for --enable-cpplibs) ;;
esac], [disable_cpplibs=false])
AM_CONDITIONAL(FLaC__WITH_CPPLIBS, [test "x$disable_cpplibs" != xtrue])

AC_ARG_ENABLE([oss-fuzzers],
  [AS_HELP_STRING([--enable-oss-fuzzers],
    [Whether to generate the fuzzers for OSS-Fuzz (Clang only)])],
  [have_oss_fuzzers=yes], [have_oss_fuzzers=no])

if test "x$have_oss_fuzzers" = "xyes"; then
  if test "x$xiph_cv_c_compiler_clang" = "xyes" ; then
    AM_CONDITIONAL([USE_OSSFUZZERS], [test "x$have_oss_fuzzers" = "xyes"])
    if test "x$LIB_FUZZING_ENGINE" = "x" ; then
      # Only set this if it is empty.
	  LIB_FUZZING_ENGINE=-fsanitize=fuzzer
    fi
  else
    AM_CONDITIONAL([USE_OSSFUZZERS], [test "false" = "true"])
    # Disable fuzzer if the compiler is not Clang.
    AC_MSG_WARN([*** Ozz-Fuzz is disabled because that requires the Clang compiler.])
    have_oss_fuzzers="no (compiler is not Clang)"
  fi
else
  AM_CONDITIONAL([USE_OSSFUZZERS], [test "false" = "true"])
fi

AC_SUBST([LIB_FUZZING_ENGINE])

dnl check for ogg library
AC_ARG_ENABLE([ogg],
	AS_HELP_STRING([--disable-ogg],[Disable ogg support (default: test for libogg)]),
	[ want_ogg=$enableval ], [ want_ogg=yes ] )

if test "x$want_ogg" != "xno"; then
	XIPH_PATH_OGG(have_ogg=yes, AC_MSG_WARN([*** Ogg development environment not installed - Ogg support will not be built]))
fi

FLAC__HAS_OGG=0
AM_CONDITIONAL(FLaC__HAS_OGG, [test "x$have_ogg" = xyes])
if test "x$have_ogg" = xyes ; then
	FLAC__HAS_OGG=1
	OGG_PACKAGE="ogg"
else
	have_ogg=no
fi
AC_DEFINE_UNQUOTED([FLAC__HAS_OGG],$FLAC__HAS_OGG,[define if you have the ogg library])
AC_SUBST(FLAC__HAS_OGG)
AC_SUBST(OGG_PACKAGE)

dnl Build programs?
AC_ARG_ENABLE([programs],
	AS_HELP_STRING([--disable-programs], [Do not build and install flac and metaflac]))
AM_CONDITIONAL(FLaC__WITH_PROGRAMS, [test "x$enable_programs" != "xno"])

dnl Build examples?
AC_ARG_ENABLE([examples],
	AS_HELP_STRING([--disable-examples], [Do not build and install examples]))
AM_CONDITIONAL([EXAMPLES], [test "x$enable_examples" != "xno"])

dnl Ask git which version FLAC is
AC_ARG_ENABLE([version-from-git],
	AS_HELP_STRING([--disable-version-from-git], [Do not use git tag, commit hash and commit date for version number]),
       [ enable_version_from_git=$enableval ], [ enable_version_from_git=yes ])


dnl check for i18n(internationalization); these are from libiconv/gettext
AM_ICONV
AM_LANGINFO_CODESET

AC_CHECK_PROGS(PANDOC, pandoc)
AM_CONDITIONAL(FLaC__HAS_PANDOC, test -n "$PANDOC")
if test -n "$PANDOC" ; then
	AC_DEFINE(FLAC__HAS_PANDOC)
	AH_TEMPLATE(FLAC__HAS_PANDOC, [define if you have pandoc])
else
if test -f "$srcdir/man/flac.1" && test -f "$srcdir/man/metaflac.1" ; then
	HAS_PREBUILT_MANPAGES=yes
fi
fi
AM_CONDITIONAL(FLaC__HAS_PREBUILT_MANPAGES, test "x$HAS_PREBUILT_MANPAGES" = "xyes")

AC_CHECK_LIB(rt, clock_gettime,
        LIB_CLOCK_GETTIME=-lrt
        AC_DEFINE(HAVE_CLOCK_GETTIME)
        AH_TEMPLATE(HAVE_CLOCK_GETTIME, [define if you have clock_gettime]))
AC_SUBST(LIB_CLOCK_GETTIME)

XIPH_GCC_VERSION dnl Sets a non-zero GCC_XXX_VERSION for gcc, not clang. checks below rely on that..

if test x$ac_cv_c_compiler_gnu = xyes -o x$xiph_cv_c_compiler_clang = xyes ; then
	dnl Prepend defaults to CFLAGS for GCC and Clang
	AS_IF([test "x${ax_enable_debug}" = "xno"], [
		CFLAGS="-O3 -funroll-loops $CFLAGS"
		CXXFLAGS="-O3 $CXXFLAGS"
	])

	CFLAGS="$CFLAGS -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wnested-externs -Wshadow -Wundef -Wmissing-declarations -Winline " # -Wcast-qual -Wbad-function-cast -Wwrite-strings -Wconversion
	CXXFLAGS="$CXXFLAGS -Wall -Wextra -Wcast-align -Wshadow -Wwrite-strings -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wsign-promo -Wundef " # -Wcast-qual -Wbad-function-cast -Wwrite-strings -Woverloaded-virtual -Wmissing-declarations

	XIPH_ADD_CFLAGS([-Wdeclaration-after-statement])

	dnl some distributions (such as Gentoo) have _FORTIFY_SOURCE always
	dnl enabled. We test for this situation in order to prevent polluting
	dnl the console with messages of macro redefinitions.

	if test "$enable_fortify_source" = "yes" && test "x$mingw_has_memset_chk" != "xno"
	then
		AX_ADD_FORTIFY_SOURCE
	fi

	AC_LANG_PUSH([C++])
	XIPH_ADD_CXXFLAGS([-Weffc++])
	AC_LANG_POP([C++])

	if test x$xiph_cv_c_compiler_clang = xyes -a "$OBJ_FORMAT" = elf; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
	elif test "$GCC_MAJOR_VERSION" -ge 4 && test "$OBJ_FORMAT" = elf; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
		fi


	if test x$xiph_cv_c_compiler_clang = xyes -a "$OBJ_FORMAT" = macho; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
	elif test "$GCC_MAJOR_VERSION" -ge 4 && test "$OBJ_FORMAT" = macho; then
		CPPFLAGS="$CPPFLAGS -DFLAC__USE_VISIBILITY_ATTR"
		CFLAGS="$CFLAGS -fvisibility=hidden"
		CXXFLAGS="$CXXFLAGS -fvisibility=hidden"
		fi

	if test "x$GCC_MAJOR_VERSION$GCC_MINOR_VERSION" = "x42" ; then
		XIPH_ADD_CFLAGS([-fgnu89-inline])
		fi

	if test "x$GCC_MAJOR_VERSION$GCC_MINOR_VERSION" = "x47" ; then
		XIPH_ADD_CFLAGS([-fno-inline-small-functions])
		fi

	fi

case "$host_os" in
	"mingw32"|"os2")
	if test "$host_cpu" = "i686"; then
		XIPH_ADD_CFLAGS([-mstackrealign])
		fi
	esac

if test x$enable_werror = "xyes" ; then
	XIPH_ADD_CFLAGS([-Werror])
	AC_LANG_PUSH([C++])
	XIPH_ADD_CXXFLAGS([-Werror])
	AC_LANG_POP([C++])
	fi

if test x$enable_stack_smash_protection = "xyes" && test "x$mingw_has_stack_chk_fail" != "xno" ; then
	XIPH_GCC_STACK_PROTECTOR
	XIPH_GXX_STACK_PROTECTOR
else
	enable_stack_smash_protection=no
fi

AH_VERBATIM([FLAC_API_EXPORTS],
[/* libtool defines DLL_EXPORT for windows dll builds,
   but flac code relies on FLAC_API_EXPORTS instead. */
#ifdef DLL_EXPORT
#ifdef __cplusplus
# define FLACPP_API_EXPORTS
#else
# define FLAC_API_EXPORTS
#endif
#endif])

if test x$enable_static = "xyes" ; then
dnl for correct FLAC_API
	CPPFLAGS="-DFLAC__NO_DLL $CPPFLAGS"
	fi

AC_CHECK_PROG(GIT_FOUND,git,yes)

if test x$GIT_FOUND$enable_version_from_git = "xyesyes"; then
	GIT_COMMIT_TAG=`git -C $srcdir --git-dir=.git describe --tags --exact-match 2>/dev/null`
	GIT_COMMIT_HASH=`git -C $srcdir --git-dir=.git log -1 --pretty=format:%h 2>/dev/null`
	GIT_COMMIT_DATE=`git -C $srcdir --git-dir=.git log -1 --pretty=format:%cd --date=format:%Y%m%d 2>/dev/null`
	if test ${#GIT_COMMIT_HASH} = 8 && test ${#GIT_COMMIT_DATE} = 8; then
		GIT_COMMIT_VERSION_AVAIL=yes
		if test ${#GIT_COMMIT_TAG} != 0 ; then
			GIT_COMMIT_TAG_AVAIL=yes
			AC_DEFINE_UNQUOTED(GIT_COMMIT_TAG, "${GIT_COMMIT_TAG}", "Define to the tag of the current git HEAD")
		else
			GIT_COMMIT_VERSION_HASH=yes
		fi
		AC_DEFINE_UNQUOTED(GIT_COMMIT_HASH, "${GIT_COMMIT_HASH}", "Define to the short hash of the current git HEAD")
		AC_DEFINE_UNQUOTED(GIT_COMMIT_DATE, "${GIT_COMMIT_DATE}", "Define to the commit date of the current git HEAD")
	fi
fi

AC_SUBST(GIT_COMMIT_VERSION_HASH)

AX_CHECK_COMPILE_FLAG([-fassociative-math -fno-signed-zeros -fno-trapping-math -freciprocal-math],HAVE_ASSOC_MATH=yes)
AM_CONDITIONAL(ASSOC_MATH_AVAILABLE, test "x$HAVE_ASSOC_MATH" = "xyes")


AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/libFLAC/Makefile \
	src/libFLAC/flac.pc \
	src/libFLAC/include/Makefile \
	src/libFLAC/include/private/Makefile \
	src/libFLAC/include/protected/Makefile \
	src/libFLAC++/Makefile \
	src/libFLAC++/flac++.pc \
	src/flac/Makefile \
	src/metaflac/Makefile \
	src/share/Makefile \
	src/test_grabbag/Makefile \
	src/test_grabbag/cuesheet/Makefile \
	src/test_grabbag/picture/Makefile \
	src/test_libs_common/Makefile \
	src/test_libFLAC/Makefile \
	src/test_libFLAC++/Makefile \
	src/test_seeking/Makefile \
	src/test_streams/Makefile \
	src/utils/Makefile \
	src/utils/flacdiff/Makefile \
	src/utils/flactimer/Makefile \
	examples/Makefile \
	examples/c/Makefile \
	examples/c/decode/Makefile \
	examples/c/decode/file/Makefile \
	examples/c/encode/Makefile \
	examples/c/encode/file/Makefile \
	examples/cpp/Makefile \
	examples/cpp/decode/Makefile \
	examples/cpp/decode/file/Makefile \
	examples/cpp/encode/Makefile \
	examples/cpp/encode/file/Makefile \
	include/Makefile \
	include/FLAC/Makefile \
	include/FLAC++/Makefile \
	include/share/Makefile \
	include/share/grabbag/Makefile \
	include/test_libs_common/Makefile \
	doc/Doxyfile \
	doc/Makefile \
	doc/images/Makefile \
	m4/Makefile \
	man/Makefile \
	test/common.sh \
	test/Makefile \
	test/cuesheets/Makefile \
	test/foreign-metadata-test-files/Makefile \
	test/flac-to-flac-metadata-test-files/Makefile \
	test/metaflac-test-files/Makefile \
	test/pictures/Makefile \
	microbench/Makefile \
	oss-fuzz/Makefile
])
AC_OUTPUT

AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    FLAC version : ............................ ${VERSION}

    Host CPU : ................................ ${host_cpu}
    Host Vendor : ............................. ${host_vendor}
    Host OS : ................................. ${host_os}
])

if test x$GIT_COMMIT_TAG_AVAIL = xyes ; then
	echo "    Version string : reference libFLAC ${GIT_COMMIT_TAG} ${GIT_COMMIT_DATE}"
elif test x$GIT_COMMIT_VERSION_AVAIL = xyes ; then
	echo "    Version string : reference libFLAC git-${GIT_COMMIT_HASH} ${GIT_COMMIT_DATE}"
fi
	echo ""
	echo "    Compiler is GCC : ......................... ${ac_cv_c_compiler_gnu}"
if test x$ac_cv_c_compiler_gnu = xyes ; then
	echo "    GCC version : ............................. ${GCC_VERSION}"
fi
	echo "    Compiler is Clang : ....................... ${xiph_cv_c_compiler_clang}"
	echo "    Asm optimizations : ....................... ${asm_optimisation}"
	echo "    Ogg/FLAC support : ........................ ${have_ogg}"
	echo "    Stack protector  : ........................ ${enable_stack_smash_protection}"
	echo "    Fuzzing support (Clang only) : ............ ${have_oss_fuzzers}"
echo
