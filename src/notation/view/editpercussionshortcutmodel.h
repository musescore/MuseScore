/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include <QKeySequence>

#include "modularity/ioc.h"
#include "iinteractive.h"

namespace mu::notation {
class EditPercussionShortcutModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(QString originShortcutText READ originShortcutText NOTIFY originShortcutTextChanged)
    Q_PROPERTY(QString newShortcutText READ newShortcutText NOTIFY newShortcutTextChanged)
    Q_PROPERTY(QString informationText READ informationText NOTIFY newShortcutTextChanged)

    Q_PROPERTY(bool cleared READ cleared NOTIFY clearedChanged)

    Inject<muse::IInteractive> interactive = { this };

public:
    explicit EditPercussionShortcutModel(QObject* parent = nullptr);

    Q_INVOKABLE void load(const QVariant& originDrum, const QVariantList& drumsWithShortcut, const QVariantList& applicationShortcuts);
    Q_INVOKABLE void inputKey(Qt::Key key);
    Q_INVOKABLE void clear();
    Q_INVOKABLE bool trySave();

    Q_INVOKABLE int conflictDrumPitch() const;

    QString originShortcutText() const;
    QString newShortcutText() const;
    QString informationText() const;

    bool cleared() const { return m_cleared; }

signals:
    void originShortcutTextChanged();
    void newShortcutTextChanged();
    void clearedChanged();

private:
    bool checkDrumShortcutsForConflict();
    bool checkApplicationShortcutsForConflict();

    QString getConflictWarningText() const;

    bool needIgnoreKey(const Qt::Key& key) const;

    QVariantMap m_originDrum;

    QKeySequence m_newShortcut;
    QVariantMap m_conflictShortcut;

    QVariantList m_drumsWithShortcut;
    QVariantList m_applicationShortcuts;

    bool m_conflictInAppShortcuts = false;
    bool m_cleared = false;
};
}
