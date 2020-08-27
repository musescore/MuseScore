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
#include "actionsdispatcher.h"
#include "log.h"
#include "actionable.h"

using namespace mu::actions;

ActionsDispatcher::~ActionsDispatcher()
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        Clients& clients = it->second;
        for (auto cit = clients.begin(); cit != clients.end(); ++cit) {
            Actionable* client = cit->first;
            client->setDispatcher(nullptr);
        }
    }
}

void ActionsDispatcher::dispatch(const ActionName& action)
{
    static ActionData dummy;
    dispatch(action, dummy);
}

void ActionsDispatcher::dispatch(const ActionName& action, const ActionData& data)
{
    auto it = m_clients.find(action);
    if (it == m_clients.end()) {
        LOGW() << "not registred action: " << action;
        return;
    }

    int canReceiveCount = 0;
    const Clients& clients = it->second;
    for (auto cit = clients.cbegin(); cit != clients.cend(); ++cit) {
        const Actionable* client = cit->first;
        if (client->canReceiveAction(action)) {
            ++canReceiveCount;
            const CallBacks& callbacks = cit->second;
            auto cbit = callbacks.find(action);
            IF_ASSERT_FAILED(cbit != callbacks.end()) {
                continue;
            }

            const ActionCallBackWithNameAndData& callback = cbit->second;
            LOGI() << "try call action: " << action;
            callback(action, data);
        }
    }

    if (canReceiveCount == 0) {
        LOGI() << "no one can handle the action: " << action;
    } else if (canReceiveCount > 1) {
        LOGW() << "More than one client can handle the action, this is not a typical situation.";
    }
}

void ActionsDispatcher::unReg(Actionable* client)
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        Clients& clients = it->second;
        clients.erase(client);
    }
    client->setDispatcher(nullptr);
}

void ActionsDispatcher::reg(Actionable* client, const ActionName& action, const ActionCallBackWithNameAndData& call)
{
    client->setDispatcher(this);

    Clients& clients = m_clients[action];
    CallBacks& callbacks = clients[client];
    callbacks.insert({ action, call });
}
