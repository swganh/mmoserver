AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# ConnectionServer - executable
bin_PROGRAMS = connserver
connserver_SOURCES = \
	ClientManager.cpp \
	ConnectionDispatch.cpp \
	ConnectionServer.cpp \
	MessageRouter.cpp \
	ServerManager.cpp
	
connserver_CPPFLAGS = $(MYSQL_CFLAGS) -fno-strict-aliasing -Wno-overflow -Wno-invalid-offsetof
connserver_LDADD = $(MYSQL_LDFLAGS) \
  $(BOOST_LDFLAGS) \
  $(BOOST_THREAD_LIB) \
  $(BOOST_SYSTEM_LIB) \
  -lglog \
  $(top_builddir)/src/Utils/libutils.la \
  $(top_builddir)/src/Common/libcommon.la \
  $(top_builddir)/src/DatabaseManager/libdatabasemanager.la \
  $(top_builddir)/src/NetworkManager/libnetworkmanager.la
  