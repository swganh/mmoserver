AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS)

# NetworkManager library - noinstall shared library
noinst_LTLIBRARIES = libnetworkmanager.la
libnetworkmanager_la_SOURCES = CompCryptor.cpp \
  NetConfig.cpp \
  NetworkClient.cpp \
  NetworkManager.cpp \
  PacketFactory.cpp \
  Service.cpp \
  Session.cpp \
  SessionFactory.cpp \
  SocketReadThread.cpp \
  SocketWriteThread.cpp

libnetworkmanager_la_CPPFLAGS = -Wall -fshort-wchar