
# Pull in architecture
ifeq ($(ARCHITECTURE), x64)
include $(dir $(lastword $(MAKEFILE_LIST)))/arch/arch_x64.mk
else
$(error Invalid ARCHITECTURE (must be "x64"))
endif

# Pull in platform
ifeq ($(PLATFORM), win32)
include $(dir $(lastword $(MAKEFILE_LIST)))../platform/platform_win32.mk
else
$(error Invalid PLATFORM (must be "win32"))
endif

# Pull in graphics api
ifeq ($(GRAPHICS_API), vulkan)
include $(dir $(lastword $(MAKEFILE_LIST)))../graphics_api/gfxapi_vulkan.mk
else
$(error Invalid GRAPHICS_API (must be "vulkan"))
endif

# Pull in binary type
ifeq ($(BINARY_TYPE), release)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_release.mk
else
ifeq ($(BINARY_TYPE), debug)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_debug.mk
else
ifeq ($(BINARY_TYPE), develop)
include $(dir $(lastword $(MAKEFILE_LIST)))../binary_type/bintype_develop.mk
else
$(error Invalid BINARY_TYPE (must be "release", "develop", or "debug"))
endif
endif
endif

ALL_COMPILER_FLAGS := $(ARCH_CXX_FLAGS) $(PLATFORM_C_FLAGS) $(PLATFORM_CXX_FLAGS) $(PLATFORM_INCLUDES) $(PLATFORM_LIB_INCLUDES) $(GFXAPI_INCLUDES) $(GFXAPI_LIB_INCLUDES) $(BINTYPE_CXX_FLAGS) $(PROJECT_C_FLAGS) $(PROJECT_CXX_FLAGS) $(PROJECT_INCLUDES) $(PROJECT_LIB_INCLUDES) -Os

ALL_LIBS := $(PLATFORM_LIBS) $(PROJECT_LIBS)

ifneq ($(strip $(VULKAN_SDK)),)
GLSLC ?= $(VULKAN_SDK)/bin/glslc.exe
endif

# Common compile recipes
%.vert.spv : %.vert.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.vert.d -DDD_VERTEX_SHADER                  -mfmt=bin --target-env=vulkan1.3 -fshader-stage=vert        -I$(CURDIR)/../shader/include -Werror -Os -c $< -o $(CURDIR)/../build/resource/shader/$*.vert.spv

%.frag.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.frag.d -DDD_FRAGMENT_SHADER                -mfmt=bin --target-env=vulkan1.3 -fshader-stage=frag        -I$(CURDIR)/../shader/include -Werror -Os -c $< -o $(CURDIR)/../build/resource/shader/$*.frag.spv

%.tsct.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.tsct.d -DDD_TESSELLATION_CONTROL_SHADER    -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesscontrol -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.tsct.spv

%.tsev.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.tsev.d -DDD_TESSELLATION_EVALUATION_SHADER -mfmt=bin --target-env=vulkan1.3 -fshader-stage=tesseval    -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.tsev.spv

%.geom.spv : %.frag.sh
	@echo $(notdir $<)	
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.geom.d -DDD_GEOMETRY_SHADER                -mfmt=bin --target-env=vulkan1.3 -fshader-stage=geom        -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.geom.spv

%.comp.spv : %.frag.sh
	@echo $(notdir $<)
	@$(GLSLC) -MD -MF $(DEPSDIR)/$*.comp.d -DDD_COMPUTE_SHADER                 -mfmt=bin --target-env=vulkan1.3 -fshader-stage=comp        -I$(CURDIR)/../shader/include -werror -O -c $< -o $(CURDIR)/../build/resource/shader/$*.comp.spv

%.o : %.cpp
	@echo $(notdir $<)
	@$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(ALL_COMPILER_FLAGS) -c $< -o $@ $(ALL_LIBS)

%.a :
	@rm -f $@
	$(AR) -rc $@ $^

%.hpp.gch: %.hpp
	@echo Precompiling $(notdir $<)
	$(CXX) -w -x c++-header -MMD -MP -MQ$@ -MF $(DEPSDIR)/$(notdir $*).d $(ALL_COMPILER_FLAGS) -c $< -o $@ $(ALL_LIBS)

%.exe:
	@echo Compiling $@
	$(CXX) $(ALL_COMPILER_FLAGS) $(EXE_FLAGS) -o $@ $^ $(ALL_LIBS)
