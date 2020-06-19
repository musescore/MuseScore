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
#ifndef MU_ACTIONS_IACTIONSDISPATCHER_H
#define MU_ACTIONS_IACTIONSDISPATCHER_H

#include <functional>
#include "modularity/imoduleexport.h"
#include "actiontypes.h"

namespace mu {
namespace actions {
class Actionable;
class IActionsDispatcher : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IActionsDispatcher)
public:
    ~IActionsDispatcher() = default;

    using ActionCallBack = std::function<void ()>;
    using ActionCallBackWithName = std::function<void (const ActionName&)>;
    using ActionCallBackWithNameAndData = std::function<void (const ActionName&, const ActionData& data)>;

    virtual void dispatch(const ActionName& a) = 0;
    virtual void dispatch(const ActionName& a, const ActionData& data) = 0;

    virtual void unReg(Actionable* client) = 0;
    virtual void reg(Actionable* client, const ActionName& action, const ActionCallBackWithNameAndData& call) = 0;

    void reg(Actionable* client, const ActionName& action, const ActionCallBack& call)
    {
        reg(client, action, [call](const ActionName&, const ActionData&) { call(); });
    }

    void reg(Actionable* client, const ActionName& action, const ActionCallBackWithName& call)
    {
        reg(client, action, [call](const ActionName& action, const ActionData&) { call(action); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionName& action, T* caller, void (T::* func)(const ActionName& action))
    {
        reg(client, action, [caller, func](const ActionName& action) { (caller->*func)(action); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionName& action, T* caller, void (T::* func)())
    {
        reg(client, action, [caller, func](const ActionName&) { (caller->*func)(); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionName& action, T* caller, void (T::* func)(const ActionName& action,
                                                                                       const ActionData& data))
    {
        reg(client, action,[caller, func](const ActionName& a, const ActionData& data) { (caller->*func)(a, data); });
    }

    template<typename T>
    void reg(Actionable* client, const ActionName& action, T* caller, void (T::* func)(const ActionData& data))
    {
        reg(client, action, [caller, func](const ActionName&, const ActionData& data) { (caller->*func)(data); });
    }
};
}
}

#endif // MU_ACTIONS_IACTIONSDISPATCHER_H
