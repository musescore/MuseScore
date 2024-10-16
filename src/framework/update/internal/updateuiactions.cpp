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
#include "updateuiactions.h"

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace muse::update;
using namespace muse::ui;
using namespace muse::actions;

const UiActionList UpdateUiActions::m_actions = {
    UiAction("check-update",
             UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "Check for &update")
             )
};

UpdateUiActions::UpdateUiActions(std::shared_ptr<UpdateActionController> controller, const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx), m_controller(controller)
{
}

const UiActionList& UpdateUiActions::actionsList() const
{
    return m_actions;
}

bool UpdateUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool UpdateUiActions::actionChecked(const UiAction&) const
{
    return false;
}

muse::async::Channel<ActionCodeList> UpdateUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<ActionCodeList> UpdateUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
