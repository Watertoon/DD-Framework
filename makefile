.SUFFIXES:
# Complex makefile for basic projects

THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIRECTORY := $(abspath $(dir $(THIS_MAKEFILE)))

# Include libraries
ifeq ($(strip $(VULKAN_SDK)),)
$(error "VULKAN_SDK is not set in your environment.")
endif

DEFINES  := -DDD_DEBUG
LIBDIRS  := $(VULKAN_SDK) third_party
INCLUDES := include

# Compiler options
CXX_FLAGS := -std=gnu++20 -m64 -msse4.1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-rtti -fno-exceptions
CXX_WARNS := -Wall -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wno-invalid-offsetof -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wextra -Werror -Wno-missing-field-initializers

# Release and Debug mode options 
RELEASE_FLAGS := -O3 -flto -g -gdwarf-4
DEBUG_FLAGS   := -g -Og -flto -gdwarf-4

# Source input file iteration methods
DIRECTORY_WILDCARD  =   $(foreach d,$(wildcard $(1:=/*)),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))
GET_ALL_SOURCE_DIRS =$1 $(foreach d,$(wildcard $1/*),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))

SOURCE_DIRS=$(call GET_ALL_SOURCE_DIRS,source)

FIND_SOURCE_FILES=$(foreach dir,$1,$(notdir $(wildcard $(dir)/*.$2)))

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Our input
export CPP_FILES            :=   $(call FIND_SOURCE_FILES,$(SOURCE_DIRS),cpp)
export OFILES               :=   $(CPP_FILES:.cpp=.o)

export PRECOMPILED_HEADER   :=   $(CURDIR)/include/dd.hpp
export GCH_FILES			:=   $(PRECOMPILED_HEADER:.hpp=.hpp.gch)

export TARGET     := dd

export EXE_CPPS   := main.cpp
export EXE_O      := $(EXE_CPPS:.cpp=.o)
export EXE_NAME   := learn

export INCLUDE    := $(foreach dir,$(INCLUDE_DIRS),-I$(CURDIR)/$(dir)) \
					 $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					 -I.
export LIBINC     := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					 -L.

# Include
export INCLUDE_DIRS := -I$(CURDIR)/include -I$(CURDIR)/third_party/include $(INCLUDE)

# Libs
export LIBS := -lstdc++ -lgdi32 -luser32 -lkernel32 -lopengl32

# Compiler Flags
export COMPILER_FLAGS :=  $(RELEASE_FLAGS) $(CXX_FLAGS) $(CXX_WARNS) $(INCLUDE_DIRS) $(LIBINC)

# Set make's prequisite paths
export VPATH := $(foreach dir,$(SOURCE_DIRS),$(CURDIR)/$(dir))\
                $(CURDIR)/include

.PHONY: clean all debug release lib/$(TARGET).a build/$(EXE_NAME).exe

all: build/$(EXE_NAME).exe

list:
	@echo $(SOURCE_DIRS)
	@echo $(CD)

clean:
	@echo cleaning ...
	@rm -fr build release_deps lib $(GCH_FILES)

lib:
	@[ -d $@ ] || mkdir -p $@

release_deps:
	@[ -d $@ ] || mkdir -p $@

build:
	@[ -d $@ ] || mkdir -p $@

lib/$(TARGET).a: lib release_deps $(SOURCE_DIRS) $(INCLUDES)
	@$(MAKE) BUILD=release_deps OUTPUTA=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/Makefile

build/$(EXE_NAME).exe: build release_deps $(SOURCE_DIRS) $(INCLUDES)
	@$(MAKE) BUILD=release_deps OUTPUTEXE=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/Makefile
	@echo $(OUTPUT)
	@echo Copying resources...
	@set CURRENT_DIRECTORY=%CD%
	@robocopy $(CURRENT_DIRECTORY)/resources/ $(CURRENT_DIRECTORY)/build/resources //E //NJH //NJS //NS //NC //NFL //NDL

else

# Dependencies for make
DEPENDS := $(OFILES:.o=.d) $(foreach hdr,$(GCH_FILES:.hpp.gch=.d),$(notdir $(hdr)))

$(OUTPUTEXE)  : $(OFILES)

$(OFILES)     : $(GCH_FILES)

-include $(DEPENDS)

endif

%.o : %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(COMPILER_FLAGS) -c $< -o $@ $(LIBS)

%.a:
	@rm -f $@
	@$(AR) -rc $@ $^

%.hpp.gch: %.hpp
	@echo Precompiling $(notdir $<)
	$(CXX) -w -x c++-header -MMD -MP -MQ$@ -MF $(DEPSDIR)/$(notdir $*).d $(COMPILER_FLAGS) -c $< -o $@ $(LIBS)

%.exe:
	@echo Compiling $@
	$(CXX) $(COMPILER_FLAGS) -pie -o $@ $(OFILES) $(LIBS)
