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

#include "uiengine.h"

#include <QApplication>
#include <QQmlEngine>
#include <QStringList>
#include <QDir>
#include <QQmlContext>

#include "log.h"

using namespace mu::ui;

UiEngine* UiEngine::instance()
{
    static UiEngine e;
    return &e;
}

UiEngine::UiEngine()
{
    m_translation = new QmlTranslation(this);
    m_interactiveProvider = std::make_shared<InteractiveProvider>();
    m_api = new QmlApi(this);
    m_tooltip = new QmlToolTip(this);

    //! NOTE At the moment, UiTheme is also QProxyStyle
    //! Inside the theme, QApplication::setStyle(this) is calling and the QStyleSheetStyle becomes as parent.
    //! So, the UiTheme will be deleted when will deleted the application (as a child of QStyleSheetStyle).
    m_theme = new UiTheme();
}

UiEngine::~UiEngine()
{
    delete m_translation;
}

void UiEngine::quit()
{
    if (!m_engine) {
        return;
    }

    m_engine->quit();
    delete m_engine;
    m_engine = nullptr;
}

QQuickItem* UiEngine::rootItem() const
{
    return m_rootItem;
}

void UiEngine::setRootItem(QQuickItem* rootItem)
{
    if (m_rootItem == rootItem) {
        return;
    }

    m_rootItem = rootItem;
    emit rootItemChanged(m_rootItem);
}

QQmlEngine* UiEngine::engine()
{
    if (m_engine) {
        return m_engine;
    }

    setup(new QQmlEngine(this));

    return m_engine;
}

void UiEngine::moveQQmlEngine(QQmlEngine* e)
{
    setup(e);
}

void UiEngine::setup(QQmlEngine* engine)
{
    IF_ASSERT_FAILED_X(!m_engine, "UiEngine already inited") {
        return;
    }

    m_engine = engine;
    m_theme->init();
    QJSValue ui = m_engine->newQObject(this);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_engine->globalObject().setProperty("ui", ui);

    m_engine->rootContext()->setContextProperty("api", m_api);

    QJSValue translator = m_engine->newQObject(m_translation);
    QQmlEngine::setObjectOwnership(m_translation, QQmlEngine::CppOwnership);
    m_translationFunction = translator.property("translate");

    QJSValue qsTrc = m_engine->evaluate(
        "(function(context, text, disambiguation = \"\", n = -1) {"
        "    return ui.trc(context, text, disambiguation, n);"
        "})");
    m_engine->globalObject().setProperty("qsTrc", qsTrc);

    languagesService()->currentLanguageChanged().onNotify(this, [this]() {
        emit translationChanged();
    });

#ifdef Q_OS_WIN
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../qml"));
    m_engine->addImportPath(dir.absolutePath());
#endif

    m_engine->addImportPath(":/qml");

#ifdef MUE_ENABLE_LOAD_QML_FROM_SOURCE
    for (const QString& path : m_sourceImportPaths) {
        m_engine->addImportPath(path);
    }
#endif
}

void UiEngine::addSourceImportPath(const QString& path)
{
#ifdef MUE_ENABLE_LOAD_QML_FROM_SOURCE
    LOGD() << path;
    m_sourceImportPaths << path;
    if (m_engine) {
        m_engine->addImportPath(path);
    }
#else
    Q_UNUSED(path);
#endif
}

QmlApi* UiEngine::api() const
{
    return m_api;
}

UiTheme* UiEngine::theme() const
{
    return m_theme;
}

QmlToolTip* UiEngine::tooltip() const
{
    return m_tooltip;
}

QJSValue UiEngine::translationFunction() const
{
    return m_translationFunction;
}

InteractiveProvider* UiEngine::interactiveProvider_property() const
{
    return m_interactiveProvider.get();
}

std::shared_ptr<InteractiveProvider> UiEngine::interactiveProvider() const
{
    return m_interactiveProvider;
}

Qt::KeyboardModifiers UiEngine::keyboardModifiers() const
{
    return QGuiApplication::keyboardModifiers();
}

Qt::LayoutDirection UiEngine::currentLanguageLayoutDirection() const
{
    return languagesService()->currentLanguage().direction;
}

QQmlEngine* UiEngine::qmlEngine() const
{
    return const_cast<UiEngine*>(this)->engine();
}

void UiEngine::clearComponentCache()
{
    engine()->clearComponentCache();
}
