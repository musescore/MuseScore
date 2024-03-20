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
#include "extensionsuiactions.h"

#include "context/uicontext.h"
#include "global/types/translatablestring.h"

#include "log.h"

using namespace mu::ui;
using namespace mu::actions;
using namespace mu::extensions;

static UiAction MANAGE_ACTION = UiAction(
    "manage-plugins",
    mu::context::UiCtxAny,
    mu::context::CTX_ANY,
    mu::TranslatableString("action", "&Manage plugins…"),
    mu::TranslatableString("action", "Manage plugins…")
    );

const mu::ui::UiActionList& ExtensionsUiActions::actionsList() const
{
    UiActionList result;

    for (const Manifest& m : provider()->manifestList()) {
        for (const Action& a : m.actions) {
            UiAction action;
            action.code = makeUriQuery(m.uri, a.code).toString();
            action.uiCtx = m.requiresScore ? mu::context::UiCtxNotationOpened : mu::context::UiCtxAny;
            action.scCtx = m.requiresScore ? mu::context::CTX_NOTATION_OPENED : mu::context::CTX_ANY;
            action.description = TranslatableString("extensions", "Run plugin %1 (%2)").arg(m.title, a.title);
            action.title = action.description;

            result.push_back(std::move(action));
        }
    }

    result.push_back(MANAGE_ACTION);

    m_actions = result;

    return m_actions;
}

bool ExtensionsUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> ExtensionsUiActions::actionEnabledChanged() const
{
    static async::Channel<actions::ActionCodeList> ch;
    return ch;
}

bool ExtensionsUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> ExtensionsUiActions::actionCheckedChanged() const
{
    static async::Channel<actions::ActionCodeList> ch;
    return ch;
}
