/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "context/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

static const muse::actions::ActionCode MASTERPALETTE_CODE("masterpalette");

const UiActionList PaletteUiActions::m_actions = {
    UiAction(MASTERPALETTE_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "&Master palette"),
             TranslatableString("action", "Open master palette…"),
             Checkable::Yes
             ),
    UiAction("palette-search",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_ANY,
             TranslatableString("action", "Palette search"),
             TranslatableString("action", "Search palettes")
             ),
    UiAction("time-signature-properties",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Time signature properties…"),
             TranslatableString("action", "Time signature properties…")
             ),
    UiAction("customize-kit",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Customize kit…"),
             TranslatableString("action", "Customize kit…")
             ),
    UiAction("apply-current-palette-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Apply current palette element"),
             TranslatableString("action", "Apply current palette element")
             ),
    UiAction("show-keys",
             mu::context::UiCtxProjectOpened,
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

muse::async::Channel<ActionCodeList> PaletteUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<ActionCodeList> PaletteUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
