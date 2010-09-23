#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([MMOServer], [2.0.0], [http://github.com/swganh/mmoserver/issues])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_PREREQ([2.2])
LT_INIT([dlopen])

AC_LANG([C++])

AC_CONFIG_MACRO_DIR([tools/m4])
AC_CONFIG_SRCDIR([README])
AC_CONFIG_HEADERS([config.h])

# Configure options: --enable-debug[=no].
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
    [enable debug code (default is no)])],
  [debug="$withval"], [debug=no])

##########################################################################
# adding the tbb library (e.g. with static name 'libtbb.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of tbb
AC_ARG_WITH([tbb],
    [AS_HELP_STRING([--with-tbb=prefix],
        [try this for a non-standard install prefix of the tbb library])],
    [TBBPATHSET=1],
    [TBBPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $TBBPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_tbb/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_tbb/lib"
fi

##########################################################################

##########################################################################
# adding the glm library
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of glm
AC_ARG_WITH([glm],
    [AS_HELP_STRING([--with-glm=prefix],
        [try this for a non-standard install prefix of the glm library])],
    [GLMPATHSET=1],
    [GLMPATHSET=0])
	
# if optional parameter used, extend path flags for compliler and linker
if test $GLMPATHSET = 1 ; then
    # extend the compiler flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_glm/include"
fi

##########################################################################

##########################################################################
# adding the glog library (e.g. with static name 'libglog.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of XXX
AC_ARG_WITH([glog],
    [AS_HELP_STRING([--with-glog=prefix],
        [try this for a non-standard install prefix of the glog library])],
    [GLOGPATHSET=1],
    [GLOGPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $GLOGPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_glog/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_glog/lib"
fi

##########################################################################

##########################################################################
# adding the gtest library (e.g. with static name 'libgtest.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of gtest
AC_ARG_WITH([gtest],
    [AS_HELP_STRING([--with-gtest=prefix],
        [try this for a non-standard install prefix of the gtest library])],
    [GTESTPATHSET=1],
    [GTESTPATHSET=0])
	
# if optional parameter used, extend path flags for compliler and linker
if test $GTESTPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_gtest/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_gtest/lib"
fi

##########################################################################

##########################################################################
# adding the mysqlcppconn library (e.g. with static name 'libmysqlcppconn.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of mysqlcppconn
AC_ARG_WITH([mysqlcppconn],
    [AS_HELP_STRING([--with-mysqlcppconn=prefix],
        [try this for a non-standard install prefix of the mysqlcppconn library])],
    [MYSQLCPPCONNPATHSET=1],
    [MYSQLCPPCONNPATHSET=0])
	
# if optional parameter used, extend path flags for compliler and linker
if test $MYSQLCPPCONNPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_mysqlcppconn/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_mysqlcppconn/lib"
fi

##########################################################################

# store current user given compiler flags to avoid default setup via AC_PROG_CXX
OLD_CXXFLAGS=$CXXFLAGS

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP

# Check for doxygen program.
AC_PROG_TRY_DOXYGEN
if test -z "$DOXYGEN"; then
  echo "-----------------------------------------"
  echo " No Doxygen program found - continuing"
  echo " without Doxygen documentation support."
  echo "-----------------------------------------"
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test "x$DOXYGEN" != x])

# reset compiler flags to initial flags
CXXFLAGS=$OLD_CXXFLAGS
  
AC_PROG_CC

# Checks for libraries.
AX_BOOST_BASE([1.44])
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
AX_BOOST_THREAD

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h memory.h stddef.h stdlib.h string.h sys/socket.h sys/time.h termios.h unistd.h wchar.h wctype.h])
AX_CXX_HEADER_STDCXX_0X

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AX_CXX_COMPILE_STDCXX_0X

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit floor gettimeofday inet_ntoa localtime_r memset pow socket sqrt strchr strstr])
AX_LIB_MYSQL

