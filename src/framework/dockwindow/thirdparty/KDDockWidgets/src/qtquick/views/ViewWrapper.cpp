/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "ViewWrapper_p.h"
#include "qtquick/views/RubberBand.h"
#include "qtquick/views/View.h"
#include "core/View_p.h"

#include "qtquick/views/DockWidget.h"
#include "qtquick/views/FloatingWindow.h"
#include "qtquick/views/Group.h"
#include "qtquick/views/MainWindow.h"
#include "qtquick/views/Separator.h"
#include "core/layouting/Item_p.h"
// #include "qtquick/views/SideBar.h"
#include "qtquick/views/Stack.h"
#include "qtquick/views/TabBar.h"
#include "qtquick/views/TitleBar.h"
#include "qtquick/views/MDILayout.h"
#include "qtquick/views/DropArea.h"

#include "../Window_p.h"

#include "kddockwidgets/core/DropArea.h"
#include "kddockwidgets/core/MDILayout.h"
// #include "views/MDIArea.h"

#include <QtQuick/private/qquickitem_p.h>
#include <QDebug>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets {
static Core::Controller *controllerForItem(QQuickItem *item)
{
    // KDDW deals in views, but sometimes we might get a native type like QWidget, for example if
    // you call someview->window(). This function let's us retrieve the actual controller of the
    // stray QWidget.

    for (int i = int(Core::ViewType::FIRST); i <= int(::Core::ViewType::LAST); i *= 2) {
        // Using a for+switch pattern so that compiler reminds us if new enumerators are added to
        // enum
        switch (Core::ViewType(i)) {
        case Core::ViewType::Group:
            if (auto view = qobject_cast<Group *>(item))
                return view->controller();
            break;
        case Core::ViewType::TitleBar:
            if (auto view = qobject_cast<TitleBar *>(item))
                return view->controller();
            break;
        case Core::ViewType::TabBar:
            if (auto view = qobject_cast<TabBar *>(item))
                return view->controller();
            break;
        case Core::ViewType::Stack:
            if (auto view = qobject_cast<Stack *>(item))
                return view->controller();
            break;
        case Core::ViewType::FloatingWindow:
            if (auto view = qobject_cast<FloatingWindow *>(item))
                return view->controller();
            break;
        case Core::ViewType::Separator:
            if (auto view = qobject_cast<Separator *>(item))
                return view->controller();
            break;
        case Core::ViewType::DockWidget:
            if (auto view = qobject_cast<QtQuick::DockWidget *>(item))
                return view->controller();
            break;
        case Core::ViewType::DropArea:
            if (auto view = qobject_cast<DropArea *>(item))
                return view->controller();
            break;
        case Core::ViewType::MDILayout:
            if (auto view = qobject_cast<MDILayout *>(item))
                return view->controller();
            break;
        case Core::ViewType::SideBar:
            // Not implemented for QtQuick yet
            break;
        case Core::ViewType::MainWindow:
            if (auto view = qobject_cast<MainWindow *>(item))
                return view->controller();
            break;
        case Core::ViewType::RubberBand:
        case Core::ViewType::LayoutItem:
        case Core::ViewType::ViewWrapper:
        case Core::ViewType::DropAreaIndicatorOverlay:
        case Core::ViewType::None:
            // skip internal types
            continue;
        }
    }

    return nullptr;
}
}

ViewWrapper::ViewWrapper(QObject *item)
    : ViewWrapper(qobject_cast<QQuickItem *>(item))
{
}

ViewWrapper::ViewWrapper(QQuickItem *item)
    : QtCommon::ViewWrapper(controllerForItem(item), item)
    , m_item(item)
{
}

QRect ViewWrapper::geometry() const
{
    if (isRootView()) {
        if (QWindow *w = m_item->window()) {
            return w->geometry();
        }
    }

    return QRect(QPointF(m_item->x(), m_item->y()).toPoint(), m_item->size().toSize());
}

QPoint ViewWrapper::mapToGlobal(QPoint localPt) const
{
    return m_item->mapToGlobal(localPt).toPoint();
}

QPoint ViewWrapper::mapFromGlobal(QPoint globalPt) const
{
    return m_item->mapFromGlobal(globalPt).toPoint();
}

void ViewWrapper::setGeometry(QRect rect)
{
    setSize(rect.width(), rect.height());
    ViewWrapper::move(rect.topLeft().x(), rect.topLeft().y());
}

