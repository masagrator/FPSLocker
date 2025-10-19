# =============================================================================
# Configuration
# =============================================================================

# --- Compilers ---
CC = gcc
CXX = g++

# --- Directories ---
# Set the directory where your source files are
SRC_DIR = source
# Set the directory where built object files will go
BUILD_DIR = build

# --- Target ---
# Set the name of your final executable
TARGET = my_program

# --- Flags ---
# -I$(SRC_DIR): Tell GCC to look for headers in the source directory
#               (This allows #include "utils/my_header.h" from anywhere)
INCLUDE_FLAGS = -I$(SRC_DIR)

# -g:         Adds debugging information
# -Wall:      Enables all common warnings
# -Wextra:    Enables even more warnings
# -MMD -MP:   Generates dependency files (.d) for header tracking
DEP_FLAGS = -MMD -MP
COMMON_FLAGS = -g -Wall -Wextra $(DEP_FLAGS) $(INCLUDE_FLAGS) -DNDEBUG -DASMJIT_EMBED -DASMJIT_BUILD_RELEASE -DASMJIT_NO_X86 -DASMJIT_NO_DEPRECATED -DASMJIT_NO_ABI_NAMESPACE -DASMJIT_NO_JIT -DASMJIT_NO_LOGGING -DASMJIT_NO_VALIDATION

# Flags for C compilation
CFLAGS = -std=c23 $(COMMON_FLAGS)

# Flags for C++ compilation
CXXFLAGS = -std=c++23 $(COMMON_FLAGS)

# --- Linker Flags ---
LDFLAGS =
LDLIBS =

# =============================================================================
# File Discovery (Recursive)
# =============================================================================

# Use 'find' to recursively locate all source files
SRCS_C := $(shell find $(SRC_DIR) -name "*.c")

SRCS_CXX := $(shell find $(SRC_DIR) -name "*.cpp" -o -name "*.cxx" -o -name "*.cc")

# Generate object file names, mapping the source tree to the build tree
# (e.g., "source/utils/math.c" becomes "build/utils/math.o")
OBJS_C := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS_C))

OBJS_CXX := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter %.cpp, $(SRCS_CXX)))
OBJS_CXX += $(patsubst $(SRC_DIR)/%.cxx,$(BUILD_DIR)/%.o,$(filter %.cxx, $(SRCS_CXX)))
OBJS_CXX += $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(filter %.cc, $(SRCS_CXX)))

# Combine all object files
OBJS := $(OBJS_C) $(OBJS_CXX)

# Generate dependency file names (e.g., "build/utils/math.o" becomes "build/utils/math.d")
DEPS := $(OBJS:%.o=%.d)

# =============================================================================
# Main Targets
# =============================================================================

# The default target, called when you just type "make"
all: $(TARGET)

# Rule to link the final executable
# We use $(CXX) (g++) for linking to ensure C++ libraries are included
$(TARGET): $(OBJS)
	@echo "==> Linking $@"
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo "==> Build complete: $(TARGET)"

# Phony targets don't represent files
.PHONY: all clean

# Rule to clean up build files (removes the entire build dir and the target)
clean:
	@echo "==> Cleaning all built files"
	rm -rf $(BUILD_DIR) $(TARGET)

# =============================================================================
# Compilation Rules
# =============================================================================

# This pattern rule handles all C/C++ files.
# It matches a target like "build/utils/math.o" with a source like "source/utils/math.c"
# It also creates the directory (e.g., "build/utils") before compiling.

# --- C Compilation ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "==> Compiling C: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# --- C++ Compilation ---
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "==> Compiling C++: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cxx
	@mkdir -p $(dir $@)
	@echo "==> Compiling C++: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(dir $@)
	@echo "==> Compiling C++: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# =============================================================================
# Dependency Inclusion
# =============================================================================

# Include the generated dependency files
# The hyphen "-" tells 'make' to not error if the files don't exist
-include $(DEPS)