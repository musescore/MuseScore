/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "global/realfn.h"
#include "mpe/mpetypes.h"

#include "../../dom/score.h"
#include "../../dom/repeatlist.h"
#include "../../dom/tempo.h"

#include "../../types/constants.h"

namespace mu::engraving {
inline muse::mpe::timestamp_t timestampFromTicks(const Score* score, const int tick)
{
    return score->repeatList().utick2utime(tick) * 1000000;
}

inline int timestampToTick(const Score* score, const muse::mpe::timestamp_t timestamp)
{
    return score->repeatList().utime2utick(timestamp / 1000000.f);
}

// Returns the section-break pause at `tick` in microseconds, or 0.
//
// Section-break pauses should fire once — after the final repeat iteration —
// but tick2time() includes the pause at every segment boundary.  To avoid
// double-counting, we only return the pause for notes whose segment is the
// one that carries it (i.e. the last segment in the repeat).
//
// Non-section-break pauses (caesuras, Fermata, etc.) are returned unconditionally.
inline muse::mpe::duration_t pauseUs(const Score* score, const int tick, const int noteStartTick,
                                     const int tickPositionOffset)
{
    double secs = score->tempomap()->pauseSecs(tick);
    if (muse::RealIsNull(secs)) {
        return 0;
    }

    // Determine whether `tick` is a section-break boundary.
    bool isSectionBreak = false;
    for (const RepeatSegment* s : score->repeatList()) {
        if (s->pause > 0.0 && s->tick + s->len() == tick) {
            isSectionBreak = true;
            break;
        }
    }

    if (!isSectionBreak) {
        return secs * 1000000;
    }

    // Section break: only subtract the pause if the note lives in the segment
    // that carries it.  Other segments had the pause already removed from their
    // time delta in updateTempo(), so subtracting again would double-count.
    int utick = noteStartTick + tickPositionOffset;
    for (const RepeatSegment* s : score->repeatList()) {
        if (utick >= s->utick && utick < s->utick + s->len()) {
            return s->pause > 0.0 ? secs * 1000000 : 0;
        }
    }

    return 0;
}

inline muse::mpe::duration_t durationFromStartAndEndTick(const Score* score, const int startTick, const int endTick,
                                                         const int tickPositionOffset)
{
    muse::mpe::timestamp_t startTimestamp = timestampFromTicks(score, startTick + tickPositionOffset);
    muse::mpe::timestamp_t endTimestamp = timestampFromTicks(score, endTick + tickPositionOffset);
    muse::mpe::duration_t pause = pauseUs(score, endTick, startTick, tickPositionOffset);

    return endTimestamp - startTimestamp - pause;
}

inline muse::mpe::duration_t durationFromStartAndTicks(const Score* score, const int startTick, const int durationTicks,
                                                       const int tickPositionOffset)
{
    return durationFromStartAndEndTick(score, startTick, startTick + durationTicks, tickPositionOffset);
}

inline muse::mpe::TimestampAndDuration timestampAndDurationFromStartAndDurationTicks(const Score* score,
                                                                                     const int startTick, const int durationTicks,
                                                                                     const int tickPositionOffset)
{
    int startTickWithOffset = startTick + tickPositionOffset;
    muse::mpe::timestamp_t startTimestamp = timestampFromTicks(score, startTickWithOffset);
    muse::mpe::timestamp_t endTimestamp = timestampFromTicks(score, startTickWithOffset + durationTicks);
    muse::mpe::duration_t pause = pauseUs(score, startTick + durationTicks, startTick, tickPositionOffset);
    muse::mpe::duration_t duration = endTimestamp - startTimestamp - pause;

    return { startTimestamp, duration };
}

inline muse::mpe::duration_t durationFromTempoAndTicks(const double beatsPerSecond, const int durationTicks,
                                                       const int ticksPerBeat = Constants::DIVISION)
{
    float beatsNumber = static_cast<float>(durationTicks) / static_cast<float>(ticksPerBeat);

    return (beatsNumber / beatsPerSecond) * 1000000;
}

inline int ticksFromTempoAndDuration(const double beatsPerSecond, const muse::mpe::duration_t duration,
                                     const int ticksPerBeat = Constants::DIVISION)
{
    return (duration * beatsPerSecond * ticksPerBeat) / 1000000;
}

static constexpr int CROTCHET_TICKS = Constants::DIVISION;
static constexpr int QUAVER_TICKS = Constants::DIVISION / 2;
static constexpr int SEMIQUAVER_TICKS = Constants::DIVISION / 4;
static constexpr int DEMISEMIQUAVER_TICKS = Constants::DIVISION / 8;

static const double PRESTISSIMO_BPS_BOUND = muse::RealRound(200 /*bpm*/ / 60.f /*secs*/, 2);
static const double PRESTO_BPS_BOUND = muse::RealRound(168 /*bpm*/ / 60.f /*secs*/, 2);
static const double ALLEGRO_BPS_BOUND = muse::RealRound(120 /*bpm*/ / 60.f /*secs*/, 2);
static const double MODERATO_BPS_BOUND = muse::RealRound(108 /*bpm*/ / 60.f /*secs*/, 2);
static const double ANDANTE_BPS_BOUND = muse::RealRound(76 /*bpm*/ / 60.f /*secs*/, 2);
static const double ADAGIO_BPS_BOUND = muse::RealRound(66 /*bpm*/ / 60.f /*secs*/, 2);
static const double LENTO_BPS_BOUND = muse::RealRound(40 /*bpm*/ / 60.f /*secs*/, 2);
static const double GRAVE_BPS_BOUND = muse::RealRound(20 /*bpm*/ / 60.f /*secs*/, 2);
}
