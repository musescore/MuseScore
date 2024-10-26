/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/DragController_p.h"

#include <QMimeData>

namespace KDDockWidgets {

namespace Core {

// Used on wayland only to use QDrag instead of setting geometry on mouse-move.
class StateDraggingWayland : public StateDragging
{
    Q_OBJECT
public:
    explicit StateDraggingWayland(DragController *parent);
    ~StateDraggingWayland() override;
    void onEntry() override;

    bool handleMouseButtonRelease(QPoint globalPos) override;
    bool handleDragEnter(DragMoveEvent *, DropArea *) override;
    bool handleDragMove(DragMoveEvent *, DropArea *) override;
    bool handleDragLeave(DropArea *) override;
    bool handleDrop(DropEvent *, DropArea *) override;
    bool handleMouseMove(QPoint globalPos) override;
};

// A sub-class just so we don't use QMimeData directly. We'll only accept drops if its mime data
// Can be qobject_casted to this class. For safety.
class WaylandMimeData : public QMimeData
{
    Q_OBJECT
public:
};

}
}
