/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Separator.h"
#include "core/Separator.h"
#include "core/Logging_p.h"
#include "core/View_p.h"
#include "core/layouting/Item_p.h"

#include "qtquick/ViewFactory.h"
#include "qtquick/Platform.h"

#include <QTimer>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

Separator::Separator(Core::Separator *controller, QQuickItem *parent)
    : QtQuick::View(controller, Core::ViewType::Separator, parent)
    , m_controller(controller)
{
}

void Separator::init()
{
    View::createItem(plat()->viewFactory()->separatorFilename().toString(), this);

    // Only set on Separator::init(), so single-shot
    QTimer::singleShot(0, this, &Separator::isVerticalChanged);
}

bool Separator::isVertical() const
{
    return m_controller->isVertical();
}

void Separator::onMousePressed()
{
    if (Core::View::d->freed())
        return;

    m_controller->onMousePress();
}

void Separator::onMouseMoved(QPointF localPos)
{
    if (Core::View::d->freed())
        return;

    const QPointF pos = QQuickItem::mapToItem(parentItem(), localPos);
    m_controller->onMouseMove(pos.toPoint());
}

void Separator::onMouseReleased()
{
    if (Core::View::d->freed())
        return;

    m_controller->onMouseReleased();
}

void Separator::onMouseDoubleClicked()
{
    if (Core::View::d->freed())
        return;

    m_controller->onMouseDoubleClick();
}

QSize Separator::minSize() const
{
    // Min sizes don't really make sense for separators, as they are fixed size
    return { 0, 0 };
}
