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
#ifndef MU_NOTATION_IMSCZMETAREADER_H
#define MU_NOTATION_IMSCZMETAREADER_H

#include <QString>

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "retval.h"
#include "notationtypes.h"

namespace mu {
namespace notation {
class IMsczMetaReader : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMsczMetaReader)

public:
    virtual ~IMsczMetaReader() = default;

    virtual RetVal<Meta> readMeta(const io::path& filePath) const = 0;
};
}
}

#endif // MU_NOTATION_IMSCZMETAREADER_H
