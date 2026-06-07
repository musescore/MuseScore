# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
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

muse_create_module(iex_encore)

target_sources(iex_encore PRIVATE
    enc-module.cpp
    enc-module.h
    ienc-importconfiguration.h
    internal/enc-importconfiguration.h
    internal/enc-importconfiguration.cpp
    internal/parser/parsers-note.cpp
    internal/parser/parsers-chord.cpp
    internal/parser/parsers-ornament.cpp
    internal/parser/parsers-lyric-tie.cpp
    internal/parser/parsers-measure.cpp
    internal/parser/parsers-metadata.cpp
    internal/parser/parsers-text.cpp
    internal/parser/parsers-root.cpp
    internal/parser/parsers-encoding.h
    internal/parser/parsers-encoding.cpp
    internal/parser/readers.h
    internal/parser/readers.cpp
    internal/parser/readers-v0xc4-base.h
    internal/parser/readers-v0xc4-base.cpp
    internal/parser/readers-v0xc4.h
    internal/parser/readers-v0xc4.cpp
    internal/parser/elem.h
    internal/importer/mappers-clefs.cpp
    internal/importer/mappers-instruments.cpp
    internal/importer/mappers-tempo.cpp
    internal/importer/mappers-articulations.cpp
    internal/importer/mappers.h
    internal/parser/ticks.cpp
    internal/parser/ticks.h
    internal/importer/emitters-tuplets.cpp
    internal/importer/emitters-tuplets-groups.cpp
    internal/importer/emitters-tuplets.h
    internal/notationencreader.cpp
    internal/notationencreader.h
    internal/importer/import.cpp
    internal/importer/page-layout.h
    internal/importer/page-layout.cpp
    internal/importer/debug-dump.h
    internal/importer/debug-dump.cpp
    internal/importer/ctx.h
    internal/importer/builders.h
    internal/importer/builders-parts.cpp
    internal/importer/builders-measures.cpp
    internal/importer/emitters-internal.h
    internal/importer/coords.h
    internal/importer/coords.cpp
    internal/importer/durations.h
    internal/importer/durations.cpp
    internal/importer/emitters-note.cpp
    internal/importer/emitters-note-artic.cpp
    internal/importer/emitters-rest.cpp
    internal/importer/emitters-tempo.cpp
    internal/importer/emitters-fill.cpp
    internal/importer/emitters-overfill.cpp
    internal/importer/emitters.cpp
    internal/importer/import.h
    internal/importer/import-options.h
)

# While the module is partially populated, unity chunk boundaries shift and a source can lose
# the neighbor includes it sees in the full module; one unity unit keeps CI-equivalent
# semantics without those boundaries. Removed when every module source is in place.
set_target_properties(iex_encore PROPERTIES UNITY_BUILD_BATCH_SIZE 0)

target_link_libraries(iex_encore PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
