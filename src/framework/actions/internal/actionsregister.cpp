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
#include "actionsregister.h"

using namespace mu::actions;

void ActionsRegister::reg(const std::shared_ptr<IModuleActions>& actions)
{
    m_modules.push_back(actions);
}

const ActionItem& ActionsRegister::action(const ActionCode& name) const
{
    for (const std::shared_ptr<IModuleActions>& m : m_modules) {
        const ActionItem& a = m->action(name);
        if (a.isValid()) {
            return a;
        }
    }

    static ActionItem null;
    return null;
}
