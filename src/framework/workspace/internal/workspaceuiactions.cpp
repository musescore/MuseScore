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
#include "workspaceuiactions.h"

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace muse;
using namespace muse::workspace;
using namespace muse::ui;
using namespace muse::actions;

const UiActionList WorkspaceUiActions::m_actions = {
    UiAction("select-workspace",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Select workspace")
             ),
    UiAction("configure-workspaces",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Configure workspace"),
             TranslatableString("action", "Configure workspace…")
             )
};

WorkspaceUiActions::WorkspaceUiActions(std::shared_ptr<WorkspaceActionController> controller, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_controller(controller)
{
}

const UiActionList& WorkspaceUiActions::actionsList() const
{
    return m_actions;
}

bool WorkspaceUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool WorkspaceUiActions::actionChecked(const UiAction&) const
{
    return false;
}

async::Channel<ActionCodeList> WorkspaceUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

async::Channel<ActionCodeList> WorkspaceUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
