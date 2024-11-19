/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <ClassicDropIndicatorOverlay.h>
#include "core/DropArea.h"
#include <geometry_helpers_p.h>
#include <ClassicIndicatorWindowViewInterface.h>
#include <core/View.h>
#include <core/Group.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class ClassicDropIndicatorOverlay_wrapper : public ::KDDockWidgets::Core::ClassicDropIndicatorOverlay
{
public:
    ~ClassicDropIndicatorOverlay_wrapper();
    ClassicDropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea);
    virtual bool dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const;
    virtual bool dropIndicatorVisible_nocallback(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::DropLocation hover_impl(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_impl_nocallback(KDDockWidgets::Point globalPos);
    KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *indicatorWindow() const;
    virtual void onHoveredGroupChanged(KDDockWidgets::Core::Group *arg__1);
    virtual void onHoveredGroupChanged_nocallback(KDDockWidgets::Core::Group *arg__1);
    bool onResize(KDDockWidgets::Size newSize);
    virtual KDDockWidgets::Point posForIndicator(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const;
    KDDockWidgets::Core::View *rubberBand() const;
    virtual void setCurrentDropLocation(KDDockWidgets::DropLocation arg__1);
    virtual void setCurrentDropLocation_nocallback(KDDockWidgets::DropLocation arg__1);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    virtual void updateVisibility();
    virtual void updateVisibility_nocallback();
    typedef bool (*Callback_dropIndicatorVisible)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_dropIndicatorVisible m_dropIndicatorVisibleCallback = nullptr;
    typedef KDDockWidgets::DropLocation (*Callback_hover_impl)(void *, KDDockWidgets::Point *globalPos);
    Callback_hover_impl m_hover_implCallback = nullptr;
    typedef void (*Callback_onHoveredGroupChanged)(void *, KDDockWidgets::Core::Group *arg__1);
    Callback_onHoveredGroupChanged m_onHoveredGroupChangedCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_posForIndicator)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_posForIndicator m_posForIndicatorCallback = nullptr;
    typedef void (*Callback_setCurrentDropLocation)(void *, KDDockWidgets::DropLocation arg__1);
    Callback_setCurrentDropLocation m_setCurrentDropLocationCallback = nullptr;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
    typedef void (*Callback_updateVisibility)(void *);
    Callback_updateVisibility m_updateVisibilityCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::ClassicDropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea)
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__constructor_DropArea(void *dropArea_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::hover_impl(KDDockWidgets::Point globalPos)
DOCKS_EXPORT int c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::indicatorWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__indicatorWindow(void *thisObj);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::onResize(KDDockWidgets::Size newSize)
DOCKS_EXPORT bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onResize_Size(void *thisObj, void *newSize_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::posForIndicator(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::rubberBand() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__rubberBand(void *thisObj);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::updateVisibility()
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__updateVisibility(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay_Finalizer(void *cppObj);
}
