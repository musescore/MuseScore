# This file is part of KDDockWidgets.
#
# SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Sergio Martins <sergio.martins@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

# Pass the following variables to cmake to control the build:
#
# -DKDDockWidgets_QT6=[true|false] Build against Qt6 rather than Qt5
# Default=false (Qt5 will be used even if Qt6 is available)
#
# -DKDDockWidgets_STATIC=[true|false] Build static versions of the libraries
# Default=false
#
# -DKDDockWidgets_TESTS=[true|false] Build the test harness. Currently ignored
# (except for Python bindings) unless KDDockWidgets_DEVELOPER_MODE=True.
# Default=false
#
# -DKDDockWidgets_EXAMPLES=[true|false] Build the examples. Default=true
#
# -DKDDockWidgets_DOCS=[true|false] Build the API documentation. Enables the
# 'docs' build target. Default=false
#
# -DKDDockWidgets_PYTHON_BINDINGS=[true|false] Build/Generate python bindings.
# Always false for Debug builds (If your shiboken or pyside is installed in a
# non-standard locations try passing the SHIBOKEN_CUSTOM_PREFIX and
# PYSIDE_CUSTOM_PREFIX variables.) Default=false
#
# -DKDDockWidgets_PYTHON_BINDINGS_INSTALL_PREFIX=[path] Set an alternative
# install path for Python bindings Default=CMAKE_INSTALL_PREFIX
#
# -DKDDockWidgets_FRONTENDS='qtwidgets;qtquick' Semicolon separated list of
# frontends to enable. If not specified, Qt frontends will be enabled based on
# availability of libraries on your system.

# ## DO NOT USE IF YOU ARE AN END-USER.  FOR THE DEVELOPERS ONLY!! # Special
# CMake Options for Developers
#
# -DKDDockWidgets_DEVELOPER_MODE=[true|false] Configure the build for a
# developer setup. Enables some features that are not geared towards end-users.
# Forces the test harness to be built. Default=false
#
# -DKDDockWidgets_WERROR=[true|false] Compile with the -Werror gcc/clang option
# (always true for developer-mode) Default=false
#
# -DKDDockWidgets_LINTER=[true|false] Build the layout linter. Ignored unless
# KDDockWidgets_DEVELOPER_MODE=True Default=true
#
# -DKDDockWidgets_CODE_COVERAGE=[true|false] Enable coverage reporting. Ignored
# unless KDDockWidgets_DEVELOPER_MODE=True Default=false

cmake_minimum_required(VERSION 3.15)

# Allow using a non-KDAB install location.
set(KDAB_INSTALL
    True
    CACHE INTERNAL "Install to default KDAB Location"
)

