/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
#include "apiregister.h"

#include <QQmlEngine>
#include <QMetaObject>
#include <QMetaMethod>

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

ApiRegister::~ApiRegister()
{
    for (auto& e : m_apiengines) {
        delete e.apiengine;
    }
}

void ApiRegister::regApiCreator(const std::string& module, const std::string& api, ICreator* c)
{
    ApiCreator ac;
    {
        auto it = m_creators.find(api);
        if (it != m_creators.end()) {
            ac = it->second;
        }
    }

    IF_ASSERT_FAILED(!ac.c) {
        LOGE() << "already registered creator for api: " << api << ", before creator will deleted";
        delete ac.c;
    }

    ac.module = module;
    ac.c = c;
    m_creators[api] = ac;

    // register for Qml
    std::string name;
    {
        auto pos = api.find('.');
        if (pos != std::string::npos) {
            name = api.substr(pos + 1);
        }
    }

    IF_ASSERT_FAILED(!name.empty()) {
        return;
    }

    qmlRegisterSingletonType(api.c_str(), 1, 0, name.c_str(), [this, api](QQmlEngine*, QJSEngine* jsengine) -> QJSValue {
        auto obj = createApi(api, makeApiEngine(jsengine));
        bool isNeedDelete = obj.second;
        QJSEngine::setObjectOwnership(obj.first, isNeedDelete ? QJSEngine::JavaScriptOwnership : QJSEngine::CppOwnership);
        return jsengine->newQObject(obj.first);
    });
}

JsApiEngine* ApiRegister::makeApiEngine(QJSEngine* jsengine)
{
    auto it = std::find_if(m_apiengines.begin(), m_apiengines.end(), [jsengine](const ApiEngine& e) {
        return e.jsengine == jsengine;
    });

    if (it != m_apiengines.end()) {
        return it->apiengine;
    }

    ApiEngine e;
    e.jsengine = jsengine;
    e.apiengine = new JsApiEngine(jsengine, muse::modularity::globalCtx());
    m_apiengines.push_back(e);

    QObject::connect(jsengine, &QJSEngine::destroyed, [this, jsengine]() {
        auto it = std::find_if(m_apiengines.begin(), m_apiengines.end(), [jsengine](const ApiEngine& e) {
            return e.jsengine == jsengine;
        });

        if (it != m_apiengines.end()) {
            delete it->apiengine;
            m_apiengines.erase(it);
        }
    });

    return e.apiengine;
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

void ApiRegister::regEnum(const char* uri, const char* name, const QMetaEnum& meta, EnumType type)
{
    qmlRegisterSingletonType(uri, 1, 0, name, [meta, type](QQmlEngine*, QJSEngine* jsengine) -> QJSValue {
        QJSValue enumObj = jsengine->newObject();

        for (int i = 0; i < meta.keyCount(); ++i) {
            QString key = QString::fromLatin1(meta.key(i));
            if (type == EnumType::String) {
                enumObj.setProperty(key, key);
            } else {
                int val = meta.value(i);
                enumObj.setProperty(key, val);
            }
        }

        QJSValue freezeFn = jsengine->evaluate("Object.freeze");
        return freezeFn.call({ enumObj });
    });
}

void ApiRegister::regGlobalEnum(const std::string& module, const QMetaEnum& meta,
                                EnumType type,
                                const std::string& name)
{
    std::string ename = name.empty() ? std::string(meta.enumName()) : name;
    m_globalEnums.push_back({ module, ename, meta, type });
}

const std::vector<ApiRegister::GlobalEnum>& ApiRegister::globalEnums() const
{
    return m_globalEnums;
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

    int apiversion() const override
    {
        return 2;
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

    QJSValue freeze(const QJSValue& val) override
    {
        static QJSValue freezeFn = engine.evaluate("Object.freeze");
        return freezeFn.call({ val });
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
        api.prefix = QString::fromStdString(p.first); // like MuseInternal.Dispatcher

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
