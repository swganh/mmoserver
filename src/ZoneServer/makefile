#
# makefile for ZoneServer project.
#
APP = Release-Linux/ZoneServer
APPD = Debug-Linux/ZoneServer_d

MODULES =	WorldManager \
			Utils \
			ScriptEngine \
			python \
			NetworkManager \
			GameManager \
			DatabaseManager \
			AssetManager 

ARCHIVES :=	$(addprefix -l, $(MODULES))
ARCHIVESD := $(addprefix -l, $(addsuffix _d, $(MODULES)))

#
# Standard Macros
#
AR = ar
GFLAGS =
GET = get
ASFLAGS =
MAS = mas
AS = as
FC = f77
CFLAGS = -fno-exceptions
CC = gcc
LDFLAGS = -static
LD = gcc
LFLAGS =
LEX = lex
YFLAGS =
YACC = yacc
LOADLIBS =
MAKE = make
MAKEARGS = 'SHELL=/bin/sh'
SHELL = /bin/sh
MAKEFLAGS = b


#
# File Paths
#
INC_DIR = -I ../ -I ./include
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
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -g3  $(INC_DIR) -c $< -o $@ 2> gcc.%@.stdout; cat gcc.%@.stdout | ../tools/linux/gnu2msdev; rm -f gcc.%@.stdout;
	
Release-Linux/%.o : %.cpp
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -o3 $(INC_DIR) -c $< -o $@ 2> gcc.%@.stdout; cat gcc.%@.stdout | ../tools/linux/gnu2msdev; rm -f gcc.%@.stdout;

#
# Build our lib archive fileobject files here
#
$(APPD) : $(OBJECTD)
	@echo "Linking $(APPD)...";
	$(LD) $(LIBD_DIR) $(ARCHIVESD) $(addprefix Debug-Linux/, $(?F));

$(APP) : $(OBJECT)
	@echo "Linking $(APP)...";
	$(LD) $(LIBD_DIR) $(ARCHIVES) $(addprefix Release-Linux/, $(?F));


