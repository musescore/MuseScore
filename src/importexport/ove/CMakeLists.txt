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

muse_create_module(iex_ove)

target_sources(iex_ove PRIVATE
    ovemodule.cpp
    ovemodule.h
    ioveconfiguration.h
    internal/oveconfiguration.cpp
    internal/oveconfiguration.h
    internal/overeader.cpp
    internal/overeader.h
    internal/importove.cpp
    internal/ove.cpp
    internal/ove.h
)

target_link_libraries(iex_ove PRIVATE
    engraving
    Qt::Core5Compat
)
