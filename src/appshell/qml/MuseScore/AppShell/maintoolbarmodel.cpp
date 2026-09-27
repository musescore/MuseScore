/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "maintoolbarmodel.h"

#include "translation.h"

using namespace mu::appshell;

static const QString HOME_PAGE("musescore://home");
static const QString NOTATION_PAGE("musescore://notation");
static const QString NOTATION_REVIEW_PAGE("musescore://notation/review");
static const QString PUBLISH_PAGE("musescore://publish");
static const QString DEVTOOLS_PAGE("musescore://devtools");

static const QString TITLE_KEY("title");
static const QString URI_KEY("uri");
static const QString IS_TITLE_BOLD_KEY("isTitleBold");
static const QString IS_CHECKED_KEY("isChecked");

static bool isNotationPageUri(const QString& uri)
{
    return uri == NOTATION_PAGE || uri == NOTATION_REVIEW_PAGE;
}

static bool isItemChecked(const QString& itemUri, const QString& currentUri)
{
    return itemUri == currentUri || (isNotationPageUri(itemUri) && isNotationPageUri(currentUri));
}

inline QVariantMap buildItem(const QString& title, const QString& uri, const QString& currentUri)
{
    QVariantMap item;
    item[TITLE_KEY] = title;
    item[URI_KEY] = uri;
    item[IS_TITLE_BOLD_KEY] = false;
    item[IS_CHECKED_KEY] = isItemChecked(uri, currentUri);

    return item;
}

MainToolBarModel::MainToolBarModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this)), m_currentUri(HOME_PAGE)
{
}

QVariant MainToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const QVariantMap& item = m_items.at(index.row());
    switch (role) {
    case TitleRole: return item[TITLE_KEY];
    case UriRole: return item[URI_KEY];
    case IsTitleBoldRole: return item[IS_TITLE_BOLD_KEY];
    case IsCheckedRole: return item[IS_CHECKED_KEY];
    }

    return QVariant();
}

int MainToolBarModel::rowCount(const QModelIndex&) const
{
    return m_items.size();
}

QHash<int, QByteArray> MainToolBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, TITLE_KEY.toUtf8() },
        { UriRole, URI_KEY.toUtf8() },
        { IsTitleBoldRole, IS_TITLE_BOLD_KEY.toUtf8() },
        { IsCheckedRole, IS_CHECKED_KEY.toUtf8() },
    };

    return roles;
}

void MainToolBarModel::load()
{
    beginResetModel();

    m_items.clear();
    m_items << buildItem(muse::qtrc("appshell", "Home"), HOME_PAGE, m_currentUri);
    m_items << buildItem(muse::qtrc("appshell", "Score"), NOTATION_PAGE, m_currentUri);
    m_items << buildItem(muse::qtrc("appshell", "Publish"), PUBLISH_PAGE, m_currentUri);

    if (globalConfiguration()->devModeEnabled()) {
        m_items << buildItem(muse::qtrc("appshell", "DevTools"), DEVTOOLS_PAGE, m_currentUri);
    }

    endResetModel();

    updateNotationPageItem();

    context()->currentProjectChanged().onNotify(this, [this]() {
        updateNotationPageItem();
    });
}

QString MainToolBarModel::currentUri() const
{
    return m_currentUri;
}

void MainToolBarModel::setCurrentUri(const QString& uri)
{
    if (m_currentUri == uri) {
        return;
    }

    m_currentUri = uri;
    emit currentUriChanged();

    updateCheckedState();
}

void MainToolBarModel::updateNotationPageItem()
{
    for (int i = 0; i < m_items.size(); ++i) {
        QVariantMap& item = m_items[i];

        if (isNotationPageUri(item[URI_KEY].toString())) {
            item[URI_KEY] = QString::fromStdString(openProjectScenario()->resolveNotationPageUri().toString());
            item[IS_TITLE_BOLD_KEY] = context()->currentProject() != nullptr;

            QModelIndex modelIndex = index(i);
            emit dataChanged(modelIndex, modelIndex, { UriRole, IsTitleBoldRole });

            break;
        }
    }
}

void MainToolBarModel::updateCheckedState()
{
    for (int i = 0; i < m_items.size(); ++i) {
        QVariantMap& item = m_items[i];

        QVariant& checkedValue = item[IS_CHECKED_KEY];
        bool checked = isItemChecked(item[URI_KEY].toString(), m_currentUri);
        if (checkedValue.toBool() == checked) {
            continue;
        }

        checkedValue = checked;

        QModelIndex modelIndex = index(i);
        emit dataChanged(modelIndex, modelIndex, { IsCheckedRole });
    }
}
