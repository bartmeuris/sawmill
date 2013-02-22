#############################################################################
## GNU Makefile for sawmill
##
## Author: Bart Meuris
##
#############################################################################

#CC    ?= gcc
#CXX   ?= g++
CC    ?= clang
CXX   ?= clang++
LINK  := $(CXX)
STRIP := strip
SYMLINK := ln -f -s
PBC   :=/usr/bin/protoc
colon := :
empty :=
space := $(empty) $(empty)
SILENT :=


#############################################################################
# Project settings
#

DEFAULT_BUILD ?= debug

APP_NAME        := sawmill
VER_MAJOR     := 0
VER_MINOR     := 1

# Main application objects
OBJECTS := \
	sawmill.o \
	configmanager.o \
	# End of list

# Protocol buffer objects
PB_OBJECTS := \
	logevent.pb.o \
	command.pb.o \
	# End of list


SHARED_LIBS   := pthread
# OpenSSL for MD5 calculation
SHARED_LIBS   := $(SHARED_LIBS):crypto
# Comment/uncomment the following lines to produce a staticly linked executable
SHARED_LIBS   := $(SHARED_LIBS):protobuf:boost_program_options:boost_regex:boost_filesystem:boost_system:boost_iostreams:zmq
#STATIC_LIBS   := protobuf:boost_program_options:boost_regex:boost_filesystem:boost_system:boost_iostreams:zmq

C_DIRS        := .:./src
H_DIRS        := .:./src
PB_DIRS       := ./protocolbuffers

LIB_DIRS      := /usr/lib:/usr/local/lib

DEFINES       := 

CFLAGS += -pipe -Wall -pedantic 
LFLAGS += -Wall

#############################################################################
# Auto defined/derived variables
#

BUILD_DATETIME     := $(shell date "+%Y%m%d%H%M%S")
BUILD_MACHINE_NAME := $(shell hostname)
OS_NAME            := $(shell uname)

ifneq ($(STATIC_LIBS),)
  STATIC_LIBS    := $(subst $(colon)$(space),$(colon),$(STATIC_LIBS))
  LIBS           := $(subst $(colon),.a$(colon),$(STATIC_LIBS)).a
  LIBS           := lib$(subst $(colon),$(space)lib,$(LIBS))
  LIBDEPS_STATIC := $(LIBS)
endif

ifneq ($(SHARED_LIBS),)
  SHARED_LIBS    := $(subst $(colon)$(space),$(colon),$(SHARED_LIBS))
  LIBS           := -l$(subst $(colon), -l,$(SHARED_LIBS))
  LIBDEPS_SHARED := $(LIBS)
endif

ifneq ($(PB_DIRS),)
  PB_DIRS         := $(subst $(colon)$(space),$(colon),$(PB_DIRS))
  PB_INCLUDE := $(shell printf -- '-I$(subst $(colon),\n-I,$(PB_DIRS))\n' | uniq)
endif
ifneq ($(H_DIRS),)
  H_DIRS         := $(subst $(colon)$(space),$(colon),$(H_DIRS))
  CFLAGS_INCLUDE := $(shell printf -- '-I$(subst $(colon),\n-I,$(H_DIRS))\n' | uniq)
endif

# Determine Defines
ifeq ($(DEFINES),)
  DEFINES     := VER_MAJOR=$(VER_MAJOR):VER_MINOR=$(VER_MINOR)
else
  DEFINES     := $(DEFINES):VER_MAJOR=$(VER_MAJOR):VER_MINOR=$(VER_MINOR)
endif
DEFINES := $(DEFINES)$(colon)APP_NAME="\"$(APP_NAME)\""
DEFINES := $(DEFINES)$(colon)OS_NAME="\"$(OS_NAME)\""
DEFINES := $(DEFINES)$(colon)BUILD_MACHINE_NAME="\"$(BUILD_MACHINE_NAME)\""
DEFINES := $(DEFINES)$(colon)BUILD_DATETIME="\"$(BUILD_DATETIME)\""

###

APP_NAME_DEBUG   := $(APP_NAME)_debug
APP_NAME_RELEASE := $(APP_NAME)_release

# Normal dependencies and objects
DEPENDENCIES       := $(OBJECTS:%.o=%.d) $(PB_OBJECTS:.o:.d)
OBJECTS_DEBUG      := $(OBJECTS:%.o=%.do)
OBJECTS_RELEASE    := $(OBJECTS)

# Protocol buffer generated files and objects
PB_GENS := $(PB_OBJECTS:.pb.o=.pb.cc) $(PB_OBJECTS:.pb.o=.pb.h)
PB_OBJECTS_DEBUG   := $(PB_OBJECTS:%.o=%.do)
PB_OBJECTS_RELEASE := $(PB_OBJECTS:%.o=%.o)

