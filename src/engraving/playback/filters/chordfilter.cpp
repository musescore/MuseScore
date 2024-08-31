/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#include "chordfilter.h"

#include "dom/chord.h"
#include "dom/note.h"
#include "dom/tie.h"

#include "internal/tremolofilter.h"

using namespace mu::engraving;
using namespace muse;

bool ChordFilter::isPlayable(const EngravingItem* item, const RenderingContext& ctx)
{
    return TremoloFilter::isItemPlayable(item, ctx);
}

static bool containsTieEnd(const Chord* chord)
{
    for (const Note* note : chord->notes()) {
        if ((note->tieBack() && note->tieBack()->playSpanner()) && !(note->tieFor() && note->tieFor()->playSpanner())) {
            return true;
        }
    }

    return false;
}

static bool containsTieStart(const Chord* chord)
{
    for (const Note* note : chord->notes()) {
        if (!(note->tieBack() && note->tieBack()->playSpanner()) && (note->tieFor() && note->tieFor()->playSpanner())) {
            return true;
        }
    }

    return false;
}

void ChordFilter::validateArticulations(const EngravingItem* item, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item && item->isChord()) {
        return;
    }

    static const mpe::ArticulationTypeSet LAST_TIED_NOTE_ALLOWED_TYPES = {
        mpe::ArticulationType::Staccato,
        mpe::ArticulationType::Staccatissimo,
        mpe::ArticulationType::Tenuto
    };

    if (result.empty()) {
        return;
    }

    if (!result.containsAnyOf(LAST_TIED_NOTE_ALLOWED_TYPES.cbegin(),
                              LAST_TIED_NOTE_ALLOWED_TYPES.cend())) {
        return;
    }

    const Chord* chord = toChord(item);

    if (containsTieEnd(chord)) {
        mpe::ArticulationMap filteredMap;

        for (const mpe::ArticulationType type : LAST_TIED_NOTE_ALLOWED_TYPES) {
            auto search = result.find(type);

            if (search != result.cend()) {
                filteredMap.emplace(search->first, search->second);
            }
        }

        result = filteredMap;
        return;
    }

    if (containsTieStart(chord)) {
        for (const mpe::ArticulationType type : LAST_TIED_NOTE_ALLOWED_TYPES) {
            auto search = result.find(type);

            if (search != result.cend()) {
                result.erase(type);
            }
        }
        return;
    }
}
