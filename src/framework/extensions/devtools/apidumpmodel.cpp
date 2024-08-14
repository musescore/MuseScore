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
#include "apidumpmodel.h"

#include "log.h"

using namespace muse::extensions;
using namespace muse::api;

ApiDumpModel::ApiDumpModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant ApiDumpModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const Item& item = m_list.at(index.row());
    switch (role) {
    case rData: return QVariant::fromValue(item.data);
    case rGroup: return QVariant::fromValue(item.group);
    default: break;
    }

    return QVariant();
}

int ApiDumpModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_list.count();
}

QHash<int, QByteArray> ApiDumpModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { rData, "dataRole" },
        { rGroup, "groupRole" },
    };
    return roles;
}

void ApiDumpModel::load()
{
    beginResetModel();

    m_allList.clear();

    IApiRegister::Dump dump = apiRegister()->dump();

    for (const IApiRegister::Dump::Api& api : dump.apis) {
        for (const IApiRegister::Dump::Method& me : api.methods) {
            Item item;
            item.group = QString::fromStdString(api.prefix);
            item.data = QString::fromStdString(me.sig + " - " + me.doc);

            m_allList.append(std::move(item));
        }
    }

    m_list = m_allList;

    endResetModel();
}

void ApiDumpModel::find(const QString& str)
{
    beginResetModel();

    m_searchText = str;

    if (m_searchText.isEmpty()) {
        m_list = m_allList;
    } else {
        m_list.clear();
        for (const Item& item : m_allList) {
            if (item.data.contains(m_searchText, Qt::CaseInsensitive)
                || item.group.contains(m_searchText, Qt::CaseInsensitive)) {
                m_list.append(item);
            }
        }
    }

    endResetModel();
}

void ApiDumpModel::print()
{
    QString str;
    QTextStream ts(&str);
    for (const Item& item : m_list) {
        ts << item.group << "." << item.data << "\r\n";
    }

    std::cout << str.toStdString() << std::endl;
}
