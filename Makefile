#############################################################################
## GNU Makefile for sawmill
##
## Author: Bart Meuris
##
#############################################################################

#CC    ?= gcc
#CXX   ?= g++
CC    := clang
CXX   := clang++
LINK  := $(CXX)
OBJCOPY := objcopy
STRIP := strip
SYMLINK := ln -f -s
PBC   :=/usr/bin/protoc
colon := :
empty :=
space := $(empty) $(empty)
VERBOSE := 1
STATIC :=

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
	version.o \
	configmanager.o \
	# End of list

# Protocol buffer objects
PB_OBJECTS := \
	logevent.pb.o \
	command.pb.o \
	# End of list

VERSION_GENFILE := version_gen.h

SHARED_LIBS   :=
STATIC_LIBS   :=

SHARED_LIBS   += pthread
ifneq ($(STATIC),)
  STATIC_LIBS   += crypto
  STATIC_LIBS   += protobuf
  STATIC_LIBS   += boost_program_options boost_regex boost_filesystem boost_system boost_iostreams
  STATIC_LIBS   += zmq
else
  SHARED_LIBS   += crypto
  SHARED_LIBS   += protobuf
  SHARED_LIBS   += boost_program_options boost_regex boost_filesystem boost_system boost_iostreams
  SHARED_LIBS   += zmq
endif

C_DIRS        := \
				. \
				./src \
              # End of list
H_DIRS        := \
				. \
				./src \
              # End of list
PB_DIRS       := \
				./protocolbuffers
              # End of list

LIB_DIRS      := \
                 /usr/lib \
                 /usr/local/lib \
				 /usr/lib/x86_64-linux-gnu \
              # End of list

DEFINES       := 

CFLAGS += -pipe -Wall -pedantic 
#CFLAGS += -m32
#CFLAGS += -m64
LFLAGS += -Wall

#############################################################################
# Auto defined/derived variables
#

BUILD_DATETIME     := $(shell date "+%F %T%:::z")
BUILD_MACHINE_NAME := $(shell hostname)
OS_NAME            := $(shell uname)

# git stuff
GIT_HASH      := $(shell git log --pretty=format:'%h' -n 1)
GIT_HASH_LONG := $(shell git log --pretty=format:'%H' -n 1)
GIT_MODIFIED  := $(shell git status --porcelain | grep -v "^??" | wc -l)
GIT_USER      := $(shell git config user.name)
GIT_EMAIL     := $(shell git config user.email)


ifneq ($(STATIC_LIBS),)
  LIBS           := $(addprefix lib,$(STATIC_LIBS))
  LIBS           := $(addsuffix .a,$(LIBS))
  LIBDEPS_STATIC := $(LIBS)
endif

ifneq ($(SHARED_LIBS),)
  LIBS           := $(addprefix -l,$(SHARED_LIBS))
  LIBDEPS_SHARED := $(LIBS)
endif

ifneq ($(PB_DIRS),)
  PB_INCLUDE := $(addprefix -I,$(PB_DIRS))
endif
ifneq ($(H_DIRS),)
  CFLAGS_INCLUDE := $(addprefix -I,$(H_DIRS))
endif

# Determine Defines
DEFINES += APP_NAME="\"$(APP_NAME)\""
ifneq ($(IGNORE_RELEASE_CHANGES),)
DEFINES += IGNORE_RELEASE_CHANGES
endif

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

CFLAGS_DEFINE := $(addprefix -D,$(DEFINES))

CFLAGS_DEBUG := $(CFLAGS) -g -O0 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE) -DDEBUG
LFLAGS_DEBUG := $(LFLAGS) -g -O0

# Enable optimization level 3 in release mode. Add debug symbols, which will be saved externally.
CFLAGS_RELEASE := $(CFLAGS) -g -O3 $(CFLAGS_INCLUDE) $(CFLAGS_DEFINE)
LFLAGS_RELEASE := $(LFLAGS) -g -O3

#CXXFLAGS += -std=c++11
CXXFLAGS += -pedantic

CXXFLAGS_DEBUG   := $(CFLAGS_DEBUG) $(CXXFLAGS)
CXXFLAGS_RELEASE := $(CFLAGS_RELEASE) $(CXXFLAGS)

CFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)
CXXFLAGS_DEPENDENCIES := $(CFLAGS_DEFINE) $(CFLAGS_INCLUDE)

ifeq ($(VERBOSE),)
	SILENT := @
