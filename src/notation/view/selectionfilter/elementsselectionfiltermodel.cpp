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

#include "elementsselectionfiltermodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;
using namespace mu::engraving;

ElementsSelectionFilterModel::ElementsSelectionFilterModel(QObject* parent)
    : AbstractSelectionFilterModel(parent)
{
}

void ElementsSelectionFilterModel::loadTypes()
{
    m_types << ElementsSelectionFilterTypes::ALL;

    for (size_t i = 0; i < NUM_ELEMENTS_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<ElementsSelectionFilterTypes>(1 << i);
    }
}

QString ElementsSelectionFilterModel::titleForType(const SelectionFilterTypesVariant& variant) const
{
    const ElementsSelectionFilterTypes type = std::get<ElementsSelectionFilterTypes>(variant);

    switch (type) {
    case ElementsSelectionFilterTypes::ALL:
        return muse::qtrc("notation", "All");
    case ElementsSelectionFilterTypes::DYNAMIC:
        return muse::qtrc("notation", "Dynamics");
    case ElementsSelectionFilterTypes::HAIRPIN:
        return muse::qtrc("notation", "Hairpins");
    case ElementsSelectionFilterTypes::FINGERING:
        return muse::qtrc("notation", "Fingerings");
    case ElementsSelectionFilterTypes::LYRICS:
        return muse::qtrc("notation", "Lyrics");
    case ElementsSelectionFilterTypes::CHORD_SYMBOL:
        return muse::qtrc("notation", "Chord symbols");
    case ElementsSelectionFilterTypes::OTHER_TEXT:
        return muse::qtrc("notation", "Other text");
    case ElementsSelectionFilterTypes::ARTICULATION:
        return muse::qtrc("notation", "Articulations");
    case ElementsSelectionFilterTypes::ORNAMENT:
        return muse::qtrc("notation", "Ornaments");
    case ElementsSelectionFilterTypes::SLUR:
        return muse::qtrc("notation", "Slurs");
    case ElementsSelectionFilterTypes::FIGURED_BASS:
        return muse::qtrc("notation", "Figured bass");
    case ElementsSelectionFilterTypes::OTTAVA:
        return muse::qtrc("notation", "Ottavas");
    case ElementsSelectionFilterTypes::PEDAL_LINE:
        return muse::qtrc("notation", "Pedal lines");
    case ElementsSelectionFilterTypes::OTHER_LINE:
        return muse::qtrc("notation", "Other lines");
    case ElementsSelectionFilterTypes::ARPEGGIO:
        return muse::qtrc("notation", "Arpeggios");
    case ElementsSelectionFilterTypes::GLISSANDO:
        return muse::qtrc("notation", "Glissandos");
    case ElementsSelectionFilterTypes::FRET_DIAGRAM:
        return muse::qtrc("notation", "Fretboard diagrams");
    case ElementsSelectionFilterTypes::BREATH:
        return muse::qtrc("notation", "Breath marks");
    case ElementsSelectionFilterTypes::TREMOLO:
        return muse::qtrc("notation", "Tremolos");
    case ElementsSelectionFilterTypes::GRACE_NOTE:
        return muse::qtrc("notation", "Grace notes");
    case ElementsSelectionFilterTypes::NONE:
        break;
    }

    return {};
}
