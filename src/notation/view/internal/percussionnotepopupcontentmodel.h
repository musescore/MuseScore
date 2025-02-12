/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include "context/iglobalcontext.h"

#include "async/asyncable.h"

namespace mu::notation {
class PercussionNotePopupContentModel : public QObject, public muse::Injectable, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(bool shouldShowButtons READ shouldShowButtons NOTIFY shouldShowButtonsChanged)
    Q_PROPERTY(QString percussionNoteName READ percussionNoteName NOTIFY percussionNoteNameChanged)
    Q_PROPERTY(QString keyboardShortcut READ keyboardShortcut NOTIFY keyboardShortcutChanged)

    muse::Inject<mu::context::IGlobalContext> globalContext = { this };

public:
    explicit PercussionNotePopupContentModel(QObject* parent = nullptr);

    Q_INVOKABLE void init();

    Q_INVOKABLE void prevDrumNote();
    Q_INVOKABLE void nextDrumNote();

    bool shouldShowButtons() const;
    QString percussionNoteName() const;
    QString keyboardShortcut() const;

signals:
    void shouldShowButtonsChanged();
    void percussionNoteNameChanged();
    void keyboardShortcutChanged();

private:
    INotationInteractionPtr interaction() const;
    INotationNoteInputPtr noteInput() const;

    const mu::engraving::ShadowNote* currentShadowNote() const;
    const Drumset* currentDrumset() const;

    int currentDrumPitch() const;
};
}
