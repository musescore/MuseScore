#!/usr/bin/env -S cmake -P

# This cross-platform script automates the process of creating a build directory and
# compiling MuseScore. Despite being written in the CMake language, you should think
# of this file as like a shell script or batch file rather than as part of the CMake
# project tree. As always, the project tree starts in the top-level CMakeLists.txt.

string(TIMESTAMP SCRIPT_START_TIMESTAMP "%s" UTC)

if(NOT DEFINED CMAKE_SCRIPT_MODE_FILE)
    file(RELATIVE_PATH SCRIPT_PATH "${CMAKE_BINARY_DIR}" "${CMAKE_CURRENT_LIST_FILE}")
    message(FATAL_ERROR
        "This file is a script. You should run it with:\n"
        "  \$ cmake -P ${SCRIPT_PATH} [args...]\n"
        "Don't try to use it in other CMake files with include().\n"
        "See https://cmake.org/cmake/help/latest/manual/cmake.1.html#run-a-script"
    )
endif()

cmake_minimum_required(VERSION 3.22) # should match version in CMakeLists.txt

# CMake arguments up to '-P' (ignore these)
set(i "1")
while(i LESS "${CMAKE_ARGC}")
    if("${CMAKE_ARGV${i}}" STREQUAL "-P")
        math(EXPR i "${i} + 1") # ignore '-P' option
        break() # done with CMake arguments
    endif()
    math(EXPR i "${i} + 1") # next argument
endwhile()

# Script arguments (store these for later processing)
# By convention, SCRIPT_ARG[0] is the name of the script
set(SCRIPT_ARGS "") # empty argument list
while(i LESS "${CMAKE_ARGC}")
    list(APPEND SCRIPT_ARGS "${CMAKE_ARGV${i}}")
    math(EXPR i "${i} + 1") # next argument
endwhile()

# load custom CMake functions and macros
include("${CMAKE_CURRENT_LIST_DIR}/buildscripts/cmake/GetUtilsFunctions.cmake") # "fn__" namespace

# Set the name of the build folder (just the folder name, not the full path)
function(build_folder
    VAR_NAME # Name of variable to store build folder name
)
    # Windows has a 260 character limit on file paths, so the build folder
    # name must be abbreviated to prevent build files exceeding this limit.
    # The limit applies to the *entire* path, not just individual components.
    # Abbreviation is not essential on other platforms but is still welcome.
    if(WIN32)
        set(PLATFORM "Win")
    elseif(APPLE)
        set(PLATFORM "Mac")
    else()
        set(PLATFORM "${CMAKE_HOST_SYSTEM_NAME}")
    endif()
    set(GEN_SHORT "${GENERATOR}")
    string(REGEX REPLACE ".*Visual Studio ([0-9]+).*" "VS\\1" GEN_SHORT "${GEN_SHORT}")
    string(REGEX REPLACE ".*MinGW.*" "MinGW" GEN_SHORT "${GEN_SHORT}")
    string(REGEX REPLACE ".*Makefile.*" "Make" GEN_SHORT "${GEN_SHORT}")
    set(BUILD_FOLDER "${PLATFORM}-Qt${QT_VERSION}-${QT_COMPILER}-${GEN_SHORT}-${BUILD_TYPE}")
    string(REGEX REPLACE "[ /\\]" "" BUILD_FOLDER "${BUILD_FOLDER}") # remove spaces and slashes
    set("${VAR_NAME}" "${BUILD_FOLDER}" PARENT_SCOPE)
endfunction()

# Default values for independent build variables
set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}") # MuseScore repo (directory of this script)
set(ALL_BUILDS_PATH "${SOURCE_PATH}/builds") # Where all build folders will be created

# CPUS
cmake_host_system_information(RESULT CPUS QUERY NUMBER_OF_LOGICAL_CORES)
if(NOT "${CPUS}" GREATER "0")
    include(ProcessorCount)
    ProcessorCount(CPUS)
endif()

# Overrides
# Use this file to replace build variable defaults with your own values.
# E.g. set your own GENERATOR, BUILD_TYPE, BUILD_FOLDER, or CPUS.
if(EXISTS "${SOURCE_PATH}/build_overrides.cmake")
    message(STATUS "Including personal build overrides")
    include("${SOURCE_PATH}/build_overrides.cmake")
endif()

# QTDIR environment variable is set automatically by Qt Creator. It can also
# be set manually in build_overrides.cmake for use with other IDEs.
# Typical value for ENV{QTDIR}: /path/to/Qt/<version>/<compiler>
if(DEFINED ENV{QTDIR})
    # Add selected build kit to PATH.
    if(WIN32)
        set(ENV{PATH} "$ENV{QTDIR}/bin;$ENV{PATH}") # semicolon
    else()
        set(ENV{PATH} "$ENV{QTDIR}/bin:$ENV{PATH}") # colon
    endif()
endif()

fn__require_program(QMAKE Qt --version "https://musescore.org/en/handbook/developers-handbook/compilation" qmake6 qmake)
fn__set_qt_variables("${QMAKE}")
message(STATUS "QT_LOCATION: ${QT_LOCATION}")
message(STATUS "QT_VERSION: ${QT_VERSION}")
message(STATUS "QT_COMPILER: ${QT_COMPILER}")

