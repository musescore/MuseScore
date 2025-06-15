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

declare_module(iex_guitarpro)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/guitarpromodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/guitarpromodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iguitarproconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarprodrumset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarprodrumset.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp5.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp6.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp7.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importptb.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importptb.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/continiouselementsbuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/continiouselementsbuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/utils.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/guitarbendimporttypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/guitarbendimporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/guitarbendimporter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/benddatacontext.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/benddatacollector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/benddatacollector.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/bendinfoconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/bendinfoconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/benddataprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/benddataprocessor.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/benddatacontextsplitchord.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/benddatacollectorsplitchord.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/benddatacollectorsplitchord.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/benddataprocessorsplitchord.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/benddataprocessorsplitchord.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/bendchorddurationsplitter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarbendimport/splitchord/bendchorddurationsplitter.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp6dombuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp6dombuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp7dombuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp7dombuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp67dombuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gp67dombuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpbar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpbar.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpbeat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpbeat.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpdommodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpdommodel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpdrumsetresolver.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpdrumsetresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpmasterbar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpmasterbar.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpmastertracks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpmastertracks.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpnote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpnote.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gprhythm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gprhythm.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpscore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpscore.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gptrack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gptrack.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpvoice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/gpvoice.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/igpdombuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/gtp/inoteproperty.h
    )

set(MODULE_SRC ${MODULE_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproconfiguration.h
    )

set(MODULE_LINK
    engraving
    )

setup_module()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
