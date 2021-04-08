#
# SPDX-FileCopyrightText: 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

#  SHIBOKEN_INCLUDE_DIR        - Directories to include to use SHIBOKEN
#  SHIBOKEN_LIBRARY            - Files to link against to use SHIBOKEN
#  SHIBOKEN_BINARY             - Executable name
#  SHIBOKEN_BUILD_TYPE         - Tells if Shiboken was compiled in Release or Debug mode.

# You can install Shiboken from Qt repository with
# pip3 install --index-url=https://download.qt.io/official_releases/QtForPython --trusted-host download.qt.io shiboken6-generator


set(SHIBOKEN_FOUND FALSE)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "if True:
       import os
       try:
        import shiboken6_generator
        print(shiboken6_generator.__path__[0])
       except:
        exit()
    "
    OUTPUT_VARIABLE SHIBOKEN_GENERATOR_BASEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "if True:
       import os
       try:
        import shiboken6
        print(shiboken6.__path__[0])
       except:
        exit()
    "
    OUTPUT_VARIABLE SHIBOKEN_BASEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "if True:
       import os
       import shiboken6
       print(';'.join(filter(None, map(str, shiboken6.__version_info__))))
    "
    OUTPUT_VARIABLE SHIBOKEN_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
list(GET SHIBOKEN_VERSION 0 SHIBOKEN_MACRO_VERSION)
list(GET SHIBOKEN_VERSION 1 SHIBOKEN_MICRO_VERSION)
list(GET SHIBOKEN_VERSION 2 SHIBOKEN_MINOR_VERSION)
string(REPLACE ";" "." SHIBOKEN_VERSION "${SHIBOKEN_VERSION}")

message(STATUS "ShibokenGenerator base dir: ${SHIBOKEN_GENERATOR_BASEDIR}")
message(STATUS "Shiboken base dir:          ${SHIBOKEN_BASEDIR}")
message(STATUS "Shiboken custom path:       ${SHIBOKEN_CUSTOM_PREFIX}")

if(SHIBOKEN_BASEDIR)
    find_path(SHIBOKEN_INCLUDE_DIR
          shiboken.h
          PATHS ${SHIBOKEN_CUSTOM_PREFIX} ${SHIBOKEN_GENERATOR_BASEDIR}/include
          NO_DEFAULT_PATH)
    if(MSVC)
        SET(SHIBOKEN_LIBRARY_BASENAMES "shiboken6.abi3.lib")
    elseif(CYGWIN)
        SET(SHIBOKEN_LIBRARY_BASENAMES "")
    elseif(WIN32)
        SET(SHIBOKEN_LIBRARY_BASENAMES "libshiboken6.${PYSIDE2_SUFFIX}")
    elseif(APPLE)
         SET(SHIBOKEN_LIBRARY_BASENAMES
                libshiboken6.abi3.dylib
                libshiboken6.abi3.${SHIBOKEN_MACRO_VERSION}.dylib
                libshiboken6.abi3.${SHIBOKEN_MACRO_VERSION}.${SHIBOKEN_MICRO_VERSION}.dylib
                libshiboken6.abi3.${SHIBOKEN_VERSION}.dylib
        )
    else()
        SET(SHIBOKEN_LIBRARY_BASENAMES
                libshiboken6.abi3.so
                libshiboken6.abi3.so.${SHIBOKEN_MACRO_VERSION}
                libshiboken6.abi3.so.${SHIBOKEN_MACRO_VERSION}.${SHIBOKEN_MICRO_VERSION}
                libshiboken6.abi3.so.${SHIBOKEN_VERSION}
        )
    endif()

    if (NOT SHIBOKEN_INCLUDE_DIR)
        return()
    endif()
    set(SHIBOKEN_SEARCH_PATHS ${SHIBOKEN_CUSTOM_PREFIX})
    list(APPEND SHIBOKEN_SEARCH_PATHS ${SHIBOKEN_BASEDIR})
    list(APPEND SHIBOKEN_SEARCH_PATHS ${SHIBOKEN_GENERATOR_BASEDIR})
    find_file(SHIBOKEN_LIBRARY
        ${SHIBOKEN_LIBRARY_BASENAMES}
        PATHS ${SHIBOKEN_SEARCH_PATHS}
        NO_DEFAULT_PATH)

    find_program(SHIBOKEN_BINARY
        shiboken6
        PATHS ${SHIBOKEN_SEARCH_PATHS}
        NO_DEFAULT_PATH
    )
endif()
if (SHIBOKEN_INCLUDE_DIR AND SHIBOKEN_LIBRARY AND SHIBOKEN_BINARY)
    set(SHIBOKEN_FOUND TRUE)
endif()

if(SHIBOKEN_FOUND)
endif()


if(MSVC)
    # On Windows we must link to python3.dll that is a small library that links against python3x.dll
    # that allow us to choose any python3x.dll at runtime
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "if True:
            for lib in '${Python3_LIBRARIES}'.split(';'):
                if '/' in lib:
                    prefix, py = lib.rsplit('/', 1)
                    if py.startswith('python3'):
                        print(prefix + '/python3.lib')
                        break
            "
        OUTPUT_VARIABLE PYTHON_LIMITED_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
     # On Linux and MacOs our modules should not link with any python library
     # that must be handled by the main process
    set(PYTHON_LIMITED_LIBRARIES "")
endif()

if (SHIBOKEN_FOUND)
    message(STATUS "Shiboken include dir:       ${SHIBOKEN_INCLUDE_DIR}")
    message(STATUS "Shiboken library:           ${SHIBOKEN_LIBRARY}")
    message(STATUS "Shiboken binary:            ${SHIBOKEN_BINARY}")
    message(STATUS "Shiboken version:           ${SHIBOKEN_VERSION}")

    # Create shiboke2 target
    add_library(Shiboken6::libshiboken SHARED IMPORTED GLOBAL)
    if(MSVC)
        set_property(TARGET Shiboken6::libshiboken PROPERTY
            IMPORTED_IMPLIB ${SHIBOKEN_LIBRARY})
    endif()
    set_property(TARGET Shiboken6::libshiboken PROPERTY
        IMPORTED_LOCATION ${SHIBOKEN_LIBRARY})
    set_property(TARGET Shiboken6::libshiboken APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${SHIBOKEN_INCLUDE_DIR} ${Python3_INCLUDE_DIRS})
    set_property(TARGET Shiboken6::libshiboken APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES ${PYTHON_LIMITED_LIBRARIES})

    # Generator target
    add_executable(Shiboken6::shiboken IMPORTED GLOBAL)
    set_property(TARGET Shiboken6::shiboken PROPERTY
        IMPORTED_LOCATION ${SHIBOKEN_BINARY})
endif()

find_package_handle_standard_args(Shiboken6
    REQUIRED_VARS SHIBOKEN_BASEDIR SHIBOKEN_INCLUDE_DIR SHIBOKEN_LIBRARY SHIBOKEN_BINARY
    VERSION_VAR SHIBOKEN_VERSION
)

