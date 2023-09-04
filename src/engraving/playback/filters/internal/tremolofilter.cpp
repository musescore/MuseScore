/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "tremolofilter.h"

#include "dom/chord.h"
#include "dom/tremolo.h"

using namespace mu::engraving;

bool TremoloFilter::isPlayable(const EngravingItem* item, const RenderingContext& /*ctx*/)
{
    if (!item->isChord() && !item->isNote()) {
        return false;
    }

    if (item->isChord()) {
        const Chord* chord = toChord(item);
        Tremolo* tremolo = chord->tremolo();

        if (!tremolo) {
            return true;
        }

        if (!tremolo->twoNotes()) {
            return true;
        }

        return tremolo->chord1() == chord;
    }

    return true;
}
