dnl @synopsis AC_PROG_TRY_DOXYGEN
dnl
dnl AC_PROG_TRY_DOXYGEN tests for an existing doxygen program.
dnl It sets or uses the environment variable DOXYGEN.
dnl
dnl You can use the DOXYGEN variable in your Makefile.in, with
dnl @DOXYGEN@.
dnl
dnl @category Doxygen
dnl @author John Calcote <john.calcote@gmail.com>
dnl @version 2008-06-24
dnl @license AllPermissive

AC_DEFUN([AC_PROG_TRY_DOXYGEN],[
AC_REQUIRE([AC_EXEEXT])dnl
test -z "$DOXYGEN" && AC_CHECK_PROGS([DOXYGEN], [doxygen$EXEEXT])dnl
])
