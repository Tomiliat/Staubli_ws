# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /home/tomiliatsereh/backupstaubli_ws/src/descartes/descartes_tests

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests

# Utility rule file for clean_test_results_descartes_tests.

# Include the progress variables for this target.
include CMakeFiles/clean_test_results_descartes_tests.dir/progress.make

CMakeFiles/clean_test_results_descartes_tests:
	/usr/bin/python2 /opt/ros/melodic/share/catkin/cmake/test/remove_test_results.py /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests/test_results/descartes_tests

clean_test_results_descartes_tests: CMakeFiles/clean_test_results_descartes_tests
clean_test_results_descartes_tests: CMakeFiles/clean_test_results_descartes_tests.dir/build.make

.PHONY : clean_test_results_descartes_tests

# Rule to build all files generated by this target.
CMakeFiles/clean_test_results_descartes_tests.dir/build: clean_test_results_descartes_tests

.PHONY : CMakeFiles/clean_test_results_descartes_tests.dir/build

CMakeFiles/clean_test_results_descartes_tests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/clean_test_results_descartes_tests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/clean_test_results_descartes_tests.dir/clean

CMakeFiles/clean_test_results_descartes_tests.dir/depend:
	cd /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tomiliatsereh/backupstaubli_ws/src/descartes/descartes_tests /home/tomiliatsereh/backupstaubli_ws/src/descartes/descartes_tests /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests /home/tomiliatsereh/backupstaubli_ws/build/descartes_tests/CMakeFiles/clean_test_results_descartes_tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/clean_test_results_descartes_tests.dir/depend

