#
# makefile for NetworkManager project.
#
ARCHIVE = Release-Linux/libNetworkManager.a
ARCHIVED = Debug-Linux/libNetworkManager_d.a

CFLAGS = -pedantic -Wall

#
# File Paths
#
INC_DIR = -I ../ -I ../depends/linux/include
LIB_DIR =
LIBS = 

OBJECT = $(addprefix Release-Linux/, $(addsuffix .o, $(notdir $(basename $(wildcard *.cpp)))))
OBJECTD = $(addprefix Debug-Linux/, $(addsuffix .o, $(notdir $(basename $(wildcard *.cpp)))))

#
# Build targets
#
all : debug release

debug : debugsetup $(ARCHIVED)

release : releasesetup $(ARCHIVE)

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
$(ARCHIVED) : $(OBJECTD)
	@echo "Updating library archive...";
	@$(AR) cr $@ $(addprefix Debug-Linux/, $(?F));

$(ARCHIVE) : $(OBJECT)
	@echo "Updating library archive...";
	@$(AR) cr $@ $(addprefix Release-Linux/, $(?F));


