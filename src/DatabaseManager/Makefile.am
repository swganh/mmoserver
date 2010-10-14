AM_CXXFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) @AM_CXXFLAGS@

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
libdatabasemanager_la_LIBADD = $(top_builddir)/src/Utils/libutils.la