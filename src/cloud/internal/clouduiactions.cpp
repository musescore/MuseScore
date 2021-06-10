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

#include "clouduiactions.h"
#include "cloudactionscontroller.h"

#include "context/uicontext.h"

using namespace mu::cloud;
using namespace mu::ui;

static const mu::actions::ActionCode SAVE_ONLINE_CODE("file-save-online");

const UiActionList CloudUiActions::m_actions = {
    UiAction(SAVE_ONLINE_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Upload to MuseScore.com"),
             QT_TRANSLATE_NOOP("action", "Save score on MuseScore.com"),
             IconCode::Code::CLOUD_FILE
             ),
};

CloudUiActions::CloudUiActions(std::shared_ptr<CloudActionsController> controller)
    : m_controller(controller)
{
}

const UiActionList& CloudUiActions::actionsList() const
{
    return m_actions;
}

bool CloudUiActions::actionEnabled(const UiAction& act) const
{
    return m_controller->canReceiveAction(act.code);
}

bool CloudUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> CloudUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> CloudUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
