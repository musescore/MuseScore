#
# SPDX-FileCopyrightText: 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

#  PYSIDE_BASEDIR       - Top of the PySide6 installation
#  PYSIDE_INCLUDE_DIR   - Directories to include to use PySide6
#  PYSIDE_LIBRARY       - Files to link against to use PySide6
#  PYSIDE_TYPESYSTEMS   - Type system files that should be used by other bindings extending PySide6
#
# You can install PySide6 from Qt repository with
# pip3 install --index-url=https://download.qt.io/official_releases/QtForPython --trusted-host download.qt.io pyside6

set(PYSIDE6_FOUND FALSE)

# extract python library basename
list(GET Python3_LIBRARIES 0 PYTHON_LIBRARY_FILENAME)
get_filename_component(PYTHON_LIBRARY_FILENAME ${PYTHON_LIBRARY_FILENAME} NAME)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "if True:
       import os, sys
       try:
        import PySide6.QtCore as QtCore
        print(os.path.dirname(QtCore.__file__))
       except Exception as error:
        print(error, file=sys.stderr)
        exit()
    "
    OUTPUT_VARIABLE PYSIDE6_BASEDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(PYSIDE6_BASEDIR)
    set(PYSIDE_BASEDIR ${PYSIDE6_BASEDIR} CACHE PATH "Top level install of PySide6" FORCE)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "if True:
           import os
           import PySide6.QtCore as QtCore
           print(os.path.basename(QtCore.__file__).split('.', 1)[1])
        "
        OUTPUT_VARIABLE PYSIDE6_SUFFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "if True:
           import os
           import PySide6.QtCore as QtCore
           print(';'.join(map(str, QtCore.__version_info__)))
        "
        OUTPUT_VARIABLE PYSIDE6_SO_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    list(GET PYSIDE6_SO_VERSION 0 PYSIDE6_SO_MACRO_VERSION)
    list(GET PYSIDE6_SO_VERSION 1 PYSIDE6_SO_MICRO_VERSION)
    list(GET PYSIDE6_SO_VERSION 2 PYSIDE6_SO_MINOR_VERSION)
    string(REPLACE ";" "." PYSIDE6_SO_VERSION "${PYSIDE6_SO_VERSION}")

    if(NOT APPLE)
        set(PYSIDE6_SUFFIX "${PYSIDE6_SUFFIX}.${PYSIDE6_SO_MACRO_VERSION}.${PYSIDE6_SO_MICRO_VERSION}")
      else()
        string(REPLACE ".so" "" PYSIDE6_SUFFIX ${PYSIDE6_SUFFIX})
        set(PYSIDE6_SUFFIX "${PYSIDE6_SUFFIX}.${PYSIDE6_SO_MACRO_VERSION}.${PYSIDE6_SO_MICRO_VERSION}.dylib")
    endif()

    set(PYSIDE6_FOUND TRUE)
    message(STATUS "PySide6 base dir:           ${PYSIDE6_BASEDIR}" )
    message(STATUS "PySide6 suffix:             ${PYSIDE6_SUFFIX}")
endif()

if (PYSIDE6_FOUND)
    #PySide
    #===============================================================================
    find_path(PYSIDE_INCLUDE_DIR
        pyside.h
        PATHS ${PYSIDE6_BASEDIR}/include ${PYSIDE6_CUSTOM_PREFIX}/include/PySide6
        NO_DEFAULT_PATH)

    # Platform specific library names
    if(MSVC)
        SET(PYSIDE_LIBRARY_BASENAMES "pyside6.abi3.lib")
    elseif(CYGWIN)
        SET(PYSIDE_LIBRARY_BASENAMES "")
    elseif(WIN32)
        SET(PYSIDE_LIBRARY_BASENAMES "libpyside6.${PYSIDE6_SUFFIX}")
    else()
        SET(PYSIDE_LIBRARY_BASENAMES "libpyside6.${PYSIDE6_SUFFIX}")
    endif()

    find_file(PYSIDE_LIBRARY
        ${PYSIDE_LIBRARY_BASENAMES}
        PATHS ${PYSIDE6_BASEDIR} ${PYSIDE6_CUSTOM_PREFIX}/lib
        NO_DEFAULT_PATH)

    find_path(PYSIDE_TYPESYSTEMS
        typesystem_core.xml
        PATHS ${PYSIDE6_BASEDIR}/typesystems ${PYSIDE6_CUSTOM_PREFIX}/share/PySide6/typesystems
        NO_DEFAULT_PATH)
endif()

if(PYSIDE6_FOUND)
    message(STATUS "PySide include dir:         ${PYSIDE_INCLUDE_DIR}")
    message(STATUS "PySide library:             ${PYSIDE_LIBRARY}")
    message(STATUS "PySide typesystems:         ${PYSIDE_TYPESYSTEMS}")
    message(STATUS "PySide6 version:            ${PYSIDE6_SO_VERSION}")

    # Create PySide6 target
    add_library(PySide6::pyside6 SHARED IMPORTED GLOBAL)
    if(MSVC)
        set_property(TARGET PySide6::pyside6 PROPERTY
            IMPORTED_IMPLIB ${PYSIDE_LIBRARY})
    endif()
    set_property(TARGET PySide6::pyside6 PROPERTY
        IMPORTED_LOCATION ${PYSIDE_LIBRARY})
    set_property(TARGET PySide6::pyside6 APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES
        ${PYSIDE_INCLUDE_DIR}
        ${PYSIDE_INCLUDE_DIR}/QtCore/
        ${PYSIDE_INCLUDE_DIR}/QtGui/
        ${PYSIDE_INCLUDE_DIR}/QtWidgets/
        ${Python3_INCLUDE_DIRS}
    )
endif()


find_package_handle_standard_args(PySide6
    REQUIRED_VARS PYSIDE6_BASEDIR PYSIDE_INCLUDE_DIR PYSIDE_LIBRARY PYSIDE_TYPESYSTEMS
    VERSION_VAR PYSIDE6_SO_VERSION
)
