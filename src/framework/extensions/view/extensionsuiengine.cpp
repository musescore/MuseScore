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
#include "extensionsuiengine.h"

#include <QQmlEngine>
#include <QQmlContext>

using namespace muse::extensions;
namespace muse::extensions {
class QmlApiEngine : public muse::api::IApiEngine
{
public:
    QmlApiEngine(QQmlEngine* e, const modularity::ContextPtr& iocContext)
        : m_engine(e), m_iocContext(iocContext) {}

    const modularity::ContextPtr& iocContext() const override
    {
        return m_iocContext;
    }

    QJSValue newQObject(QObject* o) override
    {
        if (!o->parent()) {
            o->setParent(m_engine);
        }
        return m_engine->newQObject(o);
    }

    QJSValue newObject() override
    {
        return m_engine->newObject();
    }

    QJSValue newArray(size_t length = 0) override
    {
        return m_engine->newArray(uint(length));
    }

private:
    QQmlEngine* m_engine = nullptr;
    const modularity::ContextPtr& m_iocContext;
};
}

ExtensionsUiEngine::~ExtensionsUiEngine()
{
    delete m_api;
    delete m_apiEngine;
    delete m_engine;

    delete m_apiV1;
    delete m_apiEngineV1;
    delete m_engineV1;
}

void ExtensionsUiEngine::setup()
{
    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    m_engine->rootContext()->setContextProperty("ui", ui);

    m_apiEngine = new QmlApiEngine(m_engine, iocContext());
    m_api = new api::ExtApi(m_apiEngine, m_engine);
    m_engine->globalObject().setProperty("api", m_engine->newQObject(m_api));

    //! NOTE We prohibit importing default modules;
    //! only what is in the `api` folder will be imported.
    m_engine->addImportPath(":/api");
}

QQmlEngine* ExtensionsUiEngine::engine()
{
    if (!m_engine) {
        m_engine = new QQmlEngine(this);
        setup();
    }

    return m_engine;
}

QQmlEngine* ExtensionsUiEngine::qmlEngine() const
{
    return const_cast<ExtensionsUiEngine*>(this)->engine();
}

// =====================================================
// Api V1
// =====================================================
QQmlEngine* ExtensionsUiEngine::engineV1()
{
    if (!m_engineV1) {
        m_engineV1 = new QQmlEngine(this);
        setupV1();
    }

    return m_engineV1;
}

void ExtensionsUiEngine::setupV1()
{
    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    m_engineV1->rootContext()->setContextProperty("ui", ui);

    m_apiEngineV1 = new QmlApiEngine(m_engineV1, iocContext());
    m_apiV1 = new apiv1::ExtApiV1(m_apiEngineV1, m_engineV1);
    m_engineV1->globalObject().setProperty("api", m_engineV1->newQObject(m_apiV1));

    //! NOTE Old plugins could use standard modules (for example MuseScore.UiComponents),
    //! we need to think about how to limit this or quickly abandon old plugins.
    m_engineV1->addImportPath(":/qml");
}

QQmlEngine* ExtensionsUiEngine::qmlEngineApiV1() const
{
    return const_cast<ExtensionsUiEngine*>(this)->engineV1();
}
