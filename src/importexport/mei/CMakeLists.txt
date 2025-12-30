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

muse_create_module(iex_mei)

target_sources(iex_mei PRIVATE
    imeiconfiguration.h
    meimodule.cpp
    meimodule.h
    internal/meiconfiguration.cpp
    internal/meiconfiguration.h
    internal/meiconverter.cpp
    internal/meiconverter.h
    internal/meiexporter.cpp
    internal/meiexporter.h
    internal/meiimporter.cpp
    internal/meiimporter.h
    internal/meireader.cpp
    internal/meireader.h
    internal/meiwriter.cpp
    internal/meiwriter.h
    internal/libmei.h

    thirdparty/libmei/att.cpp
    thirdparty/libmei/att.h
    thirdparty/libmei/attalternates.h
    thirdparty/libmei/attdef.h
    thirdparty/libmei/element.h

    thirdparty/libmei/attconverter.cpp
    thirdparty/libmei/attconverter.h
    thirdparty/libmei/atts_cmn.cpp
    thirdparty/libmei/atts_cmn.h
    thirdparty/libmei/atts_cmnornaments.cpp
    thirdparty/libmei/atts_cmnornaments.h
    thirdparty/libmei/atts_externalsymbols.cpp
    thirdparty/libmei/atts_externalsymbols.h
    thirdparty/libmei/atts_fingering.cpp
    thirdparty/libmei/atts_fingering.h
    thirdparty/libmei/atts_gestural.cpp
    thirdparty/libmei/atts_gestural.h
    thirdparty/libmei/atts_harmony.cpp
    thirdparty/libmei/atts_harmony.h
    thirdparty/libmei/atts_midi.cpp
    thirdparty/libmei/atts_midi.h
    thirdparty/libmei/atts_shared.cpp
    thirdparty/libmei/atts_shared.h
    thirdparty/libmei/atts_stringtab.cpp
    thirdparty/libmei/atts_stringtab.h
    thirdparty/libmei/atts_visual.cpp
    thirdparty/libmei/atts_visual.h

    thirdparty/libmei/cmn.cpp
    thirdparty/libmei/cmn.h
    thirdparty/libmei/cmnornaments.cpp
    thirdparty/libmei/cmnornaments.h
    thirdparty/libmei/fingering.cpp
    thirdparty/libmei/fingering.h
    thirdparty/libmei/harmony.cpp
    thirdparty/libmei/harmony.h
    thirdparty/libmei/lyrics.cpp
    thirdparty/libmei/lyrics.h
    thirdparty/libmei/midi.cpp
    thirdparty/libmei/midi.h
    thirdparty/libmei/shared.cpp
    thirdparty/libmei/shared.h   
)

target_include_directories(iex_mei PRIVATE
    internal
    ${CMAKE_SOURCE_DIR}/src/framework/global/thirdparty/pugixml
)

target_link_libraries(iex_mei PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
