#
# Check for supported __attribute__ features
#
# AC_C_ATTRIBUTE(FEATURE, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
AC_DEFUN([AC_C_ATTRIBUTE],
[AS_VAR_PUSHDEF([CACHEVAR], [ax_cv_c_attribute_$1])dnl
AC_CACHE_CHECK([for  __attribute__ (($1))],
  CACHEVAR,[
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
    [[ void foo(void) __attribute__ (($1)); ]])],
    [AS_VAR_SET(CACHEVAR, [yes])],
    [AS_VAR_SET(CACHEVAR, [no])])])
AS_VAR_IF(CACHEVAR,yes,
  [m4_default([$2], :)],
  [m4_default([$3], :)])
AS_VAR_POPDEF([CACHEVAR])dnl
])dnl
