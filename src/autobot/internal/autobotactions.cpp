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
#include "autobotactions.h"

#include "context/uicontext.h"
#include "types/translatablestring.h"

using namespace mu::ui;
using namespace mu::actions;
using namespace mu::autobot;

const UiActionList AutobotActions::m_actions = {
    UiAction("autobot-show-batchtests",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show batch tests…")
             ),
    UiAction("autobot-show-scripts",
             ActionCategory::Undefined,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Show &scripts…")
             ),
};

const UiActionList& AutobotActions::actionsList() const
{
    return m_actions;
}

bool AutobotActions::actionEnabled(const UiAction&) const
{
    return true;
}

mu::async::Channel<ActionCodeList> AutobotActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool AutobotActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<ActionCodeList> AutobotActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
