/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


#include "../qtcommon/Screen_p.h"
#include "../qtcommon/Window_p.h"
#include "../qtcommon/Platform.h"

#include <QWindow>
#include <QScreen>
#include <QVariant>

#include <QtGui/private/qhighdpiscaling_p.h>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtCommon;

Window::Window(QWindow *window)
    : m_window(window)
{
    Q_ASSERT(window);
}

Window::~Window()
{
}

void Window::onScreenChanged(QObject *context, WindowScreenChangedCallback callback)
{
    // Window_qt can't have a "screenChanged" signal since it's a short-lived object which
    // just wraps QWindow API. Instead, connects need to be done directly to QWindow
    QWindow *window = m_window; // copy before "this" is deleted
    context = context ? context : m_window;
    QObject::connect(m_window, &QWindow::screenChanged, context, [context, window, callback] {
        callback(context, Platform_qt::instance()->windowFromQWindow(window));
    });
}

void Window::setWindowState(WindowState state)
{
    m_window->setWindowState(( Qt::WindowState )state);
}

WindowState Window::windowState() const
{
    return WindowState(m_window->windowState());
}

QRect Window::geometry() const
{
    return m_window->geometry();
}

void Window::setProperty(const char *name, const QVariant &value)
{
    Q_ASSERT(m_window);
    m_window->setProperty(name, value);
}

static const char *const s_kddw_hasBeenMinimizedDirectlyFromRestore = "kddw_hasBeenMinimizedDirectlyFromRestore";
void Window::setHasBeenMinimizedDirectlyFromRestore(bool has)
{
    setProperty(s_kddw_hasBeenMinimizedDirectlyFromRestore, has);
}

bool Window::hasBeenMinimizedDirectlyFromRestore() const
{
    return property(s_kddw_hasBeenMinimizedDirectlyFromRestore).toBool();
}

bool Window::isVisible() const
{
    return m_window->isVisible();
}

WId Window::handle() const
{
    if (m_window->handle())
        return m_window->winId();
    return 0;
}

QWindow *Window::qtWindow() const
{
    return m_window;
}

bool Window::equals(std::shared_ptr<Core::Window> other) const
{
    auto otherQt = static_cast<Window *>(other.get());
    return other && otherQt->m_window == m_window;
}

void Window::setFramePosition(QPoint targetPos)
{
    m_window->setFramePosition(targetPos);
}

QRect Window::frameGeometry() const
{
    return m_window->frameGeometry();
}

void Window::resize(int width, int height)
{
    m_window->resize(width, height);
}

bool Window::isActive() const
{
    return m_window->isActive();
}

QPoint Window::mapFromGlobal(QPoint globalPos) const
{
    return m_window->mapFromGlobal(globalPos);
}

QPoint Window::mapToGlobal(QPoint localPos) const
{
    return m_window->mapToGlobal(localPos);
}

Core::Screen::Ptr Window::screen() const
{
    return std::make_shared<Screen_qt>(m_window->screen());
}

void Window::destroy()
{
    delete m_window;
}

QVariant Window::property(const char *name) const
{
    return m_window->property(name);
}

QSize Window::minSize() const
{
    return m_window->minimumSize();
}

QSize Window::maxSize() const
{
    return m_window->maximumSize();
}

QPoint Window::fromNativePixels(QPoint nativePos) const
{
    return QHighDpi::fromNativePixels(nativePos, m_window.data());
}

void Window::startSystemMove()
{
    m_window->startSystemMove();
}

void Window::setGeometry(QRect geo)
{
    m_window->setGeometry(geo);
}

void Window::setVisible(bool is)
{
    m_window->setVisible(is);
}

bool Window::isFullScreen() const
{
    return m_window->windowStates() & Qt::WindowFullScreen;
}
