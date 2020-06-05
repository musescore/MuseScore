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
#ifndef MU_CONTEXT_IGLOBALCONTEXT_H
#define MU_CONTEXT_IGLOBALCONTEXT_H

#include <functional>
#include "modularity/imoduleexport.h"
#include "domain/notation/inotation.h"
#include "async/notify.h"

namespace mu {
namespace context {
class IGlobalContext : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::context::IGlobalContext)

public:
    ~IGlobalContext() = default;

    virtual void setNotation(const std::shared_ptr<domain::notation::INotation>& notation) = 0;
    virtual std::shared_ptr<domain::notation::INotation> notation() const = 0;
    virtual deto::async::Notify notationChanged() const = 0;
};
}
}

#endif // MU_CONTEXT_IGLOBALCONTEXT_H
