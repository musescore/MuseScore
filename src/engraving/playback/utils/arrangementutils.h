/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#ifndef MU_ENGRAVING_ARRANGEMENTUTILS_H
#define MU_ENGRAVING_ARRANGEMENTUTILS_H

#include "global/realfn.h"
#include "mpe/mpetypes.h"

#include "dom/score.h"
#include "dom/repeatlist.h"
#include "types/constants.h"

namespace mu::engraving {
inline mpe::timestamp_t timestampFromTicks(const Score* score, const int tick)
{
    return score->repeatList().utick2utime(tick) * 1000000;
}

inline int timestampToTick(const Score* score, const mpe::timestamp_t timestamp)
{
    return score->repeatList().utime2utick(timestamp / 1000000.f);
}

inline mpe::duration_t durationFromTicks(const double beatsPerSecond, const int durationTicks, const int ticksPerBeat = Constants::DIVISION)
{
    float beatsNumber = static_cast<float>(durationTicks) / static_cast<float>(ticksPerBeat);

    return (beatsNumber / beatsPerSecond) * 1000000;
}

static constexpr int CROTCHET_TICKS = Constants::DIVISION;
static constexpr int QUAVER_TICKS = Constants::DIVISION / 2;
static constexpr int SEMIQUAVER_TICKS = Constants::DIVISION / 4;
static constexpr int DEMISEMIQUAVER_TICKS = Constants::DIVISION / 8;

static const double PRESTISSIMO_BPS_BOUND = RealRound(200 /*bpm*/ / 60.f /*secs*/, 2);
static const double PRESTO_BPS_BOUND = RealRound(168 /*bpm*/ / 60.f /*secs*/, 2);
static const double ALLEGRO_BPS_BOUND = RealRound(120 /*bpm*/ / 60.f /*secs*/, 2);
static const double MODERATO_BPS_BOUND = RealRound(108 /*bpm*/ / 60.f /*secs*/, 2);
static const double ANDANTE_BPS_BOUND = RealRound(76 /*bpm*/ / 60.f /*secs*/, 2);
static const double ADAGIO_BPS_BOUND = RealRound(66 /*bpm*/ / 60.f /*secs*/, 2);
static const double LENTO_BPS_BOUND = RealRound(40 /*bpm*/ / 60.f /*secs*/, 2);
static const double GRAVE_BPS_BOUND = RealRound(20 /*bpm*/ / 60.f /*secs*/, 2);
}

#endif // MU_ENGRAVING_ARRANGEMENTUTILS_H
