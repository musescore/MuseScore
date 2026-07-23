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
#include "diveinfoconverter.h"

#include <engraving/dom/note.h>

#include <cmath>

using namespace mu::engraving;

namespace mu::iex::guitarpro {
static std::vector<SegmentData> classifyDiveSegments(const PitchValues& pitchValues)
{
    enum class Dir {
        NONE, UP, DOWN
    };

    std::vector<SegmentData> segments;
    Dir prevDir = Dir::NONE;
    size_t segStartIdx = 0;

    for (size_t i = 0; i + 1 < pitchValues.size(); ++i) {
        const int dp = pitchValues[i + 1].pitch - pitchValues[i].pitch;
        if (dp == 0) {
            continue;
        }

        const int qt = gpPitchToQuarterTones(dp);
        if (qt == 0) {
            continue;
        }

        Dir curDir = dp > 0 ? Dir::UP : Dir::DOWN;

        if (curDir == prevDir && !segments.empty()) {
            SegmentData& last = segments.back();
            last.quarterTones = gpPitchToQuarterTones(
                pitchValues[i + 1].pitch - pitchValues[segStartIdx].pitch);
            last.endFactor = static_cast<double>(pitchValues[i + 1].time) / PitchValue::MAX_TIME;
            continue;
        }

        segStartIdx = i;
        SegmentData seg;
        seg.quarterTones = qt;
        seg.startFactor = static_cast<double>(pitchValues[i].time) / PitchValue::MAX_TIME;
        seg.endFactor = static_cast<double>(pitchValues[i + 1].time) / PitchValue::MAX_TIME;
        segments.push_back(std::move(seg));

        prevDir = curDir;
    }

    return segments;
}

int gpPitchToQuarterTones(int gpPitch)
{
    return static_cast<int>(std::lround(static_cast<float>(gpPitch) / GP_PITCH_PER_QUARTERTONE));
}

ImportedDiveInfo DiveInfoConverter::fromPitchValues(const PitchValues& pitchValues, bool isContinuedWhammy)
{
    ImportedDiveInfo info;

    if (pitchValues.size() < 2) {
        return info;
    }

    const int originPitch = pitchValues.front().pitch;
    const int destPitch = pitchValues.back().pitch;
    const int originQuarterTones = gpPitchToQuarterTones(originPitch);

    std::vector<SegmentData> chain = classifyDiveSegments(pitchValues);

    if (isContinuedWhammy && destPitch == originPitch) {
        return info;
    } else if (!isContinuedWhammy && originPitch != 0 && destPitch == originPitch) {
        info.type = DiveType::PRE_DIVE;
        info.quarterTones = -gpPitchToQuarterTones(originPitch);
        info.endFactor = 1.0;
        return info;
    } else if (!isContinuedWhammy && originPitch != 0) {
        info.type = DiveType::PRE_DIVE;
        info.quarterTones = -originQuarterTones;
        info.endFactor = 1.0;
        info.graceDiveSegments = std::move(chain);
        return info;
    } else if (chain.empty()) {
        return info;
    }

    info.type = DiveType::DIVE;
    info.graceDiveSegments = std::move(chain);

    if (info.graceDiveSegments.size() == 1) {
        SegmentData& only = info.graceDiveSegments.front();
        only.quarterTones = gpPitchToQuarterTones(destPitch - originPitch);
        only.startFactor = static_cast<double>(pitchValues.front().time) / PitchValue::MAX_TIME;
        only.endFactor = static_cast<double>(pitchValues.back().time) / PitchValue::MAX_TIME;
    }

    return info;
}

ImportedDiveInfo DiveInfoConverter::fillDiveInfo(const Note* note, const PitchValues& pitchValues, bool isContinuedWhammy)
{
    if (!note || pitchValues.size() < 2) {
        return {};
    }

    ImportedDiveInfo info = fromPitchValues(pitchValues, isContinuedWhammy);

    if (info.type == DiveType::NONE) {
        return info;
    }

    if (info.type == DiveType::DIVE && info.graceDiveSegments.empty()) {
        info.type = DiveType::NONE;
        return info;
    }

    info.note = note;
    return info;
}
} // namespace mu::iex::guitarpro
