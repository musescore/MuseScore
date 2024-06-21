# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

set(MODULE_TEST vtests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/vtest_gtest_runner.cpp
)

set(MODULE_TEST_DEF
    -DVTEST_ROOT_DIR="${CMAKE_CURRENT_LIST_DIR}"
    -DVTEST_MSCORE_REF_BIN="${MUE_VTEST_MSCORE_REF_BIN}"
    -DVTEST_MSCORE_BIN="${INSTALL_BIN_DIR}/mscore"
    )

set(ADD_VTEST OFF)
if (MUE_BUILD_ENGRAVING_TESTS)
    set(ADD_VTEST ON)
    if (NOT EXISTS "${MUE_VTEST_MSCORE_REF_BIN}")
        set(ADD_VTEST OFF)
    endif()
endif()

if (ADD_VTEST)
    include(SetupGTest)
endif()
