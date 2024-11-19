#include "Separator.h"
/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "View.h"

#include "Config.h"
#include "core/Logging_p.h"
#include "core/View_p.h"
#include "kddockwidgets/core/Separator.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

Separator::Separator(Core::Separator *controller, Core::View *parent)
    : View(controller, Core::ViewType::Separator, View_qt::asQWidget(parent))
    , m_controller(controller)
{
    setMouseTracking(true);
}

void Separator::paintEvent(QPaintEvent *ev)
{
    if (d->freed())
        return;

    if (KDDockWidgets::Config::self().disabledPaintEvents()
        & KDDockWidgets::Config::CustomizableWidget_Separator) {
        QWidget::paintEvent(ev);
        return;
    }

    QPainter p(this);

    QStyleOption opt;
    opt.palette = palette();
    opt.rect = QWidget::rect();
    opt.state = QStyle::State_None;
    if (!m_controller->isVertical())
        opt.state |= QStyle::State_Horizontal;

    if (isEnabled())
        opt.state |= QStyle::State_Enabled;

    QWidget::parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, this);
}

void Separator::enterEvent(KDDockWidgets::Qt5Qt6Compat::QEnterEvent *)
{
    if (d->freed())
        return;

    if (m_controller->isVertical())
        setCursor(Qt::SizeVerCursor);
    else
        setCursor(Qt::SizeHorCursor);
}

void Separator::leaveEvent(QEvent *)
{
    setCursor(Qt::ArrowCursor);
}

void Separator::mousePressEvent(QMouseEvent *)
{
    if (d->freed())
        return;

    m_controller->onMousePress();
}

void Separator::mouseMoveEvent(QMouseEvent *ev)
{
    if (d->freed())
        return;

    m_controller->onMouseMove(mapToParent(ev->pos()));
}

void Separator::mouseReleaseEvent(QMouseEvent *)
{
    if (d->freed())
        return;

    m_controller->onMouseReleased();
}

void Separator::mouseDoubleClickEvent(QMouseEvent *)
{
    if (d->freed())
        return;

    m_controller->onMouseDoubleClick();
}
