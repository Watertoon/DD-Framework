.SUFFIXES:

.PHONY: all clean win32_x64_release win32_x64_vulkan_develop win32_x64_vulkan_debug

export LIBRARY_LIST     := dd unit_tester
export PROGRAM_LIST     := test_program1
export TOOL_LIST        :=
export THIRD_PARTY_LIST := 
export UNIT_TEST_LIST   := unit_tests

ALL_PROGRAMS_LIST := $(THIRD_PARTY_LIST) libraries $(TOOL_LIST) $(PROGRAM_LIST) $(UNIT_TEST_LIST)

#Valid Architectures: x64
#Valid platforms:     win32
#Valid graphics apis: vulkan
#Valid binary types:  release develop debug


define MAKE_WIN32_X64_VULKAN_RELEASE
+$(MAKE) sub_make ARCHITECTURE=x64 PLATFORM=win32 GRAPHICS_API=vulkan BINARY_TYPE=release -C $(1)

endef

define MAKE_WIN32_X64_VULKAN_DEVELOP
+$(MAKE) sub_make ARCHITECTURE=x64 PLATFORM=win32 GRAPHICS_API=vulkan BINARY_TYPE=develop -C $(1)

endef

define MAKE_WIN32_X64_VULKAN_DEBUG
+$(MAKE) sub_make ARCHITECTURE=x64 PLATFORM=win32 GRAPHICS_API=vulkan BINARY_TYPE=debug -C $(1)

endef

define MAKE_CLEAN
$(MAKE) clean -C $(1)

endef

win32_x64_vulkan_develop:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call MAKE_WIN32_X64_VULKAN_DEVELOP,$(program)))

clean:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call MAKE_CLEAN,$(program)))

win32_x64_vulkan_release:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call MAKE_WIN32_X64_VULKAN_RELEASE,$(program)))

win32_x64_vulkan_debug:
	$(foreach program,$(ALL_PROGRAMS_LIST),$(call MAKE_WIN32_X64_VULKAN_DEBUG,$(program)))
