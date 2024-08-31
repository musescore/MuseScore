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

set(MUSE_APP_NAME "MuseScore")
set(MUSE_APP_TITLE "MuseScore Studio")
set(MUSE_APP_GUI_IDENTIFIER org.musescore.${MUSE_APP_NAME})
set(MUSE_APP_VERSION_MAJOR "4")
set(MUSE_APP_VERSION_MINOR "5")
set(MUSE_APP_VERSION_PATCH "0")
set(MUSE_APP_VERSION_MAJ_MIN "${MUSE_APP_VERSION_MAJOR}.${MUSE_APP_VERSION_MINOR}")
set(MUSE_APP_VERSION "${MUSE_APP_VERSION_MAJ_MIN}.${MUSE_APP_VERSION_PATCH}")
set(MUSE_APP_VERSION_LABEL "")
set(MUSE_APP_TITLE_VERSION "${MUSE_APP_TITLE} ${MUSE_APP_VERSION_MAJOR}")
set(MUSE_APP_NAME_VERSION "${MUSE_APP_NAME} ${MUSE_APP_VERSION_MAJOR}")
set(MUSE_APP_UNSTABLE ON)
set(MUSE_APP_IS_PRERELEASE ON)

if(MUSE_APP_BUILD_MODE MATCHES "dev")
    set(MUSE_APP_RELEASE_CHANNEL "devel")
endif()

if(MUSE_APP_BUILD_MODE MATCHES "testing")
    set(MUSE_APP_RELEASE_CHANNEL "testing")
endif()

if(MUSE_APP_BUILD_MODE MATCHES "release")
    set(MUSE_APP_RELEASE_CHANNEL "stable")
endif()

# Print variables which are needed by CI build scripts.
message(STATUS "MUSE_APP_RELEASE_CHANNEL ${MUSE_APP_RELEASE_CHANNEL}")
message(STATUS "MUSE_APP_VERSION ${MUSE_APP_VERSION}")
