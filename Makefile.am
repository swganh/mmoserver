ACLOCAL_AMFLAGS = -I tools/m4 --install
AM_CXXFLAGS     = -I$(top_srcdir)/src \
  $(SWGANH_CXXFLAGS) \
  $(BOOST_CPPFLAGS)

# Utils library - noinstall shared library
noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES = src/Utils/bstring.cpp \
  src/Utils/clock.cpp \
  src/Utils/EventHandler.cpp \
  src/Utils/rand.cpp \
  src/Utils/Scheduler.cpp \
  src/Utils/StreamColors.cpp \
  src/Utils/Timer.cpp \
  src/Utils/utils.cpp \
  src/Utils/VariableTimeScheduler.cpp

libutils_la_CPPFLAGS = -fpermissive -fshort-wchar 
libutils_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# LogManager library - noinstall shared library
noinst_LTLIBRARIES += liblogmanager.la
liblogmanager_la_SOURCES = src/LogManager/Log.cpp \
  src/LogManager/LogManager.cpp
	
liblogmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 
liblogmanager_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# Common library - noinstall shared library
noinst_LTLIBRARIES += libcommon.la
libcommon_la_SOURCES = src/Common/atMacroString.cpp \
  src/Common/BuildInfo.cpp \
  src/Common/bytebuffer.cpp \
  src/Common/DispatchClient.cpp \
  src/Common/MessageDispatch.cpp \
  src/Common/MessageFactory.cpp 
    
libcommon_la_CPPFLAGS = -fpermissive -fshort-wchar 
libcommon_la_LIBADD = $(BOOST_LDFLAGS) $(BOOST_SYSTEM_LIB)

# ConfigManager library - noinstall shared library
noinst_LTLIBRARIES += libconfigmanager.la
libconfigmanager_la_SOURCES = src/ConfigManager/ConfigFile.cpp \
  src/ConfigManager/ConfigManager.cpp
    
libconfigmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 

# Math library - noinstall shared library
noinst_LTLIBRARIES += libmath.la
libmath_la_SOURCES = src/MathLib/Quaternion.cpp \
  src/MathLib/Vector3.cpp

libmath_la_CPPFLAGS = -fpermissive -fshort-wchar 

# DatabaseManager library - noinstall shared library
noinst_LTLIBRARIES += libdatabasemanager.la
libdatabasemanager_la_SOURCES = src/DatabaseManager/Database.cpp \
  src/DatabaseManager/DatabaseImplementationMySql.cpp \
  src/DatabaseManager/DatabaseManager.cpp \
  src/DatabaseManager/DatabaseResult.cpp \
  src/DatabaseManager/DatabaseWorkerThread.cpp \
  src/DatabaseManager/DataBindingFactory.cpp \
  src/DatabaseManager/Transaction.cpp

libdatabasemanager_la_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar 
libdatabasemanager_la_LIBADD = libutils.la

# NetworkManager library - noinstall shared library
noinst_LTLIBRARIES += libnetworkmanager.la
libnetworkmanager_la_SOURCES = src/NetworkManager/CompCryptor.cpp \
  src/NetworkManager/NetConfig.cpp \
  src/NetworkManager/NetworkClient.cpp \
  src/NetworkManager/NetworkManager.cpp \
  src/NetworkManager/PacketFactory.cpp \
  src/NetworkManager/Service.cpp \
  src/NetworkManager/Session.cpp \
  src/NetworkManager/SessionFactory.cpp \
  src/NetworkManager/SocketReadThread.cpp \
  src/NetworkManager/SocketWriteThread.cpp

libnetworkmanager_la_CPPFLAGS = -fpermissive -fshort-wchar 
libnetworkmanager_la_LIBADD = libutils.la libcommon.la liblogmanager.la

# ScriptEngine library - noinstall shared library
#noinst_LTLIBRARIES += libscriptengine.la
#libscriptengine_la_SOURCES = src/ScriptEngine/Script.cpp \
#  src/ScriptEngine/ScriptEngine.cpp \
#  src/ScriptEngine/ScriptEngineLib.cpp \
#  src/ScriptEngine/ScriptEventListener.cpp \
#  src/ScriptEngine/ScriptSupport.cpp \
#  src/ScriptEngine/glue_files/LuaInterface.cpp

#libscriptengine_la_CPPFLAGS = $(LUA_INCLUDE) -I$(top_srcdir)/deps/noise/src -fpermissive -fshort-wchar 
#libscriptengine_la_LIBADD = $(LUA_LIB) $(top_srcdir)/deps/noise/src/libnoise.la

# AdminServer - executable
bin_PROGRAMS = adminserver
adminserver_SOURCES = src/AdminServer/AdminServer.cpp \
  src/AdminServer/CharacterAdminHandler.cpp \
  src/AdminServer/PlanetMapHandler.cpp

adminserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar 
adminserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# ChatServer - executable
bin_PROGRAMS += chatserver
chatserver_SOURCES = src/ChatServer/Channel.cpp \
  src/ChatServer/ChatAvatarId.cpp \
  src/ChatServer/ChatManager.cpp \
  src/ChatServer/ChatMessageLib.cpp \
  src/ChatServer/ChatMessages.cpp \
  src/ChatServer/ChatServer.cpp \
  src/ChatServer/CSRManager.cpp \
  src/ChatServer/CSRMessages.cpp \
  src/ChatServer/GroupManager.cpp \
  src/ChatServer/GroupMessages.cpp \
  src/ChatServer/GroupObject.cpp \
  src/ChatServer/Player.cpp \
  src/ChatServer/TradeManagerChat.cpp \
  src/ChatServer/TradeManagerHelp.cpp \
  src/ChatServer/TradeMessages.cpp

chatserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar -Wno-invalid-offsetof
chatserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# ConnectionServer - executable
bin_PROGRAMS += connserver
connserver_SOURCES = src/ConnectionServer/ClientManager.cpp \
	src/ConnectionServer/ConnectionDispatch.cpp \
	src/ConnectionServer/ConnectionServer.cpp \
	src/ConnectionServer/MessageRouter.cpp \
	src/ConnectionServer/ServerManager.cpp
	
connserver_CPPFLAGS = $(MYSQL_CFLAGS) -fpermissive -fshort-wchar
connserver_LDADD = libutils.la liblogmanager.la libcommon.la libconfigmanager.la libdatabasemanager.la libnetworkmanager.la $(MYSQL_LDFLAGS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB)

# Keep documentation files when creating a distributable version of the source.
doc_DATA = AUTHORS ChangeLog COPYING INSTALL NEWS README

# When creating a distributable version of the source we want the .svn directories 
# removed. This hook accomplishes that.
dist-hook:
	-rm -rf 'find $(distdir) -name .svn'
