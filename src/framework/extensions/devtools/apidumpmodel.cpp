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

#include "muse_framework_config.h"
#ifdef MUSE_MODULE_AUTOBOT
#include "autobot/internal/api/scriptapi.h"
#endif

#include "log.h"

using namespace muse::extensions;
using namespace muse::api;

static const QHash<QString, QString> TYPES_MAP = {
    { "QString", "String" },
    { "QVariantMap", "Map" },
    { "QFont", "Font" },
    { "QColor", "Color" },
    { "QJSValue", "Value" }
};

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
    case rData: return QVariant::fromValue(item.sig + " - " + item.doc);
    case rGroup: return QVariant::fromValue(item.prefix);
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
    case ApiType::Autobot: {
#ifdef MUSE_MODULE_AUTOBOT
        return hasProperty(autobot::ScriptApi::staticMetaObject, module);
#else
        return false;
#endif
    }
    }

    return true;
}

static QString makeCleanType(const QString& type)
{
    QString str = TYPES_MAP.value(type, type);
    int lastNSIdx = str.lastIndexOf("::");
    if (lastNSIdx != -1) {
        str = str.mid(lastNSIdx + 2); // remove namespace
    }
    return str;
}

static QString sigToString(const IApiRegister::Dump::Sig& sig, IApiRegister::Dump::MethodType type, const QString& prefix = QString())
{
    switch (type) {
    case IApiRegister::Dump::MethodType::Property: {
        QString str = makeCleanType(sig.retType);
        str += " ";
        if (!prefix.isEmpty()) {
            str += prefix + ".";
        }
        str += sig.name;

        return str;
    } break;
    case IApiRegister::Dump::MethodType::Method: {
        QString str = makeCleanType(sig.retType);
        str += " ";
        if (!prefix.isEmpty()) {
            str += prefix + ".";
        }
        str += sig.name;
        str += "(";
        for (const IApiRegister::Dump::Arg& a : sig.args) {
            str += makeCleanType(a.type);
            str += " ";
            str += a.name;
            str += ", ";
        }

        if (!sig.args.empty()) {
            str.chop(2); // remove last `, `
        }

        str += ")";

        return str;
    } break;
    }
}

void ApiDumpModel::load()
{
    m_allList.clear();

    IApiRegister::Dump dump = apiRegister()->dump();

    for (const IApiRegister::Dump::Api& api : dump.apis) {
        for (const IApiRegister::Dump::Method& me : api.methods) {
            Item item;
            item.module = moduleFromPrefix(api.prefix);
            item.prefix = "api." + item.module;   // not api.prefix
            item.sig = sigToString(me.sig, me.type);
            item.fullSig = sigToString(me.sig, me.type, item.prefix);
            item.doc = me.doc;

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

            if (item.sig.contains(m_searchText, Qt::CaseInsensitive)
                || item.prefix.contains(m_searchText, Qt::CaseInsensitive)) {
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
        ts << item.fullSig << "\r\n";
    }

    std::cout << str.toStdString() << std::endl;
}
