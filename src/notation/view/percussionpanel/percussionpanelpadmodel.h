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

#include "global/utils.h"

#include "async/asyncable.h"
#include "async/notification.h"

#include "engraving/dom/engravingitem.h"

namespace mu::notation {
class PercussionPanelPadModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString padName READ padName NOTIFY padNameChanged)

    Q_PROPERTY(QString keyboardShortcut READ keyboardShortcut NOTIFY keyboardShortcutChanged)
    Q_PROPERTY(QString midiNote READ midiNote NOTIFY midiNoteChanged)

    Q_PROPERTY(QVariant notationPreviewItem READ notationPreviewItemVariant NOTIFY notationPreviewItemChanged)

    Q_PROPERTY(QList<QVariantMap> contextMenuItems READ contextMenuItems CONSTANT)

public:
    explicit PercussionPanelPadModel(QObject* parent = nullptr);

    QString padName() const { return m_padName; }
    void setPadName(const QString& padName);

    QString keyboardShortcut() const { return m_keyboardShortcut; }
    void setKeyboardShortcut(const QString& keyboardShortcut);

    int pitch() const { return m_pitch; }
    void setPitch(int pitch);

    QString midiNote() const { return QString::fromStdString(muse::pitchToString(m_pitch)); }

    void setNotationPreviewItem(mu::engraving::ElementPtr item);
    mu::engraving::ElementPtr notationPreviewItem() const { return m_notationPreviewItem; }

    const QVariant notationPreviewItemVariant() const;

    QList<QVariantMap> contextMenuItems() const;
    Q_INVOKABLE void handleMenuItem(const QString& itemId);

    Q_INVOKABLE void triggerPad(const Qt::KeyboardModifiers& modifiers);

    enum class PadAction {
        TRIGGER_STANDARD,
        TRIGGER_ADD,
        TRIGGER_INSERT,
        DUPLICATE,
        DELETE,
        DEFINE_SHORTCUT,
    };

    muse::async::Channel<PadAction> padActionTriggered() const { return m_padActionTriggered; }

signals:
    void padNameChanged();

    void keyboardShortcutChanged();
    void midiNoteChanged();

    void notationPreviewItemChanged();

private:
    QString m_padName;

    QString m_keyboardShortcut;
    int m_pitch = -1;

    mu::engraving::ElementPtr m_notationPreviewItem;

    muse::async::Channel<PadAction> m_padActionTriggered;
};
}
