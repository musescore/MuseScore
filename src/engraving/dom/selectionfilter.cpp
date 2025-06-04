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
#include "tuplet.h"

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
        const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
        const FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(NotesInChordSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(NotesInChordSelectionFilterTypes::ALL)
        };
        return isFiltered_Impl(m_filteredNotesInChordTypes, params);
    }
    case 2: {
        const ElementsSelectionFilterTypes type = std::get<ElementsSelectionFilterTypes>(variant);
        const FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(ElementsSelectionFilterTypes::ALL)
        };
        return isFiltered_Impl(m_filteredElementsTypes, params);
    }
    default: break;
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
        const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
        FilterParams params {
            /*otherFilter*/ static_cast<unsigned int>(type),
            /*noneMask*/ static_cast<unsigned int>(NotesInChordSelectionFilterTypes::NONE),
            /*allMask*/ static_cast<unsigned int>(NotesInChordSelectionFilterTypes::ALL)
        };
        setFiltered_Impl(m_filteredNotesInChordTypes, params, filtered);
        return;
    }
    case 2: {
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
    IF_ASSERT_FAILED(!e->isNote() && !e->isTuplet()) {
        LOGE() << "Using canSelect on a Note or Tuplet - use canSelectNoteIdx or canSelectTuplet instead";
        return true;
    }

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
    case ElementType::TIE:
    case ElementType::TIE_SEGMENT:
        return isFiltered(ElementsSelectionFilterTypes::TIE);
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
    if (e->isTextBase()) {
        return isFiltered(ElementsSelectionFilterTypes::OTHER_TEXT);
    }

    if (e->isSLine() || e->isSLineSegment()) { // NoteLine, Volta
        return isFiltered(ElementsSelectionFilterTypes::OTHER_LINE);
    }

    if (e->isChord() && toChord(e)->isGrace()) {
        return isFiltered(ElementsSelectionFilterTypes::GRACE_NOTE);
    }

    return true;
}

bool SelectionFilter::canSelectNoteIdx(size_t noteIdx, size_t totalNotesInChord, bool selectionContainsMultiNoteChords) const
{
    if (totalNotesInChord == 1) {
        //! NOTE: Always include single notes when the selection consists solely of single notes...
        return m_includeSingleNotes || !selectionContainsMultiNoteChords;
    }

    NotesInChordSelectionFilterTypes type = NotesInChordSelectionFilterTypes::NONE;
    switch (noteIdx) {
    case 0: type = NotesInChordSelectionFilterTypes::BOTTOM_NOTE;
        break;
    case 1: type = NotesInChordSelectionFilterTypes::SECOND_NOTE;
        break;
    case 2: type = NotesInChordSelectionFilterTypes::THIRD_NOTE;
        break;
    case 3: type = NotesInChordSelectionFilterTypes::FOURTH_NOTE;
        break;
    case 4: type = NotesInChordSelectionFilterTypes::FIFTH_NOTE;
        break;
    case 5: type = NotesInChordSelectionFilterTypes::SIXTH_NOTE;
        break;
    case 6: type = NotesInChordSelectionFilterTypes::SEVENTH_NOTE;
        break;
    default: break;
    }

    //! NOTE: Everything above "normal" we handle as top notes...
    const bool idxIsNormal = noteIdx < NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES - 1;
    if (noteIdx == totalNotesInChord - 1 || !idxIsNormal) {
        return isFiltered(NotesInChordSelectionFilterTypes::TOP_NOTE) || (idxIsNormal && isFiltered(type));
    }

    IF_ASSERT_FAILED(type != NotesInChordSelectionFilterTypes::NONE) {
        return true;
    }

    return isFiltered(type);
}

bool SelectionFilter::canSelectTuplet(const Tuplet* tuplet, const Fraction& selectionRangeStart, const Fraction& selectionRangeEnd,
                                      bool selectionContainsMultiNoteChords) const
{
    // Tuplets are selectable if all of their contained elements are selectable...
    for (const DurationElement* element : tuplet->elements()) {
        if (element->tick() < selectionRangeStart || element->tick() >= selectionRangeEnd) {
            return false;
        }
        switch (element->type()) {
        case ElementType::CHORD: {
            const std::vector<Note*> notes = toChord(element)->notes();
            for (size_t noteIdx = 0; noteIdx < notes.size(); ++noteIdx) {
                if (!canSelectNoteIdx(noteIdx, notes.size(), selectionContainsMultiNoteChords)) {
                    return false;
                }
            }
            break;
        }
        case ElementType::REST:
            if (!canSelect(element)) {
                return false;
            }
            break;
        case ElementType::TUPLET: {
            // Recursive call...
            if (!canSelectTuplet(toTuplet(element), selectionRangeStart, selectionRangeEnd, selectionContainsMultiNoteChords)) {
                return false;
            }
            break;
        }
        default: {
            UNREACHABLE;
            break;
        }
        }
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
