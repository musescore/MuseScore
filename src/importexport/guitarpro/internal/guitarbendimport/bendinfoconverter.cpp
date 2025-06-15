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
#include "bendinfoconverter.h"

#include <dom/note.h>

using namespace mu::engraving;

namespace mu::iex::guitarpro {
ImportedBendInfo BendInfoConverter::fillBendInfo(const Note* note, const PitchValues& pitchValues)
{
    enum class PitchValuesDiff {
        NONE,
        SAME_PITCH_AND_TIME,
        SAME_PITCH_DIFF_TIME,
        INCREASED_PITCH_SAME_TIME,
        INCREASED_PITCH_DIFF_TIME,
        DECREASED_PITCH_DIFF_TIME
    };

    using pvd = PitchValuesDiff;

    ImportedBendInfo importedInfo;
    PitchValuesDiff currentPitchDiff;
    PitchValuesDiff previousPitchDiff = PitchValuesDiff::NONE;

    auto pitchDiff = [](const PitchValue& prev, const PitchValue& cur) {
        if (prev.pitch == cur.pitch) {
            return prev.time == cur.time ? pvd::SAME_PITCH_AND_TIME : pvd::SAME_PITCH_DIFF_TIME;
        }

        if (prev.time == cur.time) {
            return pvd::INCREASED_PITCH_SAME_TIME;
        }

        return prev.pitch < cur.pitch ? pvd::INCREASED_PITCH_DIFF_TIME : pvd::DECREASED_PITCH_DIFF_TIME;
    };

    auto addPrebendOrHold = [&importedInfo](const PitchValue& start, bool noteTiedBack) {
        importedInfo.timeOffsetFromStart = start.time;
        importedInfo.pitchOffsetFromStart = start.pitch;
        if (start.pitch > 0) {
            importedInfo.type = (noteTiedBack ? BendType::TIED_TO_PREVIOUS_NOTE : BendType::PREBEND);
        }
    };

    for (size_t i = 0; i < pitchValues.size() - 1; i++) {
        currentPitchDiff = pitchDiff(pitchValues[i], pitchValues[i + 1]);
        if (currentPitchDiff == pvd::SAME_PITCH_AND_TIME) {
            continue;
        }

        if (currentPitchDiff == previousPitchDiff && !importedInfo.segments.empty()) {
            BendSegment& lastSegment = importedInfo.segments.back();
            lastSegment.middleTime = pitchValues[i].time;
            lastSegment.endTime = pitchValues[i + 1].time;
            lastSegment.endPitch = pitchValues[i + 1].pitch;
            continue;
        }

        switch (currentPitchDiff) {
        case pvd::NONE:
        case pvd::SAME_PITCH_AND_TIME:
            break;

        case pvd::SAME_PITCH_DIFF_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            } else {
                BendSegment& lastSegment = importedInfo.segments.back();
                lastSegment.middleTime = lastSegment.endTime;
                lastSegment.endTime = pitchValues[i + 1].time;
            }

            break;
        }

        case pvd::INCREASED_PITCH_SAME_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            }

            break;
        }

        case pvd::INCREASED_PITCH_DIFF_TIME:
        case pvd::DECREASED_PITCH_DIFF_TIME:
        {
            if (importedInfo.segments.empty()) {
                addPrebendOrHold(pitchValues[i], note->tieBack());
            }

            BendSegment newSegment;
            newSegment.startPitch = pitchValues[i].pitch;
            newSegment.endPitch = pitchValues[i + 1].pitch;
            newSegment.startTime = pitchValues[i].time;
            newSegment.middleTime = pitchValues[i + 1].time;
            newSegment.endTime = pitchValues[i + 1].time;
            importedInfo.segments.push_back(newSegment);
            break;
        }
        }

        previousPitchDiff = currentPitchDiff;
    }

    importedInfo.note = note;
    if (!importedInfo.segments.empty()) {
        importedInfo.segments.back().endTime = PitchValue::MAX_TIME;
    }

    for (auto& seg : importedInfo.segments) {
        if (seg.endTime - seg.middleTime == 1) {
            seg.middleTime = seg.endTime;
        }
    }

    if (importedInfo.segments.size() == 1 && importedInfo.type == BendType::NORMAL_BEND
        && importedInfo.segments.front().pitchDiff() == 25) {
        importedInfo.type = BendType::SLIGHT_BEND;
    }

    return importedInfo;
}
} // namespace mu::iex::guitarpro
