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

using namespace mu::notation;

PercussionPreferencesModel::PercussionPreferencesModel(QObject* parent)
    : QObject(parent), muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void PercussionPreferencesModel::init()
{
    configuration()->useNewPercussionPanelChanged().onNotify(this, [this]() {
        emit useNewPercussionPanelChanged();
    });

    configuration()->percussionPanelAutoShowModeChanged().onNotify(this, [this]() {
        emit percussionPanelAutoShowModeChanged();
    });

    configuration()->autoClosePercussionPanelChanged().onNotify(this, [this]() {
        emit autoClosePercussionPanelChanged();
    });

    configuration()->showPercussionPanelPadSwapDialogChanged().onNotify(this, [this]() {
        emit showPercussionPanelPadSwapDialogChanged();
    });

    configuration()->percussionPanelMoveMidiNotesAndShortcutsChanged().onNotify(this, [this]() {
        emit percussionPanelMoveMidiNotesAndShortcutsChanged();
    });
}

QVariantList PercussionPreferencesModel::autoShowModes() const
{
    static const QVariantList modes {
        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/percussion", "When an unpitched percussion staff is selected") },
            { QStringLiteral("value"), int(PercussionPanelAutoShowMode::UNPITCHED_STAFF) }
        },

        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/percussion", "When inputting notation on an unpitched percussion staff") },
            { QStringLiteral("value"), int(PercussionPanelAutoShowMode::UNPITCHED_STAFF_NOTE_INPUT) }
        },

        QVariantMap {
            { QStringLiteral("title"), muse::qtrc("notation/percussion", "Never") },
            { QStringLiteral("value"), int(PercussionPanelAutoShowMode::NEVER) }
        },
    };

    return modes;
}

int PercussionPreferencesModel::autoShowMode() const
{
    return static_cast<int>(configuration()->percussionPanelAutoShowMode());
}

bool PercussionPreferencesModel::neverAutoShow() const
{
    return configuration()->percussionPanelAutoShowMode() == PercussionPanelAutoShowMode::NEVER;
}

void PercussionPreferencesModel::setAutoShowMode(int mode)
{
    if (mode == autoShowMode()) {
        return;
    }

    configuration()->setPercussionPanelAutoShowMode(static_cast<PercussionPanelAutoShowMode>(mode));
}

bool PercussionPreferencesModel::autoClosePercussionPanel() const
{
    return configuration()->autoClosePercussionPanel();
}

void PercussionPreferencesModel::setAutoClosePercussionPanel(bool autoClose)
{
    configuration()->setAutoClosePercussionPanel(autoClose);
}

bool PercussionPreferencesModel::useNewPercussionPanel() const
{
    return configuration()->useNewPercussionPanel();
}

void PercussionPreferencesModel::setUseNewPercussionPanel(bool use)
{
    configuration()->setUseNewPercussionPanel(use);
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

QList<PercussionPreferencesModel::AutoShowMode> PercussionPreferencesModel::allAutoShowModes() const
{
    const QMap<PercussionPanelAutoShowMode, QString> modeTitles {
        { PercussionPanelAutoShowMode::UNPITCHED_STAFF,
          muse::qtrc("notation/percussion", "When an unpitched percussion staff is selected") },

        { PercussionPanelAutoShowMode::UNPITCHED_STAFF_NOTE_INPUT,
          muse::qtrc("notation/percussion", "When inputting notation on an unpitched percussion staff") },

        { PercussionPanelAutoShowMode::NEVER,
          muse::qtrc("notation/percussion", "Never") },
    };

    const PercussionPanelAutoShowMode currentMode = configuration()->percussionPanelAutoShowMode();

    QList<AutoShowMode> modes;

    for (PercussionPanelAutoShowMode type : modeTitles.keys()) {
        AutoShowMode mode;
        mode.type = type;
        mode.title = modeTitles[type];
        mode.checked = currentMode == type;

        modes << mode;
    }

    return modes;
}
