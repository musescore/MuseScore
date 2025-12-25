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

muse_create_module(iex_tabledit)

target_sources(iex_tabledit PRIVATE
    tableditmodule.cpp
    tableditmodule.h
    internal/tableditreader.cpp
    internal/tableditreader.h

    internal/importtef.cpp
    internal/importtef.h
    internal/measurehandler.cpp
    internal/measurehandler.h
    internal/note.cpp
    internal/note.h
    internal/tuplethandler.cpp
    internal/tuplethandler.h
    internal/voiceallocator.cpp
    internal/voiceallocator.h
)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
