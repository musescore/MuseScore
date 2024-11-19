/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "FloatingWindow.h"
#include "Config.h"
#include "qtquick/ViewFactory.h"

#include "TitleBar.h"

#include "core/Logging_p.h"
#include "core/Utils_p.h"
#include "core/View_p.h"
#include "core/FloatingWindow_p.h"
#include "core/layouting/Item_p.h"

#include "kddockwidgets/core/DropArea.h"
#include "kddockwidgets/core/FloatingWindow.h"
#include "kddockwidgets/core/MainWindow.h"
#include "kddockwidgets/core/TitleBar.h"

#include "core/WidgetResizeHandler_p.h"
#include "qtquick/Platform.h"
#include "qtquick/Window_p.h"
#include "qtquick/views/MainWindow.h"
#include "qtquick/views/TitleBar.h"
#include "qtquick/views/DropArea.h"

#include <QQuickView>
#include <QDebug>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets {

class QuickView : public QQuickView
{
    Q_OBJECT
public:
    explicit QuickView(QQmlEngine *qmlEngine, FloatingWindow *view)
        : QQuickView(qmlEngine, nullptr)
        , m_view(view)
    {
        if (Config::self().internalFlags() & Config::InternalFlag_UseTransparentFloatingWindow)
            setColor(QColor(Qt::transparent));

        updateSize();

        auto item = asQQuickItem(view);
        connect(item, &QQuickItem::widthChanged, this, &QuickView::onRootItemWidthChanged);
        connect(item, &QQuickItem::heightChanged, this, &QuickView::onRootItemHeightChanged);
    }

    ~QuickView() override;

    bool event(QEvent *ev) override
    {
        if (ev->type() == QEvent::FocusAboutToChange) {
            // qquickwindow.cpp::event(FocusAboutToChange) removes the item grabber. Inibit that
            return true;
        } else if (ev->type() == QEvent::Resize) {
            updateRootItemSize();
        } else if (isNonClientMouseEvent(ev) || ev->type() == QEvent::Move) {
            // Mimic QWidget behaviour: The non-client mouse events go to the QWidget not the
            // QWindow. In our case the QQuickItem. I mean, they also go to QWindow, but for our
            // QtWidgets impl we process them at the QWidget level, so use the same approach so we
            // maintain a single code path for processing mouse events
            Core::Platform::instance()->sendEvent(m_view, ev);
            return true;
        }

        if (ev->type() == QEvent::Expose) {
            Core::AtomicSanityChecks checks(m_view->rootItem());
            auto res = QQuickView::event(ev);
            return res;
        } else {
            return QQuickView::event(ev);
        }
    }

    void onRootItemWidthChanged()
    {
        setWidth(int(m_view->width()));
    }

    void onRootItemHeightChanged()
    {
        setHeight(int(m_view->height()));
    }

    void updateSize()
    {
        resize(m_view->Core::View::size());
    }

    void updateRootItemSize()
    {
        if (m_view->asFloatingWindowController()->beingDeleted())
            return;

        Core::AtomicSanityChecks checks(m_view->rootItem());
        m_view->Core::View::setSize(size());
    }

#ifdef KDDW_FRONTEND_QT_WINDOWS
    bool nativeEvent(const QByteArray &eventType, void *message,
                     Qt5Qt6Compat::qintptr *result) override
    {
        // To enable aero snap we need to tell Windows where's our custom title bar
        Core::FloatingWindow *fw = m_view->asFloatingWindowController();
        if (!fw->beingDeleted()
            && WidgetResizeHandler::handleWindowsNativeEvent(fw, eventType, message, result))
            return true;

        return QWindow::nativeEvent(eventType, message, result);
    }
#endif
private:
    FloatingWindow *const m_view;
};

QuickView::~QuickView() = default;

}


FloatingWindow::FloatingWindow(Core::FloatingWindow *controller,
                               QtQuick::MainWindow *parent,
                               Qt::WindowFlags flags)
    : QtQuick::View(controller, Core::ViewType::FloatingWindow, parent, flags)
    , m_quickWindow(new QuickView(plat()->qmlEngine(), this))
    , m_controller(controller)
{
    connect(m_quickWindow, &QWindow::windowStateChanged, this, &FloatingWindow::onWindowStateChanged);
}

FloatingWindow::~FloatingWindow()
{
    m_inDtor = true;
    setParent(static_cast<Core::View *>(nullptr));
    if (qobject_cast<QQuickView *>(m_quickWindow)) // QObject cast just to make sure the QWindow is
                                                   // not in ~QObject already
        delete m_quickWindow;
}

