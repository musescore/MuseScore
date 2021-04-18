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

using namespace mu::appshell;

void NotationPageState::init()
{
    configuration()->isNotationNavigatorVisible().ch.onReceive(this, [this](bool) {
        m_panelsVisibleChanged.send({ PanelType::NotationNavigator });
    });

    configuration()->isNotationStatusBarVisible().ch.onReceive(this, [this](bool) {
        m_panelsVisibleChanged.send({ PanelType::NotationStatusBar });
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
    case PanelType::TimeLine:
    case PanelType::Synthesizer:
    case PanelType::SelectionFilter:
    case PanelType::Piano:
    case PanelType::ComparisonTool:
    case PanelType::Undefined:
        return false;
    }

    return false;
}

void NotationPageState::setIsPanelsVisible(const std::map<PanelType, bool>& panelsVisible)
{
    PanelTypeList changedTypes;
    for (const auto& type: panelsVisible) {
        setIsPanelVisible(type.first, type.second);
        changedTypes.push_back(type.first);
    }

    m_panelsVisibleChanged.send(changedTypes);
}

mu::async::Channel<PanelTypeList> NotationPageState::panelsVisibleChanged() const
{
    return m_panelsVisibleChanged;
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
    default: {
        break;
    }
    }
}
