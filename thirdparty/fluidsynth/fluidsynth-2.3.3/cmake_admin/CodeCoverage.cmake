# Copyright (c) 2012 - 2017, Lars Bilke
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# CHANGES:
#
# 2012-01-31, Lars Bilke
# - Enable Code Coverage
#
# 2013-09-17, Joakim Söderberg
# - Added support for Clang.
# - Some additional usage instructions.
#
# 2016-02-03, Lars Bilke
# - Refactored functions to use named parameters
#
# 2017-06-02, Lars Bilke
# - Merged with modified version from github.com/ufz/ogs
#
# 2019-05-06, Anatolii Kurotych
# - Remove unnecessary --coverage flag
#
# 2019-12-13, FeRD (Frank Dana)
# - Deprecate COVERAGE_LCOVR_EXCLUDES and COVERAGE_GCOVR_EXCLUDES lists in favor
#   of tool-agnostic COVERAGE_EXCLUDES variable, or EXCLUDE setup arguments.
# - CMake 3.4+: All excludes can be specified relative to BASE_DIRECTORY
# - All setup functions: accept BASE_DIRECTORY, EXCLUDE list
# - Set lcov basedir with -b argument
# - Add automatic --demangle-cpp in lcovr, if 'c++filt' is available (can be
#   overridden with NO_DEMANGLE option in setup_target_for_coverage_lcovr().)
# - Delete output dir, .info file on 'make clean'
# - Remove Python detection, since version mismatches will break gcovr
# - Minor cleanup (lowercase function names, update examples...)
#
# 2019-12-19, FeRD (Frank Dana)
# - Rename Lcov outputs, make filtered file canonical, fix cleanup for targets
#
# 2020-01-19, Bob Apthorpe
# - Added gfortran support
#
# 2020-02-17, FeRD (Frank Dana)
# - Make all add_custom_target()s VERBATIM to auto-escape wildcard characters
#   in EXCLUDEs, and remove manual escaping from gcovr targets
#
# 2021-01-19, Robin Mueller
# - Add CODE_COVERAGE_VERBOSE option which will allow to print out commands which are run
# - Added the option for users to set the GCOVR_ADDITIONAL_ARGS variable to supply additional
#   flags to the gcovr command
#
# 2020-05-04, Mihchael Davis
#     - Add -fprofile-abs-path to make gcno files contain absolute paths
#     - Fix BASE_DIRECTORY not working when defined
#     - Change BYPRODUCT from folder to index.html to stop ninja from complaining about double defines
# USAGE:
#
# 1. Copy this file into your cmake modules path.
#
# 2. Add the following line to your CMakeLists.txt (best inside an if-condition
#    using a CMake option() to enable it just optionally):
#      include(CodeCoverage)
#
# 3. Append necessary compiler flags:
#      append_coverage_compiler_flags()
#
# 3.a (OPTIONAL) Set appropriate optimization flags, e.g. -O0, -O1 or -Og
#
# 4. If you need to exclude additional directories from the report, specify them
#    using full paths in the COVERAGE_EXCLUDES variable before calling
#    setup_target_for_coverage_*().
#    Example:
#      set(COVERAGE_EXCLUDES
#          '${PROJECT_SOURCE_DIR}/src/dir1/*'
#          '/path/to/my/src/dir2/*')
#    Or, use the EXCLUDE argument to setup_target_for_coverage_*().
#    Example:
#      setup_target_for_coverage_lcov(
#          NAME coverage
#          EXECUTABLE testrunner
#          EXCLUDE "${PROJECT_SOURCE_DIR}/src/dir1/*" "/path/to/my/src/dir2/*")
#
# 4.a NOTE: With CMake 3.4+, COVERAGE_EXCLUDES or EXCLUDE can also be set
#     relative to the BASE_DIRECTORY (default: PROJECT_SOURCE_DIR)
#     Example:
#       set(COVERAGE_EXCLUDES "dir1/*")
#       setup_target_for_coverage_gcovr_html(
#           NAME coverage
#           EXECUTABLE testrunner
#           BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
#           EXCLUDE "dir2/*")
#
# 5. Use the functions described below to create a custom make target which
#    runs your test executable and produces a code coverage report.
#
# 6. Build a Debug build:
#      cmake -DCMAKE_BUILD_TYPE=Debug ..
#      make
#      make my_coverage_target
#

