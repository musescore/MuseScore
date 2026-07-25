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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "engraving/dom/timesig.h"
#include "engraving/types/types.h"

#include "instrumentstypes.h"
#include "tempo.h"

namespace mu::notation {
struct ScoreCreateOptions
{
    bool withTempo = false;
    Tempo tempo;

    engraving::Fraction globalTimesig;
    engraving::TimeSigType timesigType = engraving::TimeSigType::NORMAL;

    engraving::Key key = engraving::Key::C;

    int totalMeasures = 0;

    bool withPickupMeasure = false;
    engraving::Fraction pickupTimesig;

    PartInstrumentList parts;
    engraving::ScoreOrder order;
};
}
