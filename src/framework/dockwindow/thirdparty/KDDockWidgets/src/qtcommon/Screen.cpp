/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Screen_p.h"

#include <QScreen>

using namespace KDDockWidgets;

Screen_qt::Screen_qt(QScreen *screen)
    : m_screen(screen)
{
}

QString Screen_qt::name() const
{
    return m_screen->name();
}

QSize Screen_qt::size() const
{
    return m_screen->size();
}

QRect Screen_qt::geometry() const
{
    return m_screen->geometry();
}

qreal Screen_qt::devicePixelRatio() const
{
    return m_screen->devicePixelRatio();
}

QSize Screen_qt::availableSize() const
{
    return m_screen->availableSize();
}

QRect Screen_qt::availableGeometry() const
{
    return m_screen->availableGeometry();
}

QSize Screen_qt::virtualSize() const
{
    return m_screen->virtualSize();
}

QRect Screen_qt::virtualGeometry() const
{
    return m_screen->virtualGeometry();
}

QScreen *Screen_qt::qtScreen() const
{
    return m_screen;
}

bool Screen_qt::equals(std::shared_ptr<Screen> other) const
{
    auto otherQt = static_cast<Screen_qt *>(other.get());
    return otherQt && otherQt->m_screen == m_screen;
}
