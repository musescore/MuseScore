//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "advancedpreferencesmodel.h"

#include "settings.h"

using namespace mu::appshell;
using namespace mu::framework;

AdvancedPreferencesModel::AdvancedPreferencesModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant AdvancedPreferencesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Settings::Item& item = m_items.at(index.row());
    switch (role) {
    case KeyRole: return QString::fromStdString(item.key.key);
    case TypeRole: return typeToString(item.value.type());
    case ValRole: return item.value.toQVariant();
    }
    return QVariant();
}

bool AdvancedPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    switch (role) {
    case ValRole:
        changeVal(index.row(), value);
        emit dataChanged(index, index, { ValRole });
        return true;
    default:
        return false;
    }
}

int AdvancedPreferencesModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> AdvancedPreferencesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { KeyRole, "keyRole" },
        { TypeRole, "typeRole" },
        { ValRole, "valueRole" }
    };

    return roles;
}

void AdvancedPreferencesModel::load()
{
    beginResetModel();

    m_items.clear();

    Settings::Items items = settings()->items();

    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        if (it->second.canBeMannualyEdited) {
            m_items << it->second;
        }
    }

    endResetModel();
}

void AdvancedPreferencesModel::changeVal(int index, QVariant newVal)
{
    Settings::Item& item = m_items[index];
    Val::Type type = item.value.type();
    item.value = Val::fromQVariant(newVal);
    item.value.setType(type);

    settings()->setValue(item.key, item.value);
}

void AdvancedPreferencesModel::resetToDefault()
{
    beginResetModel();

    for (int i = 0; i < m_items.size(); ++i) {
        changeVal(i, m_items[i].defaultValue.toQVariant());
    }

    endResetModel();
}

QString AdvancedPreferencesModel::typeToString(Val::Type type) const
{
    switch (type) {
    case Val::Type::Undefined: return "Undefined";
    case Val::Type::Bool: return "Bool";
    case Val::Type::Int: return "Int";
    case Val::Type::Double: return "Double";
    case Val::Type::String: return "String";
    case Val::Type::Color: return "Color";
    case Val::Type::Variant: return "Variant";
    }
    return "Undefined";
}
