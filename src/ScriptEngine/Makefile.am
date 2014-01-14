AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# ScriptEngine library - noinstall shared library
noinst_LTLIBRARIES = libscriptengine.la
libscriptengine_la_SOURCES = \
	Script.cpp \
  ScriptEngine.cpp \
  ScriptEngineLib.cpp \
  ScriptEventListener.cpp \
  ScriptSupport.cpp \
  glue_files/LuaInterface.cpp

libscriptengine_la_CPPFLAGS = $(LUA_INCLUDE) -I$(top_srcdir)/deps/noise/src -Wall -pedantic-errors -Wfatal-errors -fshort-wchar -Wno-write-strings