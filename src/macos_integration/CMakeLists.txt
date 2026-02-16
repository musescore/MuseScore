# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

if(NOT OS_IS_MAC)
    return()
endif()

message(STATUS "Configuring macOS integration")

set(CMAKE_OSX_ARCHITECTURES_BACKUP ${CMAKE_OSX_ARCHITECTURES})

set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "" FORCE)

enable_language(Swift)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Set up swiftrt.o and runtime library search paths
include(InitializeSwift)

# set(MACOSX_DEPLOYMENT_TARGET 12.0)
# set(CMAKE_OSX_DEPLOYMENT_TARGET 12.0)

add_executable(MuseScoreQuickLookPreviewExtension
    MACOSX_BUNDLE
    PreviewProvider.swift
    previewprovider.cpp
    previewprovider.h
)

target_link_libraries(MuseScoreQuickLookPreviewExtension PRIVATE
    muse::global
    iex_imagesexport
    project
)

target_include_directories(MuseScoreQuickLookPreviewExtension PRIVATE
    ${PROJECT_SOURCE_DIR}/src

    ${MUSE_FRAMEWORK_PATH}
    ${MUSE_FRAMEWORK_PATH}/framework
    ${MUSE_FRAMEWORK_PATH}/framework/global

    # compat
    ${MUSE_FRAMEWORK_PATH}/src
    ${MUSE_FRAMEWORK_PATH}/src/framework
    ${MUSE_FRAMEWORK_PATH}/src/framework/global
    # end compat
)

# if(MUSE_COMPILE_USE_PCH)
#     target_precompile_headers_clang_ccache(MuseScoreQuickLookPreviewExtension REUSE_FROM muse_pch)
#     target_link_libraries(MuseScoreQuickLookPreviewExtension PRIVATE muse_pch)
# endif()

target_compile_options(MuseScoreQuickLookPreviewExtension PRIVATE
    $<$<COMPILE_LANGUAGE:Swift>:-application-extension>
    "$<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>"
)

target_link_options(MuseScoreQuickLookPreviewExtension PRIVATE
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xlinker -e>"
    "$<$<COMPILE_LANGUAGE:Swift>:SHELL:-Xlinker _NSExtensionMain>"
)

set_target_properties(MuseScoreQuickLookPreviewExtension PROPERTIES
    XCODE_PRODUCT_TYPE "com.apple.product-type.app-extension"
    BUNDLE_EXTENSION appex
    XCODE_ATTRIBUTE_WRAPPER_EXTENSION appex
    XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE YES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    MACOSX_BUNDLE_BUNDLE_NAME "MuseScoreQuickLookPreviewExtension"
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_BUILD_NUMBER}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 1999-2026 MuseScore Limited. Published under the GNU General Public License version 3."
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.musescore.MuseScore.QuickLookPreviewExtension"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${MUSE_APP_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${MUSE_APP_VERSION}"
    OSX_ARCHITECTURES ""
)

set_target_properties(MuseScoreQuickLookPreviewExtension PROPERTIES
    INSTALL_RPATH "@executable_path/../../../../Frameworks;${QT_INSTALL_LIBS}"
)

install(TARGETS MuseScoreQuickLookPreviewExtension
    BUNDLE DESTINATION mscore.app/Contents/PlugIns
)

set(the_appex_path "${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreQuickLookPreviewExtension.appex")

# Platform plugin is loaded by Qt at runtime, rather than as a linked framework.
# Due to sandboxing, this does not work if the plugin is outside the .appex
# bundle, not even if it is inside the main .app bundle. So we shamelessly
# copy it into the .appex bundle.
install(FILES ${QT_INSTALL_PLUGINS}/platforms/libqcocoa.dylib
    DESTINATION ${the_appex_path}/Contents/MacOS/platforms
)

install(CODE "
    execute_process(COMMAND rm -f \"${the_appex_path}/Contents/MacOS/MuseScoreQuickLookPreviewExtension.emit-module.d\")
    execute_process(COMMAND rm -f \"${the_appex_path}/Contents/MacOS/MuseScoreQuickLookPreviewExtension.d\")
    execute_process(COMMAND codesign --force --sign - --timestamp=none --options runtime --entitlements \"${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist\" \"${the_appex_path}\")
")

set(CMAKE_OSX_ARCHITECTURES ${CMAKE_OSX_ARCHITECTURES_BACKUP} CACHE STRING "" FORCE)
