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
#ifndef MU_NOTATION_INOTATIONSELECTION_H
#define MU_NOTATION_INOTATIONSELECTION_H

#include <vector>
#include <QRectF>
#include "notationtypes.h"

namespace mu {
namespace notation {
class INotationSelection
{
public:
    virtual ~INotationSelection() = default;

    virtual bool isNone() const = 0;
    virtual bool isRange() const = 0;

    virtual Element* element() const = 0;
    virtual std::vector<Element*> elements() const = 0;

    virtual QRectF canvasBoundingRect() const = 0;
};

using INotationSelectionPtr = std::shared_ptr<INotationSelection>;
}
}

#endif // MU_NOTATION_INOTATIONSELECTION_H
