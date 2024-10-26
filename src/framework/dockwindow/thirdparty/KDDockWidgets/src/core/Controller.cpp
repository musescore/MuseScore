/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Controller.h"
#include "Controller_p.h"
#include "Platform.h"
#include "DelayedCall_p.h"
#include "View.h"
#include "Config.h"
#include "View_p.h"
#include "Logging_p.h"
#include "DragController_p.h"
#include "core/Utils_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

Controller::Controller(ViewType type, View *view)
    : d(new Private(type, view))
{
}

Controller::~Controller()
{
    d->aboutToBeDeleted.emit();

    m_inDtor = true;
    if (d->m_view && !d->m_view->inDtor())
        d->m_view->d->free();

    delete d;
}

ViewType Controller::type() const
{
    return d->m_type;
}

bool Controller::is(ViewType t) const
{
    return int(d->m_type) & int(t);
}

bool Controller::inDtor() const
{
    return m_inDtor;
}

View *Controller::view() const
{
    return d->m_view;
}

bool Controller::isVisible() const
{
    return d->m_view && d->m_view->isVisible();
}

void Controller::setVisible(bool is)
{
    if (d->m_view)
        d->m_view->setVisible(is);
}

Rect Controller::rect() const
{
    if (d->m_view)
        return d->m_view->rect();

    return {};
}

Point Controller::mapToGlobal(Point localPt) const
{
    return d->m_view->mapToGlobal(localPt);
}

int Controller::height() const
{
    return d->m_view->height();
}

int Controller::width() const
{
    return d->m_view->width();
}

Size Controller::size() const
{
    return d->m_view->size();
}

Rect Controller::geometry() const
{
    return d->m_view->geometry();
}

Point Controller::pos() const
{
    return d->m_view->geometry().topLeft();
}

int Controller::x() const
{
    return d->m_view->x();
}

int Controller::y() const
{
    return d->m_view->y();
}

bool Controller::close()
{
    return view() && view()->close();
}

std::shared_ptr<View> Controller::window() const
{
    return view()->rootView();
}

void Controller::show() const
{
    view()->show();
}

void Controller::setParentView(View *parent)
{
    setParentView_impl(parent);
    d->parentViewChanged.emit(parent);
}

void Controller::setParentView_impl(View *parent)
{
    if (auto v = view()) {
        v->setParent(parent);
    } else {
        KDDW_ERROR("No view()");
    }
}

void Controller::destroyLater()
{
#ifdef KDDW_FRONTEND_QT
    if (!usesQTBUG83030Workaround()) {
        QObject::deleteLater();
        return;
    }
#endif

    // Path for Flutter and QTBUG-83030:
    Platform::instance()->runDelayed(0, new DelayedDelete(this));
}

Controller::Private *Controller::dptr() const
{
    return d;
}

bool Controller::isFixedHeight() const
{
    if (auto v = view())
        return v->isFixedHeight();

    return false;
}

bool Controller::isFixedWidth() const
{
    if (auto v = view())
        return v->isFixedWidth();

    return false;
}
