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
#include "pluginsuiactions.h"

#include "ui/view/iconcodes.h"
#include "context/uicontext.h"

#include "containers.h"
#include "types/translatablestring.h"
#include "log.h"

using namespace mu;
using namespace mu::plugins;
using namespace mu::ui;
using namespace mu::actions;

PluginsUiActions::PluginsUiActions(std::shared_ptr<PluginsService> service)
    : m_service(service)
{
}

static UiAction MANAGE_ACTION = UiAction(
    "manage-plugins",
    ActionCategory::PLUGINS,
    mu::context::UiCtxAny,
    mu::context::CTX_ANY,
    TranslatableString("action", "&Manage plugins…"),
    TranslatableString("action", "Manage plugins…")
    );

const mu::ui::UiActionList& PluginsUiActions::actionsList() const
{
    UiActionList result;

    for (const PluginInfo& plugin : values(m_service->plugins().val)) {
        UiAction action;
        action.code = codeFromQString(plugin.codeKey);
        action.uiCtx = mu::context::UiCtxNotationOpened;
        action.scCtx = mu::context::CTX_NOTATION_OPENED;
        action.description = TranslatableString("plugins", "Run plugin %1").arg(plugin.codeKey);
        action.title = action.description;

        result.push_back(action);
    }

    result.push_back(MANAGE_ACTION);

    m_actions = result;

    return m_actions;
}

bool PluginsUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> PluginsUiActions::actionEnabledChanged() const
{
    static async::Channel<actions::ActionCodeList> ch;
    return ch;
}

bool PluginsUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> PluginsUiActions::actionCheckedChanged() const
{
    static async::Channel<actions::ActionCodeList> ch;
    return ch;
}
