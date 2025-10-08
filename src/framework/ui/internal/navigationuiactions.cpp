/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "muse_framework_config.h"

using namespace muse::ui;
using namespace muse::actions;

#ifdef MUSE_MODULE_UI_NAVIGATION_EXCLUDEPROJECT
static const std::string NAVIGATION_SHORTCUTS_CTX = muse::shortcuts::CTX_NOT_PROJECT_FOCUSED;
#else
static const std::string NAVIGATION_SHORTCUTS_CTX = muse::shortcuts::CTX_ANY;
#endif

const UiActionList NavigationUiActions::m_actions = {
    UiAction("nav-dev-show-controls",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-section",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-section",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-panel",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-panel",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-next-tab",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-prev-tab",
             ui::UiCtxAny,
             muse::shortcuts::CTX_ANY
             ),
    UiAction("nav-right",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-left",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-up",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-down",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-escape",
             ui::UiCtxAny,
             muse::shortcuts::CTX_DISABLED
             ),
    UiAction("nav-trigger-control",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-first-control",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-last-control",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-nextrow-control",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
             ),
    UiAction("nav-prevrow-control",
             ui::UiCtxAny,
             NAVIGATION_SHORTCUTS_CTX
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

muse::async::Channel<ActionCodeList> NavigationUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool NavigationUiActions::actionChecked(const UiAction&) const
{
    return false;
}

muse::async::Channel<ActionCodeList> NavigationUiActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
