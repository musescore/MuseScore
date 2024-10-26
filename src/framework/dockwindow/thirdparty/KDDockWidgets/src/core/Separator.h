/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Controller.h"
#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets {
class Config;
}

namespace KDDockWidgets::Core {

class LayoutingSeparator;
class LayoutingHost;
class ItemBoxContainer;

class DOCKS_EXPORT Separator : public Controller
{
    Q_OBJECT
public:
    typedef Vector<Separator *> List;

    /// constructs a separator
    /// @param host the host where this separator is in. Usually a DropArea.
    explicit Separator(LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *parentContainer);
    virtual ~Separator() override;

    bool isVertical() const;
    int position() const;
    void setGeometry(Rect r);

    LayoutingSeparator *asLayoutingSeparator() const;

    ///@brief Returns whether we're dragging a separator. Can be useful for the app to stop other
    /// work while we're not in the final size
    static bool isResizing();

    /// @internal Just for the unit-tests.
    /// Returns the total amount of Separator() instances currently alive.
    static int numSeparators();

public:
    void onMousePress();
    void onMouseReleased();
    void onMouseDoubleClick();
    void onMouseMove(Point pos);

private:
    friend class KDDockWidgets::Config;

    KDDW_DELETE_COPY_CTOR(Separator)
    void setLazyPosition(int);
    bool usesLazyResize() const;

    struct Private;
    Private *const d;
};

}
