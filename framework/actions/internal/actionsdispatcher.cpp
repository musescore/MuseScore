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
    auto it = m_calls.find(action);
    if (it == m_calls.end()) {
        LOGW() << "not registred action: " << action;
        return;
    }

    ActionCall& call = it->second;
    LOGI() << "try call action: " << action;
    call(action);
}

void ActionsDispatcher::reg(const ActionName& action, const ActionCall& call)
{
    auto it = m_calls.find(action);
    IF_ASSERT_FAILED_X(it == m_calls.end(), std::string("already registred action: ") + action) {
        return;
    }
    m_calls.insert({ action, call });
}