else
	SILENT :=
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
		@echo -n "Generating protocolbuffer classes for: $< "
    endif
	$(SILENT)$(PBC) $(PB_INCLUDE) --cpp_out=./ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.do: %.c
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CC) -c $(CFLAGS_DEBUG) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.do: %.cc
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.do: %.cpp
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_DEBUG) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.o: %.c
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CC) -c $(CFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.o: %.cc
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
    endif

%.o: %.cpp
    ifneq ($(SILENT),)
	    @echo -n "Compiling $< "
    endif
	$(SILENT)$(CXX) -c $(CXXFLAGS_RELEASE) -o $@ $<;
    ifneq ($(SILENT),)
	    @echo "[OK]"
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
.PHONY: all install clean release debug depclean pbclean version

all: $(DEFAULT_BUILD)

release: $(APP_NAME_RELEASE)

debug: $(APP_NAME_DEBUG)

install: all
	@echo "TODO"

version:
	-@rm -f version.do
	-@rm -f version.o
	-@rm -f $(VERSION_GENFILE)
	@echo "// Autogenerated. Do NOT commit." >> $(VERSION_GENFILE)
	@echo "#define VER_MAJOR \"$(VER_MAJOR)\"" >> $(VERSION_GENFILE)
	@echo "#define VER_MINOR \"$(VER_MINOR)\"" >> $(VERSION_GENFILE)
	@echo "" >> $(VERSION_GENFILE)
	@echo "#define OS_NAME \"$(OS_NAME)\"" >> $(VERSION_GENFILE)
	@echo "#define BUILD_MACHINE_NAME \"$(BUILD_MACHINE_NAME)\"" >> $(VERSION_GENFILE)
	@echo "#define BUILD_DATETIME \"$(BUILD_DATETIME)\"" >> $(VERSION_GENFILE)
	@echo "" >> $(VERSION_GENFILE)
	@echo "#define GIT_USER \"$(GIT_USER)\"" >> $(VERSION_GENFILE)
	@echo "#define GIT_EMAIL \"$(GIT_EMAIL)\"" >> $(VERSION_GENFILE)
	@echo "#define GIT_HASH \"$(GIT_HASH)\"" >> $(VERSION_GENFILE)
	@echo "#define GIT_HASH_LONG \"$(GIT_HASH_LONG)\"" >> $(VERSION_GENFILE)
	@echo "#define GIT_MODIFIED $(GIT_MODIFIED)" >> $(VERSION_GENFILE)
	@echo "" >> $(VERSION_GENFILE)

clean: depclean pbclean
    ifneq ($(SILENT),)
		@echo "Cleaning program files and objects..."
    endif
	$(SILENT)-rm -f $(APP_NAME_RELEASE) $(APP_NAME_RELEASE).debug $(APP_NAME_DEBUG) $(APP_NAME_DEBUG).debug $(APP_NAME)
	$(SILENT)-rm -f $(OBJECTS_DEBUG) $(OBJECTS_RELEASE)
	$(SILENT)-rm -f $(VERSION_GENFILE)
	$(SILENT)-rm -f core


pbclean: ;
    ifneq ($(SILENT),)
		@echo "Cleaning protocolbuffer files and objects..."
    endif
	$(SILENT)-rm -f $(PB_OBJECTS_DEBUG) $(PB_OBJECTS_RELEASE) $(PB_GENS);


depclean: ;
    ifneq ($(SILENT),)
		@echo "Cleaning dependency files..."
    endif
	$(SILENT)-rm -f $(DEPENDENCIES);


# Add this rule to avoid make errors when the static library could not be found in the vpath.
$(LIBDEPS_STATIC): ;

$(APP_NAME_DEBUG): version $(OBJECTS_DEBUG) $(PB_OBJECTS_DEBUG) $(LIBDEPS_STATIC)
    ifneq ($(SILENT),)
		@echo -n "Linking debug version '$(APP_NAME_DEBUG)'... "
    endif
	$(SILENT)$(LINK) $(LFLAGS_DEBUG) $(OBJECTS_DEBUG) $(PB_OBJECTS_DEBUG) $(LIBDEPS_STATIC) -o $@ $(LIBDEPS_SHARED)
	$(SILENT)$(OBJCOPY) --only-keep-debug $@ $@.debug
	$(SILENT)$(OBJCOPY) --strip-debug $@
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$@.debug $@
	$(SILENT)$(SYMLINK) $@ $(APP_NAME)
    ifneq ($(SILENT),)
		@echo "[OK]"
		@echo
    endif


$(APP_NAME_RELEASE): version $(OBJECTS_RELEASE) $(PB_OBJECTS_RELEASE) $(LIBDEPS_STATIC)
    ifneq ($(SILENT),)
	    @echo -n "Linking release version '$(APP_NAME_RELEASE)'... "
    endif
	$(SILENT)$(LINK) $(LFLAGS_RELEASE) $(OBJECTS_RELEASE) $(PB_OBJECTS_RELEASE) $(LIBDEPS_STATIC) -o $@ $(LIBDEPS_SHARED)
	$(SILENT)$(OBJCOPY) --only-keep-debug $@ $@.debug
	$(SILENT)$(OBJCOPY) --strip-debug $@
	$(SILENT)$(OBJCOPY) --add-gnu-debuglink=$@.debug $@
	$(SILENT)$(SYMLINK) $@ $(APP_NAME)
    ifneq ($(SILENT),)
	    @echo "[OK]"
		@echo
    endif

$(DEPENDENCIES): $(PB_GENS)

#############################################################################
# Include the automatically generated dependency files (*.d)
ifneq ($(findstring clean,${MAKECMDGOALS}),clean)
  -include $(DEPENDENCIES)
endif

