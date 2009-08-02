#
# makefile for ZoneServer project.
#
APP = Release-Linux/LoginServer
APPD = Debug-Linux/LoginServer_d

MODULES = \
	NetworkManager \
	DatabaseManager \
	Utils

ARCHIVES :=	$(addprefix -l, $(MODULES))
ARCHIVESD := $(addprefix -l, $(addsuffix _d, $(MODULES)))

CFLAGS = -pedantic -Wall
LIBS = -lpthread -lmysqlclient


#
# File Paths
#
INC_DIR = -I../ -I./include
LIBD_DIR := $(addsuffix /Debug-Linux/, $(addprefix -L../, $(MODULES)))
LIB_DIR := $(addsuffix /Release-Linux/, $(addprefix -L../, $(MODULES)))

OBJECT = $(addprefix Release-Linux/, $(addsuffix .o, $(notdir $(basename $(wildcard *.cpp)))))
OBJECTD = $(addprefix Debug-Linux/, $(addsuffix .o, $(notdir $(basename $(wildcard *.cpp)))))

#
# Build targets
#
all : debug release

debug : debugsetup $(APPD)

release : releasesetup $(APP)

clean: cleandebug cleanrelease

cleandebug :  
	@echo "Removing temp debug files...";
	@rm -rf Debug-Linux/*.o;
	@rm -rf Debug-Linux/*.a;

cleanrelease :  
	@echo "Removing temp release files...";
	@rm -rf Release-Linux/*.o;
	@rm -rf Release-Linux/*.a;

rebuildrelease : cleanrelease release

rebuilddebug : cleandebug debug

debugsetup : 
	@echo "*";
	@echo "*  Starting Debug Build.";
	@echo "*";
	@if test ! -d Debug-Linux; then \
		mkdir Debug-Linux; \
	fi

releasesetup : 
	@echo "*";
	@echo "*  Starting Release Build.";
	@echo "*";
	@if test ! -d Release-Linux; then \
		mkdir Release-Linux; \
	fi

#
# Compile source here.
#	
Debug-Linux/%.o : %.cpp
	$(CXX) $(CFLAGS) -g  $(INC_DIR) -c $< -o $@

Release-Linux/%.o : %.cpp
	$(CXX) $(CFLAGS) -O3 $(INC_DIR) -c $< -o $@

#
# Build our lib archive fileobject files here
#
$(APPD) : $(OBJECTD)
	@echo "Linking $(APPD)...";
	$(CXX) $(addprefix Debug-Linux/, $(?F)) $(LIBD_DIR) $(ARCHIVESD) $(LIBS) -o $@

$(APP) : $(OBJECT)
	@echo "Linking $(APP)...";
	$(CXX) $(addprefix Release-Linux/, $(?F)) $(LIB_DIR) $(ARCHIVES) $(LIBS) -o $@


