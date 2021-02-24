//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "notationpagestate.h"

#include "log.h"

using namespace mu::appshell;

void NotationPageState::init()
{
    configuration()->isNotationNavigatorVisible().ch.onReceive(this, [this](bool) {
        m_panelVisibleChanged.send(PanelType::NotationNavigator);
    });

    configuration()->isNotationStatusBarVisible().ch.onReceive(this, [this](bool) {
        m_panelVisibleChanged.send(PanelType::NotationStatusBar);
    });
}

bool NotationPageState::isPanelVisible(PanelType type) const
{
    switch (type) {
    case PanelType::Palette:
    case PanelType::Instruments:
    case PanelType::Inspector:
    case PanelType::NotationToolBar:
    case PanelType::NoteInputBar:
    case PanelType::UndoRedoToolBar:
    case PanelType::PlaybackToolBar:
        return m_panelVisibleMap[type];
    case PanelType::NotationNavigator:
        return configuration()->isNotationNavigatorVisible().val;
    case PanelType::NotationStatusBar:
        return configuration()->isNotationStatusBarVisible().val;
    case PanelType::Mixer:
        NOT_IMPLEMENTED;
        return false;
    }

    return false;
}

void NotationPageState::setIsPanelVisible(PanelType type, bool visible)
{
    switch (type) {
    case PanelType::Palette:
    case PanelType::Instruments:
    case PanelType::Inspector:
    case PanelType::NotationToolBar:
    case PanelType::NoteInputBar:
    case PanelType::UndoRedoToolBar:
    case PanelType::PlaybackToolBar: {
        m_panelVisibleMap[type] = visible;
        m_panelVisibleChanged.send(type);
        break;
    }
    case PanelType::NotationNavigator: {
        configuration()->setIsNotationNavigatorVisible(visible);
        break;
    }
    case PanelType::NotationStatusBar: {
        configuration()->setIsNotationStatusBarVisible(visible);
        break;
    }
    case PanelType::Mixer: {
        NOT_IMPLEMENTED;
        break;
    }
    }
}

mu::async::Channel<PanelType> NotationPageState::panelVisibleChanged() const
{
    return m_panelVisibleChanged;
}