include(CMakeParseArguments)

option(CODE_COVERAGE_VERBOSE "Verbose information" FALSE)

# Check prereqs
find_program( GCOV_PATH gcov )
find_program( LCOV_PATH  NAMES lcov lcov.bat lcov.exe lcov.perl)
find_program( FASTCOV_PATH NAMES fastcov fastcov.py )
find_program( GENHTML_PATH NAMES genhtml genhtml.perl genhtml.bat )
find_program( GCOVR_PATH gcovr PATHS ${CMAKE_SOURCE_DIR}/scripts/test)
find_program( CPPFILT_PATH NAMES c++filt )

if(NOT GCOV_PATH)
    message(FATAL_ERROR "gcov not found! Aborting...")
endif() # NOT GCOV_PATH

get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
list(GET LANGUAGES 0 LANG)

if("${CMAKE_${LANG}_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
    if("${CMAKE_${LANG}_COMPILER_VERSION}" VERSION_LESS 3)
        message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
    endif()
elseif(NOT CMAKE_COMPILER_IS_GNUCXX)
    if("${CMAKE_Fortran_COMPILER_ID}" MATCHES "[Ff]lang")
        # Do nothing; exit conditional without error if true
    elseif("${CMAKE_Fortran_COMPILER_ID}" MATCHES "GNU")
        # Do nothing; exit conditional without error if true
    else()
        message(FATAL_ERROR "Compiler is not GNU gcc! Aborting...")
    endif()
endif()

set(COVERAGE_COMPILER_FLAGS "-g -fprofile-arcs -ftest-coverage"
    CACHE INTERNAL "")
if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-fprofile-abs-path HAVE_fprofile_abs_path)
    if(HAVE_fprofile_abs_path)
        set(COVERAGE_COMPILER_FLAGS "${COVERAGE_COMPILER_FLAGS} -fprofile-abs-path")
    endif()
endif()

set(CMAKE_Fortran_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the Fortran compiler during coverage builds."
    FORCE )
set(CMAKE_CXX_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during coverage builds."
    FORCE )
set(CMAKE_C_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C compiler during coverage builds."
    FORCE )
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used for linking binaries during coverage builds."
    FORCE )
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used by the shared libraries linker during coverage builds."
    FORCE )
mark_as_advanced(
    CMAKE_Fortran_FLAGS_COVERAGE
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE )

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
endif() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_Fortran_COMPILER_ID STREQUAL "GNU")
    link_libraries(gcov)
endif()

# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage_lcov(
#     NAME testrunner_coverage                    # New target name
#     EXECUTABLE testrunner -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES testrunner                     # Dependencies to build first
#     BASE_DIRECTORY "../"                        # Base directory for report
#                                                 #  (defaults to PROJECT_SOURCE_DIR)
#     EXCLUDE "src/dir1/*" "src/dir2/*"           # Patterns to exclude (can be relative
#                                                 #  to BASE_DIRECTORY, with CMake 3.4+)
#     NO_DEMANGLE                                 # Don't demangle C++ symbols
#                                                 #  even if c++filt is found
# )
function(setup_target_for_coverage_lcov)

    set(options NO_DEMANGLE)
    set(oneValueArgs BASE_DIRECTORY NAME)
    set(multiValueArgs EXCLUDE EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES LCOV_ARGS GENHTML_ARGS)
    cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT LCOV_PATH)
        message(FATAL_ERROR "lcov not found! Aborting...")
    endif() # NOT LCOV_PATH

    if(NOT GENHTML_PATH)
        message(FATAL_ERROR "genhtml not found! Aborting...")
    endif() # NOT GENHTML_PATH

    # Set base directory (as absolute path), or default to PROJECT_SOURCE_DIR
    if(DEFINED Coverage_BASE_DIRECTORY)
        get_filename_component(BASEDIR ${Coverage_BASE_DIRECTORY} ABSOLUTE)
    else()
        set(BASEDIR ${PROJECT_SOURCE_DIR})
    endif()

    # Collect excludes (CMake 3.4+: Also compute absolute paths)
    set(LCOV_EXCLUDES "")
    foreach(EXCLUDE ${Coverage_EXCLUDE} ${COVERAGE_EXCLUDES} ${COVERAGE_LCOV_EXCLUDES})
        if(CMAKE_VERSION VERSION_GREATER 3.4)
            get_filename_component(EXCLUDE ${EXCLUDE} ABSOLUTE BASE_DIR ${BASEDIR})
        endif()
        list(APPEND LCOV_EXCLUDES "${EXCLUDE}")
    endforeach()
    list(REMOVE_DUPLICATES LCOV_EXCLUDES)

    # Conditional arguments
    if(CPPFILT_PATH AND NOT ${Coverage_NO_DEMANGLE})
      set(GENHTML_EXTRA_ARGS "--demangle-cpp")
    endif()

    # enable branch coverage
    set ( Coverage_LCOV_ARGS ${Coverage_LCOV_ARGS} --rc lcov_branch_coverage=1 )
    set ( Coverage_GENHTML_ARGS ${Coverage_GENHTML_ARGS} --branch-coverage )

    # Setting up commands which will be run to generate coverage data.
    # Cleanup lcov
    set(LCOV_CLEAN_CMD 
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} -directory . 
        -b ${BASEDIR} --zerocounters
    )
    # Create baseline to make sure untouched files show up in the report
    set(LCOV_BASELINE_CMD 
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} -c -i -d . -b 
        ${BASEDIR} -o ${Coverage_NAME}.base
    )
    # Run tests
    set(LCOV_EXEC_TESTS_CMD 
        ${Coverage_EXECUTABLE} ${Coverage_EXECUTABLE_ARGS}
    )    
    # Capturing lcov counters and generating report
    set(LCOV_CAPTURE_CMD 
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} --directory . -b 
        ${BASEDIR} --capture --output-file ${Coverage_NAME}.capture
    )
    # add baseline counters
    set(LCOV_BASELINE_COUNT_CMD
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} -a ${Coverage_NAME}.base 
        -a ${Coverage_NAME}.capture --output-file ${Coverage_NAME}.total
    ) 
    # filter collected data to final coverage report
    set(LCOV_FILTER_CMD 
        ${LCOV_PATH} ${Coverage_LCOV_ARGS} --gcov-tool ${GCOV_PATH} --remove 
        ${Coverage_NAME}.total ${LCOV_EXCLUDES} --output-file ${Coverage_NAME}.info
    )    
    # Generate HTML output
    set(LCOV_GEN_HTML_CMD
        ${GENHTML_PATH} ${GENHTML_EXTRA_ARGS} ${Coverage_GENHTML_ARGS} -o 
        ${Coverage_NAME} ${Coverage_NAME}.info
    )
    

    if(CODE_COVERAGE_VERBOSE)
        message(STATUS "Executed command report")
        message(STATUS "Command to clean up lcov: ")
        string(REPLACE ";" " " LCOV_CLEAN_CMD_SPACED "${LCOV_CLEAN_CMD}")
        message(STATUS "${LCOV_CLEAN_CMD_SPACED}")

        message(STATUS "Command to create baseline: ")
        string(REPLACE ";" " " LCOV_BASELINE_CMD_SPACED "${LCOV_BASELINE_CMD}")
        message(STATUS "${LCOV_BASELINE_CMD_SPACED}")

        message(STATUS "Command to run the tests: ")
        string(REPLACE ";" " " LCOV_EXEC_TESTS_CMD_SPACED "${LCOV_EXEC_TESTS_CMD}")
        message(STATUS "${LCOV_EXEC_TESTS_CMD_SPACED}")

        message(STATUS "Command to capture counters and generate report: ")
        string(REPLACE ";" " " LCOV_CAPTURE_CMD_SPACED "${LCOV_CAPTURE_CMD}")
        message(STATUS "${LCOV_CAPTURE_CMD_SPACED}")

        message(STATUS "Command to add baseline counters: ")
        string(REPLACE ";" " " LCOV_BASELINE_COUNT_CMD_SPACED "${LCOV_BASELINE_COUNT_CMD}")
        message(STATUS "${LCOV_BASELINE_COUNT_CMD_SPACED}")

        message(STATUS "Command to filter collected data: ")
        string(REPLACE ";" " " LCOV_FILTER_CMD_SPACED "${LCOV_FILTER_CMD}")
        message(STATUS "${LCOV_FILTER_CMD_SPACED}")

        message(STATUS "Command to generate lcov HTML output: ")
        string(REPLACE ";" " " LCOV_GEN_HTML_CMD_SPACED "${LCOV_GEN_HTML_CMD}")
        message(STATUS "${LCOV_GEN_HTML_CMD_SPACED}")
    endif()

    # Setup target
    add_custom_target(${Coverage_NAME}
        COMMAND ${LCOV_CLEAN_CMD}
        COMMAND ${LCOV_BASELINE_CMD} 
        COMMAND ${LCOV_EXEC_TESTS_CMD}
        COMMAND ${LCOV_CAPTURE_CMD}
        COMMAND ${LCOV_BASELINE_COUNT_CMD}
        COMMAND ${LCOV_FILTER_CMD} 
        COMMAND ${LCOV_GEN_HTML_CMD}

        # Set output files as GENERATED (will be removed on 'make clean')
        BYPRODUCTS
            ${Coverage_NAME}.base
            ${Coverage_NAME}.capture
            ${Coverage_NAME}.total
            ${Coverage_NAME}.info
            ${Coverage_NAME}/index.html
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        VERBATIM # Protect arguments to commands
        COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
    )

    # Show where to find the lcov info report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Lcov code coverage info report saved in ${Coverage_NAME}.info."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Open ./${Coverage_NAME}/index.html in your browser to view the coverage report."
    )

