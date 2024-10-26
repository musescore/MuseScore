/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Platform.h"
#include "core/Utils_p.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/qtcommon/View.h"
#include "Config.h"
#include "QmlTypes.h"

#include "Helpers_p.h"
#include "Window_p.h"
#include "views/View.h"
#include "qtquick/Window_p.h"
#include "core/DockRegistry.h"
#include "core/Platform_p.h"
#include "ViewFactory.h"
#include "views/ViewWrapper_p.h"

#include "views/DockWidget.h"
#include "DockWidgetInstantiator.h"

#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QWindow>
#include <QScreen>
#include <QQuickItem>
#include <QGuiApplication>
#include <QQmlContext>

#if defined(KDDOCKWIDGETS_STATICLIB) || defined(QT_STATIC)
static void initResources()
{
    Q_INIT_RESOURCE(kddockwidgets_resources);
    Q_INIT_RESOURCE(kddockwidgets_qtquick);
}
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

inline QQuickItem *mouseAreaForPos(QQuickItem *item, QPointF globalPos)
{
    QRectF rect = item->boundingRect();
    rect.moveTopLeft(item->mapToGlobal(QPointF(0, 0)));

    // Assumes children are inside its parent. That's fine for KDDW's purposes.
    if (!rect.contains(globalPos)) {
        return nullptr;
    }

    const QList<QQuickItem *> children = item->childItems();

    for (auto it = children.rbegin(), end = children.rend(); it != end; ++it) {
        if (QQuickItem *receiver = mouseAreaForPos(*it, globalPos))
            return receiver;
    }

    if (QLatin1String(item->metaObject()->className()) == QLatin1String("QQuickMouseArea"))
        return item;

    return nullptr;
}

Platform::Platform()
    : m_qquickHelpers(new QtQuickHelpers())
{
    init();
}

void Platform::init()
{
#if defined(KDDOCKWIDGETS_STATICLIB) || defined(QT_STATIC)
    initResources();
#endif

    KDDockWidgets::registerQmlTypes();
    QQuickWindow::setDefaultAlphaBuffer(true);

    qGuiApp->connect(qApp, &QGuiApplication::focusObjectChanged, qApp, [this](QObject *obj) {
        d->focusedViewChanged.emit(ViewWrapper::create(obj));
    });
}

Platform::~Platform()
{
    delete m_qquickHelpers;
}

const char *Platform::name() const
{
    return "qtquick";
}

std::shared_ptr<Core::View> Platform::qobjectAsView(QObject *obj) const
{
    if (obj && isWayland()) {
        if (auto dropArea = obj->property("dropAreaCpp").value<QObject *>()) {
            // This is a special case for QtQuick on wayland
            // The receiver of QDrag events needs to be a View. Which for QtWidgets is straightforward
            // but for QtQuick it's the .qml item receiving it
            obj = dropArea;
        }
    }

    return ViewWrapper::create(obj);
}

std::shared_ptr<Core::Window> Platform::windowFromQWindow(QWindow *qwindow) const
{
    return std::shared_ptr<Core::Window>(new Window(qwindow));
}

Core::ViewFactory *Platform::createDefaultViewFactory()
{
    return new ViewFactory();
}

Core::Window::Ptr Platform::windowAt(QPoint globalPos) const
{
    if (auto qwindow = qGuiApp->QGuiApplication::topLevelAt(globalPos)) {
        auto window = new Window(qwindow);
        return Core::Window::Ptr(window);
    }

    return {};
}

int Platform::screenNumberForView(Core::View *view) const
{
    if (auto item = qobject_cast<QQuickItem *>(QtCommon::View_qt::asQObject(view))) {
        if (QWindow *qtwindow = item->window())
            return screenNumberForQWindow(qtwindow);
    }

    return -1;
}

QSize Platform::screenSizeFor(Core::View *view) const
{
    if (auto item = qobject_cast<QQuickItem *>(QtCommon::View_qt::asQObject(view))) {
        if (QWindow *qtwindow = item->window())
            if (QScreen *screen = qtwindow->screen())
                return screen->size();
    }

    return {};
}

QQmlEngine *Platform::qmlEngine() const
{
    if (!m_qmlEngine)
        qWarning() << "Please call KDDockWidgets::QtQuick::Platform::self()->setQmlEngine(engine)";

    return m_qmlEngine;
}

void Platform::setQmlEngine(QQmlEngine *qmlEngine)
{
    if (m_qmlEngine) {
        qWarning() << Q_FUNC_INFO << "Already has QML engine";
        return;
    }

    if (!qmlEngine) {
        qWarning() << Q_FUNC_INFO << "Null QML engine";
        return;
    }

    m_qmlEngine = qmlEngine;

    auto dr = DockRegistry::self(); // make sure our QML types are registered
    QQmlContext *context = qmlEngine->rootContext();
    context->setContextProperty(QStringLiteral("_kddwHelpers"), m_qquickHelpers);
    context->setContextProperty(QStringLiteral("_kddwDockRegistry"), dr);
    context->setContextProperty(QStringLiteral("_kddw_widgetFactory"),
                                Config::self().viewFactory());
}

ViewFactory *Platform::viewFactory() const
{
    return static_cast<ViewFactory *>(Config::self().viewFactory());
}

Core::View *Platform::createView(Core::Controller *controller, Core::View *parent) const
{
    return new QtQuick::View(controller, Core::ViewType::None, QtQuick::asQQuickItem(parent));
}

/** static */
Platform *Platform::instance()
{
    auto p = Core::Platform::instance();
    if (p->isQtQuick())
        return static_cast<Platform *>(p);
    return nullptr;
}

bool Platform::usesFallbackMouseGrabber() const
{
    // For QtQuick we use the global event filter as mouse delivery is flaky
    // For example, the same QQuickItem that receives the press isn't receiving the mouse moves
    // when the top-level window moves.
    return true;
}

bool Platform::inDisallowedDragView(QPoint globalPos) const
{
    auto window = qobject_cast<QQuickWindow *>(qGuiApp->topLevelAt(globalPos));
    if (!window)
        return false;

    QQuickItem *item = mouseAreaForPos(window->contentItem(), globalPos);
    if (!item)
        return false;
    return item->objectName() != QLatin1String("draggableMouseArea");
}

void Platform::ungrabMouse()
{
    const QWindowList windows = qGuiApp->topLevelWindows();
    for (QWindow *window : windows) {
        if (auto quickWindow = qobject_cast<QQuickWindow *>(window)) {
            if (QQuickItem *grabber = quickWindow->mouseGrabberItem())
                grabber->ungrabMouse();
        }
    }
}

Core::DockWidget *Platform::dockWidgetForItem(QQuickItem *item)
{
    if (!item)
        return nullptr;

    if (auto dwView = qobject_cast<QtQuick::DockWidget *>(item))
        return dwView->dockWidget();

    if (auto dwi = qobject_cast<DockWidgetInstantiator *>(item))
        if (auto view = dwi->dockWidget())
            return view->dockWidget();

    return nullptr;
}
