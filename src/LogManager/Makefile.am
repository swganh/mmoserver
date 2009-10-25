AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# LogManager library - noinstall shared library
noinst_LTLIBRARIES = liblogmanager.la
liblogmanager_la_SOURCES = \
	Log.cpp \
  LogManager.cpp
	
liblogmanager_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar