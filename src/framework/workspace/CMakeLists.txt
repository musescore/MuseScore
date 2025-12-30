# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_workspace ALIAS muse::workspace)

set(MODULE_QRC workspace.qrc)

target_sources(muse_workspace PRIVATE
    workspacemodule.cpp
    workspacemodule.h
    workspacetypes.h
    workspaceerrors.h
    iworkspaceconfiguration.h
    iworkspace.h
    iworkspacemanager.h
    iworkspacesdataprovider.h

    internal/workspaceconfiguration.cpp
    internal/workspaceconfiguration.h
    internal/workspace.cpp
    internal/workspace.h
    internal/workspacemanager.cpp
    internal/workspacemanager.h
    internal/workspacesdataprovider.cpp
    internal/workspacesdataprovider.h
    internal/workspacefile.cpp
    internal/workspacefile.h
    internal/workspaceuiactions.cpp
    internal/workspaceuiactions.h
    internal/workspaceactioncontroller.cpp
    internal/workspaceactioncontroller.h
    internal/workspaceutils.cpp
    internal/workspaceutils.h
)

if (MUSE_MODULE_WORKSPACE_QML)
    add_subdirectory(qml/Muse/Workspace)
endif()