endfunction() # setup_target_for_coverage_lcov

# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage_gcovr_html(
#     NAME ctest_coverage                    # New target name
#     EXECUTABLE ctest -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES executable_target         # Dependencies to build first
#     BASE_DIRECTORY "../"                   # Base directory for report
#                                            #  (defaults to PROJECT_SOURCE_DIR)
#     EXCLUDE "src/dir1/*" "src/dir2/*"      # Patterns to exclude (can be relative
#                                            #  to BASE_DIRECTORY, with CMake 3.4+)
# )
# The user can set the variable GCOVR_ADDITIONAL_ARGS to supply additional flags to the
# GCVOR command.
function(setup_target_for_coverage_gcovr_html)

    set(options NONE)
    set(oneValueArgs BASE_DIRECTORY NAME)
    set(multiValueArgs EXCLUDE EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES)
    cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT GCOVR_PATH)
        message(FATAL_ERROR "gcovr not found! Aborting...")
    endif() # NOT GCOVR_PATH

    # Set base directory (as absolute path), or default to PROJECT_SOURCE_DIR
    if(DEFINED Coverage_BASE_DIRECTORY)
        get_filename_component(BASEDIR ${Coverage_BASE_DIRECTORY} ABSOLUTE)
    else()
        set(BASEDIR ${PROJECT_SOURCE_DIR})
    endif()

    # Collect excludes (CMake 3.4+: Also compute absolute paths)
    set(GCOVR_EXCLUDES "")
    foreach(EXCLUDE ${Coverage_EXCLUDE} ${COVERAGE_EXCLUDES} ${COVERAGE_GCOVR_EXCLUDES})
        if(CMAKE_VERSION VERSION_GREATER 3.4)
            get_filename_component(EXCLUDE ${EXCLUDE} ABSOLUTE BASE_DIR ${BASEDIR})
        endif()
        list(APPEND GCOVR_EXCLUDES "${EXCLUDE}")
    endforeach()
    list(REMOVE_DUPLICATES GCOVR_EXCLUDES)

    # Combine excludes to several -e arguments
    set(GCOVR_EXCLUDE_ARGS "")
    foreach(EXCLUDE ${GCOVR_EXCLUDES})
        list(APPEND GCOVR_EXCLUDE_ARGS "-e")
        list(APPEND GCOVR_EXCLUDE_ARGS "${EXCLUDE}")
    endforeach()

    # Set up commands which will be run to generate coverage data
    # Run tests
    set(GCOVR_HTML_EXEC_TESTS_CMD
        ${Coverage_EXECUTABLE} ${Coverage_EXECUTABLE_ARGS}
    )
    # Create folder
    set(GCOVR_HTML_FOLDER_CMD
        ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/${Coverage_NAME}
    )
    # Running gcovr
    set(GCOVR_HTML_CMD
        ${GCOVR_PATH} --html --html-details -r ${BASEDIR} ${GCOVR_ADDITIONAL_ARGS}
        ${GCOVR_EXCLUDE_ARGS} --object-directory=${PROJECT_BINARY_DIR}
        -o ${Coverage_NAME}/index.html
    )

    # The --keep option is broken since gcovr 4.0 all our gcov files are deleted before SonarQube
    # can use them. Hence create an additional sonarqube report, but do not fail if this fails.
    set(GCOVR_SONAR_CMD
        ${GCOVR_PATH} --sonarqube -r ${BASEDIR} ${GCOVR_ADDITIONAL_ARGS}
        ${GCOVR_EXCLUDE_ARGS} --object-directory=${PROJECT_BINARY_DIR}
        -o ${Coverage_NAME}/sonarqube.report || true
    )

    if(CODE_COVERAGE_VERBOSE)
        message(STATUS "Executed command report")

        message(STATUS "Command to run tests: ")
        string(REPLACE ";" " " GCOVR_HTML_EXEC_TESTS_CMD_SPACED "${GCOVR_HTML_EXEC_TESTS_CMD}")
        message(STATUS "${GCOVR_HTML_EXEC_TESTS_CMD_SPACED}")

        message(STATUS "Command to create a folder: ")
        string(REPLACE ";" " " GCOVR_HTML_FOLDER_CMD_SPACED "${GCOVR_HTML_FOLDER_CMD}")
        message(STATUS "${GCOVR_HTML_FOLDER_CMD_SPACED}")

        message(STATUS "Command to generate gcovr HTML coverage data: ")
        string(REPLACE ";" " " GCOVR_HTML_CMD_SPACED "${GCOVR_HTML_CMD}")
        message(STATUS "${GCOVR_HTML_CMD_SPACED}")
    endif()

    add_custom_target(${Coverage_NAME}
        COMMAND ${GCOVR_HTML_EXEC_TESTS_CMD}
        COMMAND ${GCOVR_HTML_FOLDER_CMD}
        COMMAND ${GCOVR_HTML_CMD}
        COMMAND ${GCOVR_SONAR_CMD}

        BYPRODUCTS ${PROJECT_BINARY_DIR}/${Coverage_NAME}/index.html  # report directory
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        VERBATIM # Protect arguments to commands
        COMMENT "Running gcovr to produce HTML code coverage report."
    )

    # Show info where to find the report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ;
        COMMENT "Open ./${Coverage_NAME}/index.html in your browser to view the coverage report."
    )

