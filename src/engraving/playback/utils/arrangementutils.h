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

#include "mpe/mpetypes.h"

#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "libmscore/tempo.h"
#include "types/constants.h"

namespace mu::engraving {
inline mpe::timestamp_t timestampFromTicks(const Ms::Score* score, const int tick)
{
    return score->repeatList().utick2utime(tick) * 1000;
}

inline mpe::duration_t durationFromTicks(const qreal beatsPerSecond, const int durationTicks, const int ticksPerBeat = Constants::division)
{
    float beatsNumber = durationTicks / static_cast<float>(ticksPerBeat);

    return (beatsNumber / beatsPerSecond) * 1000;
}

static constexpr int CROTCHET_TICKS = Ms::Constant::division;
static constexpr int SEMIQUAVER_TICKS = Ms::Constant::division / 4;
static constexpr int DEMISEMIQUAVER_TICKS = Ms::Constant::division / 8;

static constexpr qreal PRESTISSIMO_BPS_BOUND = 200 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal PRESTO_BPS_BOUND = 168 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal ALEGRO_BPS_BOUND = 120 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal MODERATO_BPS_BOUND = 108 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal ANDANTE_BPS_BOUND = 76 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal ADAGIO_BPS_BOUND = 66 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal LENTO_BPS_BOUND = 40 /*bpm*/ / 60.f /*secs*/;
static constexpr qreal GRAVE_BPS_BOUND = 20 /*bpm*/ / 60.f /*secs*/;
}

#endif // MU_ENGRAVING_ARRANGEMENTUTILS_H
