
export define MAKE_WIN32_X64_DEVELOP
$(MAKE) win32_x64__vulkan_develop ARCHITECTURE=x64 PLATFORM=win32 GRAPHICS_API=vulkan BINARY_TYPE=develop -C lib_$(1)

endef

export define MAKE_LIBRARY
$(MAKE) $(PLATFORM)_$(ARCHITECTURE)_$(GRAPHICS_API)_$(BINARY_TYPE) ARCHITECTURE=$(ARCHITECTURE) PLATFORM=$(PLATFORM) GRAPHICS_API=$(GRAPHICS_API) BINARY_TYPE=$(BINARY_TYPE) -C lib_$(1) 

endef

define MAKE_CLEAN
$(MAKE) clean -C lib_$(1) 

endef

win32_x64_vulkan_develop:
	$(foreach program,$(LIBRARY_LIST),$(call MAKE_WIN32_X64_DEVELOP,$(program)))

clean:
	$(foreach program,$(LIBRARY_LIST),$(call MAKE_CLEAN,$(program)))

sub_make:
	$(foreach program,$(LIBRARY_LIST),$(call MAKE_LIBRARY,$(program)))
