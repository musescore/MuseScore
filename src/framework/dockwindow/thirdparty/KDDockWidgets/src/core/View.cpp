/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "core/View.h"
#include "core/View_p.h"
#include "core/Utils_p.h"
#include "core/layouting/Item_p.h"
#include "core/EventFilterInterface.h"
#include "core/FloatingWindow.h"
#include "core/Group.h"
#include "core/Stack.h"
#include "core/TitleBar.h"
#include "core/TabBar.h"
#include "core/MainWindow.h"
#include "core/DropArea.h"
#include "core/MDILayout.h"
#include "core/Logging_p.h"
#include "core/Platform.h"
#include "core/Window_p.h"

#include <iostream>
#include <cstdlib>
#include <utility>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

namespace KDDockWidgets {
static qint64 s_nextId = 1;

Controller *maybeCreateController(Controller *controller, ViewType type, View *view)
{
    if (controller)
        return controller;

    if (type == ViewType::ViewWrapper)
        return new Controller(ViewType::ViewWrapper, view);

    return new Controller(ViewType::None, view);
}

}

View::View(Controller *controller, ViewType type)
    : d(new Private(this, QString::number(KDDockWidgets::s_nextId++), type))
    , m_controller(maybeCreateController(controller, type, this))
{
}

View::~View()
{
    m_inDtor = true;
    d->beingDestroyed.emit();

    if (!d->freed() && !View::is(ViewType::ViewWrapper) && !View::is(ViewType::DropAreaIndicatorOverlay)) {
        // Views should be deleted via View::free()!
        // However some of our deletes are coming from widgets parent destroying their children
        // But we want the controllers to drive things instead. For now detect the view destruction
        // and destroy its controller, which was the old behaviour.
        delete m_controller;
    }

#ifdef KDDW_FRONTEND_FLUTTER
    const auto children = m_childViews;
    for (auto child : children)
        delete child;

    m_childViews.clear();
#endif

    delete d;
}

QString View::Private::id() const
{
    return m_id;
}

ViewType View::Private::type() const
{
    return m_type;
}

void View::Private::free()
{
    if (m_freed) {
        KDDW_ERROR("Free already called");
        return;
    }

    m_freed = true;
    delete q;
}

void View::init()
{
}

bool View::Private::freed() const
{
    return m_freed;
}

bool View::inDtor() const
{
    return m_inDtor;
}

void View::setZOrder(int)
{
}

int View::zOrder() const
{
    return 0;
}

Size View::size() const
{
    return geometry().size();
}

Point View::pos() const
{
    return geometry().topLeft();
}

Rect View::rect() const
{
    return Rect(Point(0, 0), size());
}

int View::x() const
{
    return geometry().x();
}

int View::y() const
{
    return geometry().y();
}

int View::height() const
{
    return geometry().height();
}

int View::width() const
{
    return geometry().width();
}

void View::move(Point pt)
{
    move(pt.x(), pt.y());
}

void View::resize(Size sz)
{
    setSize(sz.width(), sz.height());
}

void View::setSize(Size sz)
{
    setSize(sz.width(), sz.height());
}

void View::resize(int w, int h)
{
    setSize(w, h);
}

int View::minimumWidth() const
{
    return minSize().width();
}

int View::minimumHeight() const
{
    return minSize().height();
}

Size View::screenSize() const
{
    if (auto screen = d->screen())
        return screen->size();

    return {};
}

Controller *View::controller() const
{
    return m_controller;
}

/** static */
Size View::hardcodedMinimumSize()
{
    return Core::Item::hardcodedMinimumSize;
}

bool View::is(ViewType t) const
{
    return int(d->m_type) & int(t);
}

Core::FloatingWindow *View::asFloatingWindowController() const
{
    if (m_controller && m_controller->is(ViewType::FloatingWindow))
        return object_cast<Core::FloatingWindow *>(m_controller);

    return nullptr;
}

Core::Group *View::asGroupController() const
{
    if (m_controller && m_controller->is(ViewType::Group))
        return object_cast<Core::Group *>(m_controller);

    return nullptr;
}

Core::TitleBar *View::asTitleBarController() const
{
    if (m_controller && m_controller->is(ViewType::TitleBar))
        return object_cast<Core::TitleBar *>(m_controller);

    return nullptr;
}

Core::TabBar *View::asTabBarController() const
{
    if (m_controller && m_controller->is(ViewType::TabBar))
        return object_cast<Core::TabBar *>(m_controller);

    return nullptr;
}

Core::Stack *View::asStackController() const
{
    if (m_controller && m_controller->is(ViewType::Stack))
        return object_cast<Core::Stack *>(m_controller);

    return nullptr;
}

Core::DockWidget *View::asDockWidgetController() const
{
    if (m_controller && m_controller->is(ViewType::DockWidget))
        return object_cast<Core::DockWidget *>(m_controller);

    return nullptr;
}

Core::MainWindow *View::asMainWindowController() const
{
    if (m_controller && m_controller->is(ViewType::MainWindow))
        return object_cast<Core::MainWindow *>(m_controller);

    return nullptr;
}

