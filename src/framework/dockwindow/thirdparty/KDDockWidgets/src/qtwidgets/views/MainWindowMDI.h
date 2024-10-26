/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief MainWindow sub-class which uses MDI as a layout
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_MAINWINDOW_MDI_QTWIDGETS_H
#define KD_MAINWINDOW_MDI_QTWIDGETS_H
#pragma once

#include "kddockwidgets/core/views/MainWindowMDIViewInterface.h"
#include "kddockwidgets/qtwidgets/views/MainWindow.h"

namespace KDDockWidgets {

namespace QtWidgets {

/// @brief MainWindow sub-class which uses MDI as a layout
class DOCKS_EXPORT MainWindowMDI : public QtWidgets::MainWindow,
                                   public Core::MainWindowMDIViewInterface
{
    Q_OBJECT
public:
    using Core::MainWindowMDIViewInterface::addDockWidget;

    ///@brief Constructor. See base class documentation
    explicit MainWindowMDI(const QString &uniqueName, QWidget *parent = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags());

    ///@brief Destructor
    ~MainWindowMDI() override;
};

}

}

#endif
