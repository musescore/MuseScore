/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "LayoutingHost_p.h"

namespace KDDockWidgets {

namespace Core {

class Separator;
class LayoutingHost;
class ItemBoxContainer;

class DOCKS_EXPORT LayoutingSeparator
{
public:
    typedef Vector<LayoutingSeparator *> List;

    explicit LayoutingSeparator(LayoutingHost *, Qt::Orientation, Core::ItemBoxContainer *container);
    virtual ~LayoutingSeparator();
    virtual Rect geometry() const = 0;
    virtual void setGeometry(Rect r) = 0;
    virtual void raise();
    virtual void free();

    int position() const;
    bool isVertical() const;
    ItemBoxContainer *parentContainer() const;
    Qt::Orientation orientation() const;
    void setGeometry(int pos, int pos2, int length);

    bool isBeingDragged() const;
    void onMousePress();
    void onMouseRelease();

    int onMouseMove(Point pos, bool moveSeparator = true);

    LayoutingHost *const m_host;
    const Qt::Orientation m_orientation;
    Core::ItemBoxContainer *const m_parentContainer;

    static LayoutingSeparator *s_separatorBeingDragged;

private:
    int offset() const;
    LayoutingSeparator(const LayoutingSeparator &) = delete;
    LayoutingSeparator &operator=(const LayoutingSeparator &) = delete;
};

}

}
