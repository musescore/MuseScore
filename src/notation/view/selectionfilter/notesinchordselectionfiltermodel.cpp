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
    if (!selection || !selection->isRange()) {
        return false;
    }
    const INotationSelectionRangePtr range = selection->range();
    return range ? range->containsMultiNoteChords() : false;
}

void NotesInChordSelectionFilterModel::loadTypes()
{
    for (size_t i = 0; i < NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES; i++) {
        m_types << static_cast<NotesInChordSelectionFilterTypes>(1 << i);
    }
}

void NotesInChordSelectionFilterModel::selectAll()
{
    setIncludeSingleNotes(true);
    AbstractSelectionFilterModel::selectAll();
    return;
}

void NotesInChordSelectionFilterModel::clearAll()
{
    setIncludeSingleNotes(false);
    AbstractSelectionFilterModel::clearAll();
    return;
}

bool NotesInChordSelectionFilterModel::isAllSelected() const
{
    return includeSingleNotes() && AbstractSelectionFilterModel::isAllSelected();
}

bool NotesInChordSelectionFilterModel::isNoneSelected() const
{
    return !includeSingleNotes() && AbstractSelectionFilterModel::isNoneSelected();
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

void NotesInChordSelectionFilterModel::setFiltered(const SelectionFilterTypesVariant& variant, bool filtered)
{
    AbstractSelectionFilterModel::setFiltered(variant, filtered);

    const NotesInChordSelectionFilterTypes type = std::get<NotesInChordSelectionFilterTypes>(variant);
    const bool topNoteIsNormal = m_topNoteIdx < NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES - 1;
    if (type == NotesInChordSelectionFilterTypes::TOP_NOTE && topNoteIsNormal) {
        // Toggling TOP_NOTE should also toggle the flag for the associated index...
        const NotesInChordSelectionFilterTypes indexType = typeForNoteIdx(m_topNoteIdx);
        AbstractSelectionFilterModel::setFiltered(indexType, filtered);
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
    emit maskStatesChanged();
}

bool NotesInChordSelectionFilterModel::multipleIndicesSelected() const
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
    const size_t topIndex = NUM_NOTES_IN_CHORD_SELECTION_FILTER_TYPES - 1;
    if (noteIdx >= topIndex) {
        // noteIdx is outwith the "normal" range and therefore has no directly associated type...
        return NotesInChordSelectionFilterTypes::NONE;
    }
    const size_t typeIndex = topIndex - noteIdx; // Types/buttons are in reverse order...
    const SelectionFilterTypesVariant variant = m_types.at(typeIndex);
    return std::get<NotesInChordSelectionFilterTypes>(variant);
}

void NotesInChordSelectionFilterModel::updateTopNoteIdx()
{
    const INotationSelectionFilterPtr filter = currentNotationSelectionFilter();
    const INotationSelectionPtr selection = currentNotationInteraction() ? currentNotationInteraction()->selection() : nullptr;
    const INotationSelectionRangePtr range = selection ? selection->range() : nullptr;
    if (!filter || !range) {
        m_topNoteIdx = muse::nidx;
        return;
    }

    m_topNoteIdx = muse::nidx;

    const track_idx_t startTrack = staff2track(range->startStaffIndex());
    const track_idx_t endTrack = staff2track(range->endStaffIndex());

    Segment* startSeg = range->rangeStartSegment();
    const Segment* endSeg = range->rangeEndSegment();

    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        for (Segment* seg = startSeg; seg != endSeg; seg = seg->next1MM()) {
            IF_ASSERT_FAILED(seg && seg->tick() < endSeg->tick()) {
                break;
            }
            const EngravingItem* e = seg->element(track);
            if (!seg->enabled() || !e || !e->isChord()) {
                continue;
            }
            const std::vector<Note*> notes = toChord(e)->notes();
            const size_t currIdx = notes.size() - 1;
            if (m_topNoteIdx == muse::nidx || currIdx > m_topNoteIdx) {
                m_topNoteIdx = currIdx;
            }
        }
    }
}

void NotesInChordSelectionFilterModel::onSelectionChanged()
{
    updateTopNoteIdx();
    emit dataChanged(this->index(0), this->index(rowCount() - 1), { TitleRole, IsAllowedRole });
    AbstractSelectionFilterModel::onSelectionChanged();
}
