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
#ifndef MU_NOTATION_NOTATIONSELECTIONRANGE_H
#define MU_NOTATION_NOTATIONSELECTIONRANGE_H

#include "inotationselectionrange.h"

#include "igetscore.h"

namespace Ms {
class Score;
}

namespace mu::notation {
class NotationSelectionRange : public INotationSelectionRange
{
public:
    NotationSelectionRange(IGetScore* getScore);

    int startStaffIndex() const override;
    Fraction startTick() const override;

    int endStaffIndex() const override;
    Fraction endTick() const override;

    int startMeasureIndex() const override;
    int endMeasureIndex() const override;

    std::vector<QRectF> boundingArea() const override;

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

    struct MeasureRange {
        int startIndex = 0;
        int endIndex = 0;
    };
    MeasureRange measureRange() const;

    IGetScore* m_getScore = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONSELECTIONRANGE_H