# If native C++0x support is not provided then a compiler flag
# is required for g++ to compile the source. Check if the two
# compiler options are supported and add one to the CXXFLAGS,
# prefer standard c++ over gnu
if test "$ax_cv_cxx_compile_cxx0x_native" = no; then
  if test "$ax_cv_cxx_compile_cxx0x_cxx" = yes; then
    AM_CXXFLAGS="${AM_CXXFLAGS} -std=c++0x"
  elif test "$ax_cv_cxx_compile_cxx0x_gxx" = yes; then
    AM_CXXFLAGS="${AM_CXXFLAGS} -std=gnu++0x"
  fi
fi

##########################################################################
# check for tbb library (Intel Thread Building Blocks)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -ltbb"

# check for tbb library headers   
AC_MSG_CHECKING([for the tbb library headers])
# try to compile a file that includes a header of the library tbb
AC_COMPILE_IFELSE([[#include <tbb/atomic.h>]],
    [AC_MSG_RESULT([found])
        # try to link the function 'tbb::atomic< T >' out of library glog
        AC_MSG_CHECKING([whether the tbb library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <tbb/atomic.h>]],
                [[tbb::atomic<int> some_int;]])],
            [AC_MSG_RESULT([yes])
                FOUND_TBB=1;],
            [AC_MSG_RESULT([no])
                LIBS=$OLD_LIBS; dnl reset to old value since tbb was not found
                FOUND_TBB=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_TBB=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_TBB != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The tbb library was not found!])
    if test  $TBBPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_tbb' for the tbb library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-tbb=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([ tbb library is an essential dependency : cannot build and stop here !])
fi

##########################################################################

##########################################################################
# check for glm headers (OpenGL Mathmatics library)
##########################################################################

# store current CXXFLAGS and merge with AM_CXXFLAGS for compilation check   
OLD_CXXFLAGS=$CXXFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"

# check for glm library headers   
AC_MSG_CHECKING([for the glm library headers])
# try to compile a file that includes a header of the library glm
AC_COMPILE_IFELSE([[#include <glm/glm.hpp>]],
    [AC_MSG_RESULT([found])
        FOUND_GLM=1;],
    [AC_MSG_RESULT([not found])
        FOUND_GLM=0;])

# reset original CXXFLAGS
CXXFLAGS=$OLD_CXXFLAGS

# handle results
if test $FOUND_GLM != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The glm library was not found!])
    if test  $GLMPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_glm' for the glm library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-glm=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([glm library is an essential dependency : cannot build and stop here !])
fi

##########################################################################

##########################################################################
# check for glog library (google log library)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lglog"

# check for glog library headers   
AC_MSG_CHECKING([for the glog library headers])
# try to compile a file that includes a header of the library glog
AC_COMPILE_IFELSE([[#include <glog/logging.h>]],
    [AC_MSG_RESULT([found])
        # try to link the function 'google::InitGoogleLogging' out of library glog
        AC_MSG_CHECKING([whether the glog library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <glog/logging.h>]],
                [[google::InitGoogleLogging("");]])],
            [AC_MSG_RESULT([yes])
                FOUND_GLOG=1;],
            [AC_MSG_RESULT([no])
                LIBS=$OLD_LIBS; dnl reset to old value since glog was not found
                FOUND_GLOG=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_GLOG=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_GLOG != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The glog library was not found!])
    if test  $GLOGPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_glog' for the glog library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-glog=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([ glog library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


##########################################################################
# check for gtest library (google test library)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lgtest"

# check for gtest library headers   
AC_MSG_CHECKING([for the gtest library headers])
# try to compile a file that includes a header of the library gtest
AC_COMPILE_IFELSE([[#include <gtest/gtest.h>]],
    [AC_MSG_RESULT([found])
        # try to link the function 'testing::InitGoogleTest' out of library gtest
        AC_MSG_CHECKING([whether the gtest library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <gtest/gtest.h>]],
                [[int argc = 0; char **argv; testing::InitGoogleTest(&argc, argv);]])],
            [AC_MSG_RESULT([yes])
                FOUND_GTEST=1;],
            [AC_MSG_RESULT([no])
                FOUND_GTEST=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_GTEST=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_GTEST != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The gtest library was not found!])
    if test  $GTESTPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_gtest' for the gtest library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-gtest=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([ gtest library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


##########################################################################
# check for mysqlcppconn library (Mysql Connector/C++ library)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lmysqlcppconn"

# check for glog library headers   
AC_MSG_CHECKING([for the mysqlcppconn library headers])
# try to compile a file that includes a header of the library mysqlcppconn
AC_COMPILE_IFELSE([[#include <cppconn/driver.h>]],
    [AC_MSG_RESULT([found])
        # try to link the function 'get_driver_instance' out of library mysqlcppconn
        AC_MSG_CHECKING([whether the mysqlcppconn library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <mysql_driver.h>]],
                [[get_driver_instance();]])],
            [AC_MSG_RESULT([yes])
                FOUND_MYSQLCPPCONN=1;],
            [AC_MSG_RESULT([no])
                LIBS=$OLD_LIBS; dnl reset to old value since mysqlcppconn was not found
                FOUND_MYSQLCPPCONN=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_MYSQLCPPCONN=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_MYSQLCPPCONN != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The mysqlcppconn library was not found!])
    if test  $MYSQLCPPCONNPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_mysqlcppconn' for the mysqlcppconn library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-mysqlcppconn=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([ mysqlcppconn library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


# Configure DEBUG source code, if requested.
if test "x$debug" = xyes; then
  AC_DEFINE([_DEBUG], [], [Define to enable Server Directory debug mode features])
  AM_CXXFLAGS="${AM_CXXFLAGS} -g -Wall -Werror -Wno-uninitialized -O0"
else
  AC_DEFINE([NDEBUG], [], [Define to enable Server Directory release mode features])
  AM_CXXFLAGS="${AM_CXXFLAGS} -O3"
fi

# distribute the changed variables among the Makefiles
AC_SUBST([LIBS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

AC_CONFIG_FILES([Makefile
                 docs/Makefile
                 src/ChatServer/Makefile
                 src/Common/Makefile
                 src/ConnectionServer/Makefile
                 src/DatabaseManager/Makefile
                 src/LoginServer/Makefile
                 src/Makefile
                 src/MathLib/Makefile
                 src/MessageLib/Makefile
                 src/NetworkManager/Makefile
                 src/PingServer/Makefile
                 src/Utils/Makefile
                 src/ZoneServer/Makefile])
AC_OUTPUT
echo "
  ($PACKAGE_NAME) v$PACKAGE_VERSION
  Prefix..........: $prefix
  Debug Build.....: $debug
  C++ Compiler....: $CXX $AM_CXXFLAGS $DEFS $DEFAULT_INCLUDES $INCLUDES
  Linker..........: $LD $LDFLAGS $LIBS
  GLOG Compiler...: ${GLOGINC:-INSTALLED}
  GLOG Linker.....: ${GLOGLIB:-INSTALLED}
  GTEST Compiler..: ${GTESTINC:-INSTALLED}
  GTEST Linker....: ${GTESTLIB:-INSTALLED}
  cppconn Compiler: ${CPPCONNINC:-INSTALLED}
  cppconn Linker..: ${CPPCONNLIB:-INSTALLED}
  Boost Compiler..: $BOOST_CPPFLAGS
  Boost Linker....: $BOOST_LDFLAGS $BOOST_SYSTEM_LIB $BOOST_THREAD_LIB $BOOST_PROGRAM_OPTIONS_LIB
  MySQL Compiler..: $MYSQL_CFLAGS
  MySQL Linker....: $MYSQL_LDFLAGS
"