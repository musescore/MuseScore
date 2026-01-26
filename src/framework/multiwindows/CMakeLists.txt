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

muse_create_module(muse_multiwindows ALIAS muse::multiwindows)

target_sources(muse_multiwindows PRIVATE
    multiwindowsmodule.cpp
    multiwindowsmodule.h
    imultiwindowsprovider.h
    iprojectprovider.h
    resourcelockguard.h

    internal/multiwindowsuiactions.cpp
    internal/multiwindowsuiactions.h

    # One process (multi window)
    internal/oneprocess/oneprocessprovider.cpp
    internal/oneprocess/oneprocessprovider.h

    # Multi process
    internal/multiprocess/multiprocessprovider.cpp
    internal/multiprocess/multiprocessprovider.h

    internal/multiprocess/ipc/ipclog.h
    internal/multiprocess/ipc/ipc.cpp
    internal/multiprocess/ipc/ipc.h
    internal/multiprocess/ipc/ipcchannel.cpp
    internal/multiprocess/ipc/ipcchannel.h
    internal/multiprocess/ipc/ipcserver.cpp
    internal/multiprocess/ipc/ipcserver.h
    internal/multiprocess/ipc/ipcsocket.cpp
    internal/multiprocess/ipc/ipcsocket.h
    internal/multiprocess/ipc/ipclock.cpp
    internal/multiprocess/ipc/ipclock.h
    internal/multiprocess/ipc/ipcloop.cpp
    internal/multiprocess/ipc/ipcloop.h
)

if (MUSE_MODULE_MULTIWINDOWS_QML)
    add_subdirectory(qml/Muse/MultiWindows)
endif()
