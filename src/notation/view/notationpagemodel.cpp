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

using namespace mu::notation;

NotationPageModel::NotationPageModel(QObject* parent)
    : QObject(parent)
{
}

void NotationPageModel::init()
{
    configuration()->isPalettePanelVisible().ch.onReceive(this, [this](bool visible) {
        emit isPalettePanelVisibleChanged(visible);
    });

    configuration()->isInstrumentsPanelVisible().ch.onReceive(this, [this](bool visible) {
        emit isInstrumentsPanelVisibleChanged(visible);
    });

    configuration()->isInspectorPanelVisible().ch.onReceive(this, [this](bool visible) {
        emit isInspectorPanelVisibleChanged(visible);
    });

    configuration()->isStatusBarVisible().ch.onReceive(this, [this](bool visible) {
        emit isStatusBarVisibleChanged(visible);
    });

    configuration()->isNoteInputBarVisible().ch.onReceive(this, [this](bool visible) {
        emit isNoteInputBarVisibleChanged(visible);
    });

    configuration()->isNotationToolBarVisible().ch.onReceive(this, [this](bool visible) {
        emit isNotationToolBarVisibleChanged(visible);
    });
}

bool NotationPageModel::isPalettePanelVisible() const
{
    return configuration()->isPalettePanelVisible().val;
}

bool NotationPageModel::isInstrumentsPanelVisible() const
{
    return configuration()->isInstrumentsPanelVisible().val;
}

bool NotationPageModel::isInspectorPanelVisible() const
{
    return configuration()->isInspectorPanelVisible().val;
}

bool NotationPageModel::isStatusBarVisible() const
{
    return configuration()->isStatusBarVisible().val;
}

bool NotationPageModel::isNoteInputBarVisible() const
{
    return configuration()->isNoteInputBarVisible().val;
}

bool NotationPageModel::isNotationToolBarVisible() const
{
    return configuration()->isNotationToolBarVisible().val;
}

void NotationPageModel::setIsPalettePanelVisible(bool visible)
{
    if (isPalettePanelVisible() == visible) {
        return;
    }

    configuration()->setIsPalettePanelVisible(visible);
    emit isPalettePanelVisibleChanged(visible);
}

void NotationPageModel::setIsInstrumentsPanelVisible(bool visible)
{
    if (isInstrumentsPanelVisible() == visible) {
        return;
    }

    configuration()->setIsInstrumentsPanelVisible(visible);
    emit isInstrumentsPanelVisibleChanged(visible);
}

void NotationPageModel::setIsInspectorPanelVisible(bool visible)
{
    if (isInspectorPanelVisible() == visible) {
        return;
    }

    configuration()->setIsInspectorPanelVisible(visible);
    emit isInspectorPanelVisibleChanged(visible);
}

void NotationPageModel::setIsStatusBarVisible(bool visible)
{
    if (isStatusBarVisible() == visible) {
        return;
    }

    configuration()->setIsStatusBarVisible(visible);
    emit isStatusBarVisibleChanged(visible);
}

void NotationPageModel::setIsNoteInputBarVisible(bool visible)
{
    if (isNoteInputBarVisible() == visible) {
        return;
    }

    configuration()->setIsNoteInputBarVisible(visible);
    emit isNoteInputBarVisibleChanged(visible);
}

void NotationPageModel::setIsNotationToolBarVisible(bool visible)
{
    if (isNotationToolBarVisible() == visible) {
        return;
    }

    configuration()->setIsNotationToolBarVisible(visible);
    emit isNotationToolBarVisibleChanged(visible);
}
