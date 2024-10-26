/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "SideBar.h"

#include "kddockwidgets/core/DockWidget.h"
#include "kddockwidgets/core/SideBar.h"
#include "kddockwidgets/core/MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QAbstractButton>
#include <QStyle>
#include <QStyleOptionToolButton>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

SideBar::SideBar(Core::SideBar *controller, QWidget *parent)
    : View(controller, Type::SideBar, parent)
    , Core::SideBarViewInterface(controller)
{
}

void SideBar::init()
{
    if (m_sideBar->isVertical())
        m_layout = new QVBoxLayout(this);
    else
        m_layout = new QHBoxLayout(this);

    m_layout->setSpacing(1);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addStretch();
}

void SideBar::addDockWidget_Impl(Core::DockWidget *dw)
{
    auto button = createButton(dw, this);
    button->setText(dw->title());
    connect(dw, &Core::DockWidget::titleChanged, button, &SideBarButton::setText);
    connect(dw, &Core::DockWidget::isOverlayedChanged, button,
            [button] { button->update(); });
    connect(dw, &Core::DockWidget::removedFromSideBar, button, &QObject::deleteLater);
    connect(dw, &QObject::destroyed, button, &QObject::deleteLater);
    connect(button, &SideBarButton::clicked, this, [this, dw] { m_sideBar->onButtonClicked(dw); });

    const int count = m_layout->count();
    m_layout->insertWidget(count - 1, button);
}

void SideBar::removeDockWidget_Impl(Core::DockWidget *)
{
    // Nothing is needed. Button is removed automatically.
}

bool SideBar::isVertical() const
{
    return m_sideBar->isVertical();
}

SideBarButton *SideBar::createButton(Core::DockWidget *dw,
                                     SideBar *parent) const
{
    return new SideBarButton(dw, parent);
}

SideBarButton::SideBarButton(Core::DockWidget *dw, SideBar *parent)
    : QToolButton(parent)
    , m_sideBar(parent)
    , m_dockWidget(dw)
{
}

bool SideBarButton::isVertical() const
{
    return m_sideBar->isVertical();
}

void SideBarButton::paintEvent(QPaintEvent *)
{
    if (!m_dockWidget) {
        // Can happen during destruction
        return;
    }

    // Draw to an horizontal button, it's easier. Rotate later.
    QPixmap pixmap((isVertical() ? size().transposed() : size()) * devicePixelRatioF());
    pixmap.setDevicePixelRatio(devicePixelRatioF());

    {
        pixmap.fill(Qt::transparent);

        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        const bool isHovered = opt.state & QStyle::State_MouseOver;
        // const bool isOverlayed = m_dockWidget->isOverlayed(); // We could style different if it's
        // open const bool isHoveredOrOverlayed = isHovered || isOverlayed;

        QPainter p(&pixmap);

        const QRect r = isVertical() ? rect().transposed() : rect();
        const QRect textRect = r.adjusted(3, 0, 5, 0);
        // p.drawRect(r.adjusted(0, 0, -1, -1));
        p.setPen(palette().color(QPalette::Text));
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());

        QPen pen(isHovered ? palette().color(QPalette::Highlight)
                           : palette().color(QPalette::Highlight).darker());
        pen.setWidth(isHovered ? 2 : 1);
        p.setPen(pen);
        p.drawLine(3, r.bottom() - 1, r.width() - 3 * 2, r.bottom() - 1);
    }

    QPainter p(this);
    if (isVertical()) {
        pixmap = pixmap.transformed(QTransform().rotate(90));
    }

    p.drawPixmap(rect(), pixmap);
}

QSize SideBarButton::sizeHint() const
{
    const QSize hint = QToolButton::sizeHint();
    return isVertical() ? (hint.transposed() + QSize(2, 0)) : (hint + QSize(0, 2));
}
