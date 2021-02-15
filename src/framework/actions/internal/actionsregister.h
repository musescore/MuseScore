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
#ifndef MU_ACTIONS_ACTIONSREGISTER_H
#define MU_ACTIONS_ACTIONSREGISTER_H

#include <vector>
#include "../iactionsregister.h"

namespace mu::actions {
class ActionsRegister : public IActionsRegister
{
public:
    ActionsRegister() = default;

    void reg(const std::shared_ptr<IModuleActions>& actions) override;
    const ActionItem& action(const ActionCode& name) const override;

private:

    std::vector<std::shared_ptr<IModuleActions> > m_modules;
};
}

#endif // MU_ACTIONS_ACTIONSREGISTER_H
