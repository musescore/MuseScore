//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
