AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# Utils library - noinstall shared library
noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = \
  bstring.cpp \
  clock.cpp \
  EventHandler.cpp \
  rand.cpp \
  Scheduler.cpp \
  StreamColors.cpp \
  Timer.cpp \
  utils.cpp \
  VariableTimeScheduler.cpp

libutils_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar