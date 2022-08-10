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
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-next-section",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-prev-section",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-next-panel",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-prev-panel",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-next-tab",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-prev-tab",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY
             ),
    UiAction("nav-right",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-left",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-up",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-down",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-escape",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-trigger-control",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-first-control",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-last-control",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-nextrow-control",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
             ),
    UiAction("nav-prevrow-control",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_NOT_NOTATION_FOCUSED
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