std::shared_ptr<Core::View> ViewWrapper::childViewAt(QPoint p) const
{
    auto child = m_item->childAt(p.x(), p.y());
    return child ? QtQuick::View::asQQuickWrapper(child) : nullptr;
}

std::shared_ptr<Core::Window> ViewWrapper::window() const
{
    if (QWindow *w = m_item->window()) {
        auto windowqtquick = new Window(w);
        return std::shared_ptr<Core::Window>(windowqtquick);
    }

    return {};
}

bool ViewWrapper::isRootView() const
{
    return QtQuick::View::isRootView(m_item);
}

void ViewWrapper::setVisible(bool is)
{
    if (isRootView()) {
        if (QWindow *w = m_item->window()) {
            if (is && !w->isVisible()) {
                w->show();
            } else if (!is && w->isVisible()) {
                w->hide();
            }
        }
    }

    m_item->setVisible(is);
}

bool ViewWrapper::isVisible() const
{
    if (QWindow *w = m_item->window()) {
        if (!w->isVisible())
            return false;
    }

    return m_item->isVisible();
}

bool ViewWrapper::isExplicitlyHidden() const
{
    auto priv = QQuickItemPrivate::get(m_item);
    return !priv->explicitVisible;
}

void ViewWrapper::move(int x, int y)
{
    if (isRootView()) {
        if (QWindow *w = m_item->window()) {
            w->setPosition(x, y);
            return;
        }
    }

    m_item->setX(x);
    m_item->setY(y);
    enableAttribute(Qt::WA_Moved);
}

void ViewWrapper::activateWindow()
{
    if (QWindow *w = m_item->window())
        w->requestActivate();
}

bool ViewWrapper::isMaximized() const
{
    if (QWindow *w = m_item->window())
        return w->windowStates() & Qt::WindowMaximized;

    return false;
}

bool ViewWrapper::isMinimized() const
{
    if (QWindow *w = m_item->window())
        return w->windowStates() & Qt::WindowMinimized;

    return false;
}

QSize ViewWrapper::maxSizeHint() const
{
    if (auto view = unwrap()) {
        return view->maxSizeHint();
    } else {
        const QSize max = m_item->property("kddockwidgets_max_size").toSize();
        return max.isEmpty() ? Core::Item::hardcodedMaximumSize
                             : max.boundedTo(Core::Item::hardcodedMaximumSize);
    }
}

void ViewWrapper::setSize(int w, int h)
{
    if (isRootView()) {
        if (QWindow *window = m_item->window()) {
            QRect windowGeo = window->geometry();
            windowGeo.setSize(QSize(w, h));
            window->setGeometry(windowGeo);
        }
    }

    m_item->setSize(QSizeF(w, h));
}

bool ViewWrapper::is(Core::ViewType t) const
{
    switch (t) {
    case Core::ViewType::Group:
        return qobject_cast<Group *>(m_item);
    case Core::ViewType::TitleBar:
        return qobject_cast<TitleBar *>(m_item);
    case Core::ViewType::TabBar:
        return qobject_cast<TabBar *>(m_item);
    case Core::ViewType::Stack:
        return qobject_cast<Stack *>(m_item);
    case Core::ViewType::FloatingWindow:
        return qobject_cast<FloatingWindow *>(m_item);
    case Core::ViewType::Separator:
        return qobject_cast<Separator *>(m_item);
    case Core::ViewType::DockWidget:
        return qobject_cast<QtQuick::DockWidget *>(m_item);
    case Core::ViewType::SideBar:
        return false; // QtQuick doesn't support sidebar yet
        // return qobject_cast<SideBar *>(m_item);
    case Core::ViewType::MainWindow:
        return qobject_cast<QtQuick::MainWindow *>(m_item);
    case Core::ViewType::DropArea:
        return qobject_cast<DropArea *>(m_item);
    case Core::ViewType::MDILayout:
        return qobject_cast<MDILayout *>(m_item);
    case Core::ViewType::RubberBand:
        return qobject_cast<RubberBand *>(m_item);
    case Core::ViewType::LayoutItem:
    case Core::ViewType::None:
    case Core::ViewType::DropAreaIndicatorOverlay:
        qWarning() << Q_FUNC_INFO << "These are framework internals that are not wrapped";
        return false;
    case Core::ViewType::ViewWrapper:
        return true;
    }

    qWarning() << Q_FUNC_INFO << "Unknown type" << static_cast<int>(t);
    return false;
}

