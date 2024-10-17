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

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::diagnostics;

const UiActionList DiagnosticsActions::m_actions = {
    UiAction("diagnostic-save-diagnostic-files",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Save diagnostic files")
             ),
    UiAction("diagnostic-show-paths",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show p&aths…")
             ),
    UiAction("diagnostic-show-profiler",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show pr&ofiler…")
             ),
    UiAction("diagnostic-show-graphicsinfo",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show g&raphics info…")
             ),
    UiAction("diagnostic-show-navigation-tree",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show &navigation tree…")
             ),
    UiAction("diagnostic-show-accessible-tree",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show &accessibility tree…")
             ),
    UiAction("diagnostic-accessible-tree-dump",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "&Dump accessibility tree to console")
             ),
    UiAction("diagnostic-show-engraving-elements",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show engraving &elements")
             ),
    UiAction("diagnostic-show-actions",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Show &actions list")
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

muse::async::Channel<ActionCodeList> DiagnosticsActions::actionEnabledChanged() const
{
    static muse::async::Channel<ActionCodeList> ch;
    return ch;
}

bool DiagnosticsActions::actionChecked(const UiAction&) const
{
    return false;
}

muse::async::Channel<ActionCodeList> DiagnosticsActions::actionCheckedChanged() const
{
    static muse::async::Channel<ActionCodeList> ch;
    return ch;
}
