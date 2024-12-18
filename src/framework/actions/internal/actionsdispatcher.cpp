/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "actionsdispatcher.h"

#include "actionable.h"

#include "log.h"

using namespace muse::actions;

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

void ActionsDispatcher::dispatch(const ActionCode& actionCode)
{
    static ActionData dummy;
    dispatch(actionCode, dummy);
}

void ActionsDispatcher::dispatch(const ActionCode& actionCode, const ActionData& data)
{
    auto it = m_clients.find(actionCode);
    if (it == m_clients.end()) {
        LOGW() << "not a registered action: " << actionCode;
        return;
    }

    doDispatch(it->second, actionCode, data);
}

void ActionsDispatcher::dispatch(const ActionQuery& actionQuery)
{
    //! NOTE Try find full query
    const std::string full = actionQuery.toString();
    auto it = m_clients.find(full);
    if (it != m_clients.end()) {
        static ActionData dummy;
        doDispatch(it->second, full, dummy);
        return;
    }

    const ActionCode code = actionQuery.uri().toString();
    it = m_clients.find(code);
    if (it != m_clients.end()) {
        LOGW() << "not a registered action: " << code;
        return;
    }

    const ActionQuery::Params& params = actionQuery.params();
    ActionData data;
    int i = 0;
    for (const auto& p : params) {
        const Val& val = p.second;
        switch (val.type()) {
        case Val::Type::Bool:
            data.setArg<bool>(i, val.toBool());
            break;
        case Val::Type::String:
            data.setArg<std::string>(i, val.toString());
            break;
        default:
            UNREACHABLE;
            data.setArg<int>(i, 0); // dummy
        }
    }

    doDispatch(it->second, code, data);
}

void ActionsDispatcher::doDispatch(const Clients& clients, const ActionCode& actionCode, const ActionData& data)
{
    int canReceiveCount = 0;
    for (auto cit = clients.cbegin(); cit != clients.cend(); ++cit) {
        const Actionable* client = cit->first;
        if (client->canReceiveAction(actionCode)) {
            ++canReceiveCount;
            const CallBacks& callbacks = cit->second;
            auto cbit = callbacks.find(actionCode);
            IF_ASSERT_FAILED(cbit != callbacks.end()) {
                continue;
            }

            const ActionCallBackWithNameAndData& callback = cbit->second;
            LOGI() << "try call action: " << actionCode;
            callback(actionCode, data);
        }
    }

    if (canReceiveCount == 0) {
        LOGI() << "no one can handle the action: " << actionCode;
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

void ActionsDispatcher::reg(Actionable* client, const ActionCode& actionCode, const ActionCallBackWithNameAndData& call)
{
    client->setDispatcher(this);

    Clients& clients = m_clients[actionCode];
    CallBacks& callbacks = clients[client];
    callbacks.insert({ actionCode, call });
}

void ActionsDispatcher::reg(Actionable* client, const ActionQuery& actionQuery, const ActionCallBackWithQuery& call)
{
    reg(client, actionQuery.toString(), [call](const ActionCode& action, const ActionData&) { call(ActionQuery(action)); });
}

bool ActionsDispatcher::isReg(Actionable* client) const
{
    return client->isDispatcher(this);
}

ActionCodeList ActionsDispatcher::actionList() const
{
    ActionCodeList list;
    list.reserve(m_clients.size());
    for (const auto& p : m_clients) {
        list.push_back(p.first);
    }
    return list;
}