# Process script arguments
set(i "1")
list(LENGTH SCRIPT_ARGS nargs)
while(i LESS "${nargs}")
    list(GET SCRIPT_ARGS "${i}" ARG)
    if("${ARG}" STREQUAL "clean")
        set(ARG_CLEAN "TRUE")
    elseif("${ARG}" STREQUAL "configure")
        set(ARG_CONFIGURE "TRUE")
    elseif("${ARG}" STREQUAL "build")
        set(ARG_CONFIGURE "TRUE")
        set(ARG_BUILD "TRUE")
    elseif("${ARG}" STREQUAL "install")
        set(ARG_CONFIGURE "TRUE")
        set(ARG_BUILD "TRUE")
        set(ARG_INSTALL "TRUE")
    elseif("${ARG}" STREQUAL "run")
        set(ARG_RUN "TRUE")
    else()
        # Other arguments are used by certain subprocesses in build steps
        if(ARG_RUN)
            list(APPEND RUN_ARGS "${ARG}") # args after "run" belong to Run step
        else()
            list(APPEND CONFIGURE_ARGS "${ARG}") # all other args belong to Configure step
        endif()
    endif()
    math(EXPR i "${i} + 1") # next argument
endwhile()

# Default if no build steps given as arguments
if(NOT (ARG_CLEAN OR ARG_CONFIGURE OR ARG_BUILD OR ARG_INSTALL OR ARG_RUN))
    set(ARG_CONFIGURE "TRUE")
    set(ARG_BUILD "TRUE")
    set(ARG_INSTALL "TRUE")
endif()

# Default values for build variables that depend on other build variables.
# These will only be set here if they were not already defined elsewhere,
# e.g. by script arguments or in build_overrides.cmake.

fn__get_option(GENERATOR -G ${CONFIGURE_ARGS})
if(WIN32)
    fn__set_default(GENERATOR "Visual Studio 17 2022")
else()
    fn__set_default(GENERATOR "Unix Makefiles")
endif()

fn__get_option(BUILD_TYPE -DCMAKE_BUILD_TYPE ${CONFIGURE_ARGS})
fn__set_default(BUILD_TYPE "RelWithDebInfo")

if(NOT DEFINED BUILD_FOLDER)
    build_folder(BUILD_FOLDER)
endif()
set(BUILD_PATH "${ALL_BUILDS_PATH}/${BUILD_FOLDER}")

fn__get_option(INSTALL_PATH -DCMAKE_INSTALL_PREFIX ${CONFIGURE_ARGS})
fn__set_default(INSTALL_PATH "install") # relative to BUILD_PATH

# MSCORE_EXECUTABLE (path relative to INSTALL_PATH)
if(WIN32)
    fn__set_default(MSCORE_EXECUTABLE "bin/MuseScore4.exe")
elseif(APPLE)
    fn__set_default(MSCORE_EXECUTABLE "mscore.app/Contents/MacOS/mscore")
else()
    fn__set_default(MSCORE_EXECUTABLE "bin/mscore")
endif()

# make paths absolute if they are not already
get_filename_component(BUILD_PATH "${BUILD_PATH}" ABSOLUTE BASE_DIR "${SOURCE_PATH}")
get_filename_component(INSTALL_PATH "${INSTALL_PATH}" ABSOLUTE BASE_DIR "${BUILD_PATH}")
get_filename_component(MSCORE_EXECUTABLE "${MSCORE_EXECUTABLE}" ABSOLUTE BASE_DIR "${INSTALL_PATH}")

message(STATUS "SOURCE_PATH:       ${SOURCE_PATH}")
message(STATUS "BUILD_PATH:        ${BUILD_PATH}")
message(STATUS "INSTALL_PATH:      ${INSTALL_PATH}")
message(STATUS "MSCORE_EXECUTABLE: ${MSCORE_EXECUTABLE}")
message(STATUS "CPUS: ${CPUS}")
message(STATUS "GENERATOR: ${GENERATOR}")
message(STATUS "BUILD_TYPE: ${BUILD_TYPE}")

list(APPEND CONFIGURE_ARGS "-G" "${GENERATOR}")
list(APPEND CONFIGURE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_PATH}")
list(APPEND CONFIGURE_ARGS "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")

if(QT_COMPILER MATCHES "_64$" OR QT_COMPILER MATCHES "mingw64")
    list(APPEND CONFIGURE_ARGS "-DMUSE_COMPILE_BUILD_64=ON")
else()
    list(APPEND CONFIGURE_ARGS "-DMUSE_COMPILE_BUILD_64=OFF")
endif()

#### ACTUAL BUILD STEPS START HERE ####

# Clean - delete an existing build directory.
#
# We usually avoid this because performing a clean build takes much longer
# than an incremental build, but it is occasionally necessary. If you
# encounter errors during a build then you should try doing a clean build.

