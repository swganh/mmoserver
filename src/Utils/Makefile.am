AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# Utils library - noinstall shared library
noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = ActiveObject.cpp \
  bstring.cpp \
  clock.cpp \
  EventHandler.cpp \
  MathFunctions.cpp \
  rand.cpp \
  Scheduler.cpp \
  StreamColors.cpp \
  Timer.cpp \
  utils.cpp \
  VariableTimeScheduler.cpp

libutils_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar

TESTS=
check_PROGRAMS=

TESTS += tests/active_object
check_PROGRAMS += tests/active_object
tests_active_object_SOURCES = active_object_unittest.cc
tests_active_object_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  -ltbb \
  libutils.la
tests_active_object_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/cmpistr
check_PROGRAMS += tests/cmpistr
tests_cmpistr_SOURCES = cmpistr_unittest.cc
tests_cmpistr_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libutils.la
tests_cmpistr_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/concurrent_queue
check_PROGRAMS += tests/concurrent_queue
tests_concurrent_queue_SOURCES = concurrent_queue_unittest.cc
tests_concurrent_queue_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libutils.la
tests_concurrent_queue_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/math_functionality
check_PROGRAMS += tests/math_functionality
tests_math_functionality_SOURCES = math_functions_unittest.cc
tests_math_functionality_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libutils.la
tests_math_functionality_CPPFLAGS = $(GTEST_INCLUDE)
