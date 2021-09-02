/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_NOTATION_NOTATIONELEMENTS_H
#define MU_NOTATION_NOTATIONELEMENTS_H

#include "inotationelements.h"
#include "igetscore.h"

namespace mu::notation {
class NotationElements : public INotationElements
{
public:
    NotationElements(IGetScore* getScore);

    Ms::Score* msScore() const override;

    EngravingItem* search(const std::string& searchText) const override;
    std::vector<EngravingItem*> elements(const FilterElementsOptions& elementsOptions) const override;

    Measure* measure(const int measureIndex) const override;
    PageList pages() const override;

private:
    Ms::Score* score() const;

    Ms::RehearsalMark* rehearsalMark(const std::string& name) const;
    Ms::Page* page(const int pageIndex) const;

    std::vector<EngravingItem*> allScoreElements() const;

    std::vector<EngravingItem*> filterElements(const FilterElementsOptions* elementsOptions) const;
    std::vector<EngravingItem*> filterNotes(const FilterNotesOptions* notesOptions) const;

    ElementPattern* constructElementPattern(const FilterElementsOptions* elementsOptions) const;
    Ms::NotePattern* constructNotePattern(const FilterNotesOptions* notesOptions) const;

    IGetScore* m_getScore = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONELEMENTS_H
