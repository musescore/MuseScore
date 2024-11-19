/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockWidget.h"

#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/DockWidget.h"
#include "core/DockWidget_p.h"
#include "kddockwidgets/core/Group.h"
#include "qtquick/Platform.h"
#include "qtquick/views/TitleBar.h"
#include "qtquick/views/Group.h"
#include "qtquick/ViewFactory.h"
#include "qtquick/views/ViewWrapper_p.h"

#include <Config.h>
#include <QQuickItem>

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

class QtQuick::DockWidget::Private
{
public:
    Private(DockWidget *view, QQmlEngine *qmlengine)
        : q(view)
        , m_visualItem(
              DockWidget::createItem(qmlengine, plat()->viewFactory()->dockwidgetFilename().toString()))
        , m_qmlEngine(qmlengine)
    {
        Q_ASSERT(m_visualItem);
        m_visualItem->setParent(view);
        m_visualItem->setParentItem(view);
    }

    DockWidget *const q;
    QQuickItem *const m_visualItem;
    QQmlEngine *const m_qmlEngine;
};

DockWidget::DockWidget(const QString &uniqueName, DockWidgetOptions options,
                       LayoutSaverOptions layoutSaverOptions,
                       Qt::WindowFlags windowFlags, QQmlEngine *engine)
    : View(new Core::DockWidget(this, uniqueName, options, layoutSaverOptions), Core::ViewType::DockWidget,
           nullptr, windowFlags)
    , Core::DockWidgetViewInterface(asDockWidgetController())
    , d(new Private(this, engine ? engine : plat()->qmlEngine()))
{
    QQuickItem::setFlag(ItemIsFocusScope);
    setFocusPolicy(Qt::StrongFocus);

    // To mimic what QtWidgets does when creating a new QWidget.
    setVisible(false);

    auto dw = this->dockWidget();
    dw->d->actualTitleBarChanged.connect(&DockWidget::actualTitleBarChanged, this);
    dw->d->guestViewChanged.connect([this, dw] {
        if (auto guest = dw->guestView()) {
            guest->setVisible(true);
            Q_EMIT guestItemChanged();
        }
    });

    m_dockWidget->init();

    m_dockWidget->d->isFloatingChanged.connect(&DockWidget::isFloatingChanged, this);
    m_dockWidget->d->isFocusedChanged.connect(&DockWidget::isFocusedChanged, this);
    m_dockWidget->d->titleChanged.connect(&DockWidget::titleChanged, this);
    m_dockWidget->d->optionsChanged.connect(&DockWidget::optionsChanged, this);
}

DockWidget::~DockWidget()
{
    delete d;
}

void DockWidget::setGuestItem(const QString &qmlFilename, QQmlContext *context)
{
    if (QQuickItem *guest = createItem(d->m_qmlEngine, qmlFilename, context))
        setGuestItem(guest);
}

void DockWidget::setGuestItem(QQuickItem *item)
{
    auto wrapper = asQQuickWrapper(item);
    wrapper->setParent(this);
    makeItemFillParent(item);
    dockWidget()->setGuestView(wrapper);
}

QQuickItem *DockWidget::guestItem() const
{
    if (auto guest = m_dockWidget->guestView())
        return QtQuick::asQQuickItem(guest.get());

    return nullptr;
}

bool DockWidget::event(QEvent *e)
{
    if (dockWidget()->d->m_isSettingCurrent)
        return View::event(e);

    if (e->type() == QEvent::Show) {
        dockWidget()->open();
    }

    return View::event(e);
}

QSize DockWidget::minSize() const
{
    if (auto guestWidget = dockWidget()->guestView()) {
        // The guests min-size is the same as the widget's, there's no spacing or margins.

        // Return the biggest size if both are set
        return guestWidget->minSize().expandedTo(View::minSize());
    }

    return View::minSize();
}

QSize DockWidget::maxSizeHint() const
{
    if (auto guestWidget = dockWidget()->guestView()) {
        // The guests max-size is the same as the widget's, there's no spacing or margins.

        // Return the smallest size if both are set
        return guestWidget->maxSizeHint().boundedTo(View::maxSizeHint());
    }

    return View::maxSizeHint();
}

QObject *DockWidget::actualTitleBarView() const
{
    if (auto tb = actualTitleBar()) {
        return static_cast<QtQuick::TitleBar *>(tb->view());
    }

    return nullptr;
}

QQuickItem *DockWidget::groupVisualItem() const
{
    if (Core::Group *group = this->group()) {
        if (auto view = QtQuick::asView_qtquick(group->view()))
            return view->visualItem();
    }

    return nullptr;
}

void DockWidget::onGeometryUpdated()
{
    if (auto group = this->group()) {
        if (auto view = group->view()) {
            auto groupView = static_cast<Group *>(QtQuick::asView_qtquick(view));
            groupView->updateConstraints();
            groupView->updateGeometry();
        }
    }
}

QtQuick::Action *DockWidget::toggleAction() const
{
    return static_cast<QtQuick::Action *>(m_dockWidget->toggleAction());
}

QtQuick::Action *DockWidget::floatAction() const
{
    return static_cast<QtQuick::Action *>(m_dockWidget->toggleAction());
}

std::shared_ptr<Core::View> DockWidget::focusCandidate() const
{
    // For QtWidgets, if we focus the dock widget, we actually focus the user/guest widget
    // But for QtQuick, the dock widget itself is a QtQuick FocusScope, so focus that instead. QtQuick will then focus the right inner
    // widget.
    return ViewWrapper::create(const_cast<QtQuick::DockWidget *>(this));
}
