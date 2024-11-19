/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <ClassicIndicatorWindowViewInterface.h>
#include <string_p.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class ClassicIndicatorWindowViewInterface_wrapper : public ::KDDockWidgets::Core::ClassicIndicatorWindowViewInterface
{
public:
    ~ClassicIndicatorWindowViewInterface_wrapper();
    ClassicIndicatorWindowViewInterface_wrapper();
    virtual KDDockWidgets::DropLocation hover(KDDockWidgets::Point arg__1);
    virtual KDDockWidgets::DropLocation hover_nocallback(KDDockWidgets::Point arg__1);
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
    virtual KDDockWidgets::Point posForIndicator(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const;
    virtual void raise();
    virtual void raise_nocallback();
    virtual void resize(KDDockWidgets::Size arg__1);
    virtual void resize_nocallback(KDDockWidgets::Size arg__1);
    virtual void setGeometry(KDDockWidgets::Rect arg__1);
    virtual void setGeometry_nocallback(KDDockWidgets::Rect arg__1);
    virtual void setObjectName(const QString &arg__1);
    virtual void setObjectName_nocallback(const QString &arg__1);
    virtual void setVisible(bool arg__1);
    virtual void setVisible_nocallback(bool arg__1);
    virtual void updateIndicatorVisibility();
    virtual void updateIndicatorVisibility_nocallback();
    virtual void updatePositions();
    virtual void updatePositions_nocallback();
    typedef KDDockWidgets::DropLocation (*Callback_hover)(void *, KDDockWidgets::Point *arg__1);
    Callback_hover m_hoverCallback = nullptr;
    typedef bool (*Callback_isWindow)(void *);
    Callback_isWindow m_isWindowCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_posForIndicator)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_posForIndicator m_posForIndicatorCallback = nullptr;
    typedef void (*Callback_raise)(void *);
    Callback_raise m_raiseCallback = nullptr;
    typedef void (*Callback_resize)(void *, KDDockWidgets::Size *arg__1);
    Callback_resize m_resizeCallback = nullptr;
    typedef void (*Callback_setGeometry)(void *, KDDockWidgets::Rect *arg__1);
    Callback_setGeometry m_setGeometryCallback = nullptr;
    typedef void (*Callback_setObjectName)(void *, const QString &arg__1);
    Callback_setObjectName m_setObjectNameCallback = nullptr;
    typedef void (*Callback_setVisible)(void *, bool arg__1);
    Callback_setVisible m_setVisibleCallback = nullptr;
    typedef void (*Callback_updateIndicatorVisibility)(void *);
    Callback_updateIndicatorVisibility m_updateIndicatorVisibilityCallback = nullptr;
    typedef void (*Callback_updatePositions)(void *);
    Callback_updatePositions m_updatePositionsCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::ClassicIndicatorWindowViewInterface()
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__constructor();
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::hover(KDDockWidgets::Point arg__1)
DOCKS_EXPORT int c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__hover_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__isWindow(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::posForIndicator(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::raise()
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__raise(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::resize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setGeometry(KDDockWidgets::Rect arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setObjectName(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::updateIndicatorVisibility()
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updateIndicatorVisibility(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::updatePositions()
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updatePositions(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface_Finalizer(void *cppObj);
}
