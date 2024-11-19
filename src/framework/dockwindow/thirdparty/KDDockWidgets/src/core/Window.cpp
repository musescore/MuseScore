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
using namespace KDDockWidgets::Core;

Window::~Window() = default;

int Window::minWidth() const
{
    return minSize().width();
}

int Window::minHeight() const
{
    return minSize().height();
}

int Window::maxWidth() const
{
    return maxSize().width();
}

int Window::maxHeight() const
{
    return maxSize().height();
}

void Window::startSystemMove()
{
    KDDW_ERROR("Not needed in this platform");
}

Point Window::framePosition() const
{
    return frameGeometry().topLeft();
}

bool Window::containsView(View *view) const
{
    if (!view)
        return false;

    return equals(view->window());
}

bool Window::containsView(Controller *c) const
{
    if (!c)
        return false;

    return containsView(c->view());
}

Size Window::size() const
{
    return geometry().size();
}

void Window::setPosition(Point pos)
{
    Rect geo = geometry();
    geo.moveTopLeft(pos);
    setGeometry(geo);
}
