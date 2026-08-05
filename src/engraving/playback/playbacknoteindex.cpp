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

#include "playbacknoteindex.h"

#include <algorithm>
#include <iterator>
#include <limits>

#include "realfn.h"

using namespace mu::engraving;
using namespace muse::mpe;

void PlaybackNoteIndex::clear()
{
    for (std::vector<Interval>& intervals : m_intervals) {
        intervals.clear();
    }
}

void PlaybackNoteIndex::add(const PlaybackEventsMap& events)
{
    constexpr timestamp_t MAX_TIMESTAMP = std::numeric_limits<timestamp_t>::max();

    for (const auto& [eventTimestamp, eventList] : events) {
        (void)eventTimestamp;

        for (const PlaybackEvent& event : eventList) {
            const muse::mpe::NoteEvent* noteEvent = std::get_if<muse::mpe::NoteEvent>(&event);
            if (!noteEvent) {
                continue;
            }

            const ArrangementContext& arrangement = noteEvent->arrangementCtx();
            if (arrangement.actualDuration <= 0) {
                continue;
            }

            const float pitchSteps = ZERO_PITCH_LEVEL_MIDI_EQUIVALENT
                                     + noteEvent->pitchCtx().nominalPitchLevel / static_cast<float>(PITCH_LEVEL_STEP);
            //! NOTE: The keyboard cannot represent tuning offsets, so show the nearest equal-tempered key,
            //! matching MuseSampler's pitch carrier selection.
            const int pitch = muse::RealRound(pitchSteps, 0);
            if (pitch < 0 || pitch > 127) {
                continue;
            }

            const bool openEnded = arrangement.actualDuration == INFINITE_DURATION;
            timestamp_t end = MAX_TIMESTAMP;
            if (!openEnded && arrangement.actualTimestamp <= MAX_TIMESTAMP - arrangement.actualDuration) {
                //! NOTE: duration is positive here, so a negative timestamp can be added safely.
                //! Keep finite overflowing intervals saturated at MAX_TIMESTAMP.
                end = arrangement.actualTimestamp + arrangement.actualDuration;
            }

            if (!openEnded && end <= arrangement.actualTimestamp) {
                continue;
            }

            m_intervals[static_cast<size_t>(pitch)].push_back({
                arrangement.actualTimestamp,
                end,
                openEnded
            });
        }
    }
}

void PlaybackNoteIndex::finalize()
{
    for (std::vector<Interval>& intervals : m_intervals) {
        if (intervals.empty()) {
            continue;
        }

        std::sort(intervals.begin(), intervals.end(), [](const Interval& left, const Interval& right) {
            return left.start < right.start || (left.start == right.start && left.end < right.end);
        });

        size_t mergedSize = 1;
        for (size_t i = 1; i < intervals.size(); ++i) {
            Interval& merged = intervals[mergedSize - 1];
            const Interval& current = intervals[i];

            if (merged.openEnded || current.start <= merged.end) {
                merged.end = std::max(merged.end, current.end);
                merged.openEnded = merged.openEnded || current.openEnded;
                continue;
            }

            intervals[mergedSize++] = current;
        }

        intervals.resize(mergedSize);
    }
}

std::vector<muse::midi::note_idx_t> PlaybackNoteIndex::activePitches(timestamp_t timestamp) const
{
    std::vector<muse::midi::note_idx_t> result;

    for (size_t pitch = 0; pitch < m_intervals.size(); ++pitch) {
        const std::vector<Interval>& intervals = m_intervals[pitch];
        const auto upperBound = std::upper_bound(intervals.cbegin(), intervals.cend(), timestamp,
                                                 [](timestamp_t value, const Interval& interval) {
            return value < interval.start;
        });

        if (upperBound == intervals.cbegin()) {
            continue;
        }

        const Interval& candidate = *std::prev(upperBound);
        if (candidate.openEnded || timestamp < candidate.end) {
            result.push_back(static_cast<muse::midi::note_idx_t>(pitch));
        }
    }

    return result;
}
