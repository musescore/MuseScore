/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/QtCompat_p.h"

#include <memory>

namespace KDDockWidgets {

namespace Core {

class View;

/// Each View type also has a specific Controller associated with, except for ViewType::None
enum class ViewType {
    FIRST = 1, // Keep first
    None = 0,
    Frame = 1, // deprecated name
    Group = 1,
    TitleBar = 2,
    TabBar = 4,
    Stack = 8,
    FloatingWindow = 16,
    Separator = 32,
    DockWidget = 64,
    LayoutItem = 256,
    SideBar = 512,
    MainWindow = 1024,
    ViewWrapper = 2048,
    DropArea = 4096,
    MDILayout = 8192,
    RubberBand = 0x4000,
    DropAreaIndicatorOverlay = 0x8000,

    LAST = RubberBand // Keep last
};

class DOCKS_EXPORT Controller : public Object
{
    Q_OBJECT
public:
    explicit Controller(ViewType type, View *);
    virtual ~Controller();

    /// @brief Returns the view associated with this controller, if any.
    View *view() const;

    /// @brief Returns the type of this controller
    ViewType type() const;

    /// @brief Returns whether this controller is of the specified type
    bool is(ViewType) const;

    /// Returns whether the view is visible
    /// Convenience for calling View::isVisible()
    bool isVisible() const;

    /// Hides or shows the view associated with this controller
    /// Convenience for calling View::setVisible()
    void setVisible(bool);

    Rect rect() const;
    Point mapToGlobal(Point) const;
    int height() const;
    int width() const;
    Size size() const;
    Point pos() const;
    Rect geometry() const;
    int x() const;
    int y() const;
    bool close();
    std::shared_ptr<View> window() const;
    void show() const;
    bool inDtor() const;

    /// Sets the parent view
    /// In Qt this would be equivalent to calling view()->setParent(parent);
    void setParentView(View *parent);

    /// Equivalent to QObject::deleteLater() when Qt used
    /// but works with Flutter as well
    void destroyLater();

    bool isFixedHeight() const;
    bool isFixedWidth() const;

    class Private;
    Controller::Private *dptr() const;

protected:
    virtual void setParentView_impl(View *parent);
    bool m_inDtor = false;

private:
    Private *const d;
};

}

}
