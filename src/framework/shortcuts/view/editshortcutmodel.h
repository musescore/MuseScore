/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_SHORTCUTS_EDITSHORTCUTMODEL_H
#define MUSE_SHORTCUTS_EDITSHORTCUTMODEL_H

#include <QObject>
#include <QKeySequence>

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"

class QKeySequence;

namespace muse::shortcuts {
class EditShortcutModel : public QObject, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString originSequence READ originSequenceInNativeFormat NOTIFY originSequenceChanged)
    Q_PROPERTY(QString newSequence READ newSequenceInNativeFormat NOTIFY newSequenceChanged)
    Q_PROPERTY(QString conflictWarning READ conflictWarning NOTIFY newSequenceChanged)

    Q_PROPERTY(bool cleared READ cleared NOTIFY clearedChanged)

    Inject<IInteractive> interactive = { this };

public:
    explicit EditShortcutModel(QObject* parent = nullptr);

    QString originSequenceInNativeFormat() const;
    QString newSequenceInNativeFormat() const;
    QString conflictWarning() const;
    bool cleared() const { return m_cleared; }
    bool isShiftAllowed(Qt::Key key);

    Q_INVOKABLE void load(const QVariant& shortcut, const QVariantList& allShortcuts);
    Q_INVOKABLE void inputKey(Qt::Key key, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void trySave();

signals:
    void originSequenceChanged();
    void newSequenceChanged();
    void clearedChanged();

    void applyNewSequenceRequested(const QString& newSequence, int conflictShortcutIndex = -1);

private:
    void clearNewSequence();

    QString newSequence() const;
    void checkNewSequenceForConflicts();

    QVariantList m_allShortcuts;

    QString m_originSequence;
    QString m_originShortcutTitle;

    QVariantList m_potentialConflictShortcuts;
    QVariantMap m_conflictShortcut;

    QKeySequence m_newSequence;

    bool m_cleared = false;
};
}

#endif // MUSE_SHORTCUTS_EDITSHORTCUTMODEL_H
