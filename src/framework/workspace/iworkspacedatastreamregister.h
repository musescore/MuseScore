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
#ifndef MU_WORKSPACE_IWORKSPACEDATASTREAMREGISTER_H
#define MU_WORKSPACE_IWORKSPACEDATASTREAMREGISTER_H

#include <string>
#include "modularity/imoduleexport.h"
#include "iworkspacedatastream.h"

namespace mu {
namespace workspace {
class IWorkspaceDataStreamRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceDataStreamRegister)
public:
    virtual ~IWorkspaceDataStreamRegister() = default;

    virtual void regStream(const std::string& tag, std::shared_ptr<IWorkspaceDataStream> stream) = 0;
    virtual std::shared_ptr<IWorkspaceDataStream> stream(const std::string& tag) const = 0;
};
}
}

#endif // MU_WORKSPACE_IWORKSPACEDATASTREAMREGISTER_H
