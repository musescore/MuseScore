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

#include "selectionfilter.h"

using namespace mu::engraving;

struct FilterParams {
    unsigned int otherFilter = 0;
    unsigned int noneMask = 0;
    unsigned int allMask = 0;
};

static bool isFiltered_Impl(const unsigned int& currentFilter, const FilterParams& params)
{
    if (params.otherFilter == params.noneMask || params.otherFilter == params.allMask) {
        return currentFilter == params.otherFilter;
    }
    return currentFilter & params.otherFilter;
}

static void setFiltered_Impl(unsigned int& currentFilter, FilterParams& params, bool filtered)
{
    if (params.otherFilter == params.noneMask) {
        params.otherFilter = params.allMask;
        setFiltered_Impl(currentFilter, params, !filtered);
        return;
    }
    filtered ? currentFilter |= params.otherFilter : currentFilter &= ~params.otherFilter;
}

bool SelectionFilter::isFiltered(const SelectionFilterTypesVariant& variant) const
{
    switch (variant.index()) {
    case 0: {
        const VoicesSelectionFilterTypes type = std::get<VoicesSelectionFilterTypes>(variant);
        const FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(VoicesSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(VoicesSelectionFilterTypes::ALL)
        };
        return isFiltered_Impl(m_filteredVoicesTypes, params);
    }
    case 1: {
        const ElementsSelectionFilterTypes type = std::get<ElementsSelectionFilterTypes>(variant);
        const FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::ALL)
        };
        return isFiltered_Impl(m_filteredElementsTypes, params);
    }
    default: UNREACHABLE;
    }

    UNREACHABLE;
    return true;
}

void SelectionFilter::setFiltered(const SelectionFilterTypesVariant& variant, bool filtered)
{
    switch (variant.index()) {
    case 0: {
        const VoicesSelectionFilterTypes type = std::get<VoicesSelectionFilterTypes>(variant);
        FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(VoicesSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(VoicesSelectionFilterTypes::ALL)
        };
        setFiltered_Impl(m_filteredVoicesTypes, params, filtered);
        return;
    }
    case 1: {
        const ElementsSelectionFilterTypes type = std::get<ElementsSelectionFilterTypes>(variant);
        FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::ALL)
        };
        setFiltered_Impl(m_filteredElementsTypes, params, filtered);
        return;
    }
    default: break;
    }

    UNREACHABLE;
}

bool SelectionFilter::canSelect(const EngravingItem* e) const
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
    case ElementType::HAMMER_ON_PULL_OFF:
    case ElementType::HAMMER_ON_PULL_OFF_SEGMENT:
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

bool SelectionFilter::canSelectVoice(track_idx_t track) const
{
    voice_idx_t voice = track2voice(track);
    switch (voice) {
    case 0:
        return isFiltered(VoicesSelectionFilterTypes::FIRST_VOICE);
    case 1:
        return isFiltered(VoicesSelectionFilterTypes::SECOND_VOICE);
    case 2:
        return isFiltered(VoicesSelectionFilterTypes::THIRD_VOICE);
    case 3:
        return isFiltered(VoicesSelectionFilterTypes::FOURTH_VOICE);
    }
    return true;
}
