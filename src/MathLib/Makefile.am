AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)
  
# Math library - noinstall shared library
noinst_LTLIBRARIES = libmath.la
libmath_la_SOURCES = \
	Quaternion.cpp \
  Vector3.cpp

libmath_la_CPPFLAGS = -Wall -pedantic-errors -Wfatal-errors -fshort-wchar