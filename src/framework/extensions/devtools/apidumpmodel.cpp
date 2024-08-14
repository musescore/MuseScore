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

#include "extensions/api/extapi.h"
#include "autobot/internal/api/scriptapi.h"

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
    case rGroup: return QVariant::fromValue("api." + item.module);
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

static QString moduleFromPrefix(const QString& prefix)
{
    if (!prefix.startsWith("api.")) {
        LOGD() << "Bad prefix: " << prefix;
        return prefix;
    }

    QString module = prefix.mid(4); // remove `api.`
    int idx = module.indexOf('.');
    if (idx != -1) {
        module = module.left(idx); // remove `.v...`
    }

    return module;
}

bool ApiDumpModel::isAllowByType(const QString& module, ApiType type) const
{
    auto hasProperty = [](const QMetaObject& meta, const QString& module) {
        QByteArray ba = module.toLatin1();
        int idx = meta.indexOfProperty(ba.constData());
        return idx != -1;
    };

    switch (type) {
    case ApiType::All: return true;
    case ApiType::Extensions: return hasProperty(api::ExtApi::staticMetaObject, module);
    case ApiType::Autobot: return hasProperty(autobot::ScriptApi::staticMetaObject, module);
    }

    return true;
}

void ApiDumpModel::load()
{
    m_allList.clear();

    IApiRegister::Dump dump = apiRegister()->dump();

    for (const IApiRegister::Dump::Api& api : dump.apis) {
        for (const IApiRegister::Dump::Method& me : api.methods) {
            Item item;
            item.module = moduleFromPrefix(QString::fromStdString(api.prefix));
            item.data = QString::fromStdString(me.sig + " - " + me.doc);

            m_allList.append(std::move(item));
        }
    }

    update();
}

void ApiDumpModel::update()
{
    beginResetModel();

    if (m_searchText.isEmpty() && m_apiType == ApiType::All) {
        m_list = m_allList;
    } else {
        m_list.clear();
        for (const Item& item : m_allList) {
            if (!isAllowByType(item.module, m_apiType)) {
                continue;
            }

            if (item.data.contains(m_searchText, Qt::CaseInsensitive)
                || item.module.contains(m_searchText, Qt::CaseInsensitive)) {
                m_list.append(item);
            }
        }
    }

    endResetModel();
}

void ApiDumpModel::find(const QString& str)
{
    m_searchText = str;
    update();
}

QVariantList ApiDumpModel::apiTypes() const
{
    auto makeItem = [](const QString& text, ApiType type) {
        QVariantMap item = { { "text", text }, { "value", type } };
        return item;
    };

    QVariantList types;
    types << makeItem("All", ApiType::All);
    types << makeItem("Extensions", ApiType::Extensions);
    types << makeItem("Autobot", ApiType::Autobot);

    return types;
}

void ApiDumpModel::setApiType(ApiType type)
{
    m_apiType = type;
    update();
}

void ApiDumpModel::print()
{
    QString str;
    QTextStream ts(&str);
    for (const Item& item : m_list) {
        ts << item.module << "." << item.data << "\r\n";
    }

    std::cout << str.toStdString() << std::endl;
}
