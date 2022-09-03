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

#include "instrumentsuiactions.h"

#include "context/uicontext.h"
#include "types/translatablestring.h"

using namespace mu::instrumentsscene;
using namespace mu::ui;

const UiActionList InstrumentsUiActions::m_actions = {
    UiAction("instruments",
             ActionCategory::DialogsPanels,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Add/remove instruments…"),
             TranslatableString("action", "Add/remove instruments…")
             ),
    UiAction("change-instrument",
             ActionCategory::DialogsPanels,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select instrument…"),
             TranslatableString("action", "Select instrument…")
             )
};

const mu::ui::UiActionList& InstrumentsUiActions::actionsList() const
{
    return m_actions;
}

bool InstrumentsUiActions::actionEnabled(const UiAction&) const
{
    return true;
}

bool InstrumentsUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> InstrumentsUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> InstrumentsUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
