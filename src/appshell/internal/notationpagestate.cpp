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
#include "notationpagestate.h"

using namespace mu::appshell;

void NotationPageState::init()
{
    configuration()->isNotationNavigatorVisibleChanged().onNotify(this, [this]() {
        m_panelsVisibleChanged.send({ PanelType::NotationNavigator });
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
    case PanelType::NotationStatusBar:
    case PanelType::Mixer:
        return m_panelVisibleMap[type];
    case PanelType::NotationNavigator:
        return configuration()->isNotationNavigatorVisible();
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
    case PanelType::NotationStatusBar:
    case PanelType::Mixer:
        m_panelVisibleMap[type] = visible;
        break;
    }
    case PanelType::NotationNavigator: {
        configuration()->setIsNotationNavigatorVisible(visible);
        break;
    }
    default: {
        break;
    }
    }
}
