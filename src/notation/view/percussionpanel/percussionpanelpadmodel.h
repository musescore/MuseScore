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

#include "async/asyncable.h"
#include "async/notification.h"

#include "engraving/dom/engravingitem.h"

namespace mu::notation {
class PercussionPanelPadModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString instrumentName READ instrumentName NOTIFY instrumentNameChanged)

    Q_PROPERTY(QString keyboardShortcut READ keyboardShortcut NOTIFY keyboardShortcutChanged)
    Q_PROPERTY(QString midiNote READ midiNote NOTIFY midiNoteChanged)

    Q_PROPERTY(QVariant notationPreviewItem READ notationPreviewItemVariant NOTIFY notationPreviewItemChanged)

    Q_PROPERTY(bool isEmptySlot READ isEmptySlot NOTIFY isEmptySlotChanged)

public:
    explicit PercussionPanelPadModel(QObject* parent = nullptr);

    QString instrumentName() const { return m_instrumentName; }
    void setInstrumentName(const QString& instrumentName);

    QString keyboardShortcut() const { return m_keyboardShortcut; }
    void setKeyboardShortcut(const QString& keyboardShortcut);

    QString midiNote() const { return m_midiNote; }
    void setMidiNote(const QString& midiNote);

    void setNotationPreviewItem(mu::engraving::ElementPtr item);
    mu::engraving::ElementPtr notationPreviewItem() const { return m_notationPreviewItem; }

    const QVariant notationPreviewItemVariant() const;

    bool isEmptySlot() const { return m_isEmptySlot; }
    void setIsEmptySlot(bool isEmptySlot);

    Q_INVOKABLE void triggerPad();
    muse::async::Notification padTriggered() const { return m_triggeredNotification; }

signals:
    void instrumentNameChanged();

    void keyboardShortcutChanged();
    void midiNoteChanged();

    void notationPreviewItemChanged();

    void isEmptySlotChanged();

private:
    QString m_instrumentName;

    QString m_keyboardShortcut;
    QString m_midiNote;

    mu::engraving::ElementPtr m_notationPreviewItem;

    bool m_isEmptySlot = true;

    muse::async::Notification m_triggeredNotification;
};
}
