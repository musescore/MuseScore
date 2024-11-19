/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDDOCKWIDGETS_MDI_AREA_H
#define KDDOCKWIDGETS_MDI_AREA_H

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/qtwidgets/views/View.h"

namespace KDDockWidgets {

namespace Core {
class Group;
class DockWidget;
class MDILayout;
class DockWidgetViewInterface;
}

namespace QtWidgets {

/**
 * @brief MDIArea allows to host dock widget in MDI mode.
 * This is an alternative to using a full blown MainWindowMDI.
 * The use case is when you already have a MainWindow (doing normal docking) and you
 * want to add an area where you want to use MDI dock widgets. Instead of docking a MainWindowMDI,
 * you'd just use an MDIArea, and avoid having nested main windows.
 *
 * See examples/mdi_with_docking/.
 */
class DOCKS_EXPORT MDIArea : public QtWidgets::View<QWidget>
{
    Q_OBJECT
public:
    explicit MDIArea(QWidget *parent = nullptr);
    ~MDIArea();

    /// @brief docks the dock widgets into this MDI area, at the specified position
    void addDockWidget(Core::DockWidget *dw, QPoint localPt,
                       const InitialOption &addingOption = {});

    /// @brief Moves a dock widget @p dw to point @p pos
    void moveDockWidget(Core::DockWidget *dw, QPoint pos);

    /// @brief Sets the size of dock widget @p dw to @p size
    void resizeDockWidget(Core::DockWidget *dw, QSize size);

    /// @brief overloads
    void addDockWidget(Core::DockWidgetViewInterface *, QPoint localPt,
                       const InitialOption &addingOption = {});
    void moveDockWidget(Core::DockWidgetViewInterface *, QPoint pos);
    void resizeDockWidget(Core::DockWidgetViewInterface *, QSize size);

    /// @brief Returns the list of groups in this MDI Area
    /// Each Frame object represents a 'window' emebedded in the MDI Area
    QVector<Core::Group *> groups() const;

private:
    class Private;
    Private *const d;
};

}

}

#endif
