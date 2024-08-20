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
#include "apiregister.h"

#include <QQmlEngine>
#include <QMetaObject>
#include <QMetaMethod>

#include "global/stringutils.h"

#include "log.h"

using namespace muse::api;

struct SingletonApiCreator : public IApiRegister::ICreator
{
    ApiObject* obj = nullptr;
    SingletonApiCreator(ApiObject* o)
        : obj(o) {}

    ApiObject* create(IApiEngine*) override { return obj; }
    bool isNeedDelete() const override { return false; }
};

void ApiRegister::regApiCreator(const std::string& module, const std::string& api, ICreator* c)
{
    ApiCreator ac;
    auto it = m_creators.find(api);
    if (it != m_creators.end()) {
        ac = it->second;
    }

    IF_ASSERT_FAILED(!ac.c) {
        LOGE() << "already registered creator for api: " << api << ", before creator will deleted";
        delete ac.c;
    }

    ac.module = module;
    ac.c = c;
    m_creators[api] = ac;
}

void ApiRegister::regApiSingltone(const std::string& module, const std::string& api, ApiObject* o)
{
    regApiCreator(module, api, new SingletonApiCreator(o));
}

std::pair<ApiObject*, bool /*is need delete*/> ApiRegister::createApi(const std::string& api, IApiEngine* e) const
{
    auto it = m_creators.find(api);
    if (it == m_creators.end()) {
        LOGE() << "not registered creator for api: " << api;
        return { nullptr, false };
    }
    return { it->second.c->create(e), it->second.c->isNeedDelete() };
}

class DumpApiEngine : public IApiEngine
{
public:

    QQmlEngine engine;

    const muse::modularity::ContextPtr& iocContext() const override
    {
        static muse::modularity::ContextPtr ctx;
        return ctx;
    }

    QJSValue newQObject(QObject* o) override
    {
        return engine.newQObject(o);
    }

    QJSValue newObject() override
    {
        return engine.newObject();
    }

    QJSValue newArray(size_t length) override
    {
        return engine.newArray(static_cast<uint>(length));
    }
};

ApiRegister::Dump ApiRegister::dump() const
{
    Dump dump;
    DumpApiEngine engine;

    static const QSet<QString> qtMethods = {
        "destroyed",
        "objectName",
        "objectNameChanged",
        "deleteLater",
        "_q_reregisterTimers"
    };

    auto makePropertySig = [](const QMetaProperty& p) {
        Dump::Sig sig;
        sig.name = p.name();
        sig.retType = p.typeName();
        return sig;
    };

    auto makeMethodSig = [](const QMetaMethod& m) {
        int pcount = m.parameterCount();
        QList<QByteArray> ptypes = m.parameterTypes();
        QList<QByteArray> pnames = m.parameterNames();
        QMetaType retType = m.returnMetaType();

        Dump::Sig sig;
        sig.name = m.name();
        sig.retType = retType.name();
        for (int i = 0; i < pcount; ++i) {
            Dump::Arg a;
            a.type = ptypes.at(i).constData();
            a.name = pnames.at(i).constData();
            sig.args.push_back(std::move(a));
        }

        return sig;
    };

    for (const auto& p : m_creators) {
        Dump::Api api;
        api.prefix = QString::fromStdString(p.first); // api, like api.dispatcher

        ApiObject* obj = p.second.c->create(&engine);
        const QMetaObject* meta = obj->metaObject();

        for (int i = 0; i < meta->propertyCount(); ++i) {
            const QMetaProperty op = meta->property(i);
            if (qtMethods.contains(op.name())) {
                continue;
            }
            Dump::Method dm;
            dm.type = Dump::MethodType::Property;
            dm.sig = makePropertySig(op);
            api.methods.push_back(dm);
        }

        for (int i = 0; i < meta->methodCount(); ++i) {
            const QMetaMethod om = meta->method(i);
            QByteArray mname = om.name();
            if (qtMethods.contains(mname)) {
                continue;
            }
            if (mname.endsWith("_doc")) {
                continue;
            }

            Dump::Method dm;
            dm.type = Dump::MethodType::Method;
            dm.sig = makeMethodSig(om);

            QByteArray mname_doc = mname + "_doc()";
            int doc_idx = meta->indexOfMethod(mname_doc.constData());
            if (doc_idx != -1) {
                QMetaMethod docme = meta->method(doc_idx);
                docme.invoke(obj, Q_RETURN_ARG(QString, dm.doc));
            }

            api.methods.push_back(dm);
        }

        if (p.second.c->isNeedDelete()) {
            delete obj;
        }

        dump.apis.push_back(std::move(api));
    }

    return dump;
}
