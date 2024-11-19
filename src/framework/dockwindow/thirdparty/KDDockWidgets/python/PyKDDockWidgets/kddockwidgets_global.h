/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Renato Araujo Oliveira Filho <renato.araujo@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

// We only support Qt backend
#define KDDW_FRONTEND_QT 1
#define KDDW_FRONTEND_QTWIDGETS 1

// Make "signals:", "slots:" visible as access specifiers
#define QT_ANNOTATE_ACCESS_SPECIFIER(a) __attribute__((annotate(#a)))

// Define PYTHON_BINDINGS this will be used in some part of c++ to skip problematic parts
#define PYTHON_BINDINGS

#ifndef QT_WIDGETS_LIB
#define QT_WIDGETS_LIB
#endif

#include <kddockwidgets/KDDockWidgets.h>
#include <kddockwidgets/LayoutSaver.h>
#include <kddockwidgets/Config.h>
#include <kddockwidgets/core/views/MainWindowViewInterface.h>
#include <kddockwidgets/core/views/DockWidgetViewInterface.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtcommon/View.h>
