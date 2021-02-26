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

#include "exportscoresettingsmodel.h"

#include <QRegularExpression>

using namespace mu::userscores;
using namespace mu::framework;

ExportScoreSettingsModel::ExportScoreSettingsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ExportScoreSettingsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Settings::Item& item = m_currentItems.at(index.row());
    switch (role) {
    case SectionRole:
        return QString::fromStdString(item.key.moduleName);
    case KeyRole:
        return QString::fromStdString(item.key.key);
    case FriendlyNameRole:
        return friendlyNameFromKey(QString::fromStdString(item.key.key));
    case TypeRole:
        return typeToString(item.value.type());
    case ValRole:
        return item.value.toQVariant();
    }

    return QVariant();
}

int ExportScoreSettingsModel::rowCount(const QModelIndex&) const
{
    return m_currentItems.size();
}

QHash<int, QByteArray> ExportScoreSettingsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { SectionRole, "sectionRole" },
        { KeyRole, "keyRole" },
        { FriendlyNameRole, "friendlyNameRole" },
        { TypeRole, "typeRole" },
        { ValRole, "valRole" }
    };

    return roles;
}

void ExportScoreSettingsModel::load()
{
    beginResetModel();

    m_completeItems.clear();
    m_currentItems.clear();

    Settings::Items items = settings()->items();

    for (auto it = items.cbegin(); it != items.cend(); ++it) {
        if (it->second.key.key.find("export") != std::string::npos) {
            m_completeItems << it->second;
            m_currentItems << it->second;
        }
    }

    endResetModel();
}

void ExportScoreSettingsModel::changeVal(int idx, QVariant newVal)
{
    Settings::Item& item = m_currentItems[idx];
    Val::Type type = item.value.type();
    item.value = Val::fromQVariant(newVal);
    item.value.setType(type);

    settings()->setValue(item.key, item.value);

    emit dataChanged(index(idx), index(idx));
}

void ExportScoreSettingsModel::changeType(QString type)
{
    m_currentItems.clear();

    beginResetModel();

    for (auto item : m_completeItems) {
        if (item.key.key.find(type.toStdString()) != std::string::npos) {
            m_currentItems << item;
        }
    }

    endResetModel();
}

QString ExportScoreSettingsModel::typeToString(Val::Type t) const
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

QString ExportScoreSettingsModel::friendlyNameFromKey(QString key) const
{
    QString name = key.split("/", Qt::SkipEmptyParts).last();
    name.replace(QRegularExpression("([A-Z](?=[a-z]+)|[A-Z]+(?![a-z]))"), " \\1");
    return name;
}
