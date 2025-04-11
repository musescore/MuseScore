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

#include "modularity/ioc.h"
#include "iinteractive.h"

class QKeySequence;

namespace muse::shortcuts {
class EditShortcutModel : public QObject, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(QObject * window READ window WRITE setWindow)
    Q_PROPERTY(QString originSequence READ originSequenceInNativeFormat NOTIFY originSequenceChanged)
    Q_PROPERTY(QString newSequence READ newSequenceInNativeFormat NOTIFY newSequenceChanged)
    Q_PROPERTY(QString conflictWarning READ conflictWarning NOTIFY newSequenceChanged)
    Q_PROPERTY(QString alternatives READ alternatives NOTIFY newSequenceChanged)

    Q_PROPERTY(bool cleared READ cleared NOTIFY clearedChanged)

    Inject<IInteractive> interactive = { this };

public:
    QObject* window() const { return m_window; }
    void setWindow(QObject* window);
    explicit EditShortcutModel(QObject* parent = nullptr);

    QString originSequenceInNativeFormat() const;
    QString newSequenceInNativeFormat() const;
    QString conflictWarning() const;
    QString alternatives() const;
    bool cleared() const { return m_cleared; }

    Q_INVOKABLE void load(const QVariant& shortcut, const QVariantList& allShortcuts);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void trySave();
    Q_INVOKABLE void editShortcut(const QString& shortcutText);
    Q_INVOKABLE void newShortcutFieldFocusChanged(bool focused);
    Q_INVOKABLE void currentShortcutAcceptInProgress();

signals:
    void originSequenceChanged();
    void newSequenceChanged();
    void clearedChanged();

    void applyNewSequenceRequested(const QString& newSequence, int conflictShortcutIndex = -1);

private:
    void inputKey(QKeyEvent* keyEvent);
    bool eventFilter(QObject* watched, QEvent* event) override;
    void clearNewSequence();

    QString newSequence() const;
    void checkNewSequenceForConflicts(QKeySequence newSequence);

    QObject* m_window = nullptr;
    QVariantList m_allShortcuts;

    QString m_originSequence;
    QString m_originShortcutTitle;

    QVariantList m_potentialConflictShortcuts;
    QVariantMap m_conflictShortcut;

    std::vector<QKeySequence> m_newSequences;

    bool m_cleared = false;
    bool m_newShortcutFieldFocused = false;
    bool m_currentShortcutAcceptInProgress = false;
    int m_lastPressedShortcut = -1;

    QString m_alternatives;
};
}

#endif // MUSE_SHORTCUTS_EDITSHORTCUTMODEL_H
