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

namespace muse::media {
struct FFmpegVersionInfo {
    int avFormatVersion = -1;
    int avUtilVersion = -1;
    int avCodecVersion = -1;
    int swScaleVersion = -1;

    bool isValid() const
    {
        return avFormatVersion != -1 && avUtilVersion != -1 && avCodecVersion != -1 && swScaleVersion != -1;
    }
};

static const std::vector<std::pair<int, FFmpegVersionInfo> > FFMPEG_COMPONENTS_VERSIONS = {
    { 8, { 62, 60, 62, 9 } },
    { 7, { 61, 59, 61, 8 } },
    { 6, { 60, 58, 60, 7 } },
    { 5, { 59, 57, 59, 6 } },
    { 4, { 58, 56, 58, 5 } },
};
}
