AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)
  
# Common library - noinstall shared library
noinst_LTLIBRARIES = libcommon.la
libcommon_la_SOURCES = \
	atMacroString.cpp \
  BuildInfo.cpp \
  bytebuffer.cpp \
  DispatchClient.cpp \
  MessageDispatch.cpp \
  MessageFactory.cpp 
    
libcommon_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar