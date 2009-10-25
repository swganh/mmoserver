AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# PingServer - executable
bin_PROGRAMS = pingserver
pingserver_SOURCES = \
	PingServer.cpp

pingserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -pedantic-errors -Wfatal-errors -Wno-depricated
pingserver_LDADD = \
	../Utils/libutils.la \
	../LogManager/liblogmanager.la \
	../Common/libcommon.la \
	../ConfigManager/libconfigmanager.la \
	../DatabaseManager/libdatabasemanager.la \
	$(MYSQL_LDFLAGS) \
	$(BOOST_LDFLAGS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_SYSTEM_LIB)