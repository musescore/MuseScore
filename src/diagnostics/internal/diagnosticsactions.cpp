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
#include "types/translatablestring.h"

using namespace mu::ui;
using namespace mu::actions;
using namespace mu::diagnostics;

const UiActionList DiagnosticsActions::m_actions = {
    UiAction("diagnostic-save-diagnostic-files",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Save diagnostic files")
             ),
    UiAction("diagnostic-show-paths",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show p&aths…")
             ),
    UiAction("diagnostic-show-profiler",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show pr&ofiler…")
             ),
    UiAction("diagnostic-show-navigation-tree",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show &navigation tree…")
             ),
    UiAction("diagnostic-show-accessible-tree",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show &accessible tree…")
             ),
    UiAction("diagnostic-accessible-tree-dump",
             ActionCategory::Accessibility,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString::untranslatable("Accessible &dump")
             ),
    UiAction("diagnostic-show-engraving-elements",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Engraving &elements")
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
