# autoconf source script for generating configure

dnl The package_version file will be automatically synced to the git revision
dnl by the update_version script when configured in the repository, but will
dnl remain constant in tarball releases unless it is manually edited.
m4_define([CURRENT_VERSION],
          m4_esyscmd([ ./update_version 2>/dev/null || true
                       if test -e package_version; then
                           . ./package_version
                           printf "$PACKAGE_VERSION"
                       else
                           printf "unknown"
                       fi ]))

AC_INIT([libopusenc],[CURRENT_VERSION],[opus@xiph.org])
AC_CONFIG_SRCDIR([src/opusenc.c])
AC_CONFIG_MACRO_DIR([m4])

AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE

AM_INIT_AUTOMAKE([1.11 foreign no-define dist-zip subdir-objects])
AM_MAINTAINER_MODE([enable])

AC_C_INLINE

LT_INIT

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_DEFINE([OPE_BUILD], [], [This is a build of libopusenc])

dnl Library versioning for libtool.
dnl Please update these for releases.
dnl CURRENT, REVISION, AGE
dnl - library source changed -> increment REVISION
dnl - interfaces added/removed/changed -> increment CURRENT, REVISION = 0
dnl - interfaces added -> increment AGE
dnl - interfaces removed -> AGE = 0

OP_LT_CURRENT=4
OP_LT_REVISION=2
OP_LT_AGE=4

AC_SUBST(OP_LT_CURRENT)
AC_SUBST(OP_LT_REVISION)
AC_SUBST(OP_LT_AGE)

CC_CHECK_CFLAGS_APPEND(
  [-pedantic -Wall -Wextra -Wno-sign-compare -Wno-parentheses -Wno-long-long])

# Platform-specific tweaks
case $host in
  *-mingw*)
    # -std=c89 causes some warnings under mingw.
    CC_CHECK_CFLAGS_APPEND([-U__STRICT_ANSI__])
    # We need WINNT>=0x501 (WindowsXP) for getaddrinfo/freeaddrinfo.
    # It's okay to define this even when HTTP support is disabled, as it only
    #  affects header declarations, not linking (unless we actually use some
    #  XP-only functions).
    AC_DEFINE_UNQUOTED(_WIN32_WINNT,0x501,
     [We need at least WindowsXP for getaddrinfo/freeaddrinfo])
    host_mingw=true
    ;;
esac
AM_CONDITIONAL(OP_WIN32, test "$host_mingw" = "true")

dnl AC_ARG_ENABLE([assertions],
dnl   AS_HELP_STRING([--enable-assertions], [Enable assertions in code]),,
dnl   enable_assertions=no)

AS_IF([test "$enable_assertions" = "yes"], [
  AC_DEFINE([OP_ENABLE_ASSERTIONS], [1], [Enable assertions in code])
])

m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal([Could not locate the pkg-config autoconf macros.
Please make sure pkg-config is installed and, if necessary, set the environment
variable ACLOCAL="aclocal -I/path/to/pkg.m4".])])

PKG_CHECK_MODULES([DEPS], [opus >= 1.1])

dnl AC_ARG_ENABLE([fixed-point],
dnl   AS_HELP_STRING([--enable-fixed-point], [Enable fixed-point calculation]),,
dnl   enable_fixed_point=no)
dnl AC_ARG_ENABLE([float],
dnl   AS_HELP_STRING([--disable-float], [Disable floating-point API]),,
dnl   enable_float=yes)

AC_DEFINE([RESAMPLE_FULL_SINC_TABLE], [1], [Faster, takes more memory])

dnl AS_IF([test "$enable_float" = "no"],
dnl   [enable_fixed_point=yes
dnl    AC_DEFINE([OP_DISABLE_FLOAT_API], [1], [Disable floating-point API])
dnl   ]
dnl )

AS_IF([test "$enable_fixed_point" = "yes"],
  [AC_DEFINE([OP_FIXED_POINT], [1], [Enable fixed-point calculation])],
  [dnl This only has to be tested for if float->fixed conversions are required
   saved_LIBS="$LIBS"
   AC_SEARCH_LIBS([lrintf], [m], [
      AC_DEFINE([OP_HAVE_LRINTF], [1], [Enable use of lrintf function])
      lrintf_notice="
    Library for lrintf() ......... ${ac_cv_search_lrintf}"
   ])
   LIBS="$saved_LIBS"
  ]
)

AC_ARG_ENABLE([examples],
  AS_HELP_STRING([--disable-examples], [Do not build example applications]),,
  enable_examples=yes)
AM_CONDITIONAL([OP_ENABLE_EXAMPLES], [test "$enable_examples" = "yes"])

AS_CASE(["$ac_cv_search_lrintf"],
  ["no"],[],
  ["none required"],[],
  [lrintf_lib="$ac_cv_search_lrintf"])

AC_SUBST([lrintf_lib])

CC_ATTRIBUTE_VISIBILITY([default], [
  CC_FLAG_VISIBILITY([CFLAGS="${CFLAGS} -fvisibility=hidden"])
])

dnl Check for doxygen
AC_ARG_ENABLE([doc],
  AS_HELP_STRING([--disable-doc], [Do not build API documentation]),,
  [enable_doc=yes]
)

AS_IF([test "$enable_doc" = "yes"], [
  AC_CHECK_PROG([HAVE_DOXYGEN], [doxygen], [yes], [no])
  AC_CHECK_PROG([HAVE_DOT], [dot], [yes], [no])
],[
  HAVE_DOXYGEN=no
])

AM_CONDITIONAL([HAVE_DOXYGEN], [test "$HAVE_DOXYGEN" = "yes"])

AC_CONFIG_FILES([
  Makefile
  libopusenc.pc
  libopusenc-uninstalled.pc
  doc/Doxyfile
])
AC_CONFIG_HEADERS([config.h])
AC_OUTPUT

AC_MSG_NOTICE([
------------------------------------------------------------------------
  $PACKAGE_NAME $PACKAGE_VERSION: Automatic configuration OK.

dnl    Assertions ................... ${enable_assertions}
dnl
dnl    Fixed-point .................. ${enable_fixed_point}
dnl    Floating-point API ........... ${enable_float}${lrintf_notice}
dnl
    Hidden visibility ............ ${cc_cv_flag_visibility}

    API code examples ............ ${enable_examples}
    API documentation ............ ${enable_doc}
------------------------------------------------------------------------
])
