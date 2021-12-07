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
#include "appmenuuiactions.h"

#include "context/uicontext.h"

#include "config.h"

using namespace mu::appshell;
using namespace mu::ui;

const UiActionList AppMenuUiActions::m_actions = {
    UiAction("menu-file",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&File")
             ),
    UiAction("menu-edit",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&Edit")
             ),
    UiAction("menu-view",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&View")
             ),
    UiAction("menu-add",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&Add")
             ),
    UiAction("menu-format",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "F&ormat")
             ),
    UiAction("menu-tools",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&Tools")
             ),
    UiAction("menu-help",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&Help")
             )
#ifdef BUILD_DIAGNOSTICS
    ,
    UiAction("menu-diagnostic",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "&Diagnostic")
             )
#endif
};

const mu::ui::UiActionList& AppMenuUiActions::actionsList() const
{
    return m_actions;
}

bool AppMenuUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

bool AppMenuUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> AppMenuUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> AppMenuUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

UiActionList AppMenuUiActions::allActions()
{
    return m_actions;
}
