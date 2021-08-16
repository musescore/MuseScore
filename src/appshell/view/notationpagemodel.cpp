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
using namespace mu::notation;

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

void NotationPageModel::setPalettesPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Palettes, dockName);
}

void NotationPageModel::setInstrumentsPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Instruments, dockName);
}

void NotationPageModel::setInspectorPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Inspector, dockName);
}

void NotationPageModel::setSelectionFilterPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::SelectionFilter, dockName);
}

void NotationPageModel::setPianoRollDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Piano, dockName);
}

void NotationPageModel::setMixerDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Mixer, dockName);
}

void NotationPageModel::setTimelineDockName(const QString& dockName)
{
    setPanelDockName(PanelType::Timeline, dockName);
}

void NotationPageModel::setDrumsetPanelDockName(const QString& dockName)
{
    setPanelDockName(PanelType::DrumsetPanel, dockName);
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
        initialState[type] = m_window->isDockOpen(m_panelTypeToDockName[type]);
    }

    pageState()->setIsPanelsVisible(initialState);

    static const QMap<std::string, PanelType> actionToPanelType {
        { "toggle-navigator", PanelType::NotationNavigator },
        { "toggle-mixer", PanelType::Mixer },
        { "toggle-piano", PanelType::Piano },
        { "toggle-timeline", PanelType::Timeline },
        { "toggle-palettes", PanelType::Palettes },
        { "toggle-instruments", PanelType::Instruments },
        { "inspector", PanelType::Inspector },
        { "toggle-selection-filter", PanelType::SelectionFilter },
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

    updateDrumsetPanelVisibility();
    globalContext()->currentNotationChanged().onNotify(this, [=]() {
        updateDrumsetPanelVisibility();
    });
}

void NotationPageModel::togglePanel(PanelType type)
{
    IF_ASSERT_FAILED(m_window) {
        return;
    }

    bool visible = pageState()->isPanelVisible(type);
    pageState()->setIsPanelsVisible({ { type, !visible } });

    m_window->toggleDock(m_panelTypeToDockName[type]);
}

void NotationPageModel::updateDrumsetPanelVisibility()
{
    auto setDrumsetPanelVisible = [this](bool visible) {
        m_window->setDockOpen(m_panelTypeToDockName[PanelType::DrumsetPanel], visible);
        pageState()->setIsPanelsVisible({ { PanelType::DrumsetPanel, visible } });
    };

    setDrumsetPanelVisible(false);

    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    INotationNoteInputPtr noteInput = notation->interaction()->noteInput();
    noteInput->stateChanged().onNotify(this, [noteInput, setDrumsetPanelVisible]() {
        bool visible = noteInput->isNoteInputMode() && noteInput->state().drumset != nullptr;
        setDrumsetPanelVisible(visible);
    });
}
