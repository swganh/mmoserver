AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS)
  
# AdminServer - executable
bin_PROGRAMS = adminserver
adminserver_SOURCES = \
	AdminServer.cpp \
  CharacterAdminHandler.cpp \
  PlanetMapHandler.cpp

adminserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -fshort-wchar 
adminserver_LDADD = \
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
  