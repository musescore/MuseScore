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

# A separate executable: rpcpacker_tests.cpp includes audiotypes.h with #pragma pack(1),
# which must not meet the normally packed audio code linked into muse_audio_tests
set(MODULE_TEST muse_audio_rpcpacker_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/rpcpacker_tests.cpp
)

# Inherited from the parent directory
set(MODULE_TEST_LINK)

include(SetupGTest)
