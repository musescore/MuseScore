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
#include "../notationtypes.h"

#include "igetscore.h"

namespace Ms {
class Score;
}

namespace mu {
namespace notation {
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

    SelectionRange range() const override;

    QRectF canvasBoundingRect() const override;

    std::vector<QRectF> rangeBoundingArea() const override;

private:
    Ms::Score* score() const;

    Ms::Segment* rangeStartSegment() const;
    Ms::Segment* rangeEndSegment() const;

    int selectionLastVisibleStaff() const;

    struct RangeSection {
        const Ms::System* system = nullptr;
        const Ms::Segment* startSegment = nullptr;
        const Ms::Segment* endSegment = nullptr;
    };
    std::vector<RangeSection> splitRangeBySections(const Ms::Segment* rangeStartSegment, const Ms::Segment* rangeEndSegment) const;

    int sectionElementsMaxY(const RangeSection& selection) const;
    int sectionElementsMinY(const RangeSection& selection) const;

    IGetScore* m_getScore = nullptr;
};
}
}

#endif // MU_NOTATION_NOTATIONSELECTION_H
