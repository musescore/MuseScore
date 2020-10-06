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
SET(MUSESCORE_VERSION_MAJOR  "3")
SET(MUSESCORE_VERSION_MINOR  "5")
SET(MUSESCORE_VERSION_PATCH  "0")
SET(MUSESCORE_VERSION_LABEL  "")

message(STATUS "MUSESCORE_BUILD_CONFIG ${MUSESCORE_BUILD_CONFIG}")
if (NOT MUSESCORE_BUILD_CONFIG)
    SET(MUSESCORE_BUILD_CONFIG "dev")
endif (NOT MUSESCORE_VERSION_LABEL)

include("${CMAKE_CURRENT_LIST_DIR}/build/config/${MUSESCORE_BUILD_CONFIG}.cmake")

SET(MUSESCORE_VERSION       "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")
# Version schema x.x.x is hardcoded in source
SET(MUSESCORE_VERSION_FULL  "${MUSESCORE_VERSION}.${MUSESCORE_VERSION_PATCH}")

# Print variables which are needed by CI build scripts.
# STATUS mode makes message() command use stdout for its output.
message(STATUS "MSCORE_UNSTABLE ${MSCORE_UNSTABLE}")
message(STATUS "MSCORE_RELEASE_CHANNEL ${MSCORE_RELEASE_CHANNEL}")
message(STATUS "MUSESCORE_VERSION_FULL ${MUSESCORE_VERSION_FULL}")
if (MSCORE_UNSTABLE)
    file(READ "${CMAKE_CURRENT_LIST_DIR}/mscore/revision.h" MUSESCORE_REVISION)
    message(STATUS "VERSION ${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}b-${MUSESCORE_REVISION}")
else (MSCORE_UNSTABLE)
    message(STATUS "VERSION ${MUSESCORE_VERSION_FULL}")
endif (MSCORE_UNSTABLE)
