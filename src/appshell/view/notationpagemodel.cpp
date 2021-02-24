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
    pageState()->panelVisibleChanged().onReceive(this, [this](PanelType type) {
        notifyAboutPanelChanged(type);
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

    pageState()->setIsPanelVisible(PanelType::Palette, visible);
    emit isPalettePanelVisibleChanged();
}

void NotationPageModel::setIsInstrumentsPanelVisible(bool visible)
{
    if (isInstrumentsPanelVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::Instruments, visible);
    emit isInstrumentsPanelVisibleChanged();
}

void NotationPageModel::setIsInspectorPanelVisible(bool visible)
{
    if (isInspectorPanelVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::Inspector, visible);
    emit isInspectorPanelVisibleChanged();
}

void NotationPageModel::setIsStatusBarVisible(bool visible)
{
    if (isStatusBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::NotationStatusBar, visible);
    emit isStatusBarVisibleChanged();
}

void NotationPageModel::setIsNoteInputBarVisible(bool visible)
{
    if (isNoteInputBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::NoteInputBar, visible);
    emit isNoteInputBarVisibleChanged();
}

void NotationPageModel::setIsNotationToolBarVisible(bool visible)
{
    if (isNotationToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::NotationToolBar, visible);
    emit isNotationToolBarVisibleChanged();
}

void NotationPageModel::setIsPlaybackToolBarVisible(bool visible)
{
    if (isPlaybackToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::PlaybackToolBar, visible);
    emit isPlaybackToolBarVisibleChanged();
}

void NotationPageModel::setIsUndoRedoToolBarVisible(bool visible)
{
    if (isUndoRedoToolBarVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::UndoRedoToolBar, visible);
    emit isUndoRedoToolBarVisibleChanged();
}

void NotationPageModel::setIsNotationNavigatorVisible(bool visible)
{
    if (isNotationNavigatorVisible() == visible) {
        return;
    }

    pageState()->setIsPanelVisible(PanelType::NotationNavigator, visible);
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
    case PanelType::Mixer:
        NOT_IMPLEMENTED;
        break;
    }
}

void NotationPageModel::togglePanel(PanelType type)
{
    bool visible = pageState()->isPanelVisible(type);
    pageState()->setIsPanelVisible(type, !visible);
}
