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
#ifndef MU_WORKSPACE_IWORKSPACE_H
#define MU_WORKSPACE_IWORKSPACE_H

#include <memory>

#include "workspace/workspacetypes.h"
#include "async/channel.h"

namespace mu::workspace {
class IWorkspace
{
public:
    virtual ~IWorkspace() = default;

    virtual std::string name() const = 0;
    virtual std::string title() const = 0;

    virtual AbstractDataPtr data(WorkspaceTag tag, const std::string& name = std::string()) const = 0;
    virtual AbstractDataPtrList dataList(WorkspaceTag tag) const = 0;
    virtual void addData(AbstractDataPtr data) = 0;
    virtual async::Channel<AbstractDataPtr> dataChanged() const = 0;

    //! NOTE Only methods associations with framework.
    //! Other methods (for other data) must be in the appropriate modules.
    virtual Val settingValue(const std::string& key) const = 0;
    virtual std::vector<std::string> toolbarActions(const std::string& toolbarName) const = 0;
};

using IWorkspacePtr = std::shared_ptr<IWorkspace>;
using IWorkspacePtrList = std::vector<IWorkspacePtr>;
}

Q_DECLARE_METATYPE(mu::workspace::IWorkspacePtr)

#endif // MU_WORKSPACE_IWORKSPACE_H
