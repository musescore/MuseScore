/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_NOTATION_STYLEITEM_H
#define MU_NOTATION_STYLEITEM_H

#include "notationtypes.h"

namespace mu::notation {
class StyleItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE modifyValue NOTIFY valueChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)

    Q_PROPERTY(bool isDefault READ isDefault NOTIFY valueChanged)

public:
    explicit StyleItem(QObject* parent, const QVariant& value, const QVariant& defaultValue);

    QVariant value() const;
    bool setValue(const QVariant& value); // C++ -> QML
    void modifyValue(const QVariant& value); // QML -> C++

    QVariant defaultValue() const;

    bool isDefault() const;

    void reset();

signals:
    void valueChanged(); // C++ -> QML
    void valueModified(const QVariant& newValue); // QML -> C++

private:
    QVariant m_value;
    QVariant m_defaultValue;
};
}

#endif // MU_NOTATION_STYLEITEM_H
