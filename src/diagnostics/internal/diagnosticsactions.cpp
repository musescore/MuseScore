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
#include "diagnosticsactions.h"

#include "context/uicontext.h"

using namespace mu::ui;
using namespace mu::actions;
using namespace mu::diagnostics;

const UiActionList DiagnosticsActions::m_actions = {
    UiAction("diagnostic-show-paths",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Show paths…")
             ),
    UiAction("diagnostic-show-navigation-tree",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Show navigation tree…")
             ),
    UiAction("diagnostic-show-accessible-tree",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Show accessible tree…")
             ),
    UiAction("diagnostic-accessible-tree-dump",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Accessible dump")
             )
};

const UiActionList& DiagnosticsActions::actionsList() const
{
    return m_actions;
}

bool DiagnosticsActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> DiagnosticsActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool DiagnosticsActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> DiagnosticsActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
