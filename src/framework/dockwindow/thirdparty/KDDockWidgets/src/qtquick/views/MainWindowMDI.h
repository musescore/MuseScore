/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_MAINWINDOW_MDI_QTQUICK_H
#define KD_MAINWINDOW_MDI_QTQUICK_H
#pragma once

#include "kddockwidgets/core/views/MainWindowMDIViewInterface.h"
#include "kddockwidgets/qtquick/views/MainWindow.h"

namespace KDDockWidgets {

namespace QtQuick {

/// @brief MainWindow sub-class which uses MDI as a layout
class DOCKS_EXPORT MainWindowMDI : public QtQuick::MainWindow,
                                   public Core::MainWindowMDIViewInterface
{
    Q_OBJECT
public:
    using Core::MainWindowMDIViewInterface::addDockWidget;

    ///@brief Constructor. See base class documentation
    explicit MainWindowMDI(const QString &uniqueName, QQuickItem *parent = nullptr,
                           Qt::WindowFlags flags = Qt::WindowFlags());

    ///@brief Destructor
    ~MainWindowMDI() override;
};

}

}

#endif
