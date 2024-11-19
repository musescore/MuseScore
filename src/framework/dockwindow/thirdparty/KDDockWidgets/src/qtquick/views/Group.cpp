/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief The GUI counterpart of Frame.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "Group.h"

#include "qtquick/views/DockWidget.h"
#include "qtquick/ViewFactory.h"
#include "qtquick/Platform.h"
#include "qtquick/views/TabBar.h"
#include "qtquick/views/ViewWrapper_p.h"

#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/Stack.h"
#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/DockWidget.h"
#include "core/DockWidget_p.h"
#include "core/Group_p.h"
#include "core/layouting/Item_p.h"
#include "core/Logging_p.h"
#include "core/MDILayout.h"

#include "Stack.h"
#include "Config.h"
#include "core/WidgetResizeHandler_p.h"
#include "core/TabBar_p.h"

#include <QDebug>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets::QtQuick {

class Group::Private
{
public:
    KDBindings::ScopedConnection isMDIConnection;
    KDBindings::ScopedConnection currentDockWidgetChangedConnection;
    KDBindings::ScopedConnection updateConstraintsConnection;
};

}

Group::Group(Core::Group *controller, QQuickItem *parent)
    : QtQuick::View(controller, Core::ViewType::Group, parent)
    , Core::GroupViewInterface(controller)
    , d(new Private())
{
}

Group::~Group()
{
    delete d;

    // The QML item must be deleted with deleteLater(), as we might be currently with its mouse
    // handler in the stack. QML doesn't support it being deleted in that case.
    // So unparent it and deleteLater().
    m_visualItem->setParent(nullptr);
    m_visualItem->deleteLater();
}

void Group::init()
{
    d->updateConstraintsConnection = m_group->tabBar()->dptr()->countChanged.connect([this] {
        updateConstraints();
    });

    d->currentDockWidgetChangedConnection = m_group->tabBar()->dptr()->currentDockWidgetChanged.connect([this] {
        currentDockWidgetChanged();
    });

    connect(this, &View::geometryUpdated, this,
            [this] { Core::View::d->layoutInvalidated.emit(); });

    d->isMDIConnection = m_group->dptr()->isMDIChanged.connect([this] { Q_EMIT isMDIChanged(); });

    // Minor hack: While the controllers keep track of "current widget",
    // the QML StackLayout deals in "current index", these can differ when removing a non-current
    // tab. The currentDockWidgetChanged() won't be emitted but the index did decrement.
    // As a workaround, always emit the signal, which is harmless if not needed.

    m_group->dptr()->numDockWidgetsChanged.connect([this] { Q_EMIT currentDockWidgetChanged(); });
    m_group->dptr()->actualTitleBarChanged.connect([this] { Q_EMIT actualTitleBarChanged(); });

    connect(this, &View::itemGeometryChanged, this, [this] {
        const auto docks = m_group->dockWidgets();
        for (auto dw : docks) {
            auto dwView = static_cast<DockWidget *>(QtQuick::asView_qtquick(dw->view()));
            Q_EMIT dwView->groupGeometryChanged(geometry());
        }
    });

    QQmlComponent component(plat()->qmlEngine(), plat()->viewFactory()->groupFilename());

    m_visualItem = static_cast<QQuickItem *>(component.create());

    if (!m_visualItem) {
        qWarning() << Q_FUNC_INFO << "Failed to create item" << component.errorString();
        return;
    }

    m_visualItem->setProperty("groupCpp", QVariant::fromValue(this));
    m_visualItem->setParentItem(this);
    m_visualItem->setParent(this);
}

void Group::updateConstraints()
{
    m_group->onDockWidgetCountChanged();

    // QtQuick doesn't have layouts, so we need to do constraint propagation manually

    setProperty("kddockwidgets_min_size", minSize());
    setProperty("kddockwidgets_max_size", maxSizeHint());

    Core::View::d->layoutInvalidated.emit();
}

void Group::removeDockWidget(Core::DockWidget *dw)
{
    m_group->tabBar()->removeDockWidget(dw);
}

