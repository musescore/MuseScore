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

set(MUSESCORE_NAME "MuseScore")
set(MUSESCORE_VERSION_MAJOR  "4")
set(MUSESCORE_VERSION_MINOR  "2")
set(MUSESCORE_VERSION_PATCH  "0")
set(MUSESCORE_VERSION        "${MUSESCORE_VERSION_MAJOR}.${MUSESCORE_VERSION_MINOR}")
set(MUSESCORE_VERSION_FULL   "${MUSESCORE_VERSION}.${MUSESCORE_VERSION_PATCH}")
set(MUSESCORE_VERSION_LABEL  "")

if(MUSESCORE_BUILD_MODE MATCHES "dev")
    set(MUSESCORE_RELEASE_CHANNEL "devel")
endif()

if(MUSESCORE_BUILD_MODE MATCHES "testing")
    set(MUSESCORE_RELEASE_CHANNEL "testing")
endif()

if(MUSESCORE_BUILD_MODE MATCHES "release")
    set(MUSESCORE_RELEASE_CHANNEL "stable")
endif()

# Print variables which are needed by CI build scripts.
message(STATUS "MUSESCORE_RELEASE_CHANNEL ${MUSESCORE_RELEASE_CHANNEL}")
message(STATUS "MUSESCORE_VERSION_FULL ${MUSESCORE_VERSION_FULL}")

