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
#include "styleitem.h"

using namespace mu::notation;

StyleItem::StyleItem(QObject* parent, const QVariant& value, const QVariant& defaultValue)
    : QObject(parent), m_value(value), m_defaultValue(defaultValue)
{
}

QVariant StyleItem::value() const
{
    return m_value;
}

bool StyleItem::setValue(const QVariant& value)
{
    if (value == m_value) {
        return false;
    }

    m_value = value;
    emit valueChanged();
    return true;
}

void StyleItem::modifyValue(const QVariant& value)
{
    if (setValue(value)) {
        emit valueModified(value);
    }
}

QVariant StyleItem::defaultValue() const
{
    return m_defaultValue;
}

bool StyleItem::isDefault() const
{
    return m_value == m_defaultValue;
}

void StyleItem::reset()
{
    modifyValue(m_defaultValue);
}
