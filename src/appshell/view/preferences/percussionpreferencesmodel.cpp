/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "percussionpreferencesmodel.h"

PercussionPreferencesModel::PercussionPreferencesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void PercussionPreferencesModel::init()
{
    configuration()->useNewPercussionPanelChanged().onNotify(this, [this]() {
        emit useNewPercussionPanelChanged();
    });

    configuration()->autoShowPercussionPanelChanged().onNotify(this, [this]() {
        emit autoShowPercussionPanelChanged();
    });

    configuration()->showPercussionPanelPadSwapDialogChanged().onNotify(this, [this]() {
        emit showPercussionPanelPadSwapDialogChanged();
    });

    configuration()->percussionPanelMoveMidiNotesAndShortcutsChanged().onNotify(this, [this]() {
        emit percussionPanelMoveMidiNotesAndShortcutsChanged();
    });
}

bool PercussionPreferencesModel::useNewPercussionPanel() const
{
    return configuration()->useNewPercussionPanel();
}

void PercussionPreferencesModel::setUseNewPercussionPanel(bool use)
{
    configuration()->setUseNewPercussionPanel(use);
}

bool PercussionPreferencesModel::autoShowPercussionPanel() const
{
    return configuration()->autoShowPercussionPanel();
}

void PercussionPreferencesModel::setAutoShowPercussionPanel(bool autoShow)
{
    configuration()->setAutoShowPercussionPanel(autoShow);
}

bool PercussionPreferencesModel::showPercussionPanelPadSwapDialog() const
{
    return configuration()->showPercussionPanelPadSwapDialog();
}

void PercussionPreferencesModel::setShowPercussionPanelPadSwapDialog(bool show)
{
    configuration()->setShowPercussionPanelPadSwapDialog(show);
}

bool PercussionPreferencesModel::percussionPanelMoveMidiNotesAndShortcuts() const
{
    return configuration()->percussionPanelMoveMidiNotesAndShortcuts();
}

void PercussionPreferencesModel::setPercussionPanelMoveMidiNotesAndShortcuts(bool move)
{
    configuration()->setPercussionPanelMoveMidiNotesAndShortcuts(move);
}
