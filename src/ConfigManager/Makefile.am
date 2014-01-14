AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# ConfigManager library - noinstall shared library
noinst_LTLIBRARIES = libconfigmanager.la
libconfigmanager_la_SOURCES = \
	ConfigFile.cpp \
  ConfigManager.cpp
    
libconfigmanager_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar
libconfigmanager_la_LIBADD = ../Common/libcommon.la 