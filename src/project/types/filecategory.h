/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <string>
#include <unordered_set>

#include "containers.h"
#include "io/path.h"

namespace mu::project {
enum class FileCategory {
    Unknown,
    Audio,
    Pdf,
    Image
};

inline bool isAudioFileSuffix(const std::string& suffix)
{
    static const std::unordered_set<std::string> audioSuffixes {
        "mp3", "wav", "ogg", "flac", "aac",
    };

    return muse::contains(audioSuffixes, suffix);
}

inline bool isImageFileSuffix(const std::string& suffix)
{
    static const std::unordered_set<std::string> imageSuffixes {
        "png", "jpg", "jpeg",
    };

    return muse::contains(imageSuffixes, suffix);
}

//! NOTE: suffix must already be lowercased
inline FileCategory fileCategoryFromSuffix(const std::string& suffix)
{
    if (suffix == "pdf") {
        return FileCategory::Pdf;
    }

    if (isImageFileSuffix(suffix)) {
        return FileCategory::Image;
    }

    if (isAudioFileSuffix(suffix)) {
        return FileCategory::Audio;
    }

    return FileCategory::Unknown;
}

inline FileCategory fileCategoryFromPath(const muse::io::path_t& path)
{
    return fileCategoryFromSuffix(muse::io::suffix(path));
}
}
