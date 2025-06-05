/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "vstuiactions.h"
#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

#include "vstactionscontroller.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::vst;

const UiActionList VstUiActions::m_actions = {
    UiAction("vst-use-oldview",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Use old view"),
             Checkable::Yes
             ),
    UiAction("vst-use-newview",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Use new view"),
             Checkable::Yes
             )
};

VstUiActions::VstUiActions(const std::shared_ptr<VstActionsController>& c)
    : m_controller(c)
{
}

const UiActionList& VstUiActions::actionsList() const
{
    return m_actions;
}

bool VstUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

async::Channel<ActionCodeList> VstUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool VstUiActions::actionChecked(const UiAction& a) const
{
    return m_controller->actionChecked(a.code);
}

async::Channel<ActionCodeList> VstUiActions::actionCheckedChanged() const
{
    return m_controller->actionCheckedChanged();
}
