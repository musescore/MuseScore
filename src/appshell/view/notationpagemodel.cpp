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
#include "notationpagemodel.h"

#include "log.h"

using namespace mu::appshell;

NotationPageModel::NotationPageModel(QObject* parent)
    : QObject(parent)
{
}

void NotationPageModel::init()
{
    pageState()->panelsVisibleChanged().onReceive(this, [this](const PanelTypeList& panels) {
        for (PanelType panelType: panels) {
            notifyAboutPanelChanged(panelType);
        }
    });

    dispatcher()->reg(this, "toggle-navigator", [this]() { togglePanel(PanelType::NotationNavigator); });
    dispatcher()->reg(this, "toggle-mixer", [this]() { togglePanel(PanelType::Mixer); });
    dispatcher()->reg(this, "toggle-palette", [this]() { togglePanel(PanelType::Palette); });
    dispatcher()->reg(this, "toggle-instruments", [this]() { togglePanel(PanelType::Instruments); });
    dispatcher()->reg(this, "inspector", [this]() { togglePanel(PanelType::Inspector); });
    dispatcher()->reg(this, "toggle-statusbar", [this]() { togglePanel(PanelType::NotationStatusBar); });
    dispatcher()->reg(this, "toggle-noteinput", [this]() { togglePanel(PanelType::NoteInputBar); });
    dispatcher()->reg(this, "toggle-notationtoolbar", [this]() { togglePanel(PanelType::NotationToolBar); });
    dispatcher()->reg(this, "toggle-undoredo", [this]() { togglePanel(PanelType::UndoRedoToolBar); });
    dispatcher()->reg(this, "toggle-transport", [this]() { togglePanel(PanelType::PlaybackToolBar); });
}

void NotationPageModel::setPanelsState(const QVariantList& states)
{
    std::map<PanelType, bool> panelsState;
    for (const QVariant& state: states) {
        QMap<QString, QVariant> stateMap = state.toMap();
        for (const QString& key: stateMap.keys()) {
            PanelType panelType = panelTypeFromString(key);
            panelsState[panelType] = stateMap.value(key).toBool();
        }
    }

    pageState()->setIsPanelsVisible(panelsState);
}

bool NotationPageModel::isPalettePanelVisible() const
{
    return pageState()->isPanelVisible(PanelType::Palette);
}

bool NotationPageModel::isInstrumentsPanelVisible() const
{
    return pageState()->isPanelVisible(PanelType::Instruments);
}

bool NotationPageModel::isInspectorPanelVisible() const
{
    return pageState()->isPanelVisible(PanelType::Inspector);
}

bool NotationPageModel::isStatusBarVisible() const
{
    return pageState()->isPanelVisible(PanelType::NotationStatusBar);
}

bool NotationPageModel::isNoteInputBarVisible() const
{
    return pageState()->isPanelVisible(PanelType::NoteInputBar);
}

bool NotationPageModel::isNotationToolBarVisible() const
{
    return pageState()->isPanelVisible(PanelType::NotationToolBar);
}

bool NotationPageModel::isPlaybackToolBarVisible() const
{
    return pageState()->isPanelVisible(PanelType::PlaybackToolBar);
}

bool NotationPageModel::isUndoRedoToolBarVisible() const
{
    return pageState()->isPanelVisible(PanelType::UndoRedoToolBar);
}

bool NotationPageModel::isNotationNavigatorVisible() const
{
    return pageState()->isPanelVisible(PanelType::NotationNavigator);
}

void NotationPageModel::setIsPalettePanelVisible(bool visible)
{
    if (isPalettePanelVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::Palette, visible } });
    emit isPalettePanelVisibleChanged();
}

void NotationPageModel::setIsInstrumentsPanelVisible(bool visible)
{
    if (isInstrumentsPanelVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::Instruments, visible } });
    emit isInstrumentsPanelVisibleChanged();
}

void NotationPageModel::setIsInspectorPanelVisible(bool visible)
{
    if (isInspectorPanelVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::Inspector, visible } });
    emit isInspectorPanelVisibleChanged();
}

void NotationPageModel::setIsStatusBarVisible(bool visible)
{
    if (isStatusBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::NotationStatusBar, visible } });
    emit isStatusBarVisibleChanged();
}

void NotationPageModel::setIsNoteInputBarVisible(bool visible)
{
    if (isNoteInputBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::NoteInputBar, visible } });
    emit isNoteInputBarVisibleChanged();
}

void NotationPageModel::setIsNotationToolBarVisible(bool visible)
{
    if (isNotationToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::NotationToolBar, visible } });
    emit isNotationToolBarVisibleChanged();
}

void NotationPageModel::setIsPlaybackToolBarVisible(bool visible)
{
    if (isPlaybackToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::PlaybackToolBar, visible } });
    emit isPlaybackToolBarVisibleChanged();
}

void NotationPageModel::setIsUndoRedoToolBarVisible(bool visible)
{
    if (isUndoRedoToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::UndoRedoToolBar, visible } });
    emit isUndoRedoToolBarVisibleChanged();
}

void NotationPageModel::setIsNotationNavigatorVisible(bool visible)
{
    if (isNotationNavigatorVisible() == visible) {
        return;
    }

    pageState()->setIsPanelsVisible({ { PanelType::NotationNavigator, visible } });
    emit isNotationNavigatorVisibleChanged();
}

void NotationPageModel::notifyAboutPanelChanged(PanelType type)
{
    switch (type) {
    case PanelType::Palette:
        emit isPalettePanelVisibleChanged();
        break;
    case PanelType::Instruments:
        emit isInstrumentsPanelVisibleChanged();
        break;
    case PanelType::Inspector:
        emit isInspectorPanelVisibleChanged();
        break;
    case PanelType::NotationStatusBar:
        emit isStatusBarVisibleChanged();
        break;
    case PanelType::NoteInputBar:
        emit isNoteInputBarVisibleChanged();
        break;
    case PanelType::NotationToolBar:
        emit isNotationToolBarVisibleChanged();
        break;
    case PanelType::PlaybackToolBar:
        emit isPlaybackToolBarVisibleChanged();
        break;
    case PanelType::UndoRedoToolBar:
        emit isUndoRedoToolBarVisibleChanged();
        break;
    case PanelType::NotationNavigator:
        emit isNotationNavigatorVisibleChanged();
        break;
    default:
        NOT_IMPLEMENTED;
        break;
    }
}

void NotationPageModel::togglePanel(PanelType type)
{
    bool visible = pageState()->isPanelVisible(type);
    pageState()->setIsPanelsVisible({ { type, !visible } });
}

PanelType NotationPageModel::panelTypeFromString(const QString& string) const
{
    std::map<QString, PanelType> types = {
        { "Palette", PanelType::Palette },
        { "Instruments", PanelType::Instruments },
        { "Inspector", PanelType::Inspector },
        { "StatusBar", PanelType::NotationStatusBar },
        { "NoteInputBar", PanelType::NoteInputBar },
        { "NotationToolBar", PanelType::NotationToolBar },
        { "PlaybackToolBar", PanelType::PlaybackToolBar },
        { "UndoRedoToolBar", PanelType::UndoRedoToolBar },
        { "NotationNavigator", PanelType::NotationNavigator }
    };

    return types[string];
}
