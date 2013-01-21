#############################################################################
## GNU Makefile for sawmill
##
## Author: Bart Meuris
##
#############################################################################

colon := :
empty :=
space := $(empty) $(empty)

#############################################################################
# Project settings
#

TARGET        := sawmill
VER_MAJOR     := 0
VER_MINOR     := 1

OBJECTS := \
	logevent.pb.o \
	sawmill.o \
	# End of list

SHARED_LIBS   :=
STATIC_LIBS   := 


C_DIRS        := .
H_DIRS        := .

LIB_DIRS      :=

DEFINES       := 

#############################################################################
# Auto defined/derived variables
#
TARGET_DEBUG   := $(TARGET)_debug
TARGET_RELEASE := $(TARGET)_release

TARGET_DEBUG_DEPENDENCY_FILE   := $(TARGET_DEBUG).d
TARGET_RELEASE_DEPENDENCY_FILE := $(TARGET_RELEASE).d

DEPENDENCIES    := $(OBJECTS:%.o=%.d)
OBJECTS_DEBUG   := $(OBJECTS:%.o=%.do)
OBJECTS_RELEASE := $(OBJECTS:%.o=%.o)

BUILD_MACHINE_NAME := $(shell hostname)

ifeq ($(DEFINES),)
  DEFINES       := VER_MAJOR=$(VER_MAJOR):VER_MINOR=$(VER_MINOR)
else
  DEFINES       := $(DEFINES):VER_MAJOR=$(VER_MAJOR):VER_MINOR=$(VER_MINOR)
endif

#DEFINES := $(DEFINES):BUILD_MACHINE=$(BUILD_MACHINE_NAME)

ifeq ($(OS_NAME), Darwin)	
    DLSUFFIX := .dylib
else
    DLSUFFIX := .so
endif

VPATH := $(C_DIRS)

#############################################################################
# Auto defined/derived variables
#
vpath %.a $(LIB_DIRS)

vpath %.proto ./protocolbuffers

%.pb.cc: %.proto
	/usr/bin/protoc -I=./protocolbuffers --cpp_out=. $<;

%.do: %.c
	$(CC) $(CFLAGS_DEBUG) -o $@ $<;

%.do: %.cc
	$(CXX) $(CXXFLAGS_DEBUG) -o $@ $<;

%.do: %.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS_RELEASE) -o $@ $<;

%.o: %.cc
	$(CXX) $(CXXFLAGS_RELEASE) -o $@ $<;

%.o: %.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -o $@ $<;

# Dependency files
%.d: %.c
	@$(CC) -MM -MP $(CFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;

%.d: %.cc
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;

%.d: %.cpp
	@$(CXX) -MM -MP $(CXXFLAGS_DEPENDENCIES) -MT '$(@:%.d=%.o) $(@:%.d=%.do) $@' -o $@ $<;
			

#############################################################################
# Rules
#
.PHONY: all install clean release debug always

all: release

install:

clean:

release: $(TARGET_RELEASE)

debug: $(TARGET_DEBUG)


$(TARGET_DEBUG): $(OBJECTS_DEBUG) $(LIBDEPS_STATIC)
	$(LINK) $(LFLAGS_DEBUG) $^ -o $@ $(LIBDEPS_SHARED)
	$(SYMLINK) $@ $(TARGET)


$(TARGET_RELEASE): $(OBJECTS_RELEASE) $(LIBDEPS_STATIC)
	$(LINK) $(LFLAGS_RELEASE) $^ -o $@ $(LIBDEPS_SHARED)
	ifeq ($(OS_NAME), Darwin)
		$(STRIP) -u -r $@
	else
		$(STRIP) $@
	endif
		$(SYMLINK) $@ $(TARGET)


#############################################################################
# Include the .d files
ifneq ($(findstring clean,${MAKECMDGOALS}),clean)
include $(DEPENDENCIES)
endif

