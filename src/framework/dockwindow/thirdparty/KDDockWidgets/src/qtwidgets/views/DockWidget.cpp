/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockWidget.h"
#include "core/DockWidget_p.h"
#include "ViewWrapper_p.h"

#include <QCloseEvent>
#include <QVBoxLayout>
#include <QAction>

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

class DockWidget::Private
{
public:
    explicit Private(DockWidget *q)
        : layout(new QVBoxLayout(q))
    {
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        // propagate the max-size constraints from the guest widget to the DockWidget
        layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    }

    QVBoxLayout *const layout;
    KDBindings::ScopedConnection optionsChangedConnection;
    KDBindings::ScopedConnection guestViewChangedConnection;
    KDBindings::ScopedConnection isFocusedChangedConnection;
    KDBindings::ScopedConnection isFloatingChangedConnection;
    KDBindings::ScopedConnection isOpenChangedConnection;
    KDBindings::ScopedConnection windowActiveAboutToChangeConnection;
    KDBindings::ScopedConnection guestChangeConnection;
    KDBindings::ScopedConnection isCurrentTabConnection;
};

DockWidget::DockWidget(const QString &uniqueName, DockWidgetOptions options,
                       LayoutSaverOptions layoutSaverOptions,
                       Qt::WindowFlags windowFlags)
    : View<QWidget>(new Core::DockWidget(this, uniqueName, options, layoutSaverOptions),
                    Core::ViewType::DockWidget, nullptr, windowFlags)
    , Core::DockWidgetViewInterface(asDockWidgetController())
    , d(new Private(this))
{
    d->guestChangeConnection = m_dockWidget->d->guestViewChanged.connect([this] {
        if (auto guest = widget()) {
            QWidget::setSizePolicy(guest->sizePolicy());
            d->layout->addWidget(guest);
        }
    });

    d->optionsChangedConnection = m_dockWidget->d->optionsChanged.connect([this](KDDockWidgets::DockWidgetOptions opts) {
        Q_EMIT optionsChanged(opts);
    });

    d->guestViewChangedConnection = m_dockWidget->d->guestViewChanged.connect([this] {
        Q_EMIT guestViewChanged();
    });

    d->isFocusedChangedConnection = m_dockWidget->d->isFocusedChanged.connect([this](bool focused) {
        Q_EMIT isFocusedChanged(focused);
    });

    d->isFloatingChangedConnection = m_dockWidget->d->isFloatingChanged.connect([this](bool floating) {
        Q_EMIT isFloatingChanged(floating);
    });

    d->isOpenChangedConnection = m_dockWidget->d->isOpenChanged.connect([this](bool open) {
        Q_EMIT isOpenChanged(open);
    });

    d->windowActiveAboutToChangeConnection = m_dockWidget->d->windowActiveAboutToChange.connect([this](bool active) {
        Q_EMIT windowActiveAboutToChange(active);
    });

    d->isCurrentTabConnection = m_dockWidget->d->isCurrentTabChanged.connect([this](bool isCurrent) {
        Q_EMIT isCurrentTabChanged(isCurrent);
    });

    m_dockWidget->init();
}

DockWidget::~DockWidget()
{
    delete d;
}

void DockWidget::setWidget(QWidget *widget)
{
    m_dockWidget->setGuestView(ViewWrapper::create(widget));
}

bool DockWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Show)
        m_dockWidget->open();

    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    return QtWidgets::View<QWidget>::event(e);
}

void DockWidget::resizeEvent(QResizeEvent *e)
{
    m_dockWidget->onResize(e->size());

    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    QWidget::resizeEvent(e);
}

QWidget *DockWidget::widget() const
{
    if (auto guest = m_dockWidget->guestView())
        return View_qt::asQWidget(guest.get());

    return nullptr;
}

QAction *DockWidget::toggleAction() const
{
    return dynamic_cast<QAction *>(m_dockWidget->toggleAction());
}

QAction *DockWidget::floatAction() const
{
    return dynamic_cast<QAction *>(m_dockWidget->floatAction());
}

std::shared_ptr<Core::View> DockWidget::focusCandidate() const
{
    // For QtWidgets, if we focus the dock widget, we actually focus the user/guest widget
    // For QtQuick, the dock widget itself is a QtQuick FocusScope, so focus that instead. QtQuick will then focus the right inner
    // widget.
    return m_dockWidget->guestView();
}

void DockWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    // The guest widget didn't want it, so block it now.
    // Otherwise it goes to the QTabWidget and it will make the window float.
    ev->accept();
}
