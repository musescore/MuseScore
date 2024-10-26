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
class Margins_wrapper : public ::KDDockWidgets::Margins
{
public:
    ~Margins_wrapper();
    Margins_wrapper();
    Margins_wrapper(int l, int t, int r, int b);
    int bottom() const;
    int left() const;
    int right() const;
    int top() const;
};
}
extern "C" {
// KDDockWidgets::Margins::Margins()
DOCKS_EXPORT void *c_KDDockWidgets__Margins__constructor();
// KDDockWidgets::Margins::Margins(int l, int t, int r, int b)
DOCKS_EXPORT void *c_KDDockWidgets__Margins__constructor_int_int_int_int(int l, int t, int r, int b);
// KDDockWidgets::Margins::bottom() const
DOCKS_EXPORT int c_KDDockWidgets__Margins__bottom(void *thisObj);
// KDDockWidgets::Margins::left() const
DOCKS_EXPORT int c_KDDockWidgets__Margins__left(void *thisObj);
// KDDockWidgets::Margins::right() const
DOCKS_EXPORT int c_KDDockWidgets__Margins__right(void *thisObj);
// KDDockWidgets::Margins::top() const
DOCKS_EXPORT int c_KDDockWidgets__Margins__top(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Margins__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Margins_Finalizer(void *cppObj);
}
