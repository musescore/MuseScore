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
#ifndef MU_WORKSPACE_WORKSPACEACTIONS_H
#define MU_WORKSPACE_WORKSPACEACTIONS_H

#include "actions/imoduleactions.h"

namespace mu::workspace {
class WorkspaceActions : public actions::IModuleActions
{
public:
    const actions::ActionItem& action(const actions::ActionCode& actionCode) const override;

private:
    static const actions::ActionList m_actions;
};
}

#endif // MU_WORKSPACE_WORKSPACEACTIONS_H