DEFINES       := $(subst $(colon)$(space),$(colon),$(DEFINES))
CFLAGS_DEFINE := -D$(subst $(colon), -D,$(DEFINES))

CFLAGS_DEBUG := $(CFLAGS) -g -O0 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE) -DDEBUG
LFLAGS_DEBUG := $(LFLAGS) -g -O0

# Enable optimization level 3 in release mode and don't add debug info to the object files.
CFLAGS_RELEASE := $(CFLAGS) -O3 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE)
LFLAGS_RELEASE := $(LFLAGS) -O3

#CXXFLAGS += -std=c++11
CXXFLAGS += -pedantic

CXXFLAGS_DEBUG   := $(CFLAGS_DEBUG) $(CXXFLAGS)
CXXFLAGS_RELEASE := $(CFLAGS_RELEASE) $(CXXFLAGS)

CFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)
CXXFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)

ifneq ($(SILENT),)
	SILENT := @
endif

#############################################################################
# Build rules
#
VPATH := $(C_DIRS)
vpath %.h $(C_DIRS):$(H_DIRS)
vpath %.a $(LIB_DIRS)
vpath %.proto $(PB_DIRS)

.PRECIOUS: $(PBGENS)

%.pb.cc: %.proto
    ifneq ($(SILENT),)
	    @echo "Generating protocolbuffer classes for: $<"
    endif
	$(SILENT)$(PBC) $(PB_INCLUDE) --cpp_out=./ $<;

%.do: %.c
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CC) -c $(CFLAGS_DEBUG) -o $@ $<;

%.do: %.cc
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo " [OK]"
    endif

%.do: %.cpp
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo " [OK]"
    endif

%.o: %.c
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CC) -c $(CFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo " [OK]"
    endif

%.o: %.cc
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo " [OK]"
    endif

%.o: %.cpp
    ifneq ($(SILENT),)
	    @echo -n "Compiling $@"
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo " [OK]"
    endif

# Dependency files
%.d: %.c
	$(SILENT)@$(CC) -MM -MP $(CFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $< 2> /dev/null;

%.d: %.cc
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $< 2> /dev/null;

%.d: %.cpp
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $< 2> /dev/null;


#############################################################################
# Targets
#
.PHONY: all install clean release debug depclean pbclean

all: $(DEFAULT_BUILD)

install:
	# TODO

clean: depclean pbclean
    ifneq ($(SILENT),)
	    @echo "Cleaning program files and objects..."
    endif
	$(SILENT)-rm -f $(APP_NAME_RELEASE) $(APP_NAME_DEBUG) $(APP_NAME)
	$(SILENT)-rm -f $(OBJECTS_DEBUG) $(OBJECTS_RELEASE)
	$(SILENT)-rm -f core

pbclean:
    ifneq ($(SILENT),)
	    @echo "Cleaning protocolbuffer files and objects..."
    endif
	$(SILENT)-rm -f $(PB_OBJECTS_DEBUG) $(PB_OBJECTS_RELEASE) $(PB_GENS)

depclean:
    ifneq ($(SILENT),)
	    @echo "Cleaning dependency files..."
    endif
	$(SILENT)-rm -f $(DEPENDENCIES)

release: $(APP_NAME_RELEASE)

debug: $(APP_NAME_DEBUG)

# Add this rule to avoid make errors when the static library could not be found in the vpath.
$(LIBDEPS_STATIC): ;

$(APP_NAME_DEBUG): $(OBJECTS_DEBUG) $(PB_OBJECTS_DEBUG) $(LIBDEPS_STATIC)
    ifneq ($(SILENT),)
	    @echo -n "Linking DEBUG version..."
    endif
	$(SILENT)$(LINK) $(LFLAGS_DEBUG) $^ -o $@ $(LIBDEPS_SHARED)
	$(SILENT)$(SYMLINK) $@ $(APP_NAME)
    ifneq ($(SILENT),)
	    @echo " [OK]"
		@echo
    endif


$(APP_NAME_RELEASE): $(OBJECTS_RELEASE) $(PB_OBJECTS_RELEASE) $(LIBDEPS_STATIC)
    ifneq ($(SILENT),)
	    @echo -n "Linking RELEASE version..."
    endif
	$(SILENT)$(LINK) $(LFLAGS_RELEASE) $^ -o $@ $(LIBDEPS_SHARED)
	$(SILENT)$(STRIP) $@
	$(SILENT)$(SYMLINK) $@ $(APP_NAME)
    ifneq ($(SILENT),)
	    @echo " [OK]"
		@echo
    endif

$(DEPENDENCIES): $(PB_GENS)

#############################################################################
# Include the automatically generated dependency files (*.d)
ifneq ($(findstring clean,${MAKECMDGOALS}),clean)
  -include $(DEPENDENCIES)
endif

