#############################################################################
## GNU Makefile for sawmill
##
## Author: Bart Meuris
##
#############################################################################

CC    ?= gcc
CXX   ?= g++
LINK  := $(CXX)
STRIP := strip
SYMLINK := ln -f -s
colon := :
empty :=
space := $(empty) $(empty)

#############################################################################
# Project settings
#

DEFAULT_BUILD := debug

TARGET        := sawmill
VER_MAJOR     := 0
VER_MINOR     := 1

OBJECTS := \
	sawmill.o \
	logevent.pb.o \
	# End of list

SHARED_LIBS   := pthread
STATIC_LIBS   := protobuf


C_DIRS        := .
H_DIRS        := .
PROTO_DIRS    := ./protocolbuffers

LIB_DIRS      := /usr/lib:/usr/local/lib

DEFINES       := 

#############################################################################
# Auto defined/derived variables
#
BUILD_MACHINE_NAME := $(shell hostname)
OS_NAME := $(shell uname)

TARGET_DEBUG   := $(TARGET)_debug
TARGET_RELEASE := $(TARGET)_release

TARGET_DEBUG_DEPENDENCY_FILE   := $(TARGET_DEBUG).d
TARGET_RELEASE_DEPENDENCY_FILE := $(TARGET_RELEASE).d

DEPENDENCIES    := $(OBJECTS:%.o=%.d)
OBJECTS_DEBUG   := $(OBJECTS:%.o=%.do)
OBJECTS_RELEASE := $(OBJECTS:%.o=%.o)

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
DEFINES := $(DEFINES)$(colon)BUILD_MACHINE_NAME="\"$(BUILD_MACHINE_NAME)\""

###

DEFINES       := $(subst $(colon)$(space),$(colon),$(DEFINES))
CFLAGS_DEFINE := -D$(subst $(colon), -D,$(DEFINES))

CFLAGS_DEBUG := $(CFLAGS) -g -O1 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE) -DDEBUG
LFLAGS_DEBUG := $(LFLAGS) -g -O1

# Enable optimization level 3 in release mode and don't add debug info to the object files.
CFLAGS_RELEASE := $(CFLAGS) -O3 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE)
LFLAGS_RELEASE := $(LFLAGS) -O3

#CXXFLAGS += -std=c++11
CXXFLAGS += -pedantic

CXXFLAGS_DEBUG   := $(CFLAGS_DEBUG) $(CXXFLAGS)
CXXFLAGS_RELEASE := $(CFLAGS_RELEASE) $(CXXFLAGS)

CFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)
CXXFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)


#############################################################################
# Build rules
#
VPATH := $(C_DIRS)

vpath %.a $(LIB_DIRS)

vpath %.proto .:./protocolbuffers

%.pb.cc: %.proto
	/usr/bin/protoc -I./protocolbuffers --cpp_out=./ $<;

%.pb.h: %.proto
	/usr/bin/protoc -I./protocolbuffers --cpp_out=./ $<;

%.do: %.c
	$(CC) -c $(CFLAGS_DEBUG) -o $@ $<;

%.do: %.cc
	$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;

%.do: %.cpp
	$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;

%.o: %.c
	$(CC) -c $(CFLAGS_RELEASE) -o $@ $<;

%.o: %.cc
	$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;

# Dependency files
%.d: %.c
	@$(CC) -MM -MP $(CFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;

%.d: %.cc
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;

%.d: %.cpp
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;
			

#############################################################################
# Targets
#
.PHONY: all install clean release debug always

all: $(DEFAULT_BUILD)

install:

clean:
	-rm $(TARGET_RELEASE) $(TARGET_DEBUG) $(TARGET) *.pb.cc *.pb.h *.do *.o *.d

release: $(TARGET_RELEASE)

debug: $(TARGET_DEBUG)

# Add this rule to avoid make errors when the static library could not be found in the vpath.
$(LIBDEPS_STATIC): ;

$(TARGET_DEBUG): $(OBJECTS_DEBUG) $(LIBDEPS_STATIC)
	$(LINK) $(LFLAGS_DEBUG) $^ -o $@ $(LIBDEPS_SHARED)
	$(SYMLINK) $@ $(TARGET)


$(TARGET_RELEASE): $(OBJECTS_RELEASE) $(LIBDEPS_STATIC)
	$(LINK) $(LFLAGS_RELEASE) $^ -o $@ $(LIBDEPS_SHARED)
	$(STRIP) $@
	$(SYMLINK) $@ $(TARGET)

#############################################################################
# Include the .d files
ifneq ($(findstring clean,${MAKECMDGOALS}),clean)
  include $(DEPENDENCIES)
endif

