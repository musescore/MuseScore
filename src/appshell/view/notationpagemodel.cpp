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
#include "notationpagemodel.h"

#include "log.h"

#include "dockwindow/dockwindow.h"

using namespace mu::appshell;

NotationPageModel::NotationPageModel(QObject* parent)
    : QObject(parent)
{
}

bool NotationPageModel::isNavigatorVisible() const
{
    return pageState()->isPanelVisible(PanelType::NotationNavigator);
}

void NotationPageModel::setNoteInputBarDockName(const QString& dockName)
{
    setPanelDockName(PanelType::NoteInputBar, dockName);
}

void NotationPageModel::setNotationToolBarDockName(const QString& dockName)
{
    setPanelDockName(PanelType::NotationToolBar, dockName);
}

void NotationPageModel::setPlaybackToolBarDockName(const QString& dockName)
{
    setPanelDockName(PanelType::PlaybackToolBar, dockName);
}

void NotationPageModel::setUndoRedoToolBarDockName(const QString& dockName)
{
    setPanelDockName(PanelType::UndoRedoToolBar, dockName);
}

void NotationPageModel::setPalettePanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Palette, dockName);
}

void NotationPageModel::setInstrumentsPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Instruments, dockName);
}

void NotationPageModel::setInspectorPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Inspector, dockName);
}

void NotationPageModel::setPianoRollDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Piano, dockName);
}

void NotationPageModel::setMixerDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Mixer, dockName);
}

void NotationPageModel::setStatusBarDockName(const QString& dockName)
{
    setPanelDockName(PanelType::NotationStatusBar, dockName);
}

void NotationPageModel::setPanelDockName(PanelType type, const QString& dockName)
{
    m_panelTypeToDockName[type] = dockName;
}

void NotationPageModel::init(QQuickItem* dockWindow)
{
    m_window = dynamic_cast<dock::DockWindow*>(dockWindow);
    IF_ASSERT_FAILED(m_window) {
        return;
    }

    IF_ASSERT_FAILED(!m_panelTypeToDockName.isEmpty()) {
        return;
    }

    std::map<PanelType, bool> initialState;
    for (PanelType type : m_panelTypeToDockName.keys()) {
        initialState[type] = m_window->isDockShown(m_panelTypeToDockName[type]);
    }

    pageState()->setIsPanelsVisible(initialState);

    static const QMap<std::string, PanelType> actionToPanelType {
        { "toggle-navigator", PanelType::NotationNavigator },
        { "toggle-mixer", PanelType::Mixer },
        { "toggle-palette", PanelType::Palette },
        { "toggle-instruments", PanelType::Instruments },
        { "inspector", PanelType::Inspector },
        { "toggle-statusbar", PanelType::NotationStatusBar },
        { "toggle-noteinput", PanelType::NoteInputBar },
        { "toggle-notationtoolbar", PanelType::NotationToolBar },
        { "toggle-undoredo", PanelType::UndoRedoToolBar },
        { "toggle-transport", PanelType::PlaybackToolBar }
    };

    for (const std::string& actionCode : actionToPanelType.keys()) {
        dispatcher()->reg(this, actionCode, [=]() { togglePanel(actionToPanelType[actionCode]); });
    }

    pageState()->panelsVisibleChanged().onReceive(this, [this](PanelTypeList types) {
        for (PanelType type : types) {
            if (type == PanelType::NotationNavigator) {
                emit isNavigatorVisibleChanged();
            }
        }
    });
}

void NotationPageModel::togglePanel(PanelType type)
{
    IF_ASSERT_FAILED(m_window) {
        return;
    }

    bool visible = pageState()->isPanelVisible(type);
    pageState()->setIsPanelsVisible({ { type, !visible } });

    m_window->toggleDockVisibility(m_panelTypeToDockName[type]);
}
