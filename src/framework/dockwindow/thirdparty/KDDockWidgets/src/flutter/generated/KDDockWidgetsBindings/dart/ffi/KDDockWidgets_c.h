/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include "KDDockWidgets.h"
#include <geometry_helpers_p.h>

extern "C" {
// KDDockWidgets::fuzzyCompare(double a, double b, double epsilon)
DOCKS_EXPORT bool c_static_KDDockWidgets__fuzzyCompare_double_double_double(double a, double b, double epsilon);
// KDDockWidgets::initFrontend(KDDockWidgets::FrontendType arg__1)
DOCKS_EXPORT void c_static_KDDockWidgets__initFrontend_FrontendType(int arg__1);
// KDDockWidgets::spdlogLoggerName()
DOCKS_EXPORT const char *c_static_KDDockWidgets__spdlogLoggerName();
DOCKS_EXPORT void c_KDDockWidgets_Finalizer(void *cppObj);
}
