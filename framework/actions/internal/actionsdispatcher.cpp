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

using namespace mu::actions;

ActionsDispatcher::ActionsDispatcher()
{
}

void ActionsDispatcher::dispatch(const ActionName& action)
{
    auto it = m_callbacks.find(action);
    if (it == m_callbacks.end()) {
        LOGW() << "not registred action: " << action;
        return;
    }

    ActionCallBack3& callback = it->second;
    LOGI() << "try call action: " << action;

    static ActionData dummy;
    callback(action, dummy);
}

void ActionsDispatcher::dispatch(const ActionName& action, const ActionData& data)
{
    auto it = m_callbacks.find(action);
    if (it == m_callbacks.end()) {
        LOGW() << "not registred action: " << action;
        return;
    }

    ActionCallBack3& callback = it->second;
    LOGI() << "try call action: " << action;
    callback(action, data);
}

bool ActionsDispatcher::isRegistred(const ActionName& action) const
{
    auto it = m_callbacks.find(action);
    if (it != m_callbacks.end()) {
        return true;
    }
    return false;
}

void ActionsDispatcher::reg(const ActionName& action, const ActionCallBack3& call)
{
    IF_ASSERT_FAILED_X(!isRegistred(action), std::string("already registred action: ") + action) {
        return;
    }
    m_callbacks.insert({ action, call });
}
