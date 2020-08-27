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
#ifndef MU_NOTATION_INOTATIONREADERSREGISTER_H
#define MU_NOTATION_INOTATIONREADERSREGISTER_H

#include <string>
#include <vector>

#include "modularity/imoduleexport.h"
#include "inotationreader.h"

namespace mu {
namespace notation {
class INotationReadersRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationReadersRegister)
public:
    virtual ~INotationReadersRegister() = default;

    //! NOTE In the future, we need to replace the suffix with an enumerator
    //! or a better structure describing the format.
    virtual void reg(const std::vector<std::string>& syffixs, std::shared_ptr<INotationReader> reader) = 0;
    virtual std::shared_ptr<INotationReader> reader(const std::string& syffix) = 0;
};
}
}

#endif // MU_NOTATION_INOTATIONREADERSREGISTER_H
