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

muse_create_module(muse_multiinstances ALIAS muse::multiinstances)

target_sources(muse_multiinstances PRIVATE
    multiinstancesmodule.cpp
    multiinstancesmodule.h
    imultiinstancesprovider.h
    iprojectprovider.h
    resourcelockguard.h
    mitypes.h

    internal/multiinstancesprovider.cpp
    internal/multiinstancesprovider.h
    internal/multiinstancesuiactions.cpp
    internal/multiinstancesuiactions.h

    internal/ipc/ipclog.h
    internal/ipc/ipc.cpp
    internal/ipc/ipc.h
    internal/ipc/ipcchannel.cpp
    internal/ipc/ipcchannel.h
    internal/ipc/ipcserver.cpp
    internal/ipc/ipcserver.h
    internal/ipc/ipcsocket.cpp
    internal/ipc/ipcsocket.h
    internal/ipc/ipclock.cpp
    internal/ipc/ipclock.h
    internal/ipc/ipcloop.cpp
    internal/ipc/ipcloop.h
)

if (MUSE_MODULE_MULTIINSTANCES_QML)
    add_subdirectory(qml/Muse/MultiInstances)
endif()
