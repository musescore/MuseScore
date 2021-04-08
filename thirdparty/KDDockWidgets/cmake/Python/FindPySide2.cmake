#
# SPDX-FileCopyrightText: 2020-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

#  PYSIDE_BASEDIR       - Top of the PySide2 installation
#  PYSIDE_INCLUDE_DIR   - Directories to include to use PySide2
#  PYSIDE_LIBRARY       - Files to link against to use PySide2
#  PYSIDE_TYPESYSTEMS   - Type system files that should be used by other bindings extending PySide2
#
# You can install PySide2 from Qt repository with
# pip3 install --index-url=https://download.qt.io/official_releases/QtForPython --trusted-host download.qt.io pyside2

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PYSIDE2_PRIV QUIET pyside2)
endif()

set(PYSIDE2_FOUND FALSE)

if(PYSIDE2_PRIV_FOUND)
    set(PYSIDE2_FOUND TRUE)
    message(STATUS "Using PySide2 found in the system!")
    pkg_get_variable(SHIBOKEN_BINARY
        pyside2
        generator_location
    )
    pkg_get_variable(PYSIDE2_BASEDIR
        pyside2
        typesystemdir
    )
    pkg_get_variable(PYSIDE_INCLUDE_DIR
        pyside2
        includedir
    )
    set(PYSIDE_TYPESYSTEMS ${PYSIDE2_BASEDIR})
    set(PYSIDE2_SO_VERSION ${PYSIDE2_PRIV_VERSION})
    set(PYSIDE_LIBRARY ${PYSIDE2_PRIV_LINK_LIBRARIES})
    list(GET PYSIDE_LIBRARY 0 PYSIDE_LIBRARY)
else()
    # extract python library basename
    list(GET Python3_LIBRARIES 0 PYTHON_LIBRARY_FILENAME)
    get_filename_component(PYTHON_LIBRARY_FILENAME ${PYTHON_LIBRARY_FILENAME} NAME)

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "if True:
           import os, sys
           try:
            import PySide2.QtCore as QtCore
            print(os.path.dirname(QtCore.__file__))
           except Exception as error:
            print(error, file=sys.stderr)
            exit()
        "
        OUTPUT_VARIABLE PYSIDE2_BASEDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(PYSIDE2_BASEDIR)
        set(PYSIDE_BASEDIR ${PYSIDE2_BASEDIR} CACHE PATH "Top level install of PySide2" FORCE)
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "if True:
               import os
               import PySide2.QtCore as QtCore
               print(os.path.basename(QtCore.__file__).split('.', 1)[1])
            "
            OUTPUT_VARIABLE PYSIDE2_SUFFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "if True:
               import os
               import PySide2.QtCore as QtCore
               print(';'.join(map(str, QtCore.__version_info__)))
            "
            OUTPUT_VARIABLE PYSIDE2_SO_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        list(GET PYSIDE2_SO_VERSION 0 PYSIDE2_SO_MACRO_VERSION)
        list(GET PYSIDE2_SO_VERSION 1 PYSIDE2_SO_MICRO_VERSION)
        list(GET PYSIDE2_SO_VERSION 2 PYSIDE2_SO_MINOR_VERSION)
        string(REPLACE ";" "." PYSIDE2_SO_VERSION "${PYSIDE2_SO_VERSION}")

        if(NOT APPLE)
            set(PYSIDE2_SUFFIX "${PYSIDE2_SUFFIX}.${PYSIDE2_SO_MACRO_VERSION}.${PYSIDE2_SO_MICRO_VERSION}")
          else()
            string(REPLACE ".so" "" PYSIDE2_SUFFIX ${PYSIDE2_SUFFIX})
            set(PYSIDE2_SUFFIX "${PYSIDE2_SUFFIX}.${PYSIDE2_SO_MACRO_VERSION}.${PYSIDE2_SO_MICRO_VERSION}.dylib")
        endif()

        set(PYSIDE2_FOUND TRUE)
        message(STATUS "PySide2 base dir:           ${PYSIDE2_BASEDIR}" )
        message(STATUS "PySide2 suffix:             ${PYSIDE2_SUFFIX}")
    endif()

    if (PYSIDE2_FOUND)
        #PySide
        #===============================================================================
        find_path(PYSIDE_INCLUDE_DIR
            pyside.h
            PATHS ${PYSIDE2_BASEDIR}/include ${PYSIDE2_CUSTOM_PREFIX}/include/PySide2
            NO_DEFAULT_PATH)

        # Platform specific library names
        if(MSVC)
            SET(PYSIDE_LIBRARY_BASENAMES "pyside2.abi3.lib")
        elseif(CYGWIN)
            SET(PYSIDE_LIBRARY_BASENAMES "")
        elseif(WIN32)
            SET(PYSIDE_LIBRARY_BASENAMES "libpyside2.${PYSIDE2_SUFFIX}")
        else()
            SET(PYSIDE_LIBRARY_BASENAMES "libpyside2.${PYSIDE2_SUFFIX}")
        endif()

        find_file(PYSIDE_LIBRARY
            ${PYSIDE_LIBRARY_BASENAMES}
            PATHS ${PYSIDE2_BASEDIR} ${PYSIDE2_CUSTOM_PREFIX}/lib
            NO_DEFAULT_PATH)

        find_path(PYSIDE_TYPESYSTEMS
            typesystem_core.xml
            PATHS ${PYSIDE2_BASEDIR}/typesystems ${PYSIDE2_CUSTOM_PREFIX}/share/PySide2/typesystems
            NO_DEFAULT_PATH)
    endif()
endif()

if(PYSIDE2_FOUND)
    message(STATUS "PySide include dir:         ${PYSIDE_INCLUDE_DIR}")
    message(STATUS "PySide library:             ${PYSIDE_LIBRARY}")
    message(STATUS "PySide typesystems:         ${PYSIDE_TYPESYSTEMS}")
    message(STATUS "PySide2 version:            ${PYSIDE2_SO_VERSION}")

    # Create PySide2 target
    add_library(PySide2::pyside2 SHARED IMPORTED GLOBAL)
    if(MSVC)
        set_property(TARGET PySide2::pyside2 PROPERTY
            IMPORTED_IMPLIB ${PYSIDE_LIBRARY})
    endif()
    set_property(TARGET PySide2::pyside2 PROPERTY
        IMPORTED_LOCATION ${PYSIDE_LIBRARY})
    set_property(TARGET PySide2::pyside2 APPEND PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES
        ${PYSIDE_INCLUDE_DIR}
        ${PYSIDE_INCLUDE_DIR}/QtCore/
        ${PYSIDE_INCLUDE_DIR}/QtGui/
        ${PYSIDE_INCLUDE_DIR}/QtWidgets/
        ${Python3_INCLUDE_DIRS}
    )
endif()


find_package_handle_standard_args(PySide2
    REQUIRED_VARS PYSIDE2_BASEDIR PYSIDE_INCLUDE_DIR PYSIDE_LIBRARY PYSIDE_TYPESYSTEMS
    VERSION_VAR PYSIDE2_SO_VERSION
)
