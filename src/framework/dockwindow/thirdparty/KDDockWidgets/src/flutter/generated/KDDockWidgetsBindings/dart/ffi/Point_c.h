/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class Point_wrapper : public ::KDDockWidgets::Point
{
public:
    ~Point_wrapper();
    Point_wrapper();
    Point_wrapper(int x, int y);
    bool isNull() const;
    int manhattanLength() const;
    void setX(int x);
    void setY(int y);
    int x() const;
    int y() const;
};
}
extern "C" {
// KDDockWidgets::Point::Point()
DOCKS_EXPORT void *c_KDDockWidgets__Point__constructor();
// KDDockWidgets::Point::Point(int x, int y)
DOCKS_EXPORT void *c_KDDockWidgets__Point__constructor_int_int(int x, int y);
// KDDockWidgets::Point::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__Point__isNull(void *thisObj);
// KDDockWidgets::Point::manhattanLength() const
DOCKS_EXPORT int c_KDDockWidgets__Point__manhattanLength(void *thisObj);
// KDDockWidgets::Point::setX(int x)
DOCKS_EXPORT void c_KDDockWidgets__Point__setX_int(void *thisObj, int x);
// KDDockWidgets::Point::setY(int y)
DOCKS_EXPORT void c_KDDockWidgets__Point__setY_int(void *thisObj, int y);
// KDDockWidgets::Point::x() const
DOCKS_EXPORT int c_KDDockWidgets__Point__x(void *thisObj);
// KDDockWidgets::Point::y() const
DOCKS_EXPORT int c_KDDockWidgets__Point__y(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Point__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Point_Finalizer(void *cppObj);
}
