# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/grant/Desktop/CS480/cs480Hooks/PA4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/grant/Desktop/CS480/cs480Hooks/PA4/build

# Utility rule file for PA4_SUCCESSFUL.

# Include the progress variables for this target.
include CMakeFiles/PA4_SUCCESSFUL.dir/progress.make

CMakeFiles/PA4_SUCCESSFUL: PA4
	/usr/bin/cmake -E echo 
	/usr/bin/cmake -E echo =====================
	/usr/bin/cmake -E echo \ \ Compile\ complete!
	/usr/bin/cmake -E echo =====================
	/usr/bin/cmake -E echo /home/grant/Desktop/CS480/cs480Hooks/PA4/build

PA4_SUCCESSFUL: CMakeFiles/PA4_SUCCESSFUL
PA4_SUCCESSFUL: CMakeFiles/PA4_SUCCESSFUL.dir/build.make
.PHONY : PA4_SUCCESSFUL

# Rule to build all files generated by this target.
CMakeFiles/PA4_SUCCESSFUL.dir/build: PA4_SUCCESSFUL
.PHONY : CMakeFiles/PA4_SUCCESSFUL.dir/build

CMakeFiles/PA4_SUCCESSFUL.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/PA4_SUCCESSFUL.dir/cmake_clean.cmake
.PHONY : CMakeFiles/PA4_SUCCESSFUL.dir/clean

CMakeFiles/PA4_SUCCESSFUL.dir/depend:
	cd /home/grant/Desktop/CS480/cs480Hooks/PA4/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/grant/Desktop/CS480/cs480Hooks/PA4 /home/grant/Desktop/CS480/cs480Hooks/PA4 /home/grant/Desktop/CS480/cs480Hooks/PA4/build /home/grant/Desktop/CS480/cs480Hooks/PA4/build /home/grant/Desktop/CS480/cs480Hooks/PA4/build/CMakeFiles/PA4_SUCCESSFUL.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/PA4_SUCCESSFUL.dir/depend
