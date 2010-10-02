AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# ScriptEngine library - noinstall shared library
noinst_LTLIBRARIES = libscriptengine.la
libscriptengine_la_SOURCES = \
  Script.cpp \
  ScriptEngine.cpp \
  ScriptEngineLib.cpp \
  ScriptEventListener.cpp \
  ScriptSupport.cpp \
  glue_files/LuaInterface.cpp

libmessagelib_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -Wno-write-strings