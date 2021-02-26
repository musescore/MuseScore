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

#include "exportscoresuffixmodel.h"

#include "log.h"

#include <algorithm>

using namespace mu;
using namespace mu::userscores;

const std::string ExportScoreSuffixModel::DEFAULT_EXPORT_SUFFIX("pdf");

ExportScoreSuffixModel::ExportScoreSuffixModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_defaultSuffix(0)
{
}

void ExportScoreSuffixModel::load()
{
    beginResetModel();

    auto suffixes = writers()->getRegisteredSuffixes();

    m_defaultSuffix = static_cast<int>(std::find(suffixes.begin(), suffixes.end(), DEFAULT_EXPORT_SUFFIX) - suffixes.begin());
    LOGI() << m_defaultSuffix;

    for (std::string suffix : suffixes) {
        m_suffixes << QString::fromStdString(suffix);
    }

    endResetModel();
}

QVariant ExportScoreSuffixModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case RoleSuffix:
        return m_suffixes[index.row()];
    case RoleValue:
        return index.row();
    }

    return QVariant();
}

int ExportScoreSuffixModel::rowCount(const QModelIndex&) const
{
    return m_suffixes.size();
}

QHash<int, QByteArray> ExportScoreSuffixModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleSuffix, "suffix" },
        { RoleValue, "value" }
    };

    return roles;
}

int ExportScoreSuffixModel::getDefaultRow() const
{
    return m_defaultSuffix;
}
