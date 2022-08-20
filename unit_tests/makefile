# By W. Michael
# Tests are currently only supported on x64 and Win32

export TEST_DIRS := lib_dd

export define MAKE_TESTS
$(MAKE) $(PLATFORM)_$(ARCHITECTURE)_$(GRAPHICS_API)_$(BINARY_TYPE) ARCHITECTURE=$(ARCHITECTURE) PLATFORM=$(PLATFORM) GRAPHICS_API=$(GRAPHICS_API) BINARY_TYPE=$(BINARY_TYPE) -C $(1)

endef

export define MAKE_CLEAN
$(MAKE) clean -C $(1)

endef

clean:
	$(foreach program,$(TEST_DIRS)_tests,$(call MAKE_CLEAN,$(program)))

sub_make:
	$(foreach program,$(TEST_DIRS)_tests,$(call MAKE_TESTS,$(program)))
