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
include MainApp/CMakeFiles/MainApp.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include MainApp/CMakeFiles/MainApp.dir/compiler_depend.make

# Include the progress variables for this target.
include MainApp/CMakeFiles/MainApp.dir/progress.make

# Include the compile flags for this target's objects.
include MainApp/CMakeFiles/MainApp.dir/flags.make

MainApp/CMakeFiles/MainApp.dir/main.cpp.o: MainApp/CMakeFiles/MainApp.dir/flags.make
MainApp/CMakeFiles/MainApp.dir/main.cpp.o: /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/MainApp/main.cpp
MainApp/CMakeFiles/MainApp.dir/main.cpp.o: MainApp/CMakeFiles/MainApp.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object MainApp/CMakeFiles/MainApp.dir/main.cpp.o"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT MainApp/CMakeFiles/MainApp.dir/main.cpp.o -MF CMakeFiles/MainApp.dir/main.cpp.o.d -o CMakeFiles/MainApp.dir/main.cpp.o -c /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/MainApp/main.cpp

MainApp/CMakeFiles/MainApp.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MainApp.dir/main.cpp.i"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/MainApp/main.cpp > CMakeFiles/MainApp.dir/main.cpp.i

MainApp/CMakeFiles/MainApp.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MainApp.dir/main.cpp.s"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/MainApp/main.cpp -o CMakeFiles/MainApp.dir/main.cpp.s

# Object files for target MainApp
MainApp_OBJECTS = \
"CMakeFiles/MainApp.dir/main.cpp.o"

# External object files for target MainApp
MainApp_EXTERNAL_OBJECTS =

MainApp/MainApp: MainApp/CMakeFiles/MainApp.dir/main.cpp.o
MainApp/MainApp: MainApp/CMakeFiles/MainApp.dir/build.make
MainApp/MainApp: MathFunctions/libMathFunctions.a
MainApp/MainApp: StringUtilities/libStringUtilities.a
MainApp/MainApp: MainApp/CMakeFiles/MainApp.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable MainApp"
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/MainApp.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
MainApp/CMakeFiles/MainApp.dir/build: MainApp/MainApp
.PHONY : MainApp/CMakeFiles/MainApp.dir/build

MainApp/CMakeFiles/MainApp.dir/clean:
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp && $(CMAKE_COMMAND) -P CMakeFiles/MainApp.dir/cmake_clean.cmake
.PHONY : MainApp/CMakeFiles/MainApp.dir/clean

MainApp/CMakeFiles/MainApp.dir/depend:
	cd /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/MainApp /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp /home/sssk/kurs_3/prak5sem/cpp/assignments/lesson04/frolova/MultiModuleProject/build/MainApp/CMakeFiles/MainApp.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : MainApp/CMakeFiles/MainApp.dir/depend
