/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Draggable_p.h"
#include "DragController_p.h"
#include "WidgetResizeHandler_p.h"
#include "Utils_p.h"

#include "kddockwidgets/core/Platform.h"
#include "kddockwidgets/core/FloatingWindow.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

class Draggable::Private
{
public:
    explicit Private(View *_thisView, bool _enabled)
        : thisView(_thisView)
        , enabled(_enabled)
    {
        assert(thisView);
    }

    ObjectGuard<WidgetResizeHandler> widgetResizeHandler;
    View *const thisView;
    const bool enabled;
};

Draggable::Draggable(View *thisView, bool enabled)
    : d(new Private(thisView, enabled))
{
    if (thisView && d->enabled)
        DragController::instance()->registerDraggable(this);
}

Draggable::~Draggable()
{
    if (d->thisView && d->enabled)
        DragController::instance()->unregisterDraggable(this);

    delete d->widgetResizeHandler;
    delete d;
}

View *Draggable::asView() const
{
    return d->thisView;
}

Controller *Draggable::asController() const
{
    if (auto v = d->thisView)
        return v->controller();

    return nullptr;
}

bool Draggable::dragCanStart(Point pressPos, Point globalPos) const
{
    return (globalPos - pressPos).manhattanLength() > Core::Platform::instance()->startDragDistance();
}

void Draggable::setWidgetResizeHandler(WidgetResizeHandler *w)
{
    assert(!d->widgetResizeHandler);
    assert(w);
    d->widgetResizeHandler = w;
}

bool Draggable::isInProgrammaticDrag() const
{
    return DragController::instance()->isInProgrammaticDrag();
}
