/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "abstractscoresmodel.h"

#include "uicomponents/view/modelutils.h"

using namespace mu::project;

const QString AbstractScoresModel::NAME_KEY("name");
const QString AbstractScoresModel::PATH_KEY("path");
const QString AbstractScoresModel::SUFFIX_KEY("suffix");
//const QString AbstractScoresModel::THUMBNAIL_URL_KEY("thumbnailUrl");
const QString AbstractScoresModel::TIME_SINCE_MODIFIED_KEY("timeSinceModified");
const QString AbstractScoresModel::IS_CREATE_NEW_KEY("isCreateNew");
const QString AbstractScoresModel::IS_NO_RESULT_FOUND_KEY("isNoResultFound");
const QString AbstractScoresModel::IS_CLOUD_KEY("isCloud");

AbstractScoresModel::AbstractScoresModel(QObject* parent)
    : QAbstractListModel(parent)
{
    uicomponents::ModelUtils::connectRowCountChangedSignal(this, &AbstractScoresModel::rowCountChanged);
}

QVariant AbstractScoresModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QVariantMap item = m_items[index.row()];

    switch (role) {
    case NameRole: return item[NAME_KEY];
    case ScoreRole: return item;
    }

    return QVariant();
}

int AbstractScoresModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_items.size());
}

QHash<int, QByteArray> AbstractScoresModel::roleNames() const
{
    static const QHash<int, QByteArray> ROLE_NAMES {
        { NameRole, NAME_KEY.toUtf8() },
        { ScoreRole, "score" }
    };

    return ROLE_NAMES;
}
