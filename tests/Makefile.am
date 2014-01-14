AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS)
    
# MMOServer tests
TESTS=mmoserver_tests
check_PROGRAMS = $(TESTS)
mmoserver_tests_SOURCES = main.cpp \
	Utils/TestCmpistr.cpp

mmoserver_tests_CPPFLAGS = $(GTEST_CPPFLAGS) -Wall -pedantic-errors -Wfatal-errors
mmoserver_tests_LDADD = ../src/Utils/libutils.la \
	Utils/libutils_tests.la \
  $(BOOST_LDFLAGS) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  $(GTEST_LIBS)