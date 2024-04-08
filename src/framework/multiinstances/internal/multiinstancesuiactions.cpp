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
#include "multiinstancesuiactions.h"

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::mi;

const UiActionList MultiInstancesUiActions::m_actions = {
    UiAction("multiinstances-dev-show-info",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "&Multiinstances")
             )
};

const UiActionList& MultiInstancesUiActions::actionsList() const
{
    return m_actions;
}

bool MultiInstancesUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

async::Channel<ActionCodeList> MultiInstancesUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool MultiInstancesUiActions::actionChecked(const UiAction&) const
{
    return false;
}

async::Channel<ActionCodeList> MultiInstancesUiActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
