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
#include <QQmlApplicationEngine>
#include <QStringList>
#include <QDir>
#include <QQmlContext>

#include "global/types/color.h"
#include "graphicsapiprovider.h"

#include "log.h"

using namespace muse::ui;

namespace muse::ui {
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

UiEngine::UiEngine(const modularity::ContextPtr& iocCtx)
    : Injectable(iocCtx)
{
    m_engine = new QQmlApplicationEngine(this);
    m_apiEngine = new QmlApiEngine(m_engine, iocContext());
    m_translation = new QmlTranslation(this);
    m_interactiveProvider = std::make_shared<InteractiveProvider>(iocContext());
    m_api = new QmlApi(this, iocContext());
    m_tooltip = new QmlToolTip(this, iocContext());
    m_dataFormatter = new QmlDataFormatter(this);

    //! NOTE At the moment, UiTheme is also QProxyStyle
    //! Inside the theme, QApplication::setStyle(this) is calling and the QStyleSheetStyle becomes as parent.
    //! So, the UiTheme will be deleted when will deleted the application (as a child of QStyleSheetStyle).
    m_theme = new api::ThemeApi(m_apiEngine);
}

UiEngine::~UiEngine()
{
    delete m_translation;
}

void UiEngine::init()
{
    m_theme->init();
    m_tooltip->init();
    m_engine->rootContext()->setContextProperty("ui", this);
    m_engine->rootContext()->setContextProperty("api", m_api);

    QmlIoCContext* qmlIoc = new QmlIoCContext(this);
    qmlIoc->ctx = iocContext();
    m_engine->setProperty("ioc_context", QVariant::fromValue(qmlIoc));

    QJSValue translator = m_engine->newQObject(m_translation);
    QJSValue translateFn = translator.property("translate");
    m_engine->globalObject().setProperty("qsTrc", translateFn);

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

void UiEngine::quit()
{
    if (!m_engine) {
        return;
    }

    emit m_engine->quit();
    delete m_engine;
    m_engine = nullptr;
}

QmlDataFormatter* UiEngine::df() const
{
    return m_dataFormatter;
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

bool UiEngine::isEffectsAllowed() const
{
    if (m_isEffectsAllowed == -1) {
        m_isEffectsAllowed = GraphicsApiProvider::graphicsApi() != GraphicsApi::Software;
    }
    return m_isEffectsAllowed;
}

bool UiEngine::isSystemDragSupported() const
{
    return configuration()->isSystemDragSupported();
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

void UiEngine::updateTheme()
{
    if (!m_engine) {
        return;
    }

    theme()->update();
}

QmlApi* UiEngine::api() const
{
    return m_api;
}

muse::api::ThemeApi* UiEngine::theme() const
{
    return m_theme;
}

QmlToolTip* UiEngine::tooltip() const
{
    return m_tooltip;
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

QColor UiEngine::blendColors(const QColor& c1, const QColor& c2) const
{
    return muse::blendQColors(c1, c2);
}

QColor UiEngine::blendColors(const QColor& c1, const QColor& c2, float alpha) const
{
    return muse::blendQColors(c1, c2, alpha);
}

QColor UiEngine::colorWithAlphaF(const QColor& src, float alpha) const
{
    QColor c = src;
    c.setAlphaF(alpha);
    return c;
}

QQmlApplicationEngine* UiEngine::qmlAppEngine() const
{
    return m_engine;
}

QQmlEngine* UiEngine::qmlEngine() const
{
    return qmlAppEngine();
}

void UiEngine::clearComponentCache()
{
    m_engine->clearComponentCache();
}

GraphicsApi UiEngine::graphicsApi() const
{
    return GraphicsApiProvider::graphicsApi();
}

QString UiEngine::graphicsApiName() const
{
    return GraphicsApiProvider::graphicsApiName();
}
