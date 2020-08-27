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
#ifndef MU_NOTATION_INOTATIONREADER_H
#define MU_NOTATION_INOTATIONREADER_H

#include <memory>

#include "ret.h"
#include "io/path.h"

namespace Ms {
class MasterScore;
}

namespace mu {
namespace notation {
class INotationReader
{
public:
    virtual ~INotationReader() = default;

    virtual Ret read(Ms::MasterScore* score, const io::path& path) = 0;
};

using INotationReaderPtr = std::shared_ptr<INotationReader>;
}
}

#endif // MU_NOTATION_INOTATIONREADER_H
