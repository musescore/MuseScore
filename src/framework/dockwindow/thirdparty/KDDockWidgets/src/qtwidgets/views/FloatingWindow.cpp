/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "FloatingWindow.h"
#include "core/FloatingWindow_p.h"
#include "kddockwidgets/core/FloatingWindow.h"
#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/MainWindow.h"
#include "kddockwidgets/core/DropArea.h"

#include "core/Logging_p.h"
#include "core/Utils_p.h"
#include "core/View_p.h"
#include "core/DragController_p.h"
#include "core/WidgetResizeHandler_p.h"
#include "core/DockRegistry_p.h"

#include "TitleBar.h"

#include <QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QVBoxLayout>
#include <QWindow>
#include <QWindowStateChangeEvent>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

class FloatingWindow::Private
{
public:
    Private(FloatingWindow *q, Core::FloatingWindow *controller)
        : m_vlayout(new QVBoxLayout(q))
        , m_controller(controller)
    {
    }

    QVBoxLayout *const m_vlayout;
    Core::FloatingWindow *const m_controller;
    bool m_connectedToScreenChanged = false;
    KDBindings::ScopedConnection m_numGroupsChangedConnection;
    KDBindings::ScopedConnection m_screenChangedConnection;
};

FloatingWindow::FloatingWindow(Core::FloatingWindow *controller,
                               QMainWindow *parent, Qt::WindowFlags windowFlags)
    : View<QWidget>(controller, Core::ViewType::FloatingWindow, parent, windowFlags)
    , d(new Private(this, controller))
{
}

FloatingWindow::~FloatingWindow()
{
    delete d;
}

void FloatingWindow::paintEvent(QPaintEvent *ev)
{
    if (Config::self().disabledPaintEvents() & Config::CustomizableWidget_FloatingWindow) {
        QWidget::paintEvent(ev);
        return;
    }

    QPainter p(this);
    QPen pen(0x666666);
    pen.setWidth(1);
    pen.setJoinStyle(Qt::MiterJoin);
    p.setPen(pen);
    const qreal halfPenWidth = p.pen().widthF() / 2;
    const QRectF rectf = rect();
    p.drawRect(rectf.adjusted(halfPenWidth, halfPenWidth, -halfPenWidth, -halfPenWidth));
}

bool FloatingWindow::event(QEvent *ev)
{
    if (ev->type() == QEvent::NonClientAreaMouseButtonDblClick
        && (Config::self().flags() & Config::Flag_NativeTitleBar)) {
        if ((windowFlags() & Qt::Tool) == Qt::Tool) {
            if (Config::self().flags() & Config::Flag_DoubleClickMaximizes) {
                // Let's refuse to maximize Qt::Tool. It's not natural.
                // Just avoid this combination: Flag_NativeTitleBar + Qt::Tool +
                // Flag_DoubleClickMaximizes
            } else {
                // Double clicking a Qt::Tool title-bar. Triggers a redocking.
                if (d->m_controller->titleBar()->isFloating()) { // redocking nested floating
                                                                 // windows aren't supported
                    d->m_controller->titleBar()->onFloatClicked();
                    return true;
                }
            }
        } else {
            // A normal Qt::Window window. The OS handles the double click.
            // In general this will maximize the window, that's the native behaviour.
        }
    } else if (ev->type() == QEvent::Show && !d->m_connectedToScreenChanged) {
        // We connect after QEvent::Show, so we have a QWindow. Qt doesn't offer much API to
        // intercept screen events
        d->m_connectedToScreenChanged = true;
        window()->onScreenChanged(this, [](QObject *, auto window) {
            DockRegistry::self()->dptr()->windowChangedScreen.emit(window);
        });

        QWidget::windowHandle()->installEventFilter(this);
    } else if (ev->type() == QEvent::ActivationChange) {
        // Since QWidget is missing a signal for window activation
        d->m_controller->dptr()->activatedChanged.emit();
    } else if (ev->type() == QEvent::StatusTip && QWidget::parent()) {
        // show status tips in the main window
        return QWidget::parent()->event(ev);
    }

    return View<QWidget>::event(ev);
}

void FloatingWindow::init()
{
    d->m_numGroupsChangedConnection = d->m_controller->dptr()->numGroupsChanged.connect([this] {
        Q_EMIT numGroupsChanged();
    });

    d->m_vlayout->setSpacing(0);
    updateMargins();
    d->m_vlayout->addWidget(View_qt::asQWidget(d->m_controller->titleBar()));
    d->m_vlayout->addWidget(View_qt::asQWidget(d->m_controller->dropArea()));

    d->m_screenChangedConnection = DockRegistry::self()->dptr()->windowChangedScreen.connect([this](Core::Window::Ptr w) {
        if (View::d->isInWindow(w))
            updateMargins();
    });
}

void FloatingWindow::updateMargins()
{
    d->m_vlayout->setContentsMargins(QMargins(4, 4, 4, 4) * logicalDpiFactor(this));
}

Core::FloatingWindow *FloatingWindow::floatingWindow() const
{
    return d->m_controller;
}

bool FloatingWindow::eventFilter(QObject *watched, QEvent *ev)
{
    if (ev->type() == QEvent::WindowStateChange) {
        // QWidget::windowState() is not reliable as it's emitted both for the spontaneous (async)
        // event and non-spontaneous (sync) The sync one being useless, as the window manager can
        // still have the old state. Only emit windowStateChanged once the window manager tells us
        // the state has actually changed See also QTBUG-102430
        if (ev->spontaneous()) {
            const auto newState = WindowState(windowHandle()->windowState());
            d->m_controller->setLastWindowManagerState(newState);
            d->m_controller->dptr()->windowStateChanged.emit();
#if defined(Q_OS_WIN)
            if (window()->hasBeenMinimizedDirectlyFromRestore() && newState != WindowState::Minimized) {
                window()->setHasBeenMinimizedDirectlyFromRestore(false);
                // restore our nice frames
                WidgetResizeHandler::requestNCCALCSIZE(HWND(window()->handle()));
            }
#endif
        }
    }

    return KDDockWidgets::QtWidgets::View<QWidget>::eventFilter(watched, ev);
}

#if defined(Q_OS_WIN)
bool FloatingWindow::nativeEvent(const QByteArray &eventType, void *message,
                                 Qt5Qt6Compat::qintptr *result)
{
    auto fw = floatingWindow();
    if (fw->beingDeleted())
        return QWidget::nativeEvent(eventType, message, result);

    if (KDDockWidgets::usesAeroSnapWithCustomDecos()) {
        // To enable aero snap we need to tell Windows where's our custom title bar
        if (WidgetResizeHandler::handleWindowsNativeEvent(fw, eventType, message, result))
            return true;
    } else if (KDDockWidgets::usesNativeTitleBar()) {
        auto msg = static_cast<MSG *>(message);
        if (msg->message == WM_SIZING) {
            // Cancel any drag if we're resizing
            Core::DragController::instance()->dragCanceled.emit();
        }
    }

    return QWidget::nativeEvent(eventType, message, result);
}
#endif
