/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DragControllerWayland_p.h"
#include "core/Logging_p.h"
#include "core/ScopedValueRollback_p.h"
#include "kddockwidgets/core/Platform.h"
#include "kddockwidgets/core/DropArea.h"

using namespace KDDockWidgets::Core;

StateDraggingWayland::StateDraggingWayland(DragController *parent)
    : StateDragging(parent)
{
}

StateDraggingWayland::~StateDraggingWayland()
{
}

void StateDraggingWayland::onEntry()
{
    KDDW_DEBUG("StateDraggingWayland entered");

    if (DragController::instance()->m_inQDrag) {
        // Maybe we can exit the state due to the nested event loop of QDrag::Exec();
        KDDW_ERROR("Impossible!");
        return;
    }

    ScopedValueRollback guard(DragController::instance()->m_inQDrag, true);
    q->m_windowBeingDragged =
        std::unique_ptr<WindowBeingDragged>(new WindowBeingDraggedWayland(q->m_draggable));

    auto mimeData = new WaylandMimeData();
    Drag drag(this);
    drag.setMimeData(mimeData);
    drag.setPixmap(q->m_windowBeingDragged->pixmap());

    Platform::instance()->installGlobalEventFilter(q);
    KDDW_DEBUG("Started QDrag");
    const Qt::DropAction result = drag.exec();
    KDDW_DEBUG("QDrag finished with result={}", int(result));

    Platform::instance()->removeGlobalEventFilter(q);
    if (result == Qt::IgnoreAction)
        q->dragCanceled.emit();
}

bool StateDraggingWayland::handleMouseButtonRelease(QPoint /*globalPos*/)
{
    KDDW_DEBUG(Q_FUNC_INFO);
    q->dragCanceled.emit();
    return true;
}

bool StateDraggingWayland::handleMouseMove(QPoint)
{
    // Wayland uses QDrag to drag stuff while other platforms use mouse.
    // So override handleMouseMove() just so the regular mouse stuff doesn't run.

    return false;
}

bool StateDraggingWayland::handleDragEnter(DragMoveEvent *ev, DropArea *dropArea)
{
    auto mimeData = object_cast<const WaylandMimeData *>(ev->mimeData());
    if (!mimeData || !q->m_windowBeingDragged)
        return false; // Not for us, some other user drag.

    if (q->m_windowBeingDragged->contains(dropArea)) {
        ev->ignore();
        return true;
    }

    dropArea->hover(q->m_windowBeingDragged.get(),
                    dropArea->mapToGlobal(Qt5Qt6Compat::eventPos(ev)));

    ev->accept();
    return true;
}

bool StateDraggingWayland::handleDragLeave(DropArea *dropArea)
{
    KDDW_DEBUG(Q_FUNC_INFO);
    dropArea->removeHover();
    return true;
}

bool StateDraggingWayland::handleDrop(DropEvent *ev, DropArea *dropArea)
{
    KDDW_DEBUG(Q_FUNC_INFO);
    auto mimeData = object_cast<const WaylandMimeData *>(ev->mimeData());
    if (!mimeData || !q->m_windowBeingDragged)
        return false; // Not for us, some other user drag.

    if (dropArea->drop(q->m_windowBeingDragged.get(),
                       dropArea->mapToGlobal(Qt5Qt6Compat::eventPos(ev)))) {
        ev->setDropAction(Qt::MoveAction);
        ev->accept();
        q->dropped.emit();
    } else {
        q->dragCanceled.emit();
    }

    dropArea->removeHover();
    return true;
}

bool StateDraggingWayland::handleDragMove(DragMoveEvent *ev, DropArea *dropArea)
{
    KDDW_DEBUG("StateDraggingWayland::handleDragMove");

    auto mimeData = object_cast<const WaylandMimeData *>(ev->mimeData());
    if (!mimeData || !q->m_windowBeingDragged) {
        KDDW_DEBUG("StateDraggingWayland::handleDragMove. Early bailout hasMimeData={} windowBeingDragged={}", mimeData != nullptr, bool(q->m_windowBeingDragged));
        return false; // Not for us, some other user drag.
    }

    dropArea->hover(q->m_windowBeingDragged.get(),
                    dropArea->mapToGlobal(Qt5Qt6Compat::eventPos(ev)));

    return true;
}
