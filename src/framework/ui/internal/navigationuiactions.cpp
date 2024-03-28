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

#include "../uiaction.h"
#include "shortcuts/shortcutcontext.h"

using namespace mu::ui;
using namespace mu::actions;

const UiActionList NavigationUiActions::m_actions = {
    UiAction("nav-dev-show-controls",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-section",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-section",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-panel",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-panel",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-tab",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-tab",
             ui::UiCtxAny,
             mu::shortcuts::CTX_ANY
             ),
    UiAction("nav-right",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-left",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-up",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-down",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-escape",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-trigger-control",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-first-control",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-last-control",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-nextrow-control",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
             ),
    UiAction("nav-prevrow-control",
             ui::UiCtxAny,
             mu::shortcuts::CTX_NOT_PROJECT_FOCUSED
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
