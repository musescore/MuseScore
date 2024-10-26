/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Separator.h>
#include <geometry_helpers_p.h>
#include <core/View.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Separator_wrapper : public ::KDDockWidgets::Core::Separator
{
public:
    ~Separator_wrapper();
    static bool isResizing();
    bool isVertical() const;
    static int numSeparators();
    void onMouseDoubleClick();
    void onMouseMove(KDDockWidgets::Point pos);
    void onMousePress();
    void onMouseReleased();
    int position() const;
    void setGeometry(KDDockWidgets::Rect r);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::Separator::isResizing()
DOCKS_EXPORT bool c_static_KDDockWidgets__Core__Separator__isResizing();
// KDDockWidgets::Core::Separator::isVertical() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Separator__isVertical(void *thisObj);
// KDDockWidgets::Core::Separator::numSeparators()
DOCKS_EXPORT int c_static_KDDockWidgets__Core__Separator__numSeparators();
// KDDockWidgets::Core::Separator::onMouseDoubleClick()
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__onMouseDoubleClick(void *thisObj);
// KDDockWidgets::Core::Separator::onMouseMove(KDDockWidgets::Point pos)
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__onMouseMove_Point(void *thisObj, void *pos_);
// KDDockWidgets::Core::Separator::onMousePress()
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__onMousePress(void *thisObj);
// KDDockWidgets::Core::Separator::onMouseReleased()
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__onMouseReleased(void *thisObj);
// KDDockWidgets::Core::Separator::position() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Separator__position(void *thisObj);
// KDDockWidgets::Core::Separator::setGeometry(KDDockWidgets::Rect r)
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__setGeometry_Rect(void *thisObj, void *r_);
// KDDockWidgets::Core::Separator::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__setParentView_impl_View(void *thisObj, void *parent_);
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Separator_Finalizer(void *cppObj);
}
