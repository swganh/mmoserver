AM_CXXFLAGS = -I$(top_srcdir)/src \
  $(SWGANH_CXXFLAGS) \
  $(BOOST_CPPFLAGS)
  
# MMOServer tests
TESTS=mmoserver_tests
check_PROGRAMS = $(TESTS)
mmoserver_tests_SOURCES = main.cpp

mmoserver_tests_CPPFLAGS = $(GTEST_CPPFLAGS)
mmoserver_tests_LDADD = ../src/Utils/libutils.la \
	Utils/libutils-tests.la \
  $(BOOST_LDFLAGS) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  $(GTEST_LIBS)