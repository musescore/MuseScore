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

muse_create_module(iex_bww)

target_sources(iex_bww PRIVATE
    bwwmodule.cpp
    bwwmodule.h

    internal/notationbwwreader.cpp
    internal/notationbwwreader.h
    internal/bww/importbww.cpp
    internal/bww/lexer.cpp
    internal/bww/lexer.h
    internal/bww/parser.cpp
    internal/bww/parser.h
    internal/bww/symbols.cpp
    internal/bww/symbols.h
    internal/bww/writer.cpp
    internal/bww/writer.h
)

target_link_libraries(iex_bww PRIVATE engraving)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
