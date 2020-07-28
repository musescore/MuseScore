//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
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
#include "filtervalue.h"

using namespace mu::framework;

FilterValue::FilterValue(QObject* parent)
    : QObject(parent)
{
}

QString FilterValue::roleName() const
{
    return m_roleName;
}

QVariant FilterValue::roleValue() const
{
    return m_roleValue;
}

QVariant FilterValue::compareType() const
{
    return m_compareType;
}

void FilterValue::setRoleName(QString roleName)
{
    if (m_roleName == roleName) {
        return;
    }

    m_roleName = roleName;
    emit dataChanged();
}

void FilterValue::setRoleValue(QVariant value)
{
    if (m_roleValue == value) {
        return;
    }

    m_roleValue = value;
    emit dataChanged();
}

void FilterValue::setCompareType(QVariant type)
{
    if (m_compareType == type) {
        return;
    }

    m_compareType = type;
    emit dataChanged();
}
