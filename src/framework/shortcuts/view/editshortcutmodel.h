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
#ifndef MU_SHORTCUTS_EDITSHORTCUTMODEL_H
#define MU_SHORTCUTS_EDITSHORTCUTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "ishortcutsregister.h"
#include "ui/iuiactionsregister.h"

class QKeySequence;

namespace mu::shortcuts {
class EditShortcutModel : public QObject
{
    Q_OBJECT

    INJECT(shortcuts, framework::IInteractive, interactive)
    INJECT(shortcuts, IShortcutsRegister, shortcutsRegister)
    INJECT(shortcuts, ui::IUiActionsRegister, uiactionsRegister)

    Q_PROPERTY(QString originSequence READ originSequenceInNativeFormat NOTIFY originSequenceChanged)
    Q_PROPERTY(QString originAction READ originAction)
    Q_PROPERTY(QString newSequence READ newSequenceInNativeFormat NOTIFY newSequenceChanged)
    Q_PROPERTY(QString conflictWarning READ conflictWarning NOTIFY newSequenceChanged)

public:
    explicit EditShortcutModel(QObject* parent = nullptr);

    QString originSequenceInNativeFormat() const;
    QString newSequenceInNativeFormat() const;
    QString originAction() const;
    QString conflictWarning() const;

    Q_INVOKABLE void load(const QVariant& shortcut, const QVariantList& allShortcuts = QVariantList());
    Q_INVOKABLE void loadByAction(const QString& actionCode);
    Q_INVOKABLE void inputKey(int key, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE void applyNewSequence();
    Q_INVOKABLE void clearConflicts();
    Q_INVOKABLE void setDirectReplace(bool val = true);

signals:
    void originSequenceChanged();
    void newSequenceChanged();

    void applyNewSequenceRequested(const QString& newSequence, int conflictShortcutIndex = -1);

private:
    void clearNewSequence();

    QString newSequence() const;
    void checkNewSequenceForConflicts();

    bool directReplace = false;
    QVariantList m_allShortcuts;

    QString m_originSequence;
    QString m_originAction;
    QString m_originContext;
    QString m_originShortcutTitle;

    QVariantList m_potentialConflictShortcuts;
    QVariantMap m_conflictShortcut;

    QKeySequence m_newSequence;
};
}

#endif // MU_SHORTCUTS_EDITSHORTCUTMODEL_H
