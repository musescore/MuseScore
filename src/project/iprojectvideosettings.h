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

#include <memory>
#include <vector>

#include "async/notification.h"
#include "global/io/path.h"
#include "global/types/string.h"

namespace mu::project {
enum class VideoTimecodeDisplayMode {
    Off = 0,
    AboveBars,
    BelowBars
};

struct VideoHitPointSettings
{
    muse::String label;
    int timeMs = 0;
    int color = 0x3B94E5;

    bool operator==(const VideoHitPointSettings& other) const
    {
        return label == other.label
               && timeMs == other.timeMs
               && color == other.color;
    }

    bool operator!=(const VideoHitPointSettings& other) const
    {
        return !(*this == other);
    }
};

struct VideoAttachmentSettings
{
    muse::io::path_t path;
    int offsetMs = 0;
    float volume = 1.f;
    float balance = 0.f;
    bool muted = false;
    bool solo = false;
    double frameRate = 24.0;
    VideoTimecodeDisplayMode timecodeDisplayMode = VideoTimecodeDisplayMode::Off;
    std::vector<VideoHitPointSettings> hitPoints;

    bool isValid() const
    {
        return !path.empty();
    }

    bool operator==(const VideoAttachmentSettings& other) const
    {
        return path == other.path
               && offsetMs == other.offsetMs
               && volume == other.volume
               && balance == other.balance
               && muted == other.muted
               && solo == other.solo
               && frameRate == other.frameRate
               && timecodeDisplayMode == other.timecodeDisplayMode
               && hitPoints == other.hitPoints;
    }

    bool operator!=(const VideoAttachmentSettings& other) const
    {
        return !(*this == other);
    }
};

class IProjectVideoSettings
{
public:
    virtual ~IProjectVideoSettings() = default;

    virtual const VideoAttachmentSettings& attachment() const = 0;
    virtual void setAttachment(const VideoAttachmentSettings& attachment) = 0;
    virtual void clearAttachment() = 0;

    virtual muse::async::Notification settingsChanged() const = 0;
};

using IProjectVideoSettingsPtr = std::shared_ptr<IProjectVideoSettings>;
}
