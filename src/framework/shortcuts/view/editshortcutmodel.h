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

class QKeySequence;

namespace mu::shortcuts {
class EditShortcutModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString originSequence READ originSequenceInNativeFormat NOTIFY originSequenceChanged)
    Q_PROPERTY(QString inputedSequence READ inputedSequenceInNativeFormat NOTIFY inputedSequenceChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY inputedSequenceChanged)
    Q_PROPERTY(bool canApplyInputedSequence READ canApplyInputedSequence NOTIFY inputedSequenceChanged)

public:
    explicit EditShortcutModel(QObject* parent = nullptr);

    QString originSequenceInNativeFormat() const;
    QString inputedSequenceInNativeFormat() const;
    QString errorMessage() const;
    bool canApplyInputedSequence() const;

    Q_INVOKABLE void load(const QVariant& shortcut, const QVariantList& allShortcuts);
    Q_INVOKABLE void clear();

    Q_INVOKABLE void inputKey(int key, Qt::KeyboardModifiers modifiers);

    Q_INVOKABLE void addToOriginSequence();
    Q_INVOKABLE void replaceOriginSequence();

signals:
    void allShortcutsChanged(const QVariantList& shortcuts);
    void originSequenceChanged(const QString& sequence);
    void inputedSequenceChanged(const QString& sequence);

    void applyNewSequenceRequested(const QString& newSequence);

private:
    QString inputedSequence() const;

    void validateInputedSequence();

    QVariantList m_potentialConflictShortcuts;
    QKeySequence m_inputedSequence;
    QString m_originSequence;
    QString m_errorMessage;
};
}

#endif // MU_SHORTCUTS_EDITSHORTCUTMODEL_H
