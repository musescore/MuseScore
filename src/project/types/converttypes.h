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

#include "cloud/musescorecom/converttypes.h"
#include "projecttypes.h"

#include "global/io/path.h"

namespace mu::project {
using ConvertConfig = muse::cloud::ConvertConfig;
using ConvertType = muse::cloud::ConvertType;
using ConvertInput = muse::cloud::ConvertInput;
using ReviewRating = muse::cloud::ReviewRating;
using LinkSource = muse::cloud::LinkSource;
using LinkSources = muse::cloud::LinkSources;

enum class FileCategory {
    Unknown,
    Audio,
    Pdf,
    Image
};

inline FileCategory fileCategoryFromPath(const muse::io::path_t& path, const ConvertConfig& config)
{
    const QString ext = QString::fromStdString(muse::io::suffix(path)).toLower();
    if (ext == "pdf") {
        return FileCategory::Pdf;
    }

    //! NOTE: if config is unavailable, fall back to a best-effort guess
    const bool isImage = !config.omr.allowedExtensions.isEmpty()
                         ? config.omr.allowedExtensions.contains(ext)
                         : isImageFileSuffix(ext.toStdString());

    if (isImage) {
        return FileCategory::Image;
    }

    const bool isAudio = !config.audio2score.allowedExtensions.isEmpty()
                         ? config.audio2score.allowedExtensions.contains(ext)
                         : isAudioFileSuffix(ext.toStdString());

    return isAudio ? FileCategory::Audio : FileCategory::Unknown;
}

inline bool isSupportedExtension(const muse::io::path_t& path, const ConvertConfig& config)
{
    return fileCategoryFromPath(path, config) != FileCategory::Unknown;
}
}
