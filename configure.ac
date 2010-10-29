#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
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

##########################################################################
# adding the lua library (e.g. with static name 'liblua.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of XXX
AC_ARG_WITH([lua],
    [AS_HELP_STRING([--with-lua=prefix],
        [try this for a non-standard install prefix of the lua library])],
    [LUAPATHSET=1],
    [LUAPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $LUAPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_lua/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_lua/lib"
fi

##########################################################################

##########################################################################
# adding the tolua++ library (e.g. with static name 'libtolua++.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of XXX
AC_ARG_WITH([tolua],
    [AS_HELP_STRING([--with-tolua=prefix],
        [try this for a non-standard install prefix of the tolua library])],
    [TOLUAPATHSET=1],
    [TOLUAPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $TOLUAPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_tolua/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_tolua/lib"
fi

##########################################################################

##########################################################################
# adding the spatialindex library (e.g. with static name 'libspatialindex.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of XXX
AC_ARG_WITH([spatialindex],
    [AS_HELP_STRING([--with-spatialindex=prefix],
        [try this for a non-standard install prefix of the spatialindex library])],
    [SPATIALINDEXPATHSET=1],
    [SPATIALINDEXPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $SPATIALINDEXPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_spatialindex/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_spatialindex/lib"
fi

##########################################################################

##########################################################################
# adding the noise library (e.g. with static name 'libnoise.a')
##########################################################################

# introduce the optional configure parameter for a non-standard install prefix of XXX
AC_ARG_WITH([noise],
    [AS_HELP_STRING([--with-noise=prefix],
        [try this for a non-standard install prefix of the noise library])],
    [NOISEPATHSET=1],
    [NOISEPATHSET=0])

# if optional parameter used, extend path flags for compliler and linker
if test $NOISEPATHSET = 1 ; then
    # extend the compiler and linker flags according to the path set
    AM_CXXFLAGS="$AM_CXXFLAGS -I$with_noise/include"
    AM_LDFLAGS="$AM_LDFLAGS -L$with_noise/lib"
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
AX_BOOST_REGEX
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
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <tbb/atomic.h>
    ])],
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
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <glm/glm.hpp>
    ])],
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
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <glog/logging.h>
    ])],
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
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <gtest/gtest.h>
    ])],
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
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
	#include <mysql_driver.h>
    #include <cppconn/driver.h>
    ])],
    [AC_MSG_RESULT([found])
        # try to link the function 'get_driver_instance' out of library mysqlcppconn
        AC_MSG_CHECKING([whether the mysqlcppconn library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <mysql_driver.h>]],
                [[get_driver_instance()]])],
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


##########################################################################
# check for lua library (lua library)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -llua"

AC_LANG([C])

# check for glog library headers   
AC_MSG_CHECKING([for the lua library headers])
# try to compile a file that includes a header of the library glog
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <lua.h>
    ])],
    [AC_MSG_RESULT([found])
        # try to link the function 'lua_open' out of library lua
        AC_MSG_CHECKING([whether the lua library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[
                #include <lua.h>
                #include <lualib.h>
                #include <lauxlib.h>
                ]],
                [[lua_open()]])],
            [AC_MSG_RESULT([yes])
                FOUND_LUA=1;],
            [AC_MSG_RESULT([no])
                LIBS=$OLD_LIBS; dnl reset to old value since lua was not found
                FOUND_LUA=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_LUA=0;])

AC_LANG([C++])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_LUA != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The lua library was not found!])
    if test  $LUAPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_lua' for the lua library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-lua=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([lua library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


##########################################################################
# check for tolua library (tolua library)
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_CPPFLAGS=$CPPFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
CPPFLAGS="$AM_CXXFLAGS $CPPFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -llua -ltolua++"

# check for tolua++ library headers   
AC_MSG_CHECKING([for the tolua++ library headers])
# try to compile a file that includes a header of the library tolua++
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <tolua++.h>
    ])],
    [AC_MSG_RESULT([found])
        # try to link the type 'tolua_iscppstring' out of library glog
        AC_MSG_CHECKING([whether the tolua++ library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <tolua++.h>]],
                [[tolua_Error e]])],
            [AC_MSG_RESULT([yes])
                FOUND_TOLUA=1;],
            [AC_MSG_RESULT([no])
                LIBS=$OLD_LIBS; dnl reset to old value since tolua was not found
                FOUND_TOLUA=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_TOLUA=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_TOLUA != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The tolua++ library was not found!])
    if test  $TOLUAPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_tolua' for the tolua++ library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-tolua=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([tolua++ library is an essential dependency : cannot build and stop here !])
