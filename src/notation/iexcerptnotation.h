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
//  MERCHANTABILITY or FIT-0NESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_NOTATION_IEXCERPTNOTATION_H
#define MU_NOTATION_IEXCERPTNOTATION_H

#include "inotation.h"

namespace mu::notation {
class IExcerptNotation
{
public:
    virtual INotationPtr notation() = 0;

    virtual Meta metaInfo() const = 0;
    virtual void setMetaInfo(const Meta& meta) = 0;

    virtual INotationPtr clone() const = 0;
};
using IExcerptNotationPtr = std::shared_ptr<IExcerptNotation>;
}

#endif // MU_NOTATION_IEXCERPTNOTATION_H
