AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS)

# LoginServer - executable
bin_PROGRAMS = loginserver
loginserver_SOURCES = \
	LoginManager.cpp \
	LoginServer.cpp
	
loginserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -fshort-wchar -Wno-invalid-offsetof
loginserver_LDADD = \
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
  