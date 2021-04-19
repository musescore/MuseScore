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
#include "navigationuiactions.h"

#include "context/uicontext.h"

using namespace mu::ui;
using namespace mu::actions;

const UiActionList NavigationUiActions::m_actions = {
    UiAction("nav-dev-show-controls",
             mu::context::UiCtxAny
             ),
    UiAction("nav-next-section",
             mu::context::UiCtxAny
             ),
    UiAction("nav-prev-section",
             mu::context::UiCtxAny
             ),
    UiAction("nav-next-panel",
             mu::context::UiCtxAny
             ),
    UiAction("nav-prev-panel",
             mu::context::UiCtxAny
             ),
    UiAction("nav-right",
             mu::context::UiCtxAny
             ),
    UiAction("nav-left",
             mu::context::UiCtxAny
             ),
    UiAction("nav-up",
             mu::context::UiCtxAny
             ),
    UiAction("nav-down",
             mu::context::UiCtxAny
             ),
    UiAction("nav-escape",
             mu::context::UiCtxAny
             ),
    UiAction("nav-trigger-control",
             mu::context::UiCtxAny
             ),
    UiAction("nav-first-control",
             mu::context::UiCtxAny
             ),
    UiAction("nav-last-control",
             mu::context::UiCtxAny
             ),
    UiAction("nav-nextrow-control",
             mu::context::UiCtxAny
             ),
    UiAction("nav-prevrow-control",
             mu::context::UiCtxAny
             )
};

const UiActionList& NavigationUiActions::actionsList() const
{
    return m_actions;
}

bool NavigationUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> NavigationUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool NavigationUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> NavigationUiActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
