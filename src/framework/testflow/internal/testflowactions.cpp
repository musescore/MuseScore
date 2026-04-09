/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include "testflowactions.h"

#include "ui/uiaction.h"
#include "shortcuts/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace muse;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::testflow;

const UiActionList TestflowActions::m_actions = {
    UiAction("testflow-show-scripts",
             muse::ui::UiCtxAny,
             muse::shortcuts::CTX_DISABLED,
             TranslatableString("action", "Show &scripts…"),
             TranslatableString("action", "Show scripts")
             ),
};

const UiActionList& TestflowActions::actionsList() const
{
    return m_actions;
}

bool TestflowActions::actionEnabled(const UiAction&) const
{
    return true;
}

async::Channel<ActionCodeList> TestflowActions::actionEnabledChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}

bool TestflowActions::actionChecked(const UiAction&) const
{
    return false;
}

async::Channel<ActionCodeList> TestflowActions::actionCheckedChanged() const
{
    static async::Channel<ActionCodeList> ch;
    return ch;
}
