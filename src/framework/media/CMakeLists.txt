# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

muse_create_module(muse_media ALIAS muse::media NO_UNITY)

target_sources(muse_media PRIVATE
    mediamodule.cpp
    mediamodule.h
    imediaconfiguration.h

    ivideoencoder.h
    ivideoencoderresolver.h

    internal/mediaconfiguration.cpp
    internal/videoencoderresolver.cpp
    internal/mediaconfiguration.h
    internal/videoencoderresolver.h

    internal/ffmpegloader.cpp
    internal/ffmpegloader.h
    internal/ffmpegutils.h

    internal/ffmpeglibhandler.cpp
    internal/ffmpeglibhandler.h

    internal/videoencoder.cpp
    internal/videoencoder.h
)

target_include_directories(muse_media PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg/v8)

set(MODULE_USE_PCH OFF)
set(MODULE_USE_UNITY OFF)
