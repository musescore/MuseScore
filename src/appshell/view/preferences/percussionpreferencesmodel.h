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

#pragma once

#include <QObject>

#include "modularity/ioc.h"
#include "notation/inotationconfiguration.h"

#include "async/asyncable.h"

class PercussionPreferencesModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QVariantList autoShowModes READ autoShowModes CONSTANT)
    Q_PROPERTY(int autoShowMode READ autoShowMode WRITE setAutoShowMode NOTIFY percussionPanelAutoShowModeChanged)
    Q_PROPERTY(bool neverAutoShow READ neverAutoShow NOTIFY percussionPanelAutoShowModeChanged)

    Q_PROPERTY(bool autoClosePercussionPanel READ autoClosePercussionPanel
               WRITE setAutoClosePercussionPanel NOTIFY autoClosePercussionPanelChanged)

    Q_PROPERTY(bool useNewPercussionPanel READ useNewPercussionPanel WRITE setUseNewPercussionPanel NOTIFY useNewPercussionPanelChanged)

    Q_PROPERTY(bool showPercussionPanelPadSwapDialog READ showPercussionPanelPadSwapDialog
               WRITE setShowPercussionPanelPadSwapDialog NOTIFY showPercussionPanelPadSwapDialogChanged)

    Q_PROPERTY(bool percussionPanelMoveMidiNotesAndShortcuts READ percussionPanelMoveMidiNotesAndShortcuts
               WRITE setPercussionPanelMoveMidiNotesAndShortcuts NOTIFY percussionPanelMoveMidiNotesAndShortcutsChanged)

    muse::Inject<mu::notation::INotationConfiguration> configuration = { this };

public:
    explicit PercussionPreferencesModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    QVariantList autoShowModes() const;
    int autoShowMode() const;
    void setAutoShowMode(int mode);

    bool neverAutoShow() const;

    bool autoClosePercussionPanel() const;
    void setAutoClosePercussionPanel(bool autoClose);

    bool useNewPercussionPanel() const;
    void setUseNewPercussionPanel(bool use);

    bool showPercussionPanelPadSwapDialog() const;
    void setShowPercussionPanelPadSwapDialog(bool show);

    bool percussionPanelMoveMidiNotesAndShortcuts() const;
    void setPercussionPanelMoveMidiNotesAndShortcuts(bool move);

signals:
    void useNewPercussionPanelChanged();
    void percussionPanelAutoShowModeChanged();
    void autoClosePercussionPanelChanged();
    void showPercussionPanelPadSwapDialogChanged();
    void percussionPanelMoveMidiNotesAndShortcutsChanged();

private:
    struct AutoShowMode
    {
        mu::notation::PercussionPanelAutoShowMode type = mu::notation::PercussionPanelAutoShowMode::UNPITCHED_STAFF;
        QString title;
        bool checked = false;
    };

    QList<AutoShowMode> allAutoShowModes() const;
};