Core::DropArea *View::asDropAreaController() const
{
    if (!m_inDtor && m_controller && m_controller->is(ViewType::DropArea)) {
        return object_cast<Core::DropArea *>(m_controller);
    }
    return nullptr;
}

Core::MDILayout *View::asMDILayoutController() const
{
    if (!m_inDtor && m_controller && m_controller->is(ViewType::MDILayout))
        return object_cast<Core::MDILayout *>(m_controller);

    return nullptr;
}

Core::Layout *View::asLayout() const
{
    if (Core::DropArea *da = asDropAreaController()) {
        return da;
    } else if (Core::MDILayout *mdi = asMDILayoutController()) {
        return mdi;
    }

    return nullptr;
}

bool View::equals(const View *other) const
{
    return other && handle() == other->handle();
}

bool View::equals(const std::shared_ptr<View> &other) const
{
    if (isNull() || !other || other->isNull()) {
        // We don't care about nullity for identity
        return false;
    }

    return handle() == other->handle();
}

bool View::isNull() const
{
    return false;
}

bool View::Private::isInWindow(std::shared_ptr<Core::Window> window) const
{
    if (!window)
        return false;

    if (auto ourWindow = q->window())
        return ourWindow->equals(window);

    return false;
}

Size View::Private::parentSize() const
{
    if (auto p = q->parentView())
        return p->size();
    return {};
}

Rect View::Private::windowGeometry() const
{
    if (Core::Window::Ptr window = q->window())
        return window->geometry();

    return {};
}

void View::Private::closeRootView()
{
    if (auto view = q->rootView())
        view->close();
}

Screen::Ptr View::Private::screen() const
{
    if (Core::Window::Ptr window = q->window())
        return window->screen();

    return nullptr;
}

void View::Private::setAboutToBeDestroyed()
{
    m_aboutToBeDestroyed = true;
}

bool View::Private::aboutToBeDestroyed() const
{
    return m_aboutToBeDestroyed;
}

void View::dumpDebug()
{
    KDDW_DEBUG("View::dumpDebug: controller={}, type={}, rootController={}\n", ( void * )m_controller, int(d->type()), ( void * )rootView()->controller());
}

bool View::isFixedWidth() const
{
    return !m_inDtor && minSize().width() == maxSizeHint().width();
}

bool View::isFixedHeight() const
{
    return !m_inDtor && minSize().height() == maxSizeHint().height();
}

/** static */
Controller *View::firstParentOfType(View *view, ViewType type)
{
    auto p = view->asWrapper();
    while (p) {
        if (p->is(type))
            return p->controller();

        // Ignore QObject hierarchies spanning though multiple windows
        if (p->isRootView())
            return nullptr;

        p = p->parentView();
    }

    return nullptr;
}

Controller *View::Private::firstParentOfType(ViewType type) const
{
    return View::firstParentOfType(const_cast<View *>(q), type);
}

void View::Private::requestClose(CloseEvent *e)
{
    closeRequested.emit(e);
}

Rect View::Private::globalGeometry() const
{
    Rect geo = q->geometry();
    if (!q->isRootView())
        geo.moveTopLeft(q->mapToGlobal(Point(0, 0)));
    return geo;
}

void View::createPlatformWindow()
{
    // Only qtwidgets need this
    KDDW_ERROR("Shouldn't be called on this platform");
    std::abort();
}

std::shared_ptr<Core::Window> View::Private::transientWindow() const
{
    if (auto w = q->window())
        return w->transientParent();

    return {};
}

bool View::onResize(int w, int h)
{
    d->resized.emit(Size(w, h));
    return false;
}

bool View::onResize(Size sz)
{
    return onResize(sz.width(), sz.height());
}

/** static */
bool View::equals(const View *one, const View *two)
{
    if ((one && !two) || (!one && two))
        return false;

    if (!one && !two)
        return true;

    return one->equals(two);
}

void View::installViewEventFilter(EventFilterInterface *filter)
{
    d->m_viewEventFilters.push_back(filter);
}

void View::removeViewEventFilter(EventFilterInterface *filter)
{
    d->m_viewEventFilters.erase(
        std::remove(d->m_viewEventFilters.begin(), d->m_viewEventFilters.end(), filter),
        d->m_viewEventFilters.end());
}

bool View::deliverViewEventToFilters(Event *ev)
{
    for (Core::EventFilterInterface *filter : std::as_const(d->m_viewEventFilters)) {
        if (ev->type() == Event::Move) {
            if (filter->onMoveEvent(this))
                return true;
        } else if (auto me = mouseEvent(ev)) {
            if (filter->onMouseEvent(this, me))
                return true;

            switch (ev->type()) {
            case Event::MouseButtonPress:
                if (filter->onMouseButtonPress(this, me))
                    return true;
                break;
            case Event::MouseButtonRelease:
                if (filter->onMouseButtonRelease(this, me))
                    return true;
                break;
            case Event::MouseMove:
                if (filter->onMouseButtonMove(this, me))
                    return true;
                break;
            case Event::MouseButtonDblClick:
                if (filter->onMouseDoubleClick(this, me))
                    return true;
                break;
            default:
                break;
            }
        }
    }

    return false;
}
