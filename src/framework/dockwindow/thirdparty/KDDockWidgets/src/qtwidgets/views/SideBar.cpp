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
#include "core/DockWidget_p.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QAbstractButton>
#include <QStyle>
#include <QStyleOptionToolButton>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

namespace KDDockWidgets {
class SideBarButton::Private
{
public:
    Private(Core::DockWidget *dw, Core::SideBar *sideBar)
        : m_sideBar(sideBar)
        , m_dockWidget(dw)
    {
    }

    Core::SideBar *const m_sideBar;
    const QPointer<Core::DockWidget> m_dockWidget;

    // Connections to be disconnected when button is destroyed
    std::vector<KDBindings::ScopedConnection> m_connections;
};
}

SideBar::SideBar(Core::SideBar *controller, QWidget *parent)
    : View(controller, Core::ViewType::SideBar, parent)
    , SideBarViewInterface(controller)
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

    button->d->m_connections.push_back(dw->d->titleChanged.connect(&SideBarButton::setText, button));
    button->d->m_connections.push_back(dw->d->isOverlayedChanged.connect([button] { button->update(); }));
    button->d->m_connections.push_back(dw->d->removedFromSideBar.connect(&QObject::deleteLater, button));

    connect(dw, &QObject::destroyed, button, &QObject::deleteLater);
    connect(button, &SideBarButton::clicked, this, [this, dw] { m_sideBar->onButtonClicked(dw); });

    const int count = m_layout->count();
    m_layout->insertWidget(count - 1, button);
}

void SideBar::removeDockWidget_Impl(Core::DockWidget *)
{
    // Nothing is needed. Button is removed automatically.
}

SideBarButton *SideBar::createButton(Core::DockWidget *dw,
                                     SideBar *parent) const
{
    return new SideBarButton(dw, parent);
}

SideBarButton::SideBarButton(Core::DockWidget *dw, QtWidgets::SideBar *parent)
    : QToolButton(parent)
    , d(new Private(dw, parent->sideBar()))
{
}

SideBarButton::~SideBarButton()
{
    delete d;
}

bool SideBarButton::isVertical() const
{
    return d->m_sideBar->isVertical();
}

void SideBarButton::paintEvent(QPaintEvent *)
{
    if (!d->m_dockWidget) {
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
