#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([swganh], [0.0.0], [BUG-REPORT-ADDRESS])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror])
LT_PREREQ([2.2])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIR([tools/m4])
AC_CONFIG_SRCDIR([src/AdminServer/AdminServer.cpp])
AC_CONFIG_HEADERS([config.h])

# Create a container to store custom flags (used by automake files)
SWGANH_CXXFLAGS=""

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LEX
AC_PROG_MAKE_SET
AC_PROG_YACC

# Checks for libraries.
AX_LIB_MYSQL
ACX_PTHREAD
AX_BOOST_BASE([1.38])
AX_BOOST_ASIO
AX_BOOST_SYSTEM
AX_BOOST_THREAD
CHECK_ZLIB
AX_WITH_LUA
AX_LUA_LIBS

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h locale.h memory.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/timeb.h unistd.h])
AC_CXX_HEADER_STDCXX_TR1
AX_LUA_HEADERS

if test "$ac_cv_cxx_stdcxx_tr1" = no; then
  AC_MSG_ERROR([C++ TR1 support required to build this application.])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_C_INLINE
AC_C_RESTRICT
AC_CXX_COMPILE_STDCXX_0X

if test "$ac_cv_cxx_compile_cxx0x_native" = no &&
   test "$ac_cv_cxx_compile_cxx0x_cxx" = no &&
   test "$ac_cv_cxx_compile_cxx0x_gxx" = no; then
  AC_MSG_ERROR([C++0x features required to build this application.])
fi

# If native C++0x support is not provided then a compiler flag
# is required for g++ to compile the source. Check if the two
# compiler options are supported and add one to the CXXFLAGS,
# prefer standard c++ over gnu
if test "$ac_cv_cxx_compile_cxx0x_native" = no; then
  if test "$ac_cv_cxx_compile_cxx0x_gxx" = yes; then
    SWGANH_CXXFLAGS="-std=gnu++0x"
  elif test "$ac_cv_cxx_compile_cxx0x_cxx" = yes; then
    SWGANH_CXXFLAGS="-std=c++0x"
  fi
fi

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit floor ftime inet_ntoa memset pow select socket sqrt strchr strdup strstr])


# Preserve variables to use in automake files
AC_SUBST(SWGANH_CXXFLAGS)


AC_CONFIG_FILES([Makefile
	docs/Makefile
	src/Makefile
	src/AdminServer/Makefile
	src/ChatServer/Makefile
	src/Common/Makefile
	src/ConfigManager/Makefile
	src/ConnectionServer/Makefile
	src/DatabaseManager/Makefile
	src/LoginServer/Makefile
	src/LogManager/Makefile
	src/MathLib/Makefile
	src/MessageLib/Makefile
	src/NetworkManager/Makefile
	src/PingServer/Makefile
	src/ScriptEngine/Makefile
	src/Utils/Makefile
	src/ZoneServer/Makefile
	tests/Makefile
	tests/Utils/Makefile])
AC_OUTPUT

echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix.........: $prefix
  C++ Compiler...: $CXX $CXXFLAGS $CPPFLAGS $SWGANH_CXXFLAGS 
                   $BOOST_CPPFLAGS $MYSQL_CFLAGS
  Linker.........: $LD $LDFLAGS $BOOST_LDFLAGS 
                   $BOOST_SYSTEM_LIB
                   $BOOST_THREAD_LIB
                   $MYSQL_LDFLAGS
"
