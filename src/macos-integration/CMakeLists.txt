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

set(SWIFT_BUILD_DIR "${CMAKE_BINARY_DIR}/swift_build")
file(MAKE_DIRECTORY ${SWIFT_BUILD_DIR})

file(GLOB_RECURSE SWIFT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.swift"
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.plist"
    "${CMAKE_CURRENT_SOURCE_DIR}/**/*.entitlements"
    "${CMAKE_CURRENT_SOURCE_DIR}/Package.swift"
)

set(SWIFT_BUILD_TYPE $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,debug,release>)

add_custom_command(
    OUTPUT "${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreQLPreviewProvider"
           "${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreThumbnailProvider"
    COMMAND swift package resolve
    COMMAND swift build --build-path ${SWIFT_BUILD_DIR} -c ${SWIFT_BUILD_TYPE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${SWIFT_SOURCES}
    COMMENT "Building swift code"
)

add_custom_target(swift_package_build ALL
    DEPENDS "${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreQLPreviewProvider"
            "${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreThumbnailProvider"
)

# Install targets
install(CODE "
    # Create directory structure
    file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreQLPreviewProvider.appex/Contents/MacOS\")
    file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreThumbnailProvider.appex/Contents/MacOS\")

    # Copy Info.plist files
    file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/Sources/MuseScoreQLPreviewProvider/Info.plist\"
         DESTINATION \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreQLPreviewProvider.appex/Contents\")
    file(COPY \"${CMAKE_CURRENT_SOURCE_DIR}/Sources/MuseScoreThumbnailProvider/Info.plist\"
         DESTINATION \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreThumbnailProvider.appex/Contents\")

    # Copy executable to MacOS
    file(COPY \"${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreQLPreviewProvider\"
         DESTINATION \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreQLPreviewProvider.appex/Contents/MacOS\")
    file(COPY \"${SWIFT_BUILD_DIR}/${SWIFT_BUILD_TYPE}/MuseScoreThumbnailProvider\"
         DESTINATION \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreThumbnailProvider.appex/Contents/MacOS\")

    # Ad-hoc code sign the QuickLook extension, for dev
    execute_process(
        COMMAND codesign --force --sign - --timestamp=none --entitlements \"${CMAKE_CURRENT_SOURCE_DIR}/plugin.entitlements\"
                \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreQLPreviewProvider.appex\"
        RESULT_VARIABLE CODESIGN_RESULT
    )
    if(NOT CODESIGN_RESULT EQUAL 0)
        message(FATAL_ERROR \"Failed to ad-hoc sign QuickLook extension\")
    endif()

    # Ad-hoc code sign the Thumbnail Provider extension, for dev
    execute_process(
        COMMAND codesign --force --sign - --timestamp=none --entitlements \"${CMAKE_CURRENT_SOURCE_DIR}/plugin.entitlements\"
                \"${CMAKE_INSTALL_PREFIX}/mscore.app/Contents/PlugIns/MuseScoreThumbnailProvider.appex\"
        RESULT_VARIABLE CODESIGN_RESULT
    )
    if(NOT CODESIGN_RESULT EQUAL 0)
        message(FATAL_ERROR \"Failed to ad-hoc sign Thumbnail Provider extension\")
    endif()

    # Ad-hoc code sign the main app, for dev
    execute_process(
        COMMAND codesign --force --sign - --timestamp=none
                \"${CMAKE_INSTALL_PREFIX}/mscore.app\"
        RESULT_VARIABLE CODESIGN_RESULT
    )
    if(NOT CODESIGN_RESULT EQUAL 0)
        message(FATAL_ERROR \"Failed to ad-hoc sign main app\")
    endif()
")
