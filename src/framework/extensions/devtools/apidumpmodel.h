/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#pragma once

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "global/api/iapiregister.h"

namespace muse::extensions {
class ApiDumpModel : public QAbstractListModel
{
    Q_OBJECT

    muse::Inject<muse::api::IApiRegister> apiRegister;

public:
    ApiDumpModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum ApiType {
        All = 0,
        Extensions,
        Autobot
    };
    Q_ENUM(ApiType);

    Q_INVOKABLE void load();

    Q_INVOKABLE void find(const QString& str);

    Q_INVOKABLE QVariantList apiTypes() const;
    Q_INVOKABLE void setApiType(ApiType type);

    Q_INVOKABLE void copyWiki();
    Q_INVOKABLE void printWiki();

private:
    enum Roles {
        rData = Qt::UserRole + 1,
        rGroup
    };

    struct Item {
        QString prefix;
        QString module;
        QString sig;
        QString fullSig; // with prefix `api.module.`
        QString doc;
    };

    void update();
    bool isAllowByType(const QString& module, ApiType type) const;

    QString makeWiki() const;

    QList<Item> m_list;
    QList<Item> m_allList;
    QString m_searchText;
    ApiType m_apiType = ApiType::Extensions;
};
}
