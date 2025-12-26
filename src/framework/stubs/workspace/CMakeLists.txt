# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

muse_create_module(muse_workspace ALIAS muse::workspace STUB)

target_sources(muse_workspace PRIVATE
    workspacestubmodule.cpp
    workspacestubmodule.h
    workspacestub.cpp
    workspacestub.h
    workspacemanagerstub.cpp
    workspacemanagerstub.h
    workspaceconfigurationstub.cpp
    workspaceconfigurationstub.h
    workspacesdataproviderstub.cpp
    workspacesdataproviderstub.h
)

if (MUSE_MODULE_WORKSPACE_QML)
    add_subdirectory(qml/Muse/Workspace)
endif()
