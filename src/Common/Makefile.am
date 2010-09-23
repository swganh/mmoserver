AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@
  
# Common library - noinstall shared library
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = ApplicationService.cpp \
  atMacroString.cpp \
  BuildInfo.cpp \
  byte_buffer.cc \
  ConfigFile.cpp \
  ConfigManager.cpp \
  Crc.cpp \
  Event.cpp \
  EventDispatcher.cpp \
  HashString.cpp \
  LogManager.cpp \
  OutOfBand.cpp
    
libcommon_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors

TESTS=
check_PROGRAMS=

TESTS += tests/byte_buffer
check_PROGRAMS += tests/byte_buffer
tests_byte_buffer_SOURCES = byte_buffer_unittest.cc
tests_byte_buffer_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_byte_buffer_CPPFLAGS = $(GTEST_INCLUDE) -Wall -pedantic-errors -Wfatal-errors

TESTS += tests/event
check_PROGRAMS += tests/event
tests_event_SOURCES = event_unittest.cc
tests_event_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_event_CPPFLAGS = $(GTEST_INCLUDE) -Wall -pedantic-errors -Wfatal-errors

TESTS += tests/event_dispatcher
check_PROGRAMS += tests/event_dispatcher
tests_event_dispatcher_SOURCES = event_dispatcher_unittest.cc
tests_event_dispatcher_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la \
  $(top_builddir)/src/Utils/libutils.la
tests_event_dispatcher_CPPFLAGS = $(GTEST_INCLUDE) -Wall -pedantic-errors -Wfatal-errors

TESTS += tests/hash_string
check_PROGRAMS += tests/hash_string
tests_hash_string_SOURCES = hash_string_unittest.cc
tests_hash_string_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_hash_string_CPPFLAGS = $(GTEST_INCLUDE) -Wall -pedantic-errors -Wfatal-errors
