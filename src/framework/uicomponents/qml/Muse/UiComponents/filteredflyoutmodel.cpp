/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "filteredflyoutmodel.h"

#include "global/translation.h"

using namespace muse::uicomponents;

// Recursively traverse a flyout tree, collect all "leaves" (items without a sub item)...
static void flattenTreeModel(const QVariant& treeModel, const QString& categoryTitle, QVariantList& result, QVariant& alwaysAppend)
{
    for (const QVariant& item : treeModel.toList()) {
        QVariantMap menuItem = item.toMap();
        if (menuItem.empty()) {
            continue;
        }

        const QString title = menuItem.value("title").toString();
        if (title.isEmpty()) {
            continue;
        }

        QVariantList subItems = menuItem.value("subitems").toList();
        if (!subItems.empty()) {
            // Found parent - if it's a "filter category" all child leaves under this item
            // will prepend the title of this item to their titles...
            const bool isFilterCategory = menuItem.value("isFilterCategory").toBool();
            const QString& newCategoryTitle = isFilterCategory ? title : categoryTitle;
            flattenTreeModel(subItems, newCategoryTitle, result, alwaysAppend); // Recursive call...
            continue;
        }

        if (menuItem.value("alwaysAppend").toBool()) {
            // Always append this item to the end of our models...
            alwaysAppend = menuItem;
        }

        // Found leaf...
        if (!menuItem.value("includeInFilteredLists").toBool()) {
            continue;
        }

        const QString prefix = categoryTitle.isEmpty() ? muse::qtrc("global", "Unknown") : categoryTitle;
        menuItem.insert("title", prefix + " - " + title);

        result << menuItem;
    }
}

FilteredFlyoutModel::FilteredFlyoutModel(QObject* parent)
    : QObject(parent)
{
}

QVariant FilteredFlyoutModel::rawModel() const
{
    return m_rawModel;
}

QVariant FilteredFlyoutModel::filteredModel() const
{
    return m_filterText.isEmpty() ? m_rawModel : m_filteredModel;
}

void FilteredFlyoutModel::setRawModel(const QVariant& model)
{
    if (m_rawModel == model) {
        return;
    }
    m_rawModel = model;

    QVariantList result;
    QVariant alwaysAppend;
    flattenTreeModel(m_rawModel, QString(), result, alwaysAppend);
    m_alwaysAppend = alwaysAppend;
    m_flattenedModel = result;

    emit modelChanged();
}

void FilteredFlyoutModel::setFilterText(const QString& filterText)
{
    if (m_filterText == filterText) {
        return;
    }
    m_filterText = filterText;

    QVariantList newModel;
    newModel.reserve(m_flattenedModel.toList().size());

    QString currentPrefix;

    for (const QVariant& item : m_flattenedModel.toList()) {
        QVariantMap itemMap = item.toMap();
        const QString title = itemMap.value("title").toString();
        if (!title.contains(m_filterText, Qt::CaseInsensitive)) {
            continue;
        }
        const QString prefix = title.section("-", 0, 0);
        if (prefix != currentPrefix && !newModel.empty()) {
            newModel << QVariantMap(); // Separate by prefix...
        }
        newModel << itemMap;
        currentPrefix = prefix;
    }

    if (newModel.isEmpty()) {
        QVariantMap item;
        item.insert("checkable", true);
        item.insert("title", muse::qtrc("global", "No results found"));
        newModel << item;
    }

    if (!m_alwaysAppend.isNull()) {
        newModel << QVariantMap(); // Separator...
        newModel << m_alwaysAppend;
    }

    m_filteredModel = newModel;

    emit modelChanged();
}
