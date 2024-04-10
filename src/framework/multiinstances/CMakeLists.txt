# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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

declare_module(muse_multiinstances)
set(MODULE_ALIAS muse::multiinstances)

set(MODULE_QRC multiinstances.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/multiinstancesmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/multiinstancesmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imultiinstancesprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/iprojectprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/resourcelockguard.h
    ${CMAKE_CURRENT_LIST_DIR}/mitypes.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/multiinstancesprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/multiinstancesprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/multiinstancesuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/multiinstancesuiactions.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipclog.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipc.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcchannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcchannel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcserver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcserver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcsocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcsocket.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipclock.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipclock.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcloop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/ipc/ipcloop.h

    ${CMAKE_CURRENT_LIST_DIR}/dev/multiinstancesdevmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dev/multiinstancesdevmodel.h
    )

setup_module()
