# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/cmake/1417/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1417/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build

# Include any dependencies generated for this target.
include StringUtilities/CMakeFiles/StringUtilities.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include StringUtilities/CMakeFiles/StringUtilities.dir/compiler_depend.make

# Include the progress variables for this target.
include StringUtilities/CMakeFiles/StringUtilities.dir/progress.make

# Include the compile flags for this target's objects.
include StringUtilities/CMakeFiles/StringUtilities.dir/flags.make

StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o: StringUtilities/CMakeFiles/StringUtilities.dir/flags.make
StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o: /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/StringUtilities/StringUtilities.cpp
StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o: StringUtilities/CMakeFiles/StringUtilities.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o -MF CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o.d -o CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o -c /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/StringUtilities/StringUtilities.cpp

StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/StringUtilities.dir/StringUtilities.cpp.i"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/StringUtilities/StringUtilities.cpp > CMakeFiles/StringUtilities.dir/StringUtilities.cpp.i

StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/StringUtilities.dir/StringUtilities.cpp.s"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/StringUtilities/StringUtilities.cpp -o CMakeFiles/StringUtilities.dir/StringUtilities.cpp.s

# Object files for target StringUtilities
StringUtilities_OBJECTS = \
"CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o"

# External object files for target StringUtilities
StringUtilities_EXTERNAL_OBJECTS =

StringUtilities/libStringUtilities.a: StringUtilities/CMakeFiles/StringUtilities.dir/StringUtilities.cpp.o
StringUtilities/libStringUtilities.a: StringUtilities/CMakeFiles/StringUtilities.dir/build.make
StringUtilities/libStringUtilities.a: StringUtilities/CMakeFiles/StringUtilities.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libStringUtilities.a"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && $(CMAKE_COMMAND) -P CMakeFiles/StringUtilities.dir/cmake_clean_target.cmake
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/StringUtilities.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
StringUtilities/CMakeFiles/StringUtilities.dir/build: StringUtilities/libStringUtilities.a
.PHONY : StringUtilities/CMakeFiles/StringUtilities.dir/build

StringUtilities/CMakeFiles/StringUtilities.dir/clean:
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities && $(CMAKE_COMMAND) -P CMakeFiles/StringUtilities.dir/cmake_clean.cmake
.PHONY : StringUtilities/CMakeFiles/StringUtilities.dir/clean

StringUtilities/CMakeFiles/StringUtilities.dir/depend:
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/StringUtilities /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/StringUtilities/CMakeFiles/StringUtilities.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : StringUtilities/CMakeFiles/StringUtilities.dir/depend

