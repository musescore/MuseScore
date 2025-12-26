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

muse_create_module(iex_videoexport)

target_sources(iex_videoexport PRIVATE
    videoexportmodule.cpp
    videoexportmodule.h
    videoexporttypes.h
    ivideoexportconfiguration.h

    internal/videoexportconfiguration.cpp
    internal/videoexportconfiguration.h
    internal/videowriter.cpp
    internal/videowriter.h
    internal/videoencoder.cpp
    internal/videoencoder.h
    internal/ffmpeg.h
)

target_link_libraries(iex_videoexport PRIVATE engraving)

# FFmpeg
find_package(FFmpeg REQUIRED)

target_include_directories(iex_videoexport PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(iex_videoexport PRIVATE ${FFMPEG_LIBRARIES})
