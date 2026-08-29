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

#include <functional>
#include <memory>
#include <vector>

#include <QString>

#include "async/notification.h"
#include "global/io/path.h"
#include "global/types/string.h"

namespace mu::engraving {
class Score;
}

namespace mu::project {
enum class VideoTimecodeDisplayMode {
    Off = 0,
    AboveBars,
    BelowBars
};

struct VideoHitPointSettings
{
    //! NOTE Stable identity for this hit point, independent of its position in
    //! hitPoints (which is re-sorted chronologically on every edit). 0 means
    //! "not yet assigned" -- ProjectVideoSettings assigns a real one on load/set.
    //! Callers must reference a hit point by id, never by list position, since
    //! the position can change as a side effect of the very edit being made.
    int id = 0;
    muse::String label;
    int timeMs = 0;
    int color = 0x3B94E5;

    bool operator==(const VideoHitPointSettings& other) const
    {
        return id == other.id
               && label == other.label
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
    bool showHitPoints = true;
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
               && showHitPoints == other.showHitPoints
               && hitPoints == other.hitPoints;
    }

    bool operator!=(const VideoAttachmentSettings& other) const
    {
        return !(*this == other);
    }
};

//! NOTE Shared by the notation Timeline and the Video panel so both display the
//! exact same SMPTE-style timecode for a given position; keep this the only
//! implementation rather than duplicating it at each call site.
QString formatVideoTimecode(int videoPositionMs, double frameRate);

//! NOTE Same reasoning as formatVideoTimecode() above -- the notation Timeline
//! and the Video panel both need to convert a score tick to the corresponding
//! video-timeline position; keep this the only implementation. Returns -1 if
//! score is null.
int videoPositionMsForTick(const mu::engraving::Score* score, int tick, int offsetMs);

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

//! NOTE Shared read-modify-write helper for the common "look up the current
//! attachment, tweak one field, persist it back" sequence -- a no-op if
//! there's no attached video. Keep call sites (e.g. NotationPageModel's
//! per-field setters) this thin instead of re-deriving the same
//! guard+read+write shape at each one.
void updateVideoAttachment(const IProjectVideoSettingsPtr& settings, const std::function<void(VideoAttachmentSettings&)>& mutate);
}