int Group::currentIndex() const
{
    return m_group->currentIndex();
}

void Group::insertDockWidget(Core::DockWidget *dw, int index)
{
    QPointer<Core::Group> oldFrame = dw->d->group();
    m_group->tabBar()->insertDockWidget(index, dw, {}, {});

    dw->setParentView(ViewWrapper::create(m_stackLayout).get());
    makeItemFillParent(View::asQQuickItem(dw->view()));
    m_group->setCurrentDockWidget(dw);

    if (oldFrame && oldFrame->beingDeletedLater()) {
        // give it a push and delete it immediately.
        // Having too many deleteLater() puts us in an inconsistent state. For example if
        // LayoutSaver::saveState() would to be called while the Frame hadn't been deleted yet
        // it would count with that group unless hacks. Also the unit-tests are full of
        // waitForDeleted() due to deleteLater.

        // Ideally we would just remove the deleteLater from Group.cpp, but
        // QTabWidget::insertTab() would crash, as it accesses the old tab-widget we're stealing
        // from

        delete oldFrame;
    }
}

void Group::setStackLayout(QQuickItem *stackLayout)
{
    if (m_stackLayout || !stackLayout) {
        qWarning() << Q_FUNC_INFO << "Shouldn't happen";
        return;
    }

    m_stackLayout = stackLayout;
}

void Group::startMDIResize()
{
    if (auto handler = m_group->resizeHandler()) {
        if (handler->enabled()) {
            /// Doesn't happen, but let's be vigilant
            KDDW_ERROR("Group::startMDIResize: Handler is already enabled!");
        } else {
            handler->setEnabled(true);
        }
    } else {
        KDDW_ERROR("Group::startMDIResize: No WidgetResizeHandler found. isMDI={}", isMDI());
    }
}

QSize Group::minSize() const
{
    const QSize contentsSize = m_group->dockWidgetsMinSize();
    return contentsSize + QSize(0, nonContentsHeight());
}

QSize Group::maxSizeHint() const
{
    if (isMDI()) {
        const auto dockwidgets = m_group->dockWidgets();
        if (dockwidgets.size() == 1) {
            auto dw = dockwidgets[0];
            if (dw->inDtor()) {
                // Destruction case, nothing to do
                return View::maxSizeHint();
            }

            return (dw->view()->maxSizeHint() + QSize(0, nonContentsHeight())).boundedTo(Core::Item::hardcodedMaximumSize);
        } else {
            KDDW_WARN("Group::maxSizeHint: Max size not supported for mixed MDI case yet");
        }
    }

    return View::maxSizeHint();
}

QObject *Group::tabBarObj() const
{
    return tabBarView();
}

QQuickItem *Group::visualItem() const
{
    return m_visualItem;
}

int Group::nonContentsHeight() const
{
    return m_visualItem->property("nonContentsHeight").toInt();
}

Stack *Group::stackView() const
{
    if (auto stack = m_group->stack())
        return qobject_cast<Stack *>(asQQuickItem(stack->view()));

    return nullptr;
}

TabBar *Group::tabBarView() const
{
    if (auto tabBar = m_group->tabBar())
        return qobject_cast<TabBar *>(asQQuickItem(tabBar->view()));

    return nullptr;
}

KDDockWidgets::QtQuick::TitleBar *Group::titleBar() const
{
    if (auto tb = m_group->titleBar()) {
        return dynamic_cast<KDDockWidgets::QtQuick::TitleBar *>(tb->view());
    }

    return nullptr;
}

KDDockWidgets::QtQuick::TitleBar *Group::actualTitleBar() const
{
    if (auto tb = m_group->actualTitleBar()) {
        return dynamic_cast<KDDockWidgets::QtQuick::TitleBar *>(tb->view());
    }

    return nullptr;
}

int Group::userType() const
{
    return 0;
}

void Group::setMDISize(QSize sz)
{
    if (!isMDI() || m_inDtor)
        return;

    auto layout = m_group->mdiLayout();
    if (!layout)
        return;

    layout->resizeDockWidget(m_group, sz);
}
