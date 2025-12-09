/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "global/api/apiutils.h"

#include "log.h"

using namespace muse::extensions;

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
    m_engine->setProperty("apiversion", 2);

    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    m_engine->rootContext()->setContextProperty("ui", ui);

    m_apiEngine = new muse::api::JsApiEngine(m_engine, iocContext());
    QJSValue globalObject = m_engine->globalObject();
    m_api = new api::ExtApi(m_apiEngine, m_engine);
    globalObject.setProperty("api", m_engine->newQObject(m_api));

    const std::vector<muse::api::IApiRegister::GlobalEnum>& globalEnums = apiRegister()->globalEnums();
    for (const muse::api::IApiRegister::GlobalEnum& e : globalEnums) {
        QString name = QString::fromStdString(e.name);
        QJSValue enumObj = muse::api::enumToJsValue(m_apiEngine, e.meta, e.type);
        globalObject.setProperty(name, enumObj);
    }
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
    m_engineV1->setProperty("apiversion", 1);

    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    m_engineV1->rootContext()->setContextProperty("ui", ui);

    m_apiEngineV1 = new muse::api::JsApiEngine(m_engineV1, iocContext());
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
