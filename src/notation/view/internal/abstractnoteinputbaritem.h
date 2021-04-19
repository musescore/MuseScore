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
#ifndef MU_NOTATION_ABSTRACTNOTEINPUTBARITEM_H
#define MU_NOTATION_ABSTRACTNOTEINPUTBARITEM_H

#include <QObject>

namespace mu::notation {
class AbstractNoteInputBarItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)

public:
    enum ItemType {
        UNDEFINED = -1,
        ACTION,
        SEPARATOR
    };
    Q_ENUMS(ItemType)

    explicit AbstractNoteInputBarItem(QObject* parent = nullptr);
    explicit AbstractNoteInputBarItem(const ItemType& type, QObject* parent = nullptr);

    Q_INVOKABLE QString id() const;
    void setId(const QString& id);

    QString title() const;
    int type() const;

public slots:
    void setType(const ItemType type);
    void setTitle(QString title);

signals:
    void typeChanged(ItemType type);
    void titleChanged(QString title);

private:
    QString m_id;
    QString m_title;
    ItemType m_type = ItemType::UNDEFINED;
};
}

#endif // MU_NOTATION_ABSTRACTNOTEINPUTBARITEM_H