std::shared_ptr<Core::View> ViewWrapper::rootView() const
{
    if (Core::Window::Ptr window = this->window())
        return window->rootView();

    qWarning() << Q_FUNC_INFO << "No window present";
    return {};
}

std::shared_ptr<Core::View> ViewWrapper::parentView() const
{
    return QtQuick::View::parentViewFor(m_item);
}

void ViewWrapper::grabMouse()
{
    m_item->grabMouse();
}

void ViewWrapper::releaseMouse()
{
    m_item->ungrabMouse();
}

Qt::FocusPolicy ViewWrapper::focusPolicy() const
{
    if (auto view = unwrap()) {
        return view->focusPolicy();
    } else {
        // The DockWidget's user guest widget is not a View, it's a QQuickItem
        // QQuickItem don't have focus policy and are focusable
        return Qt::StrongFocus;
    }
}

void ViewWrapper::setFocus(Qt::FocusReason reason)
{
    m_item->QQuickItem::setFocus(true, reason);
    m_item->forceActiveFocus(reason);
}

void ViewWrapper::setFocusPolicy(Qt::FocusPolicy policy)
{
    if (auto view = unwrap()) {
        view->setFocusPolicy(policy);
    } else {
        qWarning() << Q_FUNC_INFO << "Not implemented for QtQuick";
    }
}

bool ViewWrapper::hasFocus() const
{
    return m_item->hasActiveFocus();
}

QString ViewWrapper::viewName() const
{
    return m_item->objectName();
}

bool ViewWrapper::isNull() const
{
    return m_item.data() == nullptr;
}

void ViewWrapper::setWindowTitle(const QString &title)
{
    if (QWindow *w = m_item->window())
        w->setTitle(title);
}

QPoint ViewWrapper::mapTo(View *parent, QPoint pos) const
{
    if (!parent)
        return {};

    auto parentItem = asQQuickItem(parent);
    return parentItem->mapFromGlobal(m_item->mapToGlobal(pos)).toPoint();
}

bool ViewWrapper::hasAttribute(Qt::WidgetAttribute attr) const
{
    if (auto view = unwrap()) {
        // Only real views have min size
        return view->hasAttribute(attr);
    } else {
        qFatal("not implemented");
        return false;
    }
}

void ViewWrapper::setCursor(Qt::CursorShape shape)
{
    m_item->QQuickItem::setCursor(shape);
}

QSize ViewWrapper::minSize() const
{
    if (auto view = unwrap()) {
        // Only real views have min size
        return view->minSize();
    } else {
        const QSize min = m_item->property("kddockwidgets_min_size").toSize();
        return min.expandedTo(Core::Item::hardcodedMinimumSize);
    }
}

QVector<std::shared_ptr<Core::View>> ViewWrapper::childViews() const
{
    QVector<std::shared_ptr<View>> result;
    const auto childItems = m_item->childItems();
    result.reserve(childItems.size());
    for (QQuickItem *child : childItems) {
        result << QtQuick::View::asQQuickWrapper(child);
    }

    return result;
}

void ViewWrapper::setParent(View *parent)
{
    if (auto view = unwrap()) {
        view->setParent(parent);
    } else {
        auto parentItem = QtQuick::asQQuickItem(parent);
        m_item->QQuickItem::setParent(parentItem);
        m_item->QQuickItem::setParentItem(parentItem);
    }

    m_item->setVisible(false);
}

bool ViewWrapper::close()
{
    return QtQuick::View::close(m_item);
}

Core::View *ViewWrapper::unwrap()
{
    return qobject_cast<QtQuick::View *>(m_item);
}

const Core::View *ViewWrapper::unwrap() const
{
    return qobject_cast<const QtQuick::View *>(m_item);
}

void ViewWrapper::raiseAndActivate()
{
    QtQuick::View::raiseAndActivate(m_item);
}

/*static*/
std::shared_ptr<Core::View> ViewWrapper::create(QObject *item)
{
    return create(qobject_cast<QQuickItem *>(item));
}

/*static*/
std::shared_ptr<Core::View> ViewWrapper::create(QQuickItem *item)
{
    if (!item)
        return {};

    auto wrapper = new ViewWrapper(item);
    auto sharedptr = std::shared_ptr<View>(wrapper);
    wrapper->d->m_thisWeakPtr = sharedptr;

    return sharedptr;
}