if(DEFINED CMAKE_INSTALL_PREFIX)
    if(NOT "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
        set(KDAB_INSTALL
            False
            CACHE INTERNAL "Install to non-KDAB Location"
        )
    endif()
endif()

project(
    KDDockWidgets
    DESCRIPTION "An advanced docking system for Qt"
    HOMEPAGE_URL "https://github.com/KDAB/KDDockWidgets"
    LANGUAGES CXX C
)

set(KDDockWidgets_VERSION_MAJOR 2)
set(KDDockWidgets_VERSION_MINOR 1)
set(KDDockWidgets_VERSION_PATCH 1)
set(KDDockWidgets_VERSION ${KDDockWidgets_VERSION_MAJOR}.${KDDockWidgets_VERSION_MINOR}.${KDDockWidgets_VERSION_PATCH})
set(PROJECT_VERSION ${KDDockWidgets_VERSION}) # PROJECT_VERSION is needed by some ECM modules
set(KDDockWidgets_SOVERSION "2.1")

include(FeatureSummary)

set(KDDockWidgets_FRONTENDS
    ""
    CACHE STRING "Semicolon separated list of frontends to enable (blank for autodetect)"
)

set(KDDockWidgets_LIBRARY_POSTFIX
    ""
    CACHE STRING "Appends the specified string to the library name. Not applicable to VS generator."
)

option(KDDockWidgets_QT6 "Build against Qt 6" ON)
option(KDDockWidgets_DEVELOPER_MODE "Developer Mode" OFF)
option(KDDockWidgets_PYTHON_BINDINGS "Build python bindings" OFF)
option(KDDockWidgets_STATIC "Build statically" OFF)
option(KDDockWidgets_TESTS "Build the tests" OFF)
option(KDDockWidgets_WAYLAND_TESTS "Build the wayland tests" OFF)
option(KDDockWidgets_EXAMPLES "Build the examples" ON)
option(KDDockWidgets_DOCS "Build the API documentation" OFF)
option(KDDockWidgets_WERROR "Use -Werror (will be true for developer-mode unconditionally)" OFF)
option(KDDockWidgets_X11EXTRAS
       "Link against QtX11Extras to detect if the compositor supports transparency. Not applicable on non-Linux or Qt6."
       ON
)
option(KDDockWidgets_XLib "On Linux, link against XLib, for a more robust window z-order detection." OFF)
option(KDDockWidgets_CODE_COVERAGE "Enable coverage reporting" OFF)
option(KDDockWidgets_FLUTTER_NO_BINDINGS "Don't build flutter bindings, only the flutter frontend" OFF)
option(KDDockWidgets_FLUTTER_TESTS_AOT "Flutter tests will be built in AOT mode" OFF)
option(KDDockWidgets_NO_SPDLOG "Don't use spdlog, even if it is found." OFF)
option(KDDockWidgets_USE_LLD "Use lld for linking" OFF)
option(KDDockWidgets_USE_VALGRIND "Runs the tests under valgrind" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/ECM/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/KDAB/modules")

# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git" OR KDDockWidgets_DEVELOPER_MODE)
    set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to ${default_build_type} as none was specified.")
    set(CMAKE_BUILD_TYPE
        "${default_build_type}"
        CACHE STRING "Choose the type of build." FORCE
    )

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(KDDockWidgets_XLib)
    add_definitions(-DKDDockWidgets_XLIB)
endif()

if(MSVC AND MSVC_TOOLSET_VERSION LESS 142)
    message(FATAL_ERROR "VS 2019 is the minimum required toolset")
endif()

if(KDDockWidgets_QT6)
    set(QT_VERSION_MAJOR 6)
    set(QT_MIN_VERSION "6.2.0")
else()
    set(QT_VERSION_MAJOR 5)
    set(QT_MIN_VERSION "5.15")
endif()

# BEGIN frontend enabling

set(KDDW_FRONTEND_QTWIDGETS OFF)
set(KDDW_FRONTEND_QTQUICK OFF)
set(KDDW_FRONTEND_FLUTTER OFF)

if(KDDockWidgets_FRONTENDS)
    set(KDDockWidgets_ALL_FRONTENDS "qtwidgets;qtquick;flutter;none")

    foreach(frontend ${KDDockWidgets_FRONTENDS})
        if(NOT ${frontend} IN_LIST KDDockWidgets_ALL_FRONTENDS)
            message(FATAL_ERROR "Unknown frontend ${frontend}")
        endif()
    endforeach()
endif()

message("============== ${KDDockWidgets_FRONTENDS}")

if(KDDockWidgets_FRONTENDS)
    if("qtwidgets" IN_LIST KDDockWidgets_FRONTENDS)
        find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS Widgets)
        set(KDDW_FRONTEND_QTWIDGETS ON)
    endif()

    if("qtquick" IN_LIST KDDockWidgets_FRONTENDS)
        find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} NO_MODULE REQUIRED COMPONENTS Quick QuickControls2)
        set(KDDW_FRONTEND_QTQUICK ON)
    endif()

    if("flutter" IN_LIST KDDockWidgets_FRONTENDS)
        set(KDDW_FRONTEND_FLUTTER ON)
    endif()

    if("none" IN_LIST KDDockWidgets_FRONTENDS)
        set(KDDW_FRONTEND_NONE ON)
    endif()
