dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_INIT([fdk-aac], [2.0.3], [http://sourceforge.net/projects/opencore-amr/])
AC_CONFIG_AUX_DIR(.)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([tar-ustar foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Various options for configure
AC_ARG_ENABLE([example],
            [AS_HELP_STRING([--enable-example],
                [enable example encoding program (default is no)])],
            [example=$enableval], [example=no])

dnl Automake conditionals to set
AM_CONDITIONAL(EXAMPLE, test x$example = xyes)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
LT_INIT

AC_SEARCH_LIBS([sin], [m])

dnl soname version to use
dnl goes by ‘current[:revision[:age]]’ with the soname ending up as
dnl current.age.revision
FDK_AAC_VERSION=2:3:0

AS_IF([test x$enable_shared = xyes], [LIBS_PRIVATE=$LIBS], [LIBS_PUBLIC=$LIBS])
AC_SUBST(FDK_AAC_VERSION)
AC_SUBST(LIBS_PUBLIC)
AC_SUBST(LIBS_PRIVATE)

AC_CONFIG_FILES([Makefile
                 fdk-aac.pc])
AC_OUTPUT
