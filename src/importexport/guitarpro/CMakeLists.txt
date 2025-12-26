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

muse_create_module(iex_guitarpro)

target_sources(iex_guitarpro PRIVATE
    guitarpromodule.cpp
    guitarpromodule.h
    iguitarproconfiguration.h
    internal/guitarproconfiguration.cpp
    internal/guitarproconfiguration.h
    internal/guitarprodrumset.cpp
    internal/guitarprodrumset.h
    internal/guitarproreader.cpp
    internal/guitarproreader.h

    internal/importgtp.cpp
    internal/importgtp-gp4.cpp
    internal/importgtp-gp5.cpp
    internal/importgtp-gp6.cpp
    internal/importgtp-gp7.cpp
    internal/importgtp.h
    internal/importptb.cpp
    internal/importptb.h
    internal/continiouselementsbuilder.cpp
    internal/continiouselementsbuilder.h
    internal/utils.cpp
    internal/utils.h

    internal/guitarbendimport/guitarbendimporttypes.h
    internal/guitarbendimport/guitarbendimporter.cpp
    internal/guitarbendimport/guitarbendimporter.h
    internal/guitarbendimport/benddatacontext.h
    internal/guitarbendimport/benddatacollector.cpp
    internal/guitarbendimport/benddatacollector.h
    internal/guitarbendimport/bendinfoconverter.cpp
    internal/guitarbendimport/bendinfoconverter.h
    internal/guitarbendimport/benddataprocessor.cpp
    internal/guitarbendimport/benddataprocessor.h

    internal/guitarbendimport/splitchord/benddatacontextsplitchord.h
    internal/guitarbendimport/splitchord/benddatacollectorsplitchord.cpp
    internal/guitarbendimport/splitchord/benddatacollectorsplitchord.h
    internal/guitarbendimport/splitchord/benddataprocessorsplitchord.cpp
    internal/guitarbendimport/splitchord/benddataprocessorsplitchord.h
    internal/guitarbendimport/splitchord/bendchorddurationsplitter.cpp
    internal/guitarbendimport/splitchord/bendchorddurationsplitter.h

    internal/gtp/gp6dombuilder.cpp
    internal/gtp/gp6dombuilder.h
    internal/gtp/gp7dombuilder.cpp
    internal/gtp/gp7dombuilder.h
    internal/gtp/gp67dombuilder.cpp
    internal/gtp/gp67dombuilder.h
    internal/gtp/gpbar.cpp
    internal/gtp/gpbar.h
    internal/gtp/gpbeat.cpp
    internal/gtp/gpbeat.h
    internal/gtp/gpconverter.cpp
    internal/gtp/gpconverter.h
    internal/gtp/gpdommodel.cpp
    internal/gtp/gpdommodel.h
    internal/gtp/gpdrumsetresolver.cpp
    internal/gtp/gpdrumsetresolver.h
    internal/gtp/gpmasterbar.cpp
    internal/gtp/gpmasterbar.h
    internal/gtp/gpmastertracks.cpp
    internal/gtp/gpmastertracks.h
    internal/gtp/gpnote.cpp
    internal/gtp/gpnote.h
    internal/gtp/gprhythm.cpp
    internal/gtp/gprhythm.h
    internal/gtp/gpscore.cpp
    internal/gtp/gpscore.h
    internal/gtp/gptrack.cpp
    internal/gtp/gptrack.h
    internal/gtp/gpvoice.cpp
    internal/gtp/gpvoice.h
    internal/gtp/igpdombuilder.h
    internal/gtp/inoteproperty.h
)

target_link_libraries(iex_guitarpro PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
