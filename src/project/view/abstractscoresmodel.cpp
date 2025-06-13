/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include <algorithm>

using namespace mu::project;

const QString AbstractScoresModel::NAME_KEY("name");
const QString AbstractScoresModel::PATH_KEY("path");
const QString AbstractScoresModel::COMPOSER_KEY("composer");
const QString AbstractScoresModel::ARRANGER_KEY("arranger");
const QString AbstractScoresModel::CREATION_DATE_KEY("creationDate");
const QString AbstractScoresModel::SUFFIX_KEY("suffix");
const QString AbstractScoresModel::FILE_SIZE_KEY("fileSize");
const QString AbstractScoresModel::THUMBNAIL_URL_KEY("thumbnailUrl");
const QString AbstractScoresModel::TIME_SINCE_MODIFIED_KEY("timeSinceModified");
const QString AbstractScoresModel::IS_CREATE_NEW_KEY("isCreateNew");
const QString AbstractScoresModel::IS_NO_RESULTS_FOUND_KEY("isNoResultsFound");
const QString AbstractScoresModel::IS_CLOUD_KEY("isCloud");
const QString AbstractScoresModel::CLOUD_SCORE_ID_KEY("scoreId");
const QString AbstractScoresModel::CLOUD_VISIBILITY_KEY("cloudVisibility");
const QString AbstractScoresModel::CLOUD_VIEW_COUNT_KEY("cloudViewCount");

AbstractScoresModel::AbstractScoresModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_sortKey("")
    , m_sortOrder(Qt::AscendingOrder)
{
    muse::uicomponents::ModelUtils::connectRowCountChangedSignal(this, &AbstractScoresModel::rowCountChanged);
}

QVariant AbstractScoresModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    QVariantMap item = m_items[index.row()];

    switch (role) {
    case NameRole: return item[NAME_KEY];
    case IsNoResultsFoundRole: return item[IS_NO_RESULTS_FOUND_KEY];
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
        { IsNoResultsFoundRole, IS_NO_RESULTS_FOUND_KEY.toUtf8() },
        { ScoreRole, "score" }
    };

    return ROLE_NAMES;
}

QList<int> AbstractScoresModel::nonScoreItemIndices() const
{
    return {};
}

void AbstractScoresModel::sortBy(const QString& key)
{
    if (m_sortKey == key && m_isSorted) {
        if (m_sortOrder == Qt::AscendingOrder) {
            m_sortOrder = Qt::DescendingOrder;
        } else {
            clearSort();
            return;
        }
    } else {
        m_sortKey = key;
        m_sortOrder = Qt::AscendingOrder;
        m_isSorted = true;
    }

    emit sortKeyChanged();
    emit sortOrderChanged();

    load();
}

void AbstractScoresModel::clearSort()
{
    m_isSorted = false;
    m_sortKey.clear();

    emit sortKeyChanged();
    emit sortOrderChanged();

    load();
}

void AbstractScoresModel::sortScoreItems(std::vector<QVariantMap>& items)
{
    if (m_sortKey.isEmpty() || !m_isSorted) {
        return;
    }

    std::sort(items.begin(), items.end(), [this](const QVariantMap& a, const QVariantMap& b) {
        return compareItems(a, b, m_sortKey, m_sortOrder);
    });
}

bool AbstractScoresModel::compareItems(const QVariantMap& a, const QVariantMap& b, const QString& key, Qt::SortOrder order)
{
    QVariant valueA, valueB;
    bool timeSinceOrder = false;

    if (key == "timeSinceModified") {
        timeSinceOrder = true;
        valueA = a.value("rawModifiedTime");
        valueB = b.value("rawModifiedTime");
    } else if (key == "fileSize") {
        valueA = a.value("rawFileSize");
        valueB = b.value("rawFileSize");
    } else {
        valueA = a.value(key, "");
        valueB = b.value(key, "");
    }

    bool aIsEmpty = isValueEmpty(valueA);
    bool bIsEmpty = isValueEmpty(valueB);

    if (aIsEmpty && bIsEmpty) {
        return false;
    }
    if (aIsEmpty) {
        return false;
    }
    if (bIsEmpty) {
        return true;
    }

    bool result = false;
    if (valueA.typeId() == QMetaType::QString && timeSinceOrder) {
        result = QString::localeAwareCompare(valueA.toString(), valueB.toString()) > 0;
    } else if (valueA.typeId() == QMetaType::QString) {
        result = QString::localeAwareCompare(valueA.toString(), valueB.toString()) < 0;
    } else if (valueA.canConvert<qint64>() && valueB.canConvert<qint64>()) {
        result = valueA.toLongLong() < valueB.toLongLong();
    }
    return (order == Qt::AscendingOrder) ? result : !result;
}

bool AbstractScoresModel::isValueEmpty(const QVariant& value)
{
    if (value.isNull() || !value.isValid()) {
        return true;
    }

    if (value.typeId() == QMetaType::QString) {
        QString str = value.toString().trimmed();
        return str.isEmpty() || str == "-" || str == "Composer / arranger";
    }

    return false;
}

QString AbstractScoresModel::currentSortKey() const
{
    return m_isSorted ? m_sortKey : QString();
}

Qt::SortOrder AbstractScoresModel::sortOrder() const
{
    return m_sortOrder;
}
