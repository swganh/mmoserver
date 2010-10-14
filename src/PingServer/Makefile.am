AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# PingServer - executable
bin_PROGRAMS = pingserver
pingserver_SOURCES = PingServer.cpp

pingserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -pedantic-errors -Wfatal-errors -fno-strict-aliasing -Wno-overflow
pingserver_LDADD = $(MYSQL_LDFLAGS) \
  $(BOOST_LDFLAGS) \
  $(BOOST_THREAD_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(top_builddir)/src/Utils/libutils.la \
  $(top_builddir)/src/Common/libcommon.la \
  $(top_builddir)/src/DatabaseManager/libdatabasemanager.la