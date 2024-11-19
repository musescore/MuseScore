/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/Controller.h>
#include <core/View.h>
#include <geometry_helpers_p.h>
#include <string_p.h>
#include <Object_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Controller_wrapper : public ::KDDockWidgets::Core::Controller
{
public:
    ~Controller_wrapper();
    Controller_wrapper(KDDockWidgets::Core::ViewType type, KDDockWidgets::Core::View *arg__2);
    bool close();
    void destroyLater();
    KDDockWidgets::Rect geometry() const;
    int height() const;
    bool inDtor() const;
    bool isFixedHeight() const;
    bool isFixedWidth() const;
    bool isVisible() const;
    KDDockWidgets::Point mapToGlobal(KDDockWidgets::Point arg__1) const;
    KDDockWidgets::Point pos() const;
    KDDockWidgets::Rect rect() const;
    void setParentView(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setVisible(bool arg__1);
    void show() const;
    KDDockWidgets::Size size() const;
    KDDockWidgets::Core::ViewType type() const;
    KDDockWidgets::Core::View *view() const;
    int width() const;
    int x() const;
    int y() const;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::Controller::Controller(KDDockWidgets::Core::ViewType type, KDDockWidgets::Core::View * arg__2)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__constructor_ViewType_View(int type, void *arg__2_);
// KDDockWidgets::Core::Controller::close()
DOCKS_EXPORT bool c_KDDockWidgets__Core__Controller__close(void *thisObj);
// KDDockWidgets::Core::Controller::destroyLater()
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__destroyLater(void *thisObj);
// KDDockWidgets::Core::Controller::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__geometry(void *thisObj);
// KDDockWidgets::Core::Controller::height() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Controller__height(void *thisObj);
// KDDockWidgets::Core::Controller::inDtor() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Controller__inDtor(void *thisObj);
// KDDockWidgets::Core::Controller::isFixedHeight() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Controller__isFixedHeight(void *thisObj);
// KDDockWidgets::Core::Controller::isFixedWidth() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Controller__isFixedWidth(void *thisObj);
// KDDockWidgets::Core::Controller::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Controller__isVisible(void *thisObj);
// KDDockWidgets::Core::Controller::mapToGlobal(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__mapToGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Controller::pos() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__pos(void *thisObj);
// KDDockWidgets::Core::Controller::rect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__rect(void *thisObj);
// KDDockWidgets::Core::Controller::setParentView(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__setParentView_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Controller::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Controller::setVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Controller::show() const
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__show(void *thisObj);
// KDDockWidgets::Core::Controller::size() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__size(void *thisObj);
// KDDockWidgets::Core::Controller::type() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Controller__type(void *thisObj);
// KDDockWidgets::Core::Controller::view() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Controller__view(void *thisObj);
// KDDockWidgets::Core::Controller::width() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Controller__width(void *thisObj);
// KDDockWidgets::Core::Controller::x() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Controller__x(void *thisObj);
// KDDockWidgets::Core::Controller::y() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Controller__y(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Controller_Finalizer(void *cppObj);
}
