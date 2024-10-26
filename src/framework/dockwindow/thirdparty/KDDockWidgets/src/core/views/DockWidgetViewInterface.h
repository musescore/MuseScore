/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets {

namespace Core {

class DockWidget;
class Group;
class TitleBar;
class View;

/// @brief The interface that DockWidget views should implement
class DOCKS_EXPORT DockWidgetViewInterface
{
public:
    explicit DockWidgetViewInterface(DockWidget *);
    virtual ~DockWidgetViewInterface();

    Group *group() const;
    DockWidget *dockWidget() const;
    TitleBar *actualTitleBar() const;

    bool isFocused() const;
    bool isFloating() const;
    void setFloating(bool);
    QString uniqueName() const;
    QString title() const;
    void setTitle(const QString &);
    void setAsCurrentTab();
    bool isOpen() const;
    void forceClose();
    void open();
    void raise();
    void moveToSideBar();
    void setIcon(const Icon &icon, IconPlaces places = IconPlace::All);
    Icon icon(IconPlace place = IconPlace::TitleBar) const;

    void setAffinities(const Vector<QString> &);
    void setAffinityName(const QString &name);
    Vector<QString> affinities() const;

    void addDockWidgetAsTab(DockWidgetViewInterface *other,
                            const KDDockWidgets::InitialOption &initialOption = {});

    void addDockWidgetToContainingWindow(DockWidgetViewInterface *other,
                                         KDDockWidgets::Location location,
                                         DockWidgetViewInterface *relativeTo = nullptr,
                                         const KDDockWidgets::InitialOption &initialOption = {});

    DockWidgetOptions options() const;
    void setOptions(DockWidgetOptions);

    /// @deprecated. Use open() instead.
    void show();

    /// @brief Sets this dock widgets position to pos within the MDI layout
    /// This only applies if the main window is in MDI mode, which it is not by default
    void setMDIPosition(Point pos);

    /// @brief like setMDIPosition(), but for the size.
    void setMDISize(Size size);

    /// @brief like setMDIPosition(), but for the Z
    /// only implemented for QtQuick
    void setMDIZ(int z);

    virtual std::shared_ptr<Core::View> focusCandidate() const = 0;

protected:
    DockWidget *const m_dockWidget;

    DockWidgetViewInterface(const DockWidgetViewInterface &) = delete;
    DockWidgetViewInterface &operator=(const DockWidgetViewInterface &) = delete;
};

}

}