if(ARG_CLEAN)
    message("\n~~~~ Actualizing Clean step ~~~~\n")
    message("Deleting ${BUILD_PATH}")
    file(REMOVE_RECURSE "${BUILD_PATH}")
endif()

# Configure - generate build system for the native build tool.
#
# We only do this explicitly on the very first build. On subsequent builds
# the native build tool will redo the configuration if any CMake files were
# edited. You can force this to happen by deleting CMakeCache.txt.

if(ARG_CONFIGURE AND NOT EXISTS "${BUILD_PATH}/CMakeCache.txt")
    message("\n~~~~ Actualizing Configure step ~~~~\n")
    file(MAKE_DIRECTORY "${BUILD_PATH}")
    if(QT_COMPILER MATCHES "msvc")
        set(CMAKE_WRAPPER "${SOURCE_PATH}/buildscripts/tools/cmake_wrapper.bat")
    else()
        set(CMAKE_WRAPPER "cmake")
    endif()
    fn__command_string(ARGS_STR ${CONFIGURE_ARGS})
    message("CONFIGURE_ARGS: ${ARGS_STR}")
    execute_process(
        # List of CMake arguments in CONFIGURE_ARGS variable. It must be unquoted here.
        COMMAND "${CMAKE_WRAPPER}" -S "${SOURCE_PATH}" -B . ${CONFIGURE_ARGS}
        WORKING_DIRECTORY "${BUILD_PATH}"
        RESULT_VARIABLE EXIT_STATUS
    )
    if(NOT "${EXIT_STATUS}" EQUAL "0")
        file(REMOVE "${BUILD_PATH}/CMakeCache.txt") # need to configure again next time
        message(FATAL_ERROR "Configure step failed with status ${EXIT_STATUS}. See output above for details.")
    endif()
endif()

# Build - compile code with the native build tool.
#
# We always do this. We can rely on the native build tool to be efficient and
# only (re)compile source files that have been edited since the last build.

if(ARG_BUILD)
    message("\n~~~~ Actualizing Build step ~~~~\n")
    execute_process(
        COMMAND cmake --build . --config "${BUILD_TYPE}" --parallel "${CPUS}"
        WORKING_DIRECTORY "${BUILD_PATH}"
        RESULT_VARIABLE EXIT_STATUS
    )
    if(NOT "${EXIT_STATUS}" EQUAL "0")
        message(FATAL_ERROR "Build step failed with status ${EXIT_STATUS}. See output above for details.")
    endif()
endif()

# Install - move compiled files to destination folders.
#
# Again, we always do this and rely on the tool itself to be efficient and
# only install files that have changed since last time.

if(ARG_INSTALL)
    message("\n~~~~ Actualizing Install step ~~~~\n")
    execute_process(
        COMMAND cmake --install . --config "${BUILD_TYPE}"
        WORKING_DIRECTORY "${BUILD_PATH}"
        RESULT_VARIABLE EXIT_STATUS
    )
    if(NOT "${EXIT_STATUS}" EQUAL "0")
        message(FATAL_ERROR "Install step failed with status ${EXIT_STATUS}. See output above for details.")
    endif()
endif()

# Run - attempt to run the compiled program within the installation folder.
#
# The working directory is unchanged. Use build_override.cmake to set the
# CMake variable RUN_ARGS to contain a list of arguments to pass to MuseScore
# on the command line. In addition, script arguments after "run" will be
# appended to this list, but note that certain arguments cannot be passed this
# way (e.g. --help, --version) because they cancel CMake script processing.

if(ARG_RUN)
    message("\n~~~~ Actualizing Run step ~~~~\n")
    if(WIN32)
        set(CMD "cmd.exe" "/c") # allow CMake to launch a GUI application
    else()
        set(CMD "") # not an issue on other platforms
    endif()
    fn__command_string(ARGS_STR ${RUN_ARGS})
    message("RUN_ARGS: ${ARGS_STR}")
    execute_process(
        # List of MuseScore arguments in RUN_ARGS variable. It must be unquoted here.
        COMMAND ${CMD} "${MSCORE_EXECUTABLE}" ${RUN_ARGS}
        RESULT_VARIABLE EXIT_STATUS
    )
    if(NOT "${EXIT_STATUS}" EQUAL "0")
        message(FATAL_ERROR "Run step failed with status ${EXIT_STATUS}. See output above for details.")
    endif()
endif()

# Package - create installer archive for distribution to end users.
# TODO

string(TIMESTAMP SCRIPT_END_TIMESTAMP "%s" UTC)
math(EXPR SCRIPT_ELAPSED_TIME "${SCRIPT_END_TIMESTAMP} - ${SCRIPT_START_TIMESTAMP}")
math(EXPR SCRIPT_ELAPSED_MINS "${SCRIPT_ELAPSED_TIME} / 60")
math(EXPR SCRIPT_ELAPSED_SECS "${SCRIPT_ELAPSED_TIME} % 60")
list(GET SCRIPT_ARGS "0" SCRIPT_NAME)
message("\n${SCRIPT_NAME}: Complete after ${SCRIPT_ELAPSED_MINS} minutes and ${SCRIPT_ELAPSED_SECS} seconds")
