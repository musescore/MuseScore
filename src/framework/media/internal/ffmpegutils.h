/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>

#include "io/path.h"

namespace muse::media {
using FFmpegVersion = int;
static const int FFMPEG_V8 = 8;
static const int FFMPEG_V7 = 7;
static const int FFMPEG_V6 = 6;
static const int FFMPEG_V5 = 5;
static const int FFMPEG_V4 = 4;
static const int FFMPEG_INVALID_VERION = -1;

struct FFmpegVersionInfo {
    int avFormatVersion = -1;
    int avUtilVersion = -1;
    int avCodecVersion = -1;
    int swScaleVersion = -1;
    int swResampleVersion = -1;

    bool isValid() const
    {
        return avFormatVersion != -1 && avUtilVersion != -1 && avCodecVersion != -1 && swScaleVersion != -1 && swResampleVersion != -1;
    }
};

static const std::vector<std::pair<int, FFmpegVersionInfo> > FFMPEG_COMPONENTS_VERSIONS = {
    // ffmpeg           avFormat    avUtil      avCodec     swScale    swResample
    { FFMPEG_V8,        { 62,       60,         62,         9,         6 } },
    { FFMPEG_V7,        { 61,       59,         61,         8,         5 } },
    { FFMPEG_V6,        { 60,       58,         60,         7,         4 } },
    { FFMPEG_V5,        { 59,       57,         59,         6,         4 } },
    { FFMPEG_V4,        { 58,       56,         58,         5,         3 } },
};

struct FFmpegLibPaths {
    io::path_t avUtilPath;
    io::path_t avCodecPath;
    io::path_t avFormatPath;
    io::path_t swScalePath;
    io::path_t swResamplePath;
};

FFmpegLibPaths findLibraryPaths(const io::path_t& ffmpegLibsDir);
FFmpegVersion versionFromAVFormatPath(const io::path_t& path);
}