else()
    set(ENABLED_FRONTENDS "")
    message("No frontends specified explicitly.")

    # qtwidgets
    find_package(Qt${QT_VERSION_MAJOR} ${QT_MIN_VERSION} NO_MODULE COMPONENTS Widgets Quick QuickControls2)

    if(Qt${QT_VERSION_MAJOR}Widgets_FOUND)
        list(APPEND ENABLED_FRONTENDS "qtwidgets")
        set(KDDW_FRONTEND_QTWIDGETS ON)
    endif()

    # qtquick
    if(Qt${QT_VERSION_MAJOR}Quick_FOUND AND Qt${QT_VERSION_MAJOR}QuickControls2_FOUND)
        list(APPEND ENABLED_FRONTENDS "qtquick")
        set(KDDW_FRONTEND_QTQUICK ON)
    endif()

    if(NOT ENABLED_FRONTENDS)
        message(FATAL_ERROR "Failed to enable any frontends. Please install the required libraries and try again.")
    endif()

    message("Following frontends have been enabled:")

    foreach(frontend ${ENABLED_FRONTENDS})
        message("* ${frontend}")
    endforeach()
endif()

if(KDDW_FRONTEND_QTWIDGETS OR KDDW_FRONTEND_QTQUICK)
    if(WIN32 AND (Qt6Core_VERSION VERSION_EQUAL "6.5.3" OR Qt6Core_VERSION VERSION_EQUAL "6.6.1"))
        message(
            FATAL_ERROR
                "Qt 6.5.3 and 6.6.1 are not supported on Windows, as they suffer from a Qt regression: QTBUG-117704. "
                "Please either downgrade or upgrade to 6.6.2 or superior"
        )
    endif()

    set(KDDW_FRONTEND_QT ON)
endif()

if(KDDW_FRONTEND_FLUTTER)
    if(KDDW_FRONTEND_QT)
        message(FATAL_ERROR "The flutter frontend is not compatible with the Qt frontends. Chose a single one.")
    endif()

    if(KDDockWidgets_DEVELOPER_MODE AND (APPLE OR WIN32))
        message(FATAL_ERROR "The flutter frontend only supports developer-mode on Linux. "
                            "You can still develop on Windows/macOS but wait for test results from KDAB CI."
        )
    endif()
endif()

if(KDDW_FRONTEND_NONE AND (KDDW_FRONTEND_QT OR KDDW_FRONTEND_FLUTTER))
    message(FATAL_ERROR "Frontend value \"none\" needs to be specified alone")
endif()

# END frontend enabling

if(KDDockWidgets_WAYLAND_TESTS)
    if(NOT KDDockWidgets_DEVELOPER_MODE)
        message(FATAL_ERROR "Wayland tests require developer mode")
    endif()

    if(NOT KDDW_FRONTEND_QTWIDGETS)
        message(FATAL_ERROR "Wayland tests require QtWidgets")
    endif()
endif()

set(${PROJECT_NAME}_LIBRARY_QTID "")

if(KDDW_FRONTEND_QT)
    include(KDQtInstallPaths) # to set QT_INSTALL_FOO variables
    if(KDDockWidgets_QT6)
        set(${PROJECT_NAME}_LIBRARY_QTID "-qt6")
    endif()
endif()
set(KDDockWidgets_LIBRARY_QTID "${${PROJECT_NAME}_LIBRARY_QTID}")

add_definitions(-DQT_NO_KEYWORDS)

option(KDockWidgets_PRETTY_QTWIDGETS_HEADERS
       "Install DockWidget.h and MainWindow.h as synonyms to the *.h counterparts." ${KDDW_FRONTEND_QTWIDGETS}
)

set(KDDockWidgets_DEPS "")
if(KDDW_FRONTEND_QTWIDGETS)
    set(KDDockWidgets_DEPS "widgets")
