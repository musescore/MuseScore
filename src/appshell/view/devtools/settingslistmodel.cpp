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
#include "settingslistmodel.h"

#include "log.h"
#include "settings.h"

using namespace mu;
using namespace mu::framework;

SettingListModel::SettingListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant SettingListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Settings::Item& item = m_items.at(index.row());
    switch (role) {
    case SectionRole: return QString::fromStdString(item.key.moduleName);
    case KeyRole: return QString::fromStdString(item.key.key);
    case TypeRole: return typeToString(item.value.type());
    case ValRole: return item.value.toQVariant();
    }
    return QVariant();
}

int SettingListModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> SettingListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { SectionRole, "sectionRole" },
        { KeyRole, "keyRole" },
        { TypeRole, "typeRole" },
        { ValRole, "valRole" }
    };
    return roles;
}

void SettingListModel::load()
{
    beginResetModel();

    m_items.clear();

    Settings::Items items = settings()->items();

    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        m_items << it->second;
    }

    endResetModel();
}

void SettingListModel::changeVal(int idx, QVariant newVal)
{
    LOGD() << "changeVal index: " << idx << ", newVal: " << newVal;
    Settings::Item& item = m_items[idx];
    Val::Type type = item.value.type();
    item.value = Val::fromQVariant(newVal);
    item.value.setType(type);

    settings()->setValue(item.key, item.value);

    emit dataChanged(index(idx), index(idx));
}

QString SettingListModel::typeToString(Val::Type t) const
{
    switch (t) {
    case Val::Type::Undefined: return "Undefined";
    case Val::Type::Bool:      return "Bool";
    case Val::Type::Int:       return "Int";
    case Val::Type::Double:    return "Double";
    case Val::Type::String:    return "String";
    case Val::Type::Color:     return "Color";
    case Val::Type::Variant:   return "Variant";
    }
    return "Undefined";
}
