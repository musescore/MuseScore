//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "paletteuiactions.h"

#include "context/uicontext.h"

using namespace mu::palette;
using namespace mu::ui;

static const mu::actions::ActionCode MASTERPALETTE_CODE("masterpalette");

const UiActionList PaletteUiActions::m_actions = {
    UiAction(MASTERPALETTE_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Master Palette"),
             Checkable::Yes
             ),
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
