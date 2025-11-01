/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "audiouiactions.h"

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

#include "audioactionscontroller.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::audio;

const UiActionList AudioUiActions::m_actions = {
    UiAction("action://audio/dev/use-workermode",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "1. Worker mode"),
             Checkable::Yes
             ),
    UiAction("action://audio/dev/use-drivermode",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "2. Driver mode"),
             Checkable::Yes
             ),
    UiAction("action://audio/dev/use-workerrpcmode",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_ANY,
             TranslatableString("action", "3. Worker RPC mode"),
             Checkable::Yes
             )
};

AudioUiActions::AudioUiActions(const std::shared_ptr<AudioActionsController>& c)
    : m_controller(c)
{
}

const UiActionList& AudioUiActions::actionsList() const
{
    return m_actions;
}

bool AudioUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

async::Channel<ActionCodeList> AudioUiActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool AudioUiActions::actionChecked(const UiAction& a) const
{
    return m_controller->actionChecked(a.code);
}

async::Channel<ActionCodeList> AudioUiActions::actionCheckedChanged() const
{
    return m_controller->actionCheckedChanged();
}
