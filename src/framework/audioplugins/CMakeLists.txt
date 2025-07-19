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

declare_module(muse_audioplugins)
set(MODULE_ALIAS muse::audioplugins)

set(MODULE_SRC

    ${CMAKE_CURRENT_LIST_DIR}/audiopluginsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/audiopluginsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/audiopluginstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/audiopluginserrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiopluginsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iknownaudiopluginsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiopluginsscanner.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiopluginsscannerregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiopluginmetareader.h
    ${CMAKE_CURRENT_LIST_DIR}/iaudiopluginmetareaderregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iregisteraudiopluginsscenario.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginsutils.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/knownaudiopluginsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/knownaudiopluginsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginsscannerregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginsscannerregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginmetareaderregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/audiopluginmetareaderregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/registeraudiopluginsscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/registeraudiopluginsscenario.h
)

setup_module()

if (MUSE_MODULE_AUDIOPLUGINS_TESTS)
    add_subdirectory(tests)
endif()
