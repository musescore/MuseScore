/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

using namespace mu::appshell;

static const QString HOME_PAGE("musescore://home");
static const QString NOTATION_PAGE("musescore://notation");
static const QString PUBLISH_PAGE("musescore://publish");
static const QString DEVTOOLS_PAGE("musescore://devtools");

MainToolBarModel::MainToolBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant MainToolBarModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const Item& item = m_items.at(index.row());
    switch (role) {
    case TitleRole: return item.title.qTranslated();
    case UriRole: return item.uri;
    case IsTitleBoldRole: return item.isTitleBold;
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
        { TitleRole, "title" },
        { UriRole, "uri" },
        { IsTitleBoldRole, "isTitleBold" },
    };

    return roles;
}

void MainToolBarModel::load()
{
    beginResetModel();

    m_items.clear();
    m_items << Item { TranslatableString("appshell", "Home"), HOME_PAGE };
    m_items << Item { TranslatableString("appshell", "Score"), NOTATION_PAGE };
    m_items << Item { TranslatableString("appshell", "Publish"), PUBLISH_PAGE };

    if (globalConfiguration()->devModeEnabled()) {
        //: Tools that are used by the developers of MuseScore; generally not exposed to users
        m_items << Item { TranslatableString("appshell", "DevTools"), DEVTOOLS_PAGE };
    }

    endResetModel();

    updateNotationPageItem();
    context()->currentProjectChanged().onNotify(this, [this]() {
        updateNotationPageItem();
    });

    languagesService()->currentLanguageChanged().onNotify(this, [this] {
        emit dataChanged(index(0), index(rowCount() - 1), { TitleRole });
    });
}

void MainToolBarModel::updateNotationPageItem()
{
    for (int i = 0; i < m_items.size(); ++i) {
        Item& item = m_items[i];

        if (item.uri == NOTATION_PAGE) {
            item.isTitleBold = context()->currentProject() != nullptr;

            QModelIndex modelIndex = index(i);
            emit dataChanged(modelIndex, modelIndex, { IsTitleBoldRole });

            break;
        }
    }
}
