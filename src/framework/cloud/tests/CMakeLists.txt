# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited and others
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

set(MODULE_TEST muse_cloud_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiocomservicemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/authorizationservicemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/cloudconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/musescorecomconvertservicemock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/musescorecomservicemock.h
)

if (MUSE_MODULE_CLOUD_MUSESCORECOM)
    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/musescorecomservice_tests.cpp
    )
endif()

set(MODULE_TEST_LINK
    muse_cloud
    Qt::Network
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
