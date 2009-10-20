AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS)

# ChatServer - executable
bin_PROGRAMS = chatserver
chatserver_SOURCES = \
	Channel.cpp \
  ChatAvatarId.cpp \
  ChatManager.cpp \
  ChatMessageLib.cpp \
  ChatMessages.cpp \
  ChatServer.cpp \
  CSRManager.cpp \
  CSRMessages.cpp \
  GroupManager.cpp \
  GroupMessages.cpp \
  GroupObject.cpp \
  Player.cpp \
  TradeManagerChat.cpp \
  TradeManagerHelp.cpp \
  TradeMessages.cpp

chatserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -fshort-wchar -Wno-invalid-offsetof
chatserver_LDADD = \
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