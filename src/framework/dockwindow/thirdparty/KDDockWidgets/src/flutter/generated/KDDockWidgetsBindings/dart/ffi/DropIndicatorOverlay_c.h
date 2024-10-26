/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/DropIndicatorOverlay.h>
#include "core/DropArea.h"
#include <core/View.h>
#include <core/Group.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class DropIndicatorOverlay_wrapper : public ::KDDockWidgets::Core::DropIndicatorOverlay
{
public:
    ~DropIndicatorOverlay_wrapper();
    DropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea);
    DropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea, KDDockWidgets::Core::View *view);
    KDDockWidgets::DropLocation currentDropLocation() const;
    virtual bool dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const;
    virtual bool dropIndicatorVisible_nocallback(KDDockWidgets::DropLocation arg__1) const;
    KDDockWidgets::DropLocation hover(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_impl(KDDockWidgets::Point globalPos);
    virtual KDDockWidgets::DropLocation hover_impl_nocallback(KDDockWidgets::Point globalPos);
    KDDockWidgets::Core::Group *hoveredGroup() const;
    KDDockWidgets::Rect hoveredGroupRect() const;
    bool isHovered() const;
    static KDDockWidgets::Location multisplitterLocationFor(KDDockWidgets::DropLocation arg__1);
    virtual void onHoveredGroupChanged(KDDockWidgets::Core::Group *arg__1);
    virtual void onHoveredGroupChanged_nocallback(KDDockWidgets::Core::Group *arg__1);
    virtual KDDockWidgets::Point posForIndicator(KDDockWidgets::DropLocation arg__1) const;
    virtual KDDockWidgets::Point posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const;
    void removeHover();
    virtual void setCurrentDropLocation(KDDockWidgets::DropLocation arg__1);
    virtual void setCurrentDropLocation_nocallback(KDDockWidgets::DropLocation arg__1);
    void setHoveredGroup(KDDockWidgets::Core::Group *arg__1);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setWindowBeingDragged(bool arg__1);
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
extern "C" {
// KDDockWidgets::Core::DropIndicatorOverlay::DropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea)
DOCKS_EXPORT void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea(void *dropArea_);
// KDDockWidgets::Core::DropIndicatorOverlay::DropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea, KDDockWidgets::Core::View * view)
DOCKS_EXPORT void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea_View(void *dropArea_, void *view_);
// KDDockWidgets::Core::DropIndicatorOverlay::currentDropLocation() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DropIndicatorOverlay__currentDropLocation(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::hover(KDDockWidgets::Point globalPos)
DOCKS_EXPORT int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::DropIndicatorOverlay::hover_impl(KDDockWidgets::Point globalPos)
DOCKS_EXPORT int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroup() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroup(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroupRect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroupRect(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::isHovered() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DropIndicatorOverlay__isHovered(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::multisplitterLocationFor(KDDockWidgets::DropLocation arg__1)
DOCKS_EXPORT int c_static_KDDockWidgets__Core__DropIndicatorOverlay__multisplitterLocationFor_DropLocation(int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropIndicatorOverlay::posForIndicator(KDDockWidgets::DropLocation arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::removeHover()
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__removeHover(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::setHoveredGroup(KDDockWidgets::Core::Group * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__setHoveredGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropIndicatorOverlay::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::DropIndicatorOverlay::setWindowBeingDragged(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__setWindowBeingDragged_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::updateVisibility()
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__updateVisibility(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__DropIndicatorOverlay_Finalizer(void *cppObj);
}
