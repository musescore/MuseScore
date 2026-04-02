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

set(MODULE_TEST muse_workspace_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/mocks/workspaceconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/workspace_test_helpers.h
    ${CMAKE_CURRENT_LIST_DIR}/workspace_test_helpers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspaceconfig_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacemanager_tests.cpp
)

set(MODULE_TEST_LINK
    muse::workspace
)

set(MODULE_TEST_INCLUDE
    ${MUSE_FRAMEWORK_SRC_PATH}/workspace
    ${MUSE_FRAMEWORK_SRC_PATH}/workspace/internal
    ${MUSE_FRAMEWORK_SRC_PATH}/multiwindows
)

# Paths injected as compile-time defines for use by the test environment:
#   BUILTIN_WORKSPACES_DIR - path to share/workspaces/ containing .mws files
#   WORKSPACE_CONFIG_FILE  - path to the workspace config (src/app/configs/workspaces.cfg)
set(MODULE_TEST_DEF
    BUILTIN_WORKSPACES_DIR="${CMAKE_SOURCE_DIR}/share/workspaces"
    WORKSPACE_CONFIG_FILE="${CMAKE_SOURCE_DIR}/src/app/configs/workspaces.cfg"
)

include(SetupGTest)
