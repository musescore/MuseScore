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

#include "modularity/imoduleexport.h"
#include "domain/notation/imasternotation.h"
#include "async/notification.h"

namespace mu {
namespace context {
class IGlobalContext : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::context::IGlobalContext)

public:
    ~IGlobalContext() = default;

    virtual void addMasterNotation(const domain::notation::IMasterNotationPtr& notation) = 0;
    virtual void removeMasterNotation(const domain::notation::IMasterNotationPtr& notation) = 0;
    virtual const std::vector<domain::notation::IMasterNotationPtr>& masterNotations() const = 0;
    virtual bool containsMasterNotation(const io::path& path) const = 0;

    virtual void setCurrentMasterNotation(const domain::notation::IMasterNotationPtr& notation) = 0;
    virtual domain::notation::IMasterNotationPtr currentMasterNotation() const = 0;
    virtual async::Notification currentMasterNotationChanged() const = 0;
};
}
}

#endif // MU_CONTEXT_IGLOBALCONTEXT_H