endif()

if(KDDW_FRONTEND_QTQUICK)
    set(KDDockWidgets_DEPS "${KDDockWidgets_DEPS} quick quickcontrols2")
endif()

if(NOT WIN32
   AND NOT APPLE
   AND NOT EMSCRIPTEN
   AND NOT KDDockWidgets_QT6
   AND KDDockWidgets_X11EXTRAS
)
    set(KDDockWidgets_DEPS "${KDDockWidgets_DEPS} x11extras")
endif()

if(NOT KDDockWidgets_NO_SPDLOG)
    if(KDDockWidgets_DEVELOPER_MODE AND LINUX)
        # Make sure we have at least one CI configuration running tests with spdlog enabled.
        # Our tests are log error free. Any error printed should result in a fatal failure.
        # For that we want spdlog enabled.
        find_package(spdlog 1.8.0 REQUIRED)
        find_package(fmt REQUIRED)
    else()
        # For regular user builds we don't care if people use it or not.
        find_package(spdlog 1.8.0 QUIET)
        find_package(fmt QUIET)
    endif()
endif()

# fmt 11 not supported yet, will get to it when it hits archlinux
if(spdlog_FOUND AND fmt_FOUND)
    set(KDDockWidgets_HAS_SPDLOG TRUE)
else()
    set(KDDockWidgets_HAS_SPDLOG FALSE)
endif()

# Always build the test harness in developer-mode
if(KDDockWidgets_DEVELOPER_MODE)
    set(KDDockWidgets_TESTS ON)
    set(KDDockWidgets_WERROR ON)
    include(ECMEnableSanitizers)

    # Enable assert() in developer mode, regardless of debug vs release
    add_definitions(-UNDEBUG)

    if(KDDW_FRONTEND_FLUTTER)
        include(FetchContent)
        set(GLFW_BUILD_EXAMPLE OFF)
        set(GLFW_BUILD_DOCS OFF)
        set(GLFW_BUILD_WIN32 ON)
        set(GLFW_BUILD_TESTS OFF)
        set(GLFW_USE_WAYLAND OFF)

        fetchcontent_declare(
            glfw3
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3fa2360720eeba1964df3c0ecf4b5df8648a8e52
        )
        fetchcontent_makeavailable(glfw3)
    endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(IS_CLANG_BUILD TRUE)
else()
    set(IS_CLANG_BUILD FALSE)
endif()

if(KDDW_FRONTEND_QT)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
else()
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to hidden visibility for symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Sets compiler flags for the specified target, taking platform into consideration.
macro(set_compiler_flags targetName)
    if(KDDockWidgets_DEVELOPER_MODE)
        target_compile_definitions(
            ${targetName}
            PUBLIC DOCKS_DEVELOPER_MODE DOCKS_TESTING_METHODS
            PRIVATE QT_FORCE_ASSERTS
        )

        if(NOT MSVC AND NOT APPLE)
            target_compile_options(${targetName} PRIVATE -Wall -Wextra -Woverloaded-virtual)
            if(KDDockWidgets_USE_LLD)
                target_link_options(${targetName} PRIVATE -fuse-ld=lld)
            endif()
        endif()

        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${targetName} PRIVATE -Wweak-vtables -Wextra-semi)
        endif()

        if(MSVC)
            target_compile_options(${targetName} PRIVATE /W4)
        endif()

        if(KDDW_FRONTEND_FLUTTER AND NOT MSVC)
            target_compile_options(${targetName} PRIVATE -fcoroutines -Wno-missing-field-initializers)
        endif()
    else()
        if(KDDW_FRONTEND_FLUTTER)
            # Our Dart bindings are auto-generated and they include methods guarded with DOCKS_TESTING_METHODS
            # Since we won't generate 2 sets of bindings, we need to enable the macro in release as well
            # It won't have much side-effects besides making the bindings compile.
            target_compile_definitions(${targetName} PUBLIC DOCKS_TESTING_METHODS)
        endif()
    endif()

    # Enable -Werror
    if(KDDockWidgets_WERROR AND (NOT MSVC OR IS_CLANG_BUILD)) # clang-cl accepts these too
        target_compile_options(${targetName} PRIVATE -Werror -Wundef -Wno-error=deprecated-declarations)
    endif()

    if(KDDockWidgets_HAS_SPDLOG)
        target_compile_definitions(${targetName} PRIVATE KDDW_HAS_SPDLOG)
    endif()
