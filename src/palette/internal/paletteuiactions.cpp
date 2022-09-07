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

#include "paletteuiactions.h"

#include "context/uicontext.h"
#include "types/translatablestring.h"

using namespace mu::palette;
using namespace mu::ui;

static const mu::actions::ActionCode MASTERPALETTE_CODE("masterpalette");

const UiActionList PaletteUiActions::m_actions = {
    UiAction(MASTERPALETTE_CODE,
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Master palette"),
             TranslatableString("action", "Open master palette…"),
             Checkable::Yes
             ),
    UiAction("palette-search",
             ActionCategory::Workspace,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Palette search"),
             TranslatableString("action", "Search palettes")
             ),
    UiAction("time-signature-properties",
             ActionCategory::NoteInput,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Time signature properties…"),
             TranslatableString("action", "Time signature properties…")
             ),
    UiAction("edit-drumset",
             ActionCategory::NoteInput,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Edit drumset…"),
             TranslatableString("action", "Edit drumset…")
             ),
    UiAction("show-keys",
             ActionCategory::TextLyrics,
             mu::context::UiCtxNotationOpened,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Insert special characters"),
             TranslatableString("action", "Insert special characters…")
             )
};

PaletteUiActions::PaletteUiActions(std::shared_ptr<PaletteActionsController> controller)
    : m_controller(controller)
{
}

void PaletteUiActions::init()
{
    m_controller->isMasterPaletteOpened().ch.onReceive(this, [this](bool) {
        m_actionCheckedChanged.send({ MASTERPALETTE_CODE });
    });
}

const UiActionList& PaletteUiActions::actionsList() const
{
    return m_actions;
}

bool PaletteUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool PaletteUiActions::actionChecked(const UiAction& act) const
{
    if (MASTERPALETTE_CODE == act.code) {
        return m_controller->isMasterPaletteOpened().val;
    }

    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> PaletteUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> PaletteUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
