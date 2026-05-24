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
    internal/parser/elem.h
    internal/parser/ticks.cpp
    internal/parser/ticks.h
    internal/notationencreader.cpp
    internal/notationencreader.h
    internal/importer/import.cpp
    internal/importer/durations.h
    internal/importer/durations.cpp
    internal/importer/import.h
    internal/importer/import-options.h
)

target_link_libraries(iex_encore PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