endfunction() # setup_target_for_coverage_gcovr_html

# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage_fastcov(
#     NAME testrunner_coverage                    # New target name
#     EXECUTABLE testrunner -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES testrunner                     # Dependencies to build first
#     BASE_DIRECTORY "../"                        # Base directory for report
#                                                 #  (defaults to PROJECT_SOURCE_DIR)
#     EXCLUDE "src/dir1/" "src/dir2/"             # Patterns to exclude.
#     NO_DEMANGLE                                 # Don't demangle C++ symbols
#                                                 #  even if c++filt is found
#     SKIP_HTML                                   # Don't create html report
# )
function(setup_target_for_coverage_fastcov)

    set(options NO_DEMANGLE SKIP_HTML)
    set(oneValueArgs BASE_DIRECTORY NAME)
    set(multiValueArgs EXCLUDE EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES FASTCOV_ARGS GENHTML_ARGS)
    cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT FASTCOV_PATH)
        message(FATAL_ERROR "fastcov not found! Aborting...")
    endif()

    if(NOT GENHTML_PATH)
        message(FATAL_ERROR "genhtml not found! Aborting...")
    endif()

    # Set base directory (as absolute path), or default to PROJECT_SOURCE_DIR
    if(Coverage_BASE_DIRECTORY)
        get_filename_component(BASEDIR ${Coverage_BASE_DIRECTORY} ABSOLUTE)
    else()
        set(BASEDIR ${PROJECT_SOURCE_DIR})
    endif()

    # Collect excludes (Patterns, not paths, for fastcov)
    set(FASTCOV_EXCLUDES "")
    foreach(EXCLUDE ${Coverage_EXCLUDE} ${COVERAGE_EXCLUDES} ${COVERAGE_FASTCOV_EXCLUDES})
        list(APPEND FASTCOV_EXCLUDES "${EXCLUDE}")
    endforeach()
    list(REMOVE_DUPLICATES FASTCOV_EXCLUDES)

    # Conditional arguments
    if(CPPFILT_PATH AND NOT ${Coverage_NO_DEMANGLE})
        set(GENHTML_EXTRA_ARGS "--demangle-cpp")
    endif()

    # Set up commands which will be run to generate coverage data
    set(FASTCOV_EXEC_TESTS_CMD ${Coverage_EXECUTABLE} ${Coverage_EXECUTABLE_ARGS})

    set(FASTCOV_CAPTURE_CMD ${FASTCOV_PATH} ${Coverage_FASTCOV_ARGS} --gcov ${GCOV_PATH}
        --search-directory ${BASEDIR}
        --process-gcno
        --lcov
        --output ${Coverage_NAME}.info
        --exclude ${FASTCOV_EXCLUDES}
        --exclude ${FASTCOV_EXCLUDES}
    )

    if(Coverage_SKIP_HTML)
        set(FASTCOV_HTML_CMD ";")
    else()
        set(FASTCOV_HTML_CMD ${GENHTML_PATH} ${GENHTML_EXTRA_ARGS} ${Coverage_GENHTML_ARGS}
            -o ${Coverage_NAME} ${Coverage_NAME}.info
        )
    endif()

    if(CODE_COVERAGE_VERBOSE)
        message(STATUS "Code coverage commands for target ${Coverage_NAME} (fastcov):")

        message("   Running tests:")
        string(REPLACE ";" " " FASTCOV_EXEC_TESTS_CMD_SPACED "${FASTCOV_EXEC_TESTS_CMD}")
        message("     ${FASTCOV_EXEC_TESTS_CMD_SPACED}")

        message("   Capturing fastcov counters and generating report:")
        string(REPLACE ";" " " FASTCOV_CAPTURE_CMD_SPACED "${FASTCOV_CAPTURE_CMD}")
        message("     ${FASTCOV_CAPTURE_CMD_SPACED}")

        if(NOT Coverage_SKIP_HTML)
            message("   Generating HTML report: ")
            string(REPLACE ";" " " FASTCOV_HTML_CMD_SPACED "${FASTCOV_HTML_CMD}")
            message("     ${FASTCOV_HTML_CMD_SPACED}")
        endif()
    endif()

    # Setup target
    add_custom_target(${Coverage_NAME}

        # Cleanup fastcov
        COMMAND ${FASTCOV_PATH} ${Coverage_FASTCOV_ARGS} --gcov ${GCOV_PATH}
            --search-directory ${BASEDIR}
            --zerocounters

        COMMAND ${FASTCOV_EXEC_TESTS_CMD}
        COMMAND ${FASTCOV_CAPTURE_CMD}
        COMMAND ${FASTCOV_HTML_CMD}

        # Set output files as GENERATED (will be removed on 'make clean')
        BYPRODUCTS
             ${Coverage_NAME}.info
             ${Coverage_NAME}/index.html  # report directory

        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        DEPENDS ${Coverage_DEPENDENCIES}
        VERBATIM # Protect arguments to commands
        COMMENT "Resetting code coverage counters to zero. Processing code coverage counters and generating report."
    )

    set(INFO_MSG "fastcov code coverage info report saved in ${Coverage_NAME}.info.")
    if(NOT Coverage_SKIP_HTML)
        string(APPEND INFO_MSG " Open ${PROJECT_BINARY_DIR}/${Coverage_NAME}/index.html in your browser to view the coverage report.")
    endif()
    # Show where to find the fastcov info report
    add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo ${INFO_MSG}
    )

endfunction() # setup_target_for_coverage_fastcov

function(append_coverage_compiler_flags)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
    message(STATUS "Appending code coverage compiler flags: ${COVERAGE_COMPILER_FLAGS}")
endfunction() # append_coverage_compiler_flags