QSize FloatingWindow::minSize() const
{
    // Doesn't matter if it's not visible. We don't want the min-size to jump around. Also not so
    // easy to track as we don't have layouts
    const int margins = contentsMargins();
    return m_controller->multiSplitter()->view()->minSize() + QSize(0, titleBarHeight())
        + QSize(margins * 2, margins * 2);
}

void FloatingWindow::setGeometry(QRect geo)
{
    // Not needed with QtWidgets, but needed with QtQuick as we don't have layouts
    geo.setSize(geo.size().expandedTo(minSize()));

    parentItem()->setSize(geo.size());
    m_quickWindow->setGeometry(geo);
}

int FloatingWindow::contentsMargins() const
{
    return m_visualItem->property("margins").toInt();
}

int FloatingWindow::titleBarHeight() const
{
    return m_visualItem->property("titleBarHeight").toInt();
}

QWindow *FloatingWindow::candidateParentWindow() const
{
    if (auto mainWindow = qobject_cast<MainWindow *>(QObject::parent())) {
        return mainWindow->QQuickItem::window();
    }

    return nullptr;
}

void FloatingWindow::init()
{
    if (QWindow *transientParent = candidateParentWindow()) {
        m_quickWindow->setTransientParent(candidateParentWindow());
        // This mimics the QWidget behaviour, where we not only have a transient parent but also
        // a parent for cleanup. Calling QWindow::setParent() here would clip it to the parent
        m_quickWindow->QObject::setParent(transientParent);
        m_quickWindow->setObjectName(QStringLiteral("Floating QWindow with parent")); // for debug
    } else {
        m_quickWindow->setObjectName(QStringLiteral("Floating QWindow"));
    }

    setParent(m_quickWindow->contentItem());
    WidgetResizeHandler::setupWindow(Core::Window::Ptr(new QtQuick::Window(m_quickWindow)));
    m_quickWindow->installEventFilter(this); // for window resizing
    m_controller->maybeCreateResizeHandler();

    m_visualItem = createItem(m_quickWindow->engine(),
                              plat()->viewFactory()->floatingWindowFilename().toString());
    Q_ASSERT(m_visualItem);

    // Ensure our window size is never smaller than our min-size
    Core::View::setSize(Core::View::size().expandedTo(minSize()));

    m_visualItem->setParent(this);
    m_visualItem->setParentItem(this);

    m_quickWindow->setFlags(flags());

    m_controller->updateTitleAndIcon();

#ifdef KDDW_FRONTEND_QT_WINDOWS
    // Workaround for QTBUG-120269 , no alt-tab thumbail if starting minimized.
    // Do a nice workaround of rendering transparently then minimizing that
    if (m_controller->floatingWindowFlags() & FloatingWindowFlag::StartsMinimized) {
        m_quickWindow->setOpacity(0.0);
        auto guard = new QObject(this);
        connect(m_quickWindow, &QQuickView::frameSwapped, guard, [this, guard] {
            if (m_controller->dptr()->m_minimizationPending) {
                delete guard;
                m_controller->dptr()->m_minimizationPending = false;
                showMinimized();
                m_quickWindow->setOpacity(1);
            }
        });
    }
#endif

    m_quickWindow->show();

    connect(this, &QQuickItem::visibleChanged, this, [this] {
        if (!isVisible() && !Core::View::d->aboutToBeDestroyed())
            m_controller->scheduleDeleteLater();
    });
}

QObject *FloatingWindow::titleBar() const
{
    if (auto tb = m_controller->titleBar())
        return qobject_cast<TitleBar *>(asQQuickItem(tb->view()));

    return nullptr;
}

QObject *FloatingWindow::dropArea() const
{
    if (auto da = m_controller->dropArea())
        return qobject_cast<DropArea *>(asQQuickItem(da->view()));

    return nullptr;
}

Core::Item *FloatingWindow::rootItem() const
{
    if (auto da = m_controller->dropArea())
        return da->rootItem();
    return nullptr;
}

void FloatingWindow::onWindowStateChanged(Qt::WindowState state)
{
    const auto newState = WindowState(state);
    m_controller->setLastWindowManagerState(newState);
    m_controller->dptr()->windowStateChanged.emit();
#if defined(Q_OS_WIN)
    if (window()->hasBeenMinimizedDirectlyFromRestore() && newState != WindowState::Minimized) {
        window()->setHasBeenMinimizedDirectlyFromRestore(false);
        // restore our nice frames
        WidgetResizeHandler::requestNCCALCSIZE(HWND(window()->handle()));
    }
#endif
}

#include "FloatingWindow.moc"
