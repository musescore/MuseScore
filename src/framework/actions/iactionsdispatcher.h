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
#ifndef MUSE_ACTIONS_IACTIONSDISPATCHER_H
#define MUSE_ACTIONS_IACTIONSDISPATCHER_H

#include <functional>

#include "modularity/imoduleinterface.h"
#include "actiontypes.h"

namespace muse::actions {
class Actionable;
class IActionsDispatcher : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IActionsDispatcher)
public:
    ~IActionsDispatcher() = default;

    using ActionCallBack = std::function<void ()>;
    using ActionCallBackWithName = std::function<void (const ActionCode&)>;
    using ActionCallBackWithData = std::function<void (const ActionData& data)>;
    using ActionCallBackWithNameAndData = std::function<void (const ActionCode&, const ActionData& data)>;
    using ActionCallBackWithQuery = std::function<void (const ActionQuery&)>;

    virtual void dispatch(const ActionCode& actionCode) = 0;
    virtual void dispatch(const ActionCode& actionCode, const ActionData& data) = 0;
    virtual void dispatch(const ActionQuery& actionQuery) = 0;

    virtual void unReg(Actionable* client) = 0;
    virtual void reg(Actionable* client, const ActionCode& actionCode, const ActionCallBackWithNameAndData& call) = 0;
    virtual void reg(Actionable* client, const ActionQuery& actionQuery, const ActionCallBackWithQuery& call) = 0;
    virtual bool isReg(Actionable* client) const = 0;
    virtual ActionCodeList actionList() const = 0;

    void reg(Actionable* client, const ActionCode& action, const ActionCallBack& call)
    {
        reg(client, action, [call](const ActionCode&, const ActionData&) { call(); });
    }

    void reg(Actionable* client, const ActionCode& action, const ActionCallBackWithName& call)
    {
        reg(client, action, [call](const ActionCode& action, const ActionData&) { call(action); });
    }

    void reg(Actionable* client, const ActionCode& action, const ActionCallBackWithData& call)
    {
        reg(client, action, [call](const ActionCode&, const ActionData& data) { call(data); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionCode& action, T* caller, void (T::* func)(const ActionCode& action))
    {
        reg(client, action, [caller, func](const ActionCode& action) { (caller->*func)(action); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionCode& action, T* caller, void (T::* func)())
    {
        reg(client, action, [caller, func](const ActionCode&) { (caller->*func)(); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionCode& action, T* caller, void (T::* func)(const ActionCode& action,
                                                                                       const ActionData& data))
    {
        reg(client, action, [caller, func](const ActionCode& a, const ActionData& data) { (caller->*func)(a, data); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionCode& action, T* caller, void (T::* func)(const ActionData& data))
    {
        reg(client, action, [caller, func](const ActionCode&, const ActionData& data) { (caller->*func)(data); });
    }

    // as uri query
    void reg(Actionable* client, const ActionQuery& query, const ActionCallBack& call)
    {
        reg(client, query, [call](const ActionQuery&) { call(); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionQuery& query, T* caller, void (T::* func)(const ActionQuery& query))
    {
        reg(client, query, [caller, func](const ActionQuery& query) { (caller->*func)(query); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionQuery& query, T* caller, void (T::* func)())
    {
        reg(client, query.toString(), [caller, func](const ActionQuery&) { (caller->*func)(); });
    }
};
}

#endif // MUSE_ACTIONS_IACTIONSDISPATCHER_H
