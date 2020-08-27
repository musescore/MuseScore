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
#include "notation/imasternotation.h"
#include "async/notification.h"

namespace mu {
namespace context {
class IGlobalContext : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::context::IGlobalContext)

public:
    virtual ~IGlobalContext() = default;

    virtual void addMasterNotation(const notation::IMasterNotationPtr& notation) = 0;
    virtual void removeMasterNotation(const notation::IMasterNotationPtr& notation) = 0;
    virtual const std::vector<notation::IMasterNotationPtr>& masterNotations() const = 0;
    virtual bool containsMasterNotation(const io::path& path) const = 0;

    virtual void setCurrentMasterNotation(const notation::IMasterNotationPtr& notation) = 0;
    virtual notation::IMasterNotationPtr currentMasterNotation() const = 0;
    virtual async::Notification currentMasterNotationChanged() const = 0;

    virtual void setCurrentNotation(const notation::INotationPtr& notation) = 0;
    virtual notation::INotationPtr currentNotation() const = 0;
    virtual async::Notification currentNotationChanged() const = 0;
};
}
}

#endif // MU_CONTEXT_IGLOBALCONTEXT_H
