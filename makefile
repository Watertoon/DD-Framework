# Very basic makefile for small projects

# Compiler options
COMPILER_FLAGS := -msse4.1 -ffunction-sections -fdata-sections -fno-strict-aliasing -fwrapv -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-stack-protector -fno-rtti -fno-exceptions -std=gnu++20 -m64
COMPILER_WARNS := -Wall -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wno-invalid-offsetof -Wno-format-truncation -Wno-format-zero-length -Wno-stringop-truncation -Wextra -Werror -Wno-missing-field-initializers

# Modes 
RELEASE_FLAGS := -O3 -flto -g -gdwarf-4
DEBUG_FLAGS   := -g -Og -flto

# Input
SOURCE_INPUT := main.cpp
INCLUDE      := -Iincludes -Ithird_party/includes

# Libs
STD := -lstdc++ -lgdi32 -luser32 -lkernel32 -lopengl32

#Order matters sometimes (top to bottom)
.PHONY: clean all debug release

all: clean release debug

clean:
	rm -f release.exe debug.exe debug.d

debug:
	$(CXX) -MMD -MP $(DEBUG_FLAGS) $(COMPILER_FLAGS) $(COMPILER_WARNS) $(SOURCE_INPUT) -o debug.exe $(STD) $(INCLUDE)

release:
	$(CXX) $(RELEASE_FLAGS) $(COMPILER_FLAGS) $(COMPILER_WARNS) $(SOURCE_INPUT) -DNDEBUG -o release.exe $(STD) $(INCLUDE)