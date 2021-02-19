//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_FRAMEWORK_IWORKSPACESETTINGS_H
#define MU_FRAMEWORK_IWORKSPACESETTINGS_H

#include "modularity/imoduleexport.h"
#include "val.h"
#include "async/notification.h"
#include "workspace/workspacetypes.h"

namespace mu::framework {
class IWorkspaceSettings : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceSettings)
public:

    virtual ~IWorkspaceSettings() = default;

    virtual bool isManage(workspace::WorkspaceTag tag) const = 0;

    virtual Val value(workspace::WorkspaceTag tag, const std::string& key) const = 0;
    virtual void setValue(workspace::WorkspaceTag tag, const std::string& key, const Val& value) const = 0;
    virtual async::Notification valuesChanged() const = 0;
};
}

#endif // MU_FRAMEWORK_IWORKSPACESETTINGS_H
