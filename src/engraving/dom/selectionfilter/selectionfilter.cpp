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

SelectionFilter::SelectionFilter(SelectionFilterType type)
    : m_filteredTypes(static_cast<int>(type))
{
}

bool SelectionFilter::isFiltered(SelectionFilterType type) const
{
    if (type == SelectionFilterType::NONE || type == SelectionFilterType::ALL) {
        return m_filteredTypes == static_cast<unsigned int>(type);
    }

    return m_filteredTypes & static_cast<unsigned int>(type);
}

void SelectionFilter::setFiltered(SelectionFilterType type, bool filtered)
{
    if (type == SelectionFilterType::NONE) {
        setFiltered(SelectionFilterType::ALL, !filtered);
        return;
    }

    if (filtered) {
        m_filteredTypes |= static_cast<unsigned int>(type);
    } else {
        m_filteredTypes &= ~static_cast<unsigned int>(type);
    }
}

bool SelectionFilter::canSelect(const EngravingItem* e) const
{
    switch (e->type()) {
    case ElementType::DYNAMIC:
        return isFiltered(SelectionFilterType::DYNAMIC);
    case ElementType::HAIRPIN:
    case ElementType::HAIRPIN_SEGMENT:
        return isFiltered(SelectionFilterType::HAIRPIN);
    case ElementType::ARTICULATION:
    case ElementType::VIBRATO:
    case ElementType::VIBRATO_SEGMENT:
    case ElementType::FERMATA:
        return isFiltered(SelectionFilterType::ARTICULATION);
    case ElementType::ORNAMENT:
    case ElementType::TRILL:
    case ElementType::TRILL_SEGMENT:
        return isFiltered(SelectionFilterType::ORNAMENT);
    case ElementType::LYRICS:
    case ElementType::LYRICSLINE:
    case ElementType::LYRICSLINE_SEGMENT:
    case ElementType::PARTIAL_LYRICSLINE:
    case ElementType::PARTIAL_LYRICSLINE_SEGMENT:
        return isFiltered(SelectionFilterType::LYRICS);
    case ElementType::FINGERING:
        return isFiltered(SelectionFilterType::FINGERING);
    case ElementType::HARMONY:
        return isFiltered(SelectionFilterType::CHORD_SYMBOL);
    case ElementType::SLUR:
    case ElementType::SLUR_SEGMENT:
        return isFiltered(SelectionFilterType::SLUR);
    case ElementType::FIGURED_BASS:
        return isFiltered(SelectionFilterType::FIGURED_BASS);
    case ElementType::OTTAVA:
    case ElementType::OTTAVA_SEGMENT:
        return isFiltered(SelectionFilterType::OTTAVA);
    case ElementType::PEDAL:
    case ElementType::PEDAL_SEGMENT:
        return isFiltered(SelectionFilterType::PEDAL_LINE);
    case ElementType::ARPEGGIO:
        return isFiltered(SelectionFilterType::ARPEGGIO);
    case ElementType::GLISSANDO:
    case ElementType::GLISSANDO_SEGMENT:
        return isFiltered(SelectionFilterType::GLISSANDO);
    case ElementType::FRET_DIAGRAM:
        return isFiltered(SelectionFilterType::FRET_DIAGRAM);
    case ElementType::BREATH:
        return isFiltered(SelectionFilterType::BREATH);
    case ElementType::TREMOLO_SINGLECHORD:
    case ElementType::TREMOLO_TWOCHORD:
        return isFiltered(SelectionFilterType::TREMOLO);
    default: break;
    }

    // Special cases...
    if (e->isTextBase()) { // only TEXT, INSTRCHANGE and STAFFTEXT are caught here, rest are system thus not in selection
        return isFiltered(SelectionFilterType::OTHER_TEXT);
    }

    if (e->isSLine()) { // NoteLine, Volta
        return isFiltered(SelectionFilterType::OTHER_LINE);
    }

    if (e->isChord() && toChord(e)->isGrace()) {
        return isFiltered(SelectionFilterType::GRACE_NOTE);
    }

    return true;
}

bool SelectionFilter::canSelectVoice(track_idx_t track) const
{
    voice_idx_t voice = track % VOICES;
    switch (voice) {
    case 0:
        return isFiltered(SelectionFilterType::FIRST_VOICE);
    case 1:
        return isFiltered(SelectionFilterType::SECOND_VOICE);
    case 2:
        return isFiltered(SelectionFilterType::THIRD_VOICE);
    case 3:
        return isFiltered(SelectionFilterType::FOURTH_VOICE);
    }
    return true;
}
