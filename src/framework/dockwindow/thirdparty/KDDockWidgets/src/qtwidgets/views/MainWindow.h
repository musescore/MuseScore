/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief QMainWindow sub-class to enable KDDockWidgets support.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_MAINWINDOW_H
#define KD_MAINWINDOW_H

#include "View.h"
#include "kddockwidgets/core/views/MainWindowViewInterface.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {
class MainWindow;
class SideBar;
}

namespace QtWidgets {

/**
 * @brief The QMainwindow sub-class that the application should use to be able
 * to dock KDDockWidget DockWidget instances.
 */
#ifdef PYTHON_BINDINGS
class DOCKS_EXPORT MainWindow : public QMainWindow,
                                public Core::MainWindowViewInterface
#else
class DOCKS_EXPORT MainWindow : public View<QMainWindow>,
                                public Core::MainWindowViewInterface
#endif
{
    Q_OBJECT
public:
    using MainWindowViewInterface::addDockWidget; // don't consider QMainWindow's one
    using QWidget::size;

    ///@brief Constructor. Use it as you would use QMainWindow.
    ///@param uniqueName Mandatory name that should be unique between all MainWindow instances.
    ///       This name won't be user visible and just used internally for the save/restore.
    ///@param options optional MainWindowOptions to use
    ///@param parent QObject *parent to pass to QMainWindow constructor.
    ///@param flags Window flags to  pass to QMainWindow constructor.
    explicit MainWindow(const QString &uniqueName, MainWindowOptions options = {},
                        QWidget *parent = nullptr,
                        Qt::WindowFlags flags = Qt::WindowFlags());


    /// @brief Destructor
    ~MainWindow() override;

    /// @brief returns the margins for the contents widget
    QMargins centerWidgetMargins() const override;

    /// @brief sets the margins for the contents widget
    void setCenterWidgetMargins(QMargins);

    /// @brief Sets the main window's content margins
    void setContentsMargins(int left, int top, int right, int bottom) override;

    void setPersistentCentralWidget(QWidget *widget);
    QWidget *persistentCentralWidget() const;

    ///@brief returns the internal layout
    /// this is rarely needed unless you want to layout other widgets next to the
    /// main dock widget layout
    QHBoxLayout *internalLayout() const;

    /// @brief for internal use only
    void updateMargins();

    /// These *_legacy overloads provide QDockWidget compatibility, during porting. See MainWindowOption_QDockWidgets
    void setCentralWidget_legacy(QWidget *);
    void addDockWidget_legacy(Qt::DockWidgetArea area, QDockWidget *dockwidget);
    void addDockWidget_legacy(Qt::DockWidgetArea area, QDockWidget *dockwidget,
                              Qt::Orientation orientation);
    bool restoreDockWidget_legacy(QDockWidget *dockwidget);
    void removeDockWidget_legacy(QDockWidget *dockwidget);
    Qt::DockWidgetArea dockWidgetArea_legacy(QDockWidget *dockwidget) const;
    void resizeDocks_legacy(const QList<QDockWidget *> &docks,
                            const QList<int> &sizes, Qt::Orientation orientation);
    void tabifyDockWidget_legacy(QDockWidget *first, QDockWidget *second);
    QList<QDockWidget *> tabifiedDockWidgets_legacy(QDockWidget *dockwidget) const;
    void splitDockWidget_split_legacy(QDockWidget *after, QDockWidget *dockwidget,
                                      Qt::Orientation orientation);


    /// Setups the QMainWindow central widget needed by KDDW
    /// This should almost never be used. The rare use case is if your main window is backed by a .ui file
    /// and you need to call setupUi() on it. In that case, call manualInit() after setupUi(), and don't forget to
    /// use MainWindowOption_ManualInit
    void manualInit();

Q_SIGNALS:
    void groupCountChanged(int);

protected:
    QRect centralAreaGeometry() const override;

private:
    friend class Core::MainWindow;
    using QMainWindow::setCentralWidget;
    void setCentralWidget(QWidget *); // overridden just to make it private

    class Private;
    Private *const d;
};
}
}

#endif
