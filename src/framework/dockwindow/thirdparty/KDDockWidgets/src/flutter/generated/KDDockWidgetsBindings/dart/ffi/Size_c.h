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
class Size_wrapper : public ::KDDockWidgets::Size
{
public:
    ~Size_wrapper();
    Size_wrapper();
    Size_wrapper(int width, int height);
    KDDockWidgets::Size boundedTo(KDDockWidgets::Size sz) const;
    KDDockWidgets::Size expandedTo(KDDockWidgets::Size sz) const;
    int height() const;
    bool isEmpty() const;
    bool isNull() const;
    bool isValid() const;
    void setHeight(int h);
    void setWidth(int w);
    int width() const;
};
}
extern "C" {
// KDDockWidgets::Size::Size()
DOCKS_EXPORT void *c_KDDockWidgets__Size__constructor();
// KDDockWidgets::Size::Size(int width, int height)
DOCKS_EXPORT void *c_KDDockWidgets__Size__constructor_int_int(int width, int height);
// KDDockWidgets::Size::boundedTo(KDDockWidgets::Size sz) const
DOCKS_EXPORT void *c_KDDockWidgets__Size__boundedTo_Size(void *thisObj, void *sz_);
// KDDockWidgets::Size::expandedTo(KDDockWidgets::Size sz) const
DOCKS_EXPORT void *c_KDDockWidgets__Size__expandedTo_Size(void *thisObj, void *sz_);
// KDDockWidgets::Size::height() const
DOCKS_EXPORT int c_KDDockWidgets__Size__height(void *thisObj);
// KDDockWidgets::Size::isEmpty() const
DOCKS_EXPORT bool c_KDDockWidgets__Size__isEmpty(void *thisObj);
// KDDockWidgets::Size::isNull() const
DOCKS_EXPORT bool c_KDDockWidgets__Size__isNull(void *thisObj);
// KDDockWidgets::Size::isValid() const
DOCKS_EXPORT bool c_KDDockWidgets__Size__isValid(void *thisObj);
// KDDockWidgets::Size::setHeight(int h)
DOCKS_EXPORT void c_KDDockWidgets__Size__setHeight_int(void *thisObj, int h);
// KDDockWidgets::Size::setWidth(int w)
DOCKS_EXPORT void c_KDDockWidgets__Size__setWidth_int(void *thisObj, int w);
// KDDockWidgets::Size::width() const
DOCKS_EXPORT int c_KDDockWidgets__Size__width(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Size__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Size_Finalizer(void *cppObj);
}
