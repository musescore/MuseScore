/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Screen_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

Screen::~Screen() = default;

QString Screen::name() const
{
    return QStringLiteral("dummy-screen");
}

Size Screen::size() const
{
    return geometry().size();
}

Rect Screen::geometry() const
{
    return Rect(0, 0, 1920, 1080);
}

double Screen::devicePixelRatio() const
{
    return 1.0;
}

Size Screen::availableSize() const
{
    return availableGeometry().size();
}

Rect Screen::availableGeometry() const
{
    return geometry();
}

Size Screen::virtualSize() const
{
    return size();
}

Rect Screen::virtualGeometry() const
{
    return availableGeometry();
}

bool Screen::equals(std::shared_ptr<Core::Screen>) const
{
    return true;
}
