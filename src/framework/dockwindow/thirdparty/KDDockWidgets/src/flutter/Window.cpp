/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


#include "Window_p.h"
#include "core/Logging_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

Window::Window(std::shared_ptr<Core::View> rootView)
    : Core::Window()
    , m_rootView(rootView)
{
    if (rootView)
        setGeometry(rootView->geometry());
}

Window::~Window() = default;

std::shared_ptr<Core::View> Window::rootView() const
{
    return m_rootView;
}

Core::Window::Ptr Window::transientParent() const
{
    return nullptr;
}

void Window::setGeometry(Rect r)
{
    m_geometry = r;
}

void Window::setVisible(bool is)
{
    if (is == m_isVisible)
        return;

    // FLUTTER_TODO:  Actually hide the flutter window

    m_isVisible = is;
}

bool Window::supportsHonouringLayoutMinSize() const
{
    // maybe...
    return false;
}

void Window::setWindowState(WindowState)
{
    KDDW_WARN("Window::setWindowState: Not implemented yet");
}

Rect Window::geometry() const
{
    return m_geometry;
}

bool Window::isVisible() const
{
    return m_isVisible;
}

Core::WId Window::handle() const
{
    return Core::WId(m_rootView ? m_rootView->handle() : Core::HANDLE());
}

bool Window::equals(std::shared_ptr<Core::Window> w) const
{
    if (!w)
        return false;

    auto window = std::static_pointer_cast<flutter::Window>(w);
    return window->handle() == handle();
}

void Window::setFramePosition(Point pt)
{
    m_geometry.moveTopLeft(pt);
    m_rootView->setGeometry(m_geometry);
}

Rect Window::frameGeometry() const
{
    // FLUTTER_TODO: How to get this from flutter
    return m_geometry;
}

void Window::resize(int w, int h)
{
    Rect geo = geometry();
    geo.setSize({ w, h });
    setGeometry(geo);
}

bool Window::isActive() const
{
    KDDW_WARN("Window::isActive: Not implemented yet");
    return {};
}

WindowState Window::windowState() const
{
    KDDW_WARN("Window::windowState: Not implemented yet");
    return {};
}

Point Window::mapFromGlobal(Point) const
{
    KDDW_WARN("Window::mapFromGlobal: Not implemented yet");
    return {};
}

Point Window::mapToGlobal(Point) const
{
    KDDW_WARN("Window::mapToGlobal: Not implemented yet");
    return {};
}

void Window::destroy()
{
    KDDW_WARN("Window::destroy: Not implemented yet");
}

Size Window::minSize() const
{
    KDDW_WARN("Window::minSize: Not implemented yet");
    return {};
}

Size Window::maxSize() const
{
    KDDW_WARN("Window::maxSize: Not implemented yet");
    return {};
}

Point Window::fromNativePixels(Point) const
{
    KDDW_WARN("Window::fromNativePixels: Not implemented yet");
    return {};
}

bool Window::isFullScreen() const
{
    KDDW_WARN("Window::isFullScreen: Not implemented yet");
    return {};
}

Core::Screen::Ptr Window::screen() const
{
    KDDW_WARN("Window::screen: Not implemented yet");
    return {};
}

void Window::onScreenChanged(Core::Object *, WindowScreenChangedCallback)
{
    KDDW_WARN("Window::onScreenChange: Not implemented yet");
}
