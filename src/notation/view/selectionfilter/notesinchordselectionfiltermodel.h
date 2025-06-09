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

#pragma once

#include "abstractselectionfiltermodel.h"

namespace mu::notation {
class NotesInChordSelectionFilterModel : public AbstractSelectionFilterModel
{
    Q_OBJECT

    Q_PROPERTY(bool includeSingleNotes READ includeSingleNotes WRITE setIncludeSingleNotes NOTIFY includeSingleNotesChanged)

public:
    explicit NotesInChordSelectionFilterModel(QObject* parent = nullptr);

signals:
    void includeSingleNotesChanged();

private:
    bool enabled() const override;

    void loadTypes() override;

    bool isFiltered(const SelectionFilterTypesVariant& variant) const override;
    void setFiltered(const SelectionFilterTypesVariant& variant, bool filtered) override;

    bool isAllowed(const SelectionFilterTypesVariant& variant) const override;

    SelectionFilterTypesVariant getAllMask() const override { return engraving::NotesInChordSelectionFilterTypes::ALL; }
    SelectionFilterTypesVariant getNoneMask() const override { return engraving::NotesInChordSelectionFilterTypes::NONE; }

    bool includeSingleNotes() const;
    void setIncludeSingleNotes(bool include);

    bool multipleIndicesSelected() const;

    QString titleForType(const SelectionFilterTypesVariant& variant) const override;
    size_t noteIdxForType(const engraving::NotesInChordSelectionFilterTypes& type) const;
    engraving::NotesInChordSelectionFilterTypes typeForNoteIdx(size_t noteIdx) const;
    void updateTopNoteIdx();

    void onSelectionChanged() override;

    size_t m_topNoteIdx = muse::nidx; // cached - calculating this isn't free
};
}
