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

using namespace mu::extensions;
namespace mu::extensions {
class QmlApiEngine : public mu::api::IApiEngine
{
public:
    QmlApiEngine(QQmlEngine* e)
        : m_engine(e) {}

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
};
}

ExtensionsUiEngine::~ExtensionsUiEngine()
{
    delete m_api;
    delete m_apiEngine;
    delete m_engine;

    delete m_engineApiV1;
}

void ExtensionsUiEngine::setup(QQmlEngine* e)
{
    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    e->rootContext()->setContextProperty("ui", ui);

    m_apiEngine = new QmlApiEngine(e);
    m_api = new api::QmlExtApi(m_apiEngine, e);
    m_engine->globalObject().setProperty("api", e->newQObject(m_api));

    //! NOTE We prohibit importing default modules;
    //! only what is in the `api` folder will be imported.
    e->addImportPath(":/api");
}

QQmlEngine* ExtensionsUiEngine::engine()
{
    if (!m_engine) {
        m_engine = new QQmlEngine(this);
        setup(m_engine);
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
QQmlEngine* ExtensionsUiEngine::engineApiV1()
{
    if (!m_engineApiV1) {
        m_engineApiV1 = new QQmlEngine(this);
        setupApiV1(m_engineApiV1);
    }

    return m_engineApiV1;
}

void ExtensionsUiEngine::setupApiV1(QQmlEngine* e)
{
    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    e->rootContext()->setContextProperty("ui", ui);

    //! NOTE Old plugins could use standard modules (for example MuseScore.UiComponents),
    //! we need to think about how to limit this or quickly abandon old plugins.
    e->addImportPath(":/qml");
}

QQmlEngine* ExtensionsUiEngine::qmlEngineApiV1() const
{
    return uiEngine()->qmlEngine();
    //return const_cast<ExtensionsUiEngine*>(this)->engineApiV1();
}
