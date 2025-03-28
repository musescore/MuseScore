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

#include "elementsselectionfilter.h"

using namespace mu::engraving;

ElementsSelectionFilter::ElementsSelectionFilter(ElementsSelectionFilterTypes type)
    : AbstractSelectionFilter()
{
    m_filteredTypes = static_cast<int>(type);
}

bool ElementsSelectionFilter::isFiltered(const ElementsSelectionFilterTypes& type) const
{
    return AbstractSelectionFilter::isFiltered(static_cast<unsigned int>(type));
}

void ElementsSelectionFilter::setFiltered(const ElementsSelectionFilterTypes& type, bool filtered)
{
    AbstractSelectionFilter::setFiltered(static_cast<unsigned int>(type), filtered);
}

bool ElementsSelectionFilter::canSelect(const EngravingItem* e) const
{
    switch (e->type()) {
    case ElementType::DYNAMIC:
        return isFiltered(ElementsSelectionFilterTypes::DYNAMIC);
    case ElementType::HAIRPIN:
    case ElementType::HAIRPIN_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::HAIRPIN);
    case ElementType::ARTICULATION:
    case ElementType::VIBRATO:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::FERMATA:
        return isFiltered(ElementsSelectionFilterTypes::ARTICULATION);
    case ElementType::ORNAMENT:
    case ElementType::TRILL:
    case ElementType::TRILL_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::ORNAMENT);
    case ElementType::LYRICS:
    case ElementType::LYRICSLINE:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::LYRICS);
    case ElementType::FINGERING:
        return isFiltered(ElementsSelectionFilterTypes::FINGERING);
    case ElementType::HARMONY:
        return isFiltered(ElementsSelectionFilterTypes::CHORD_SYMBOL);
    case ElementType::SLUR:
    case ElementType::SLUR_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::SLUR);
    case ElementType::FIGURED_BASS:
        return isFiltered(ElementsSelectionFilterTypes::FIGURED_BASS);
    case ElementType::OTTAVA:
    case ElementType::OTTAVA_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::OTTAVA);
    case ElementType::PEDAL:
    case ElementType::PEDAL_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::PEDAL_LINE);
    case ElementType::ARPEGGIO:
        return isFiltered(ElementsSelectionFilterTypes::ARPEGGIO);
    case ElementType::GLISSANDO:
    case ElementType::GLISSANDO_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::GLISSANDO);
    case ElementType::FRET_DIAGRAM:
        return isFiltered(ElementsSelectionFilterTypes::FRET_DIAGRAM);
    case ElementType::BREATH:
        return isFiltered(ElementsSelectionFilterTypes::BREATH);
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
        return isFiltered(ElementsSelectionFilterTypes::TREMOLO);
    default: break;
    }

    // Special cases...
    if (e->isTextBase()) { // only TEXT, INSTRCHANGE and STAFFTEXT are caught here, rest are system thus not in selection
        return isFiltered(ElementsSelectionFilterTypes::OTHER_TEXT);
    }

    if (e->isSLine()) { // NoteLine, Volta
        return isFiltered(ElementsSelectionFilterTypes::OTHER_LINE);
    }

    if (e->isChord() && toChord(e)->isGrace()) {
        return isFiltered(ElementsSelectionFilterTypes::GRACE_NOTE);
    }

    return true;
}
