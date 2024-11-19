/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "TabBar.h"
#include "DockWidget.h"
#include "Stack.h"
#include "kddockwidgets/core/DockWidget.h"
#include "kddockwidgets/core/TabBar.h"
#include "kddockwidgets/core/Stack.h"
#include "core/Utils_p.h"
#include "core/TabBar_p.h"
#include "core/Logging_p.h"
#include "Config.h"
#include "qtwidgets/ViewFactory.h"
#include "kddockwidgets/core/DockRegistry.h"

#include <QMouseEvent>
#include <QApplication>
#include <QProxyStyle>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

namespace KDDockWidgets {
namespace { // anonymous namespace to silence -Wweak-vtables
class MyProxy : public QProxyStyle
{
    Q_OBJECT
public:
    MyProxy()
        : QProxyStyle(qApp->style())
    {
        setParent(qApp);
    }

    int styleHint(QStyle::StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override
    {
        if (hint == QStyle::SH_Widget_Animation_Duration) {
            // QTabBar has a bug which causes the paint event to dereference a tab which was already
            // removed. Because, after the tab being removed, the d->pressedIndex is only reset
            // after the animation ends. So disable the animation. Crash can be repro by enabling
            // movable tabs, and detaching a tab quickly from a floating window containing two dock
            // widgets. Reproduced on Windows
            return 0;
        }
        return baseStyle()->styleHint(hint, option, widget, returnData);
    }
};
}

static MyProxy *proxyStyle()
{
    static auto *proxy = new MyProxy;
    return proxy;
}

class QtWidgets::TabBar::Private
{
public:
    explicit Private(Core::TabBar *controller)
        : m_controller(controller)
    {
    }

    void onTabMoved(int from, int to);

    Core::TabBar *const m_controller;
    KDBindings::ScopedConnection m_currentDockWidgetChangedConnection;
};

}

TabBar::TabBar(Core::TabBar *controller, QWidget *parent)
    : View(controller, Core::ViewType::TabBar, parent)
    , TabBarViewInterface(controller)
    , d(new Private(controller))
{
    setStyle(proxyStyle());
}

TabBar::~TabBar()
{
    delete d;
}

void TabBar::init()
{
    connect(this, &QTabBar::currentChanged, m_tabBar, &Core::TabBar::setCurrentIndex);
    connect(this, &QTabBar::tabMoved, this, [this](int from, int to) {
        d->onTabMoved(from, to);
    });

    d->m_currentDockWidgetChangedConnection = d->m_controller->dptr()->currentDockWidgetChanged.connect([this](KDDockWidgets::Core::DockWidget *dw) {
        Q_EMIT currentDockWidgetChanged(dw);
    });
}

int TabBar::tabAt(QPoint localPos) const
{
    return QTabBar::tabAt(localPos);
}

void TabBar::mousePressEvent(QMouseEvent *e)
{
    d->m_controller->onMousePress(e->pos());
    QTabBar::mousePressEvent(e);
}

void TabBar::mouseMoveEvent(QMouseEvent *e)
{
    if (count() > 1) {
        // Only allow to re-order tabs if we have more than 1 tab, otherwise it's just weird.
        QTabBar::mouseMoveEvent(e);
    }
}

void TabBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    d->m_controller->onMouseDoubleClick(e->pos());
}

bool TabBar::event(QEvent *ev)
{
    // Qt has a bug in QWidgetPrivate::deepestFocusProxy(), it doesn't honour visibility
    // of the focus scope. Once an hidden widget is focused the chain is broken and tab
    // stops working (#180)

    auto parent = parentWidget();
    if (!parent) {
        // NOLINTNEXTLINE(bugprone-parent-virtual-call)
        return QTabBar::event(ev);
    }

    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    const bool result = QTabBar::event(ev);

    if (ev->type() == QEvent::Show) {
        parent->setFocusProxy(this);
    } else if (ev->type() == QEvent::Hide) {
        parent->setFocusProxy(nullptr);
    }

    return result;
}

QString TabBar::text(int index) const
{
    return tabText(index);
}

QRect TabBar::rectForTab(int index) const
{
    return QTabBar::tabRect(index);
}

void TabBar::moveTabTo(int from, int to)
{
    moveTab(from, to);
}

void TabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    Q_EMIT dockWidgetInserted(index);
    Q_EMIT countChanged();
}

void TabBar::tabRemoved(int index)
{
    QTabBar::tabRemoved(index);
    Q_EMIT dockWidgetRemoved(index);
    Q_EMIT countChanged();
}

void TabBar::setCurrentIndex(int index)
{
    QTabBar::setCurrentIndex(index);
}

QTabWidget *TabBar::tabWidget() const
{
    if (auto tw = dynamic_cast<Stack *>(d->m_controller->stack()->view()))
        return tw;

    qWarning() << Q_FUNC_INFO << "Unexpected null QTabWidget";
    return nullptr;
}

void TabBar::renameTab(int index, const QString &text)
{
    setTabText(index, text);
}

void TabBar::changeTabIcon(int index, const QIcon &icon)
{
    setTabIcon(index, icon);
}

void TabBar::removeDockWidget(Core::DockWidget *dw)
{
    auto tabWidget = static_cast<QTabWidget *>(View_qt::asQWidget(m_tabBar->stack()));
    tabWidget->removeTab(m_tabBar->indexOfDockWidget(dw));
}

void TabBar::insertDockWidget(int index, Core::DockWidget *dw, const QIcon &icon,
                              const QString &title)
{
    auto tabWidget = static_cast<QTabWidget *>(View_qt::asQWidget(m_tabBar->stack()));
    tabWidget->insertTab(index, View_qt::asQWidget(dw), icon, title);
}

void TabBar::setTabsAreMovable(bool are)
{
    QTabBar::setMovable(are);
}

Core::TabBar *TabBar::tabBar() const
{
    return d->m_controller;
}

void TabBar::Private::onTabMoved(int from, int to)
{
    if (from == to || m_controller->isMovingTab())
        return;

    // !m_controller->isMovingTab() means the move was initiated by Qt
    // for example the user is reordering tabs with mouse
    // We need to tell the controller we got a new order.
    m_controller->dptr()->moveTabTo(from, to);
}

#include "TabBar.moc"
