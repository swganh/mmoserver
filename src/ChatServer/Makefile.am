AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

# ChatServer - executable
bin_PROGRAMS = chatserver
chatserver_SOURCES = \
  Channel.cpp \
  CharacterAdminHandler.cpp \
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
  PlanetMapHandler.cpp \
  Player.cpp \
  StructureManagerChat.cpp \
  TradeManagerChat.cpp \
  TradeManagerHelp.cpp \
  TradeMessages.cpp

chatserver_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -pedantic-errors -Wfatal-errors -fno-strict-aliasing -Wno-overflow -Wno-invalid-offsetof
chatserver_LDADD = $(MYSQL_LDFLAGS) \
  $(BOOST_LDFLAGS) \
  $(BOOST_THREAD_LIB) \
  $(BOOST_SYSTEM_LIB) \
  $(top_builddir)/src/Utils/libutils.la \
  $(top_builddir)/src/Common/libcommon.la \
  $(top_builddir)/src/DatabaseManager/libdatabasemanager.la \
  $(top_builddir)/src/NetworkManager/libnetworkmanager.la