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
#ifndef MU_NOTATION_INOTATIONSELECTIONRANGE_H
#define MU_NOTATION_INOTATIONSELECTIONRANGE_H

#include <vector>
#include <QRectF>
#include "../notationtypes.h"

namespace mu::notation {
class INotationSelectionRange
{
public:
    virtual ~INotationSelectionRange() = default;

    virtual int startStaffIndex() const = 0;
    virtual Fraction startTick() const = 0;

    virtual int endStaffIndex() const = 0;
    virtual Fraction endTick() const = 0;

    virtual int startMeasureIndex() const = 0;
    virtual int endMeasureIndex() const = 0;

    virtual std::vector<QRectF> boundingArea() const = 0;
};

using INotationSelectionRangePtr = std::shared_ptr<INotationSelectionRange>;
}

#endif // MU_NOTATION_INOTATIONSELECTIONRANGE_H
