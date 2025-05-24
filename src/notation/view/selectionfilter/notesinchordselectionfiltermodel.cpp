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

#include "notesinchordselectionfiltermodel.h"

#include "log.h"
#include "translation.h"

using namespace mu::notation;
using namespace mu::engraving;

NotesInChordSelectionFilterModel::NotesInChordSelectionFilterModel(QObject* parent)
    : AbstractSelectionFilterModel(parent)
{
}

bool NotesInChordSelectionFilterModel::enabled() const
{
    const INotationSelectionPtr selection = currentNotationInteraction() ? currentNotationInteraction()->selection() : nullptr;
    const INotationSelectionRangePtr range = selection ? selection->range() : nullptr;
    if (!range) {
        return false;
    }

    // Section should be enabled whenever a Chord exists within the given selection range...
    for (staff_idx_t staffIdx = range->startStaffIndex(); staffIdx < range->endStaffIndex(); ++staffIdx) {
        Segment* segment = range->rangeStartSegment();
        while (segment && segment != range->rangeEndSegment()) {
            for (voice_idx_t voiceIdx = 0; voiceIdx < VOICES; ++voiceIdx) {
                const EngravingItem* e = segment->element(staff2track(staffIdx, voiceIdx));
                if (!e || !e->isChord()) {
                    continue;
                }
                if (toChord(e)->notes().size() > 1) {
                    return true;
                }
            }
            segment = segment->next1();
        }
    }

    return false;
}

void NotesInChordSelectionFilterModel::loadTypes()
{
    for (size_t i = 0; i < NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<NotesInChordSelectionFilterTypes>(1 << i);
    }
}

bool NotesInChordSelectionFilterModel::isFiltered(const SelectionFilterTypesVariant& variant) const
{
    const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
    if (type == NotesInChordSelectionFilterTypes::TOP_NOTE) {
        // TOP_NOTE is considered filtered if the associated index is filtered, OR if TOP_NOTE itself is filtered...
        const NotesInChordSelectionFilterTypes indexType = typeForNoteIdx(m_topNoteIdx);
        if (indexType != NotesInChordSelectionFilterTypes::NONE) {
            return AbstractSelectionFilterModel::isFiltered(type) || AbstractSelectionFilterModel::isFiltered(indexType);
        }
    }
    return AbstractSelectionFilterModel::isFiltered(variant);
}

void NotesInChordSelectionFilterModel::setFiltered(const SelectionFilterTypesVariant& variant, bool filtered, bool)
{
    if (!multipleIndexesSelected() && !filtered) {
        // Attempting to de-select when only one index is selected should reset the filter to ALL...
        AbstractSelectionFilterModel::setFiltered(NotesInChordSelectionFilterTypes::ALL, true);

        IF_ASSERT_FAILED(!m_isDefault) {
            return;
        }

        m_isDefault = true;
        emit isDefaultChanged();

        return;
    }

    // First edit should force other filter flags to false...
    AbstractSelectionFilterModel::setFiltered(variant, filtered || m_isDefault, /*forceOthersToFalse*/ m_isDefault);

    const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
    const bool topNoteIsNormal = m_topNoteIdx < NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES - 1;
    if (type == NotesInChordSelectionFilterTypes::TOP_NOTE && topNoteIsNormal) {
        // Toggling TOP_NOTE should also toggle the flag for the associated index...
        const NotesInChordSelectionFilterTypes indexType = typeForNoteIdx(m_topNoteIdx);
        AbstractSelectionFilterModel::setFiltered(indexType, filtered);
    }

    if (m_isDefault) {
        m_isDefault = false;
        emit isDefaultChanged();
    }
}

bool NotesInChordSelectionFilterModel::isAllowed(const SelectionFilterTypesVariant& variant) const
{
    const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
    return noteIdxForType(type) < m_topNoteIdx || type == NotesInChordSelectionFilterTypes::TOP_NOTE;
}

bool NotesInChordSelectionFilterModel::includeSingleNotes() const
{
    return currentNotationSelectionFilter() ? currentNotationSelectionFilter()->includeSingleNotes() : false;
}

void NotesInChordSelectionFilterModel::setIncludeSingleNotes(bool include)
{
    INotationSelectionFilterPtr filter = currentNotationSelectionFilter();
    if (!filter || filter->includeSingleNotes() == include) {
        return;
    }
    filter->setIncludeSingleNotes(include);
    emit includeSingleNotesChanged();
}

