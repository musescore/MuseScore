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
#include "keynavigationuiactions.h"

#include "context/uicontext.h"

using namespace mu::ui;
using namespace mu::actions;

const UiActionList KeyNavigationUiActions::m_actions = {
    UiAction("nav-next-section",
             mu::context::UiCtxAny
             ),
    UiAction("nav-prev-section",
             mu::context::UiCtxAny
             ),
    UiAction("nav-next-subsection",
             mu::context::UiCtxAny
             ),
    UiAction("nav-prev-subsection",
             mu::context::UiCtxAny
             ),
    UiAction("nav-right",
             mu::context::UiCtxAny
             ),
    UiAction("nav-left",
             mu::context::UiCtxAny
             ),
    UiAction("nav-trigger",
             mu::context::UiCtxAny
             )
};

const UiActionList& KeyNavigationUiActions::actionsList() const
{
    return m_actions;
}

bool KeyNavigationUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> KeyNavigationUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool KeyNavigationUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> KeyNavigationUiActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
