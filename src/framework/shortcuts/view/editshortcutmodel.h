//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_SHORTCUTS_EDITSHORTCUTMODEL_H
#define MU_SHORTCUTS_EDITSHORTCUTMODEL_H

#include <QObject>

#include "modularity/ioc.h"

namespace mu::shortcuts {
class EditShortcutModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString originSequence READ originSequence NOTIFY originSequenceChanged)
    Q_PROPERTY(QString inputedSequence READ inputedSequence NOTIFY inputedSequenceChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY inputedSequenceChanged)
    Q_PROPERTY(bool canApplySequence READ canApplySequence NOTIFY inputedSequenceChanged)

public:
    explicit EditShortcutModel(QObject* parent = nullptr);

    QString originSequence() const;
    QString inputedSequence() const;
    QString errorMessage() const;
    bool canApplySequence() const;

    Q_INVOKABLE void load(const QString& sequence, const QVariantList& allShortcuts);
    Q_INVOKABLE void clear();

    Q_INVOKABLE void inputKey(int key, Qt::KeyboardModifiers modifiers);
    Q_INVOKABLE QString unitedSequence() const;

signals:
    void allShortcutsChanged(const QVariantList& shortcuts);
    void originSequenceChanged(const QString& sequence);
    void inputedSequenceChanged(const QString& sequence);

private:
    bool needIgnoreKey(int key) const;
    void validateInputedSequence();

    QVariantList m_allShortcuts;
    QKeySequence m_inputedSequence;
    QString m_originSequence;
    QString m_errorMessage;
};
}

#endif // MU_SHORTCUTS_EDITSHORTCUTMODEL_H
