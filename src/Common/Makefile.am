AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@
  
# Common library - noinstall shared library
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = ApplicationService.cc \
  atMacroString.cc \
  BuildInfo.cc \
  byte_buffer.cc \
  ConfigFile.cc \
  ConfigManager.cc \
  Crc.cc \
  Event.cc \
  EventDispatcher.cc \
  HashString.cc \
  LogManager.cc \
  OutOfBand.cc
    
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
tests_byte_buffer_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/event
check_PROGRAMS += tests/event
tests_event_SOURCES = event_unittest.cc
tests_event_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_event_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/event_dispatcher
check_PROGRAMS += tests/event_dispatcher
tests_event_dispatcher_SOURCES = event_dispatcher_unittest.cc
tests_event_dispatcher_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  -ltbb \
  libcommon.la \
  $(top_builddir)/src/Utils/libutils.la
tests_event_dispatcher_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/hash_string
check_PROGRAMS += tests/hash_string
tests_hash_string_SOURCES = hash_string_unittest.cc
tests_hash_string_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_hash_string_CPPFLAGS = $(GTEST_INCLUDE)

TESTS += tests/out_of_band
check_PROGRAMS += tests/out_of_band
tests_out_of_band_SOURCES = out_of_band_unittest.cc
tests_out_of_band_LDADD = -lgtest_main \
  $(BOOST_LDFLAGS) \
  $(BOOST_DATE_TIME_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(BOOST_THREAD_LIB) \
  libcommon.la
tests_out_of_band_CPPFLAGS = $(GTEST_INCLUDE)
