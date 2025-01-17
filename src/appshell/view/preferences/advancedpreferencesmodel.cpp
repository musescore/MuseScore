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
#include "advancedpreferencesmodel.h"

#include "settings.h"

using namespace muse;
using namespace mu;
using namespace mu::appshell;

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
    case DescriptionRole: return QString::fromStdString(item.description);
    case TypeRole: return typeToString(item.value.type());
    case ValueRole: return item.value.toQVariant();
    case MinValueRole: return !item.minValue.isNull() ? item.minValue.toQVariant() : -1000;
    case MaxValueRole: return !item.maxValue.isNull() ? item.maxValue.toQVariant() : 1000;
    }
    return QVariant();
}

bool AdvancedPreferencesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return false;
    }

    switch (role) {
    case ValueRole:
        changeVal(index.row(), Val::fromQVariant(value));
        emit dataChanged(index, index, { ValueRole });
        return true;
    default:
        return false;
    }
}

int AdvancedPreferencesModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int, QByteArray> AdvancedPreferencesModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { KeyRole, "keyRole" },
        { DescriptionRole, "descriptionRole" },
        { TypeRole, "typeRole" },
        { ValueRole, "valueRole" },
        { MinValueRole, "minValueRole" },
        { MaxValueRole, "maxValueRole" }
    };

    return roles;
}

void AdvancedPreferencesModel::load()
{
    beginResetModel();

    m_items.clear();

    Settings::Items items = settings()->items();

    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        if (it->second.canBeManuallyEdited) {
            m_items << it->second;

            muse::Settings::Key key = it->second.key;
            settings()->valueChanged(key).onReceive(this, [this, key](const Val& val) {
                QModelIndex index = findIndex(key);
                if (!index.isValid() || m_items[index.row()].value == val) {
                    return;
                }

                changeModelVal(m_items[index.row()], val);
                emit dataChanged(index, index, { ValueRole });
            });
        }
    }

    endResetModel();
}

QModelIndex AdvancedPreferencesModel::findIndex(const muse::Settings::Key& key)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].key == key) {
            return index(i, 0);
        }
    }
    return QModelIndex();
}

void AdvancedPreferencesModel::changeVal(int index, const Val& newVal)
{
    Settings::Item& item = m_items[index];
    changeModelVal(item, newVal);
    settings()->setSharedValue(item.key, item.value);
}

void AdvancedPreferencesModel::changeModelVal(Settings::Item& item, const Val& newVal)
{
    if (item.value == newVal) {
        return;
    }
    Val::Type type = item.value.type();
    item.value = newVal;
    item.value.setType(type);
}

void AdvancedPreferencesModel::resetToDefault()
{
    beginResetModel();

    for (int i = 0; i < m_items.size(); ++i) {
        changeVal(i, m_items[i].defaultValue);
    }

    endResetModel();
}

QString AdvancedPreferencesModel::typeToString(Val::Type type) const
{
    switch (type) {
    case Val::Type::Undefined: return "Undefined";
    case Val::Type::Bool: return "Bool";
    case Val::Type::Int: return "Int";
    case Val::Type::Int64: return "Int";
    case Val::Type::Double: return "Double";
    case Val::Type::String: return "String";
    case Val::Type::Color: return "Color";
    case Val::Type::List: return "List";
    case Val::Type::Map: return "Map";
    }
    return "Undefined";
}