fi

##########################################################################

##########################################################################
# check for spatialindex library
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lspatialindex"

# check for noise library headers   
AC_MSG_CHECKING([for the spatialindex library headers])
# try to compile a file that includes a header of the library spatialindex
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <spatialindex/SpatialIndex.h>
    ])],
    [AC_MSG_RESULT([found])
        # try to link the class 'SpatialIndex::Region' out of library spatialindex
        AC_MSG_CHECKING([whether the spatialindex library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <spatialindex/SpatialIndex.h>]],
                [[SpatialIndex::Region r;]])],
            [AC_MSG_RESULT([yes])
                FOUND_SPATIALINDEX=1;],
            [AC_MSG_RESULT([no])
                FOUND_SPATIALINDEX=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_SPATIALINDEX=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_SPATIALINDEX != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The spatialindex library was not found!])
    if test  $SPATIALINDEXPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_spatialindex' for the spatialindex library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-spatialindex=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([spatialindex library is an essential dependency : cannot build and stop here !])
fi

##########################################################################

##########################################################################
# check for noise library
##########################################################################

# store current *FLAGS and merge with AM_*FLAGS for compilation and linker check   
OLD_CXXFLAGS=$CXXFLAGS;
OLD_LDFLAGS=$LDFLAGS;
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

# ensure the library to check for is covered by the LIBS variable
OLD_LIBS=$LIBS
LIBS="$LIBS -lnoise"

# check for noise library headers   
AC_MSG_CHECKING([for the noise library headers])
# try to compile a file that includes a header of the library noise
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
    #include <noise/noise.h>
    ])],
    [AC_MSG_RESULT([found])
        # try to link the class 'noise::module::Perlin' out of library noise
        AC_MSG_CHECKING([whether the noise library can be linked])
        AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include <noise/noise.h>]],
                [[noise::module::Perlin p;]])],
            [AC_MSG_RESULT([yes])
                FOUND_NOISE=1;],
            [AC_MSG_RESULT([no])
                FOUND_NOISE=0;])],
    [AC_MSG_RESULT([not found])
        FOUND_NOISE=0;])

# reset original *FLAGS
LIBS=$OLD_LIBS
CXXFLAGS=$OLD_CXXFLAGS
LDFLAGS=$OLD_LDFLAGS

# handle check results
if test $FOUND_NOISE != 1; then
    AC_MSG_NOTICE([])
    AC_MSG_NOTICE([The noise library was not found!])
    if test  $NOISEPATHSET = 1 ; then
        AC_MSG_NOTICE([ The install prefix '$with_noise' for the noise library was set.])
        AC_MSG_NOTICE([ --> Maybe wrong ???])
    else
        AC_MSG_NOTICE([ No non-standard install prefix was set.])
        AC_MSG_NOTICE([ --> You might want to use '--with-noise=PREFIX' ?!?])
    fi
    AC_MSG_NOTICE([])
    AC_MSG_ERROR([noise library is an essential dependency : cannot build and stop here !])
fi

##########################################################################


# Configure options: --enable-debug[=no].
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
    [enable debug code (default is no)])],
  [_debug=yes], [_debug=no])
# Configure DEBUG source code, if requested.
if test "x$_debug" = xyes; then
  AC_DEFINE([_DEBUG], [], [Define to enable Server Directory debug mode features])
  AM_CXXFLAGS="${AM_CXXFLAGS} -g -Wall -pedantic-errors -Wfatal-errors -Werror -Wno-uninitialized -O0"
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
                 src/Makefile
                 src/ChatServer/Makefile
                 src/Common/Makefile
                 src/ConnectionServer/Makefile
                 src/DatabaseManager/Makefile
                 src/LoginServer/Makefile
                 src/MathLib/Makefile
                 src/MessageLib/Makefile
                 src/NetworkManager/Makefile
                 src/PingServer/Makefile
                 src/ScriptEngine/Makefile
                 src/SwgProtocol/Makefile
                 src/Utils/Makefile
                 src/ZoneServer/Makefile])
AC_OUTPUT
echo "
  ($PACKAGE_NAME) v$PACKAGE_VERSION
  Prefix..........: $prefix
  Debug Build.....: $_debug
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