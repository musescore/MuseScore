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
#ifndef MU_NOTATION_IMASTERNOTATION_H
#define MU_NOTATION_IMASTERNOTATION_H

#include "modularity/imoduleexport.h"
#include "iexcerptnotation.h"
#include "ret.h"
#include "io/path.h"

namespace mu {
namespace notation {
class IMasterNotation : virtual public INotation
{
public:
    virtual Ret load(const io::path& path) = 0;
    virtual io::path path() const = 0;

    virtual Ret createNew(const ScoreCreateOptions& scoreInfo) = 0;

    virtual std::vector<IExcerptNotationPtr> excerpts() const = 0;
};

using IMasterNotationPtr = std::shared_ptr<IMasterNotation>;
}
}

#endif // MU_NOTATION_IMASTERNOTATION_H
