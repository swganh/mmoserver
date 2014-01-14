AM_CXXFLAGS = -I$(top_srcdir)/src $(SWGANH_CXXFLAGS) $(BOOST_CPPFLAGS)
  
# MessageLib library - noinstall shared library
noinst_LTLIBRARIES = libmessagelib.la
libmessagelib_la_SOURCES = \
	BuildingMessages.cpp \
	CommonMessages.cpp \
	CreatureMessages.cpp \
	HarvesterMessages.cpp \
	IntangibleMessages.cpp \
	InternalMessages.cpp \
	ManSchematicMessages.cpp \
	MessageLib.cpp \
	MissionMessages.cpp \
	ObjControllerMessages.cpp \
	PlayerMessages.cpp \
	ResourceContainerMessages.cpp \
	StaticMessages.cpp \
	StructureMessages.cpp \
	SystemMessages.cpp \
	TangibleMessages.cpp
	
libmessagelib_la_CPPFLAGS = -I$(top_srcdir)/deps/spatialindex/include -I$(top_srcdir)/deps/spatialindex/tools/include -I$(top_srcdir)/deps/noise/src -Wall -pedantic-errors -Wfatal-errors -fshort-wchar 
libmessagelib_la_LIBADD = ../Utils/libutils.la
