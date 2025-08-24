/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Draggable_p.h"
#include "DragController_p.h"
#include "FloatingWindow_p.h"
#include "WidgetResizeHandler_p.h"
#include "Utils_p.h"

using namespace KDDockWidgets;

class Draggable::Private
{
public:
    explicit Private(QWidgetOrQuick *_thisWidget, bool _enabled)
        : thisWidget(_thisWidget)
        , enabled(_enabled)
    {
        Q_ASSERT(thisWidget);
    }

    QPointer<WidgetResizeHandler> widgetResizeHandler;
    QWidgetOrQuick *const thisWidget;
    const bool enabled;
};

Draggable::Draggable(QWidgetOrQuick *thisWidget, bool enabled)
    : d(new Private(thisWidget, enabled))
{
    if (thisWidget && d->enabled)
        DragController::instance()->registerDraggable(this);
}

Draggable::~Draggable()
{
    if (d->thisWidget && d->enabled)
        DragController::instance()->unregisterDraggable(this);
    delete d;
}

QWidgetOrQuick *Draggable::asWidget() const
{
    return d->thisWidget;
}

bool Draggable::dragCanStart(QPoint pressPos, QPoint globalPos) const
{
    // Start dragging single-dock floating windows almost immediately for best user experience,
    // but require some "force" for docked/tabbed widgets in order to prevent accidental undocking.
    KDDockWidgets::DockWidgetBase* dw = singleDockWidget();
    int startDragDistance = (isWindow() || (dw && dw->isFloating()))
                            ? KDDockWidgets::startDragDistance()
                            : KDDockWidgets::undockDistance();

    return (globalPos - pressPos).manhattanLength() > startDragDistance;
}

void Draggable::setWidgetResizeHandler(WidgetResizeHandler *w)
{
    Q_ASSERT(!d->widgetResizeHandler);
    Q_ASSERT(w);
    d->widgetResizeHandler = w;
}