endmacro()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APPLE)
   OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT APPLE)
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND NOT WIN32)
)
    # Linker warnings should be treated as errors
    set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--fatal-warnings ${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--fatal-warnings ${CMAKE_MODULE_LINKER_FLAGS}")

    string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" compileflags)

    if("${CMAKE_CXX_FLAGS} ${${compileflags}}" MATCHES "-fsanitize")
        set(sanitizers_enabled TRUE)
    else()
        set(sanitizers_enabled FALSE)
    endif()

    if(APPLE OR LINUX)
        # cannot enable this for clang + sanitizers
        if(NOT sanitizers_enabled OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Do not allow undefined symbols, even in non-symbolic shared libraries
            set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
            set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--no-undefined ${CMAKE_MODULE_LINKER_FLAGS}")
        endif()
    endif()
endif()

if(KDDockWidgets_STATIC)
    set(KDDockWidgets_LIBRARY_MODE "STATIC")
else()
    set(KDDockWidgets_LIBRARY_MODE "SHARED")
endif()

if(KDAB_INSTALL)
    if(UNIX)
        set(CMAKE_INSTALL_PREFIX
            "/usr/local/KDAB/KDDockWidgets-${KDDockWidgets_VERSION}"
            CACHE INTERNAL "Install to default KDAB Location"
        )
    elseif(WIN32)
        set(CMAKE_INSTALL_PREFIX
            "C:\\KDAB\\KDDockWidgets-${KDDockWidgets_VERSION}"
            CACHE INTERNAL "Install to default KDAB Location"
        )
    endif()
endif()

# setup default install locations
include(KDInstallLocation)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(KDDockWidgets_IS_ROOT_PROJECT TRUE)

    message(STATUS "Building KDDockWidgets ${KDDockWidgets_VERSION} in ${CMAKE_BUILD_TYPE} mode. "
                   "Installing to ${CMAKE_INSTALL_PREFIX}"
    )

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

    install(FILES LICENSE.txt README.md DESTINATION ${INSTALL_DOC_DIR})
    install(DIRECTORY LICENSES DESTINATION ${INSTALL_DOC_DIR})

    # Generate .pri file for qmake users
    if(NOT CMAKE_CONFIGURATION_TYPES AND KDDW_FRONTEND_QT)
        if(QT_VERSION_MAJOR EQUAL 5 OR (QT_VERSION_MAJOR EQUAL 6 AND Qt6Core_VERSION VERSION_GREATER "6.2"))
            include(ECMGeneratePriFile)
            set(PROJECT_VERSION_STRING ${KDDockWidgets_VERSION})
            ecm_generate_pri_file(
                BASE_NAME
                KDDockWidgets
                LIB_NAME
                kddockwidgets${${PROJECT_NAME}_LIBRARY_QTID}
                DEPS
                ${KDDockWidgets_DEPS}
                FILENAME_VAR
                pri_filename
                INCLUDE_INSTALL_DIR
                ${CMAKE_INSTALL_INCLUDEDIR}
            )
            install(FILES ${pri_filename} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
        endif()
    endif()
else()
    # Always disable tests, examples, docs when used as a submodule
    set(KDDockWidgets_IS_ROOT_PROJECT FALSE)
    set(KDDockWidgets_TESTS FALSE)
    set(KDDockWidgets_EXAMPLES FALSE)
    set(KDDockWidgets_DOCS FALSE)
endif()

# workaround for CMAKE_CURRENT_FUNCTION_LIST_DIR below CMake 3.17
set(KKDockWidgets_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(KDDockWidgets_TESTS)
    enable_testing()
endif()

add_subdirectory(src)

if(KDDockWidgets_PYTHON_BINDINGS)
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd]eb" OR KDDockWidgets_STATIC)
        message(FATAL_ERROR "** Python Bindings are disabled in debug or static builds.")
    endif()

    if(CMAKE_UNITY_BUILD)
        message(FATAL_ERROR "** Python Bindings are disabled in Unity builds. Try again with CMAKE_UNITY_BUILD=OFF")
    endif()

    add_subdirectory(python)
endif()

if(KDDockWidgets_EXAMPLES)
    if(KDDW_FRONTEND_QTQUICK)
        add_subdirectory(examples/qtquick)
    endif()

    if(KDDW_FRONTEND_QTWIDGETS)
        add_subdirectory(examples/dockwidgets)
        add_subdirectory(examples/minimal)
        add_subdirectory(examples/mdi)
        add_subdirectory(examples/mdi_with_docking)
        set_compiler_flags(qtwidgets_dockwidgets)
        set_compiler_flags(qtwidgets_minimal)
        set_compiler_flags(qtwidgets_mdi_with_docking)

        # Standalone layouting example
        add_subdirectory(src/core/layouting/examples/qtwidgets/)
    endif()

    if(KDDW_FRONTEND_FLUTTER AND NOT KDDockWidgets_FLUTTER_NO_BINDINGS)
        add_custom_target(
            kddw-flutter-example ALL
            DEPENDS ${CMAKE_SOURCE_DIR}/examples/flutter/build/linux/x64/debug/bundle/kddockwidgets_flutter_example
            COMMENT "Building flutter example"
        )

        if(MSVC)
            set(FLUTTER_PLATFORM "windows")
        elseif(APPLE)
            set(FLUTTER_PLATFORM "macos")
        else()
            set(FLUTTER_PLATFORM "linux")
        endif()

        add_custom_command(
            OUTPUT ${CMAKE_SOURCE_DIR}/examples/flutter/build/linux/x64/debug/bundle/kddockwidgets_flutter_example
            COMMAND flutter build ${FLUTTER_PLATFORM} --debug
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/examples/flutter
            DEPENDS ${CMAKE_SOURCE_DIR}/examples/flutter/lib/main.dart
                    ${CMAKE_SOURCE_DIR}/examples/flutter/lib/MyWidget.dart
                    ${CMAKE_SOURCE_DIR}/examples/flutter/lib/MyMenuBar.dart
            COMMENT "Building flutter example"
        )
    endif()

    if(KDDockWidgets_SLINT_LAYOUTING_EXAMPLE)
        add_subdirectory(src/core/layouting/examples/slint/)
    endif()
endif()

if(KDDockWidgets_TESTS)
    if(KDDockWidgets_DEVELOPER_MODE)
        add_subdirectory(tests)
    endif()
endif()

if(KDDockWidgets_DOCS)
    add_subdirectory(docs) # needs to go last, in case there are build source files
endif()

if(KDDockWidgets_IS_ROOT_PROJECT)
    # Add uninstall target (not for submodules since parent projects typically have uninstall too)
    include(ECMUninstallTarget)
endif()

# Deployment
if(WIN32)
    add_custom_target(
        createZipDemo
        COMMAND cmd /c ${CMAKE_CURRENT_SOURCE_DIR}\\deploy\\create-demo-win-zip.bat
                ${CMAKE_PROJECT_NAME}-Demo-${${PROJECT_NAME}_VERSION}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Target to generate the Zip demo installer for Windows"
    )
endif()

add_custom_target(
    build-book
    COMMAND mdbook build --dest-dir ${CMAKE_CURRENT_BINARY_DIR}/book
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building KDDW book"
)

if(KDDockWidgets_IS_ROOT_PROJECT)
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
