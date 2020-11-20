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

#ifndef MU_WORKSPACE_WORKSPACESETTINGSSOURCE_H
#define MU_WORKSPACE_WORKSPACESETTINGSSOURCE_H

#include "global/isettingssource.h"
#include "workspace/iworkspacemanager.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"

namespace mu::workspace {
class WorkspaceSettingsSource : public framework::ISettingsSource, public async::Asyncable
{
    INJECT(workspace, IWorkspaceManager, workspaceManager)

public:
    void init();

    bool hasValue(const std::string& key) const override;
    Val value(const std::string& key) const override;
    void setValue(const std::string& key, const Val& value) override;

    async::Notification sourceChanged() const override;

private:
    IWorkspacePtr m_currentWorkspace;
    async::Notification m_sourceChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACESETTINGSSOURCE_H
