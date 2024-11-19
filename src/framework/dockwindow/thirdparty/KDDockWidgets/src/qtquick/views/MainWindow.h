/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_MAIN_WINDOW_QUICK_P_H
#define KD_MAIN_WINDOW_QUICK_P_H

#include "View.h"
#include "kddockwidgets/core/views/MainWindowViewInterface.h"

#include <QString>

namespace KDDockWidgets {

namespace Core {
class SideBar;
}

namespace QtQuick {

///@brief A docking area for dock widgets
/// Named MainWindow as it's the QtWidgets/QMainWindow counterpart.
/// Provides the ability of accepting drops of dock widgets.
/// It's not a real QWindow and not a main window in the sense of QMainWindow. Would be overkill
/// to have tool bars, menu bar and footer in the QtQuick implementation. That's left for the user
/// to do. From QML just use DockingArea {}, which will create a this class behind the scenes. It's
/// mostly an implementation detail unless you want to use C++.

class DOCKS_EXPORT MainWindow : public QtQuick::View, public Core::MainWindowViewInterface
{
    Q_OBJECT
    Q_PROPERTY(QVector<QString> affinities READ affinities CONSTANT)
    Q_PROPERTY(QString uniqueName READ uniqueName CONSTANT)
    Q_PROPERTY(KDDockWidgets::MainWindowOptions options READ options CONSTANT)
    Q_PROPERTY(bool isMDI READ isMDI CONSTANT)
public:
    ///@brief Constructor. Use it as you would use QMainWindow.
    ///@param uniqueName Mandatory name that should be unique between all MainWindow instances.
    ///       This name won't be user visible and just used internally for the save/restore.
    ///@param options optional MainWindowOptions to use
    ///@param parent Visual parent item.
    ///@param flags Window flags to pass to top-level window, in case we're constructing it too
    explicit MainWindow(const QString &uniqueName, MainWindowOptions options = {},
                        QQuickItem *parent = nullptr, Qt::WindowFlags flags = {});

    ~MainWindow() override;

    /// @reimp
    QSize minSize() const override;

    /// @reimp
    QSize maxSizeHint() const override;

    /// Sets a persistent central widget. It can't be detached.
    /// Se docs for Core::MainWindow::setPersistentCentralView
    void setPersistentCentralView(const QString &qmlFilename);

#ifdef Q_MOC_RUN
    Q_INVOKABLE bool closeDockWidgets(bool force = false);
    Q_INVOKABLE bool sideBarIsVisible(KDDockWidgets::SideBarLocation) const;
    Q_INVOKABLE void clearSideBarOverlay(bool deleteFrame = true);
    Q_INVOKABLE void layoutEqually();
    Q_INVOKABLE bool anySideBarIsVisible() const;

    Q_INVOKABLE void moveToSideBar(const QString &dockId);
    Q_INVOKABLE void moveToSideBar(const QString &dockId, KDDockWidgets::SideBarLocation);
    Q_INVOKABLE void restoreFromSideBar(const QString &dockId);
    Q_INVOKABLE void overlayOnSideBar(const QString &dockId);
    Q_INVOKABLE void toggleOverlayOnSideBar(const QString &dockId);
    Q_INVOKABLE void layoutParentContainerEqually(const QString &dockId);
    Q_INVOKABLE void addDockWidgetAsTab(const QString &dockId);
    Q_INVOKABLE void addDockWidget(const QString &dockId, KDDockWidgets::Location,
                                   const QString &relativeToDockId = {},
                                   KDDockWidgets::InitialOption = {});

#endif

protected:
    QMargins centerWidgetMargins() const override;
    QRect centralAreaGeometry() const override;
    void setContentsMargins(int left, int top, int right, int bottom) override;

private:
    class Private;
    Private *const d;
};
}
}

#endif
