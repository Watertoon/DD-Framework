.SUFFIXES:
# Complex makefile for basic projects

THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIRECTORY := $(abspath $(dir $(THIS_MAKEFILE)))

# Include libraries
ifeq ($(strip $(VULKAN_SDK)),)
$(error "VULKAN_SDK is not set in your environment.")
endif

export GLSLC := $(VULKAN_SDK)/bin/glslc.exe

DEFINES  := -DDD_DEBUG
LIBDIRS  := $(VULKAN_SDK) third_party
INCLUDES := include

# Compiler options
CXX_FLAGS := -std=gnu++20 -m64 -msse4.1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-rtti -fno-exceptions $(DEFINES)
CXX_WARNS := -Wall -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wno-invalid-offsetof -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wextra -Werror -Wno-missing-field-initializers

# Release and Debug mode options 
RELEASE_FLAGS := -Og -flto -g -gdwarf-4
DEBUG_FLAGS   := -g -Og -flto -gdwarf-4

# Source input file iteration methods
DIRECTORY_WILDCARD  =   $(foreach d,$(wildcard $(1:=/*)),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))
GET_ALL_SOURCE_DIRS =$1 $(foreach d,$(wildcard $1/*),$(if $(wildcard $d/.),$(call DIRECTORY_WILDCARD,$d) $d,))

SOURCE_DIRS=$(call GET_ALL_SOURCE_DIRS,source)
SHADER_SOURCE_DIRS=$(call GET_ALL_SOURCE_DIRS,shader/source)

FIND_SOURCE_FILES=$(foreach dir,$1,$(notdir $(wildcard $(dir)/*.$2)))

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Our input
export CPP_FILES            :=   $(call FIND_SOURCE_FILES,$(SOURCE_DIRS),cpp)
export OFILES               :=   $(CPP_FILES:.cpp=.o)

export SH_FILES             :=   $(call FIND_SOURCE_FILES,$(SHADER_SOURCE_DIRS),sh)
export SPV_FILES             :=   $(SH_FILES:.sh=.spv)

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
export LIBS := -lstdc++ -lgdi32 -luser32 -lkernel32 -lvulkan-1

# Compiler Flags
export COMPILER_FLAGS :=  $(RELEASE_FLAGS) $(CXX_FLAGS) $(CXX_WARNS) $(INCLUDE_DIRS) $(LIBINC)

# Set make's prequisite paths
export VPATH := $(foreach dir,$(SOURCE_DIRS),$(CURDIR)/$(dir))\
                $(foreach dir,$(SHADER_SOURCE_DIRS),$(CURDIR)/$(dir))\
                $(CURDIR)/include

.PHONY: clean all debug release lib/$(TARGET).a build/$(EXE_NAME).exe

all: build/$(EXE_NAME).exe shaders

list:
	@echo $(SHADER_SOURCE_DIRS)
	@echo $(CD)
	@echo $(SH_FILES)
	@echo $(SPV_FILES)
	@echo $(SPVDIR)
	@echo $(GLSLC)
	@echo $(OFILES)

clean:
	@echo cleaning ...
	@rm -fr build release_deps shader_deps lib $(GCH_FILES)

lib:
	@[ -d $@ ] || mkdir -p $@

release_deps:
	@[ -d $@ ] || mkdir -p $@

build:
	@[ -d $@ ] || mkdir -p $@

shader_deps:
	@[ -d $@ ] || mkdir -p $@
	
build/shaders:
	@[ -d $@ ] || mkdir -p $@

lib/$(TARGET).a: lib release_deps $(SOURCE_DIRS) $(INCLUDES)
	@$(MAKE) BUILD=release_deps OUTPUTA=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/Makefile



build/$(EXE_NAME).exe: copy_resources release_deps build $(SOURCE_DIRS) $(INCLUDES)
	@$(MAKE) BUILD=release_deps OUTPUTEXE=$(CURDIR)/$@ \
	BUILD_CFLAGS="-DNDEBUG=0" \
	DEPSDIR=$(CURDIR)/release_deps \
	-C release_deps \
	-f $(CURDIR)/Makefile
	@echo $(OUTPUT)

shaders: shader_deps build/shaders $(SHADER_SOURCE_DIRS)
	@$(MAKE) SHADER=shader BUILD=shader_deps OUTPUTSH=$(CURDIR)/$@ \
	DEPSDIR=$(CURDIR)/shader_deps \
	-C shader_deps \
	-f $(CURDIR)/Makefile
	@echo $(OUTPUT)

copy_resources:
	@echo Copying resources...
	@set CURRENT_DIRECTORY=%CD%
	@robocopy $(CURRENT_DIRECTORY)/resources/ $(CURRENT_DIRECTORY)/build/resources //E //NJH //NJS //NS //NC //NFL //NDL ; test $$? -lt 7

else

ifeq ($(SHADER),shader)

DEPENDS := $(SPV_FILES:.spv=_vertex.d) $(SPV_FILES:.spv=_fragment.d)

$(SPV_FILES)  : print

print:
	@echo $(DEPENDS)

-include $(DEPENDS)

else

# Dependencies for make
DEPENDS := $(OFILES:.o=.d) $(foreach hdr,$(GCH_FILES:.hpp.gch=.d),$(notdir $(hdr)))

$(OUTPUTEXE)    : $(OFILES)

$(OFILES)       : $(GCH_FILES)

-include $(DEPENDS)

endif
endif

%.spv : %.sh
	@echo $(notdir $<)
	@echo
	@  $(GLSLC) -MD -MF $(DEPSDIR)/$*_vertex.d    -DDD_VERTEX_SHADER   -mfmt=bin --target-env=vulkan1.3 -fshader-stage=vert -I$(CURRENT_DIRECTORY)/shader/include -Werror -O -c $< -o $(CURRENT_DIRECTORY)/build/shaders/$*_vertex.spv
	@  $(GLSLC) -MD -MF $(DEPSDIR)/$*_fragment.d  -DDD_FRAGMENT_SHADER -mfmt=bin --target-env=vulkan1.3 -fshader-stage=frag -I$(CURRENT_DIRECTORY)/shader/include -Werror -O -c $< -o $(CURRENT_DIRECTORY)/build/shaders/$*_fragment.spv

# @$(GLSLC) -MD -MF $(DEPSDIR)/$*_tessellation_control.d    -DDD_TESSLLATION_CONTROL_SHADER    -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesscontrol -I$(CURRENT_DIRECTORY)/shader/include -werror -O -c $< -o $@
# @$(GLSLC) -MD -MF $(DEPSDIR)/$*_tessellation_evaluation.d -DDD_TESSLLATION_EVALUATION_SHADER -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesseval    -I$(CURRENT_DIRECTORY)/shader/include -werror -O -c $< -o $@
# @$(GLSLC) -MD -MF $(DEPSDIR)/$*_geometry.d                -DDD_GEOMETRY_SHADER               -mfmt=bin --target-env=vulkan1.3 -fshader-stage=geom        -I$(CURRENT_DIRECTORY)/shader/include -werror -O -c $< -o $@
# @$(GLSLC) -MD -MF $(DEPSDIR)/$*_compute.d                 -DDD_COMPUTE_SHADER                -mfmt=bin --target-env=vulkan1.3 -fshader-stage=comp        -I$(CURRENT_DIRECTORY)/shader/include -werror -O -c $< -o $@

%.o : %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(COMPILER_FLAGS) -c $< -o $@ $(LIBS)

%.a :
	@rm -f $@
	@$(AR) -rc $@ $^

%.hpp.gch: %.hpp
	@echo Precompiling $(notdir $<)
	$(CXX) -w -x c++-header -MMD -MP -MQ$@ -MF $(DEPSDIR)/$(notdir $*).d $(COMPILER_FLAGS) -c $< -o $@ $(LIBS)

%.exe:
	@echo Compiling $@
	$(CXX) $(COMPILER_FLAGS) -pie -o $@ $(OFILES) $(LIBS)
