AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)

# DatabaseManager library - noinstall shared library
noinst_LTLIBRARIES = libdatabasemanager.la
libdatabasemanager_la_SOURCES = \
	Database.cpp \
  DatabaseImplementationMySql.cpp \
  DatabaseManager.cpp \
  DatabaseResult.cpp \
  DatabaseWorkerThread.cpp \
  DataBindingFactory.cpp \
  Transaction.cpp

libdatabasemanager_la_CPPFLAGS = $(MYSQL_CFLAGS) -Wall -pedantic-errors -Wfatal-errors -fshort-wchar
libdatabasemanager_la_LIBADD = ../Utils/libutils.la