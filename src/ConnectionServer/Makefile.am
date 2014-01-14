AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# ConnectionServer - executable
bin_PROGRAMS = connserver
connserver_SOURCES = \
	ClientManager.cpp \
	ConnectionDispatch.cpp \
	ConnectionServer.cpp \
	MessageRouter.cpp \
	ServerManager.cpp
	
connserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -pedantic -pedantic-errors -Wfatal-errors -fshort-wchar
connserver_LDADD = \
	../Utils/libutils.la \
	../LogManager/liblogmanager.la \
	../Common/libcommon.la \
	../ConfigManager/libconfigmanager.la \
	../DatabaseManager/libdatabasemanager.la \
	../NetworkManager/libnetworkmanager.la \
	$(MYSQL_LDFLAGS) \
	$(BOOST_LDFLAGS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_SYSTEM_LIB)
  