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

muse_create_module(iex_audioexport)

target_sources(iex_audioexport PRIVATE
    audioexportmodule.cpp
    audioexportmodule.h
    iaudioexportconfiguration.h
    internal/audioexportconfiguration.cpp
    internal/audioexportconfiguration.h
    internal/abstractaudiowriter.cpp
    internal/abstractaudiowriter.h
    internal/mp3writer.cpp
    internal/mp3writer.h
    internal/wavewriter.cpp
    internal/wavewriter.h
    internal/oggwriter.cpp
    internal/oggwriter.h
    internal/flacwriter.cpp
    internal/flacwriter.h
)
