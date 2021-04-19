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
#ifndef MU_NOTATION_NOTATIONSELECTION_H
#define MU_NOTATION_NOTATIONSELECTION_H

#include "../inotationselection.h"

#include "igetscore.h"

namespace Ms {
class Score;
}

namespace mu::notation {
class NotationSelection : public INotationSelection
{
public:
    NotationSelection(IGetScore* getScore);

    bool isNone() const override;
    bool isRange() const override;

    bool canCopy() const override;
    QMimeData* mimeData() const override;

    Element* element() const override;
    std::vector<Element*> elements() const override;

    std::vector<Note*> notes(NoteFilter filter) const override;

    QRectF canvasBoundingRect() const override;

    INotationSelectionRangePtr range() const override;

private:
    Ms::Score* score() const;

    IGetScore* m_getScore = nullptr;
    INotationSelectionRangePtr m_range;
};
}

#endif // MU_NOTATION_NOTATIONSELECTION_H