bool NotesInChordSelectionFilterModel::multipleIndexesSelected() const
{
    size_t totalSelected = 0;
    for (qsizetype index = 0; index < m_types.size(); ++index) {
        const SelectionFilterTypesVariant type = m_types.at(index);
        if (!isAllowed(type) || !isFiltered(type)) {
            continue;
        }
        ++totalSelected;
        if (totalSelected > 1) {
            return true;
        }
    }
    return false;
}

QString NotesInChordSelectionFilterModel::titleForType(const SelectionFilterTypesVariant& variant) const
{
    const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);

    switch (type) {
    case NotesInChordSelectionFilterTypes::ALL:
        return muse::qtrc("notation", "All");
    case NotesInChordSelectionFilterTypes::TOP_NOTE:
        return muse::qtrc("notation", "Top note");
    case NotesInChordSelectionFilterTypes::SEVENTH_NOTE:
        return muse::qtrc("notation", "Seventh note");
    case NotesInChordSelectionFilterTypes::SIXTH_NOTE:
        return muse::qtrc("notation", "Sixth note");
    case NotesInChordSelectionFilterTypes::FIFTH_NOTE:
        return muse::qtrc("notation", "Fifth note");
    case NotesInChordSelectionFilterTypes::FOURTH_NOTE:
        return muse::qtrc("notation", "Fourth note");
    case NotesInChordSelectionFilterTypes::THIRD_NOTE:
        return muse::qtrc("notation", "Third note");
    case NotesInChordSelectionFilterTypes::SECOND_NOTE:
        return muse::qtrc("notation", "Second note");
    case NotesInChordSelectionFilterTypes::BOTTOM_NOTE:
        return muse::qtrc("notation", "Bottom note");
    case NotesInChordSelectionFilterTypes::NONE:
        break;
    }

    return {};
}

size_t NotesInChordSelectionFilterModel::noteIdxForType(const NotesInChordSelectionFilterTypes& type) const
{
    switch (type) {
    case NotesInChordSelectionFilterTypes::SEVENTH_NOTE: return 6;
    case NotesInChordSelectionFilterTypes::SIXTH_NOTE: return 5;
    case NotesInChordSelectionFilterTypes::FIFTH_NOTE: return 4;
    case NotesInChordSelectionFilterTypes::FOURTH_NOTE: return 3;
    case NotesInChordSelectionFilterTypes::THIRD_NOTE: return 2;
    case NotesInChordSelectionFilterTypes::SECOND_NOTE: return 1;
    case NotesInChordSelectionFilterTypes::BOTTOM_NOTE: return 0;
    case NotesInChordSelectionFilterTypes::TOP_NOTE:
    case NotesInChordSelectionFilterTypes::ALL:
    case NotesInChordSelectionFilterTypes::NONE:
        break;
    }
    return muse::nidx;
}

NotesInChordSelectionFilterTypes NotesInChordSelectionFilterModel::typeForNoteIdx(size_t noteIdx) const
{
    // use m_types.at row?
    switch (noteIdx) {
    case 6: return NotesInChordSelectionFilterTypes::SEVENTH_NOTE;
    case 5: return NotesInChordSelectionFilterTypes::SIXTH_NOTE;
    case 4: return NotesInChordSelectionFilterTypes::FIFTH_NOTE;
    case 3: return NotesInChordSelectionFilterTypes::FOURTH_NOTE;
    case 2: return NotesInChordSelectionFilterTypes::THIRD_NOTE;
    case 1: return NotesInChordSelectionFilterTypes::SECOND_NOTE;
    case 0: return NotesInChordSelectionFilterTypes::BOTTOM_NOTE;
    default:
        break;
    }

    return NotesInChordSelectionFilterTypes::NONE;
}

void NotesInChordSelectionFilterModel::updateTopNoteIdx()
{
    const INotationSelectionPtr selection = currentNotationInteraction() ? currentNotationInteraction()->selection() : nullptr;
    if (!selection) {
        m_topNoteIdx = muse::nidx;
        return;
    }

    m_topNoteIdx = muse::nidx;

    QSet<const Chord*> scannedChords;
    for (const Note* note : selection->notes()) {
        const Chord* chord = note->chord();
        if (scannedChords.contains(chord)) {
            continue;
        }
        const size_t currIdx = chord->notes().size() - 1;
        if (m_topNoteIdx == muse::nidx || currIdx > m_topNoteIdx) {
            m_topNoteIdx = currIdx;
        }
        scannedChords.insert(chord);
    }
}

void NotesInChordSelectionFilterModel::onSelectionChanged()
{
    updateTopNoteIdx();
    emit dataChanged(this->index(0), this->index(rowCount() - 1), { TitleRole, IsAllowedRole });
    AbstractSelectionFilterModel::onSelectionChanged();
}
