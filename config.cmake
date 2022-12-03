#=============================================================================
#  MuseScore
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2020 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

# The MuseScore version number.
SET(MUSESCORE_NAME "MuseScore")
SET(MUSESCORE_VERSION_MAJOR  "4")
SET(MUSESCORE_VERSION_MINOR  "0")
SET(MUSESCORE_VERSION_PATCH  "1")
SET(MUSESCORE_VERSION_LABEL  "")

message(STATUS "MUSESCORE_BUILD_CONFIG ${MUSESCORE_BUILD_CONFIG}")
if (NOT MUSESCORE_BUILD_CONFIG)
    SET(MUSESCORE_BUILD_CONFIG "dev")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/build/cmake/config/${MUSESCORE_BUILD_CONFIG}.cmake)

SET(MUSESCORE_VERSION       "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")
# Version schema x.x.x is hardcoded in source
SET(MUSESCORE_VERSION_FULL  "${MUSESCORE_VERSION}.${MUSESCORE_VERSION_PATCH}")

if (MUSESCORE_LABEL)
    set(MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} ${MUSESCORE_LABEL}")
endif()

if (MSCORE_UNSTABLE)
    set (MUSESCORE_NAME_VERSION "${MUSESCORE_NAME_VERSION} (${MUSESCORE_VERSION_FULL} unstable)")
endif()

# Set revision for local builds
# Before need run 'make revision' or 'msvc_build.bat revision'
include(${CMAKE_CURRENT_LIST_DIR}/build/cmake/TryUseLocalRevision.cmake)

# Print variables which are needed by CI build scripts.
# STATUS mode makes message() command use stdout for its output.
message(STATUS "MSCORE_UNSTABLE ${MSCORE_UNSTABLE}")
message(STATUS "MSCORE_RELEASE_CHANNEL ${MSCORE_RELEASE_CHANNEL}")
message(STATUS "MUSESCORE_VERSION_FULL ${MUSESCORE_VERSION_FULL}")
if (MSCORE_UNSTABLE)
    message(STATUS "VERSION ${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}b-${MUSESCORE_REVISION}")
else()
    message(STATUS "VERSION ${MUSESCORE_VERSION_FULL}")
endif()
message(STATUS "MUSESCORE_REVISION: ${MUSESCORE_REVISION}")

if (APPLE)
    SET(Mscore_INSTALL_NAME  "Contents/Resources/")
    SET(Mscore_SHARE_NAME    "mscore.app/")
elseif (MSVC OR MINGW)
    SET(Mscore_INSTALL_NAME  "")
    SET(Mscore_SHARE_NAME    "./")
else()
    SET(Mscore_INSTALL_NAME  "mscore${MSCORE_INSTALL_SUFFIX}-${MUSESCORE_VERSION}/")
    SET(Mscore_SHARE_NAME    "share/")
endif()
