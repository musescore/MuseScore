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
#ifndef MU_ACTIONS_ACTIONSDISPATCHER_H
#define MU_ACTIONS_ACTIONSDISPATCHER_H

#include <map>

#include "../iactionsdispatcher.h"

namespace mu {
namespace actions {
class ActionsDispatcher : public IActionsDispatcher
{
public:
    ActionsDispatcher() = default;
    ~ActionsDispatcher() override;

    void dispatch(const ActionName& a) override;
    void dispatch(const ActionName& action, const ActionData& data) override;

    void unReg(Actionable* client) override;
    void reg(Actionable* client, const ActionName& action, const ActionCallBackWithNameAndData& call) override;

private:

    using CallBacks = std::map<ActionName, ActionCallBackWithNameAndData>;
    using Clients = std::map<Actionable*, CallBacks>;

    std::map<ActionName, Clients > m_clients;
};
}
}

#endif // MU_ACTIONS_ACTIONSDISPATCHER_H
