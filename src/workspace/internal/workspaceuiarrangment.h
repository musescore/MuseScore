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
#ifndef MU_WORKSPACE_WORKSPACEUIARRANGMENT_H
#define MU_WORKSPACE_WORKSPACEUIARRANGMENT_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "global/iworkspaceuiarrangment.h"
#include "iworkspacemanager.h"

namespace mu::workspace {
class WorkspaceUiArrangment : public framework::IWorkspaceUiArrangment, public async::Asyncable
{
    INJECT(workspace, IWorkspaceManager, manager)

public:
    void init();

    Val value(const std::string& key) const override;
    bool setValue(const std::string& key, const Val& value) const override;
    async::Notification valuesChanged() const override;

private:
    IWorkspacePtr currentWorkspace() const;

    bool isWorkspaceManageUi() const;

    async::Notification m_valuesChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACEUIARRANGMENT_H
