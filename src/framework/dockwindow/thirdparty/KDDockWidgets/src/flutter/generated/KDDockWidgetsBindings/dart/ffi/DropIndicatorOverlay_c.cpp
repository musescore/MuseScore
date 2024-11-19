/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DropIndicatorOverlay_c.h"


#include <iostream>

#include <cassert>


namespace Dartagnan {

typedef int (*CleanupCallback)(void *thisPtr);
static CleanupCallback s_cleanupCallback = nullptr;

template<typename T>
struct ValueWrapper
{
    T value;
};

}
namespace KDDockWidgetsBindings_wrappersNS {
DropIndicatorOverlay_wrapper::DropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea)
    : ::KDDockWidgets::Core::DropIndicatorOverlay(dropArea)
{
}
DropIndicatorOverlay_wrapper::DropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea, KDDockWidgets::Core::View *view)
    : ::KDDockWidgets::Core::DropIndicatorOverlay(dropArea, view)
{
}
KDDockWidgets::DropLocation DropIndicatorOverlay_wrapper::currentDropLocation() const
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::currentDropLocation();
}
bool DropIndicatorOverlay_wrapper::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
{
    if (m_dropIndicatorVisibleCallback) {
        const void *thisPtr = this;
        return m_dropIndicatorVisibleCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        return ::KDDockWidgets::Core::DropIndicatorOverlay::dropIndicatorVisible(arg__1);
    }
}
bool DropIndicatorOverlay_wrapper::dropIndicatorVisible_nocallback(KDDockWidgets::DropLocation arg__1) const
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::dropIndicatorVisible(arg__1);
}
KDDockWidgets::DropLocation DropIndicatorOverlay_wrapper::hover(KDDockWidgets::Point globalPos)
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::hover(globalPos);
}
KDDockWidgets::DropLocation DropIndicatorOverlay_wrapper::hover_impl(KDDockWidgets::Point globalPos)
{
    if (m_hover_implCallback) {
        const void *thisPtr = this;
        return m_hover_implCallback(const_cast<void *>(thisPtr), &globalPos);
    } else {
        std::cerr << "hover_impl: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::DropLocation DropIndicatorOverlay_wrapper::hover_impl_nocallback(KDDockWidgets::Point globalPos)
{
    std::cerr << "hover_impl: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Core::Group *DropIndicatorOverlay_wrapper::hoveredGroup() const
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroup();
}
KDDockWidgets::Rect DropIndicatorOverlay_wrapper::hoveredGroupRect() const
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroupRect();
}
bool DropIndicatorOverlay_wrapper::isHovered() const
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::isHovered();
}
KDDockWidgets::Location DropIndicatorOverlay_wrapper::multisplitterLocationFor(KDDockWidgets::DropLocation arg__1)
{
    return ::KDDockWidgets::Core::DropIndicatorOverlay::multisplitterLocationFor(arg__1);
}
void DropIndicatorOverlay_wrapper::onHoveredGroupChanged(KDDockWidgets::Core::Group *arg__1)
{
    if (m_onHoveredGroupChangedCallback) {
        const void *thisPtr = this;
        m_onHoveredGroupChangedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::Core::DropIndicatorOverlay::onHoveredGroupChanged(arg__1);
    }
}
void DropIndicatorOverlay_wrapper::onHoveredGroupChanged_nocallback(KDDockWidgets::Core::Group *arg__1)
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::onHoveredGroupChanged(arg__1);
}
KDDockWidgets::Point DropIndicatorOverlay_wrapper::posForIndicator(KDDockWidgets::DropLocation arg__1) const
{
    if (m_posForIndicatorCallback) {
        const void *thisPtr = this;
        return *m_posForIndicatorCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "posForIndicator: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Point DropIndicatorOverlay_wrapper::posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const
{
    std::cerr << "posForIndicator: Warning: Calling pure-virtual\n";
    return {};
}
void DropIndicatorOverlay_wrapper::removeHover()
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::removeHover();
}
void DropIndicatorOverlay_wrapper::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
{
    if (m_setCurrentDropLocationCallback) {
        const void *thisPtr = this;
        m_setCurrentDropLocationCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::Core::DropIndicatorOverlay::setCurrentDropLocation(arg__1);
    }
}
void DropIndicatorOverlay_wrapper::setCurrentDropLocation_nocallback(KDDockWidgets::DropLocation arg__1)
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::setCurrentDropLocation(arg__1);
}
void DropIndicatorOverlay_wrapper::setHoveredGroup(KDDockWidgets::Core::Group *arg__1)
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::setHoveredGroup(arg__1);
}
void DropIndicatorOverlay_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::DropIndicatorOverlay::setParentView_impl(parent);
    }
}
void DropIndicatorOverlay_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::setParentView_impl(parent);
}
void DropIndicatorOverlay_wrapper::setWindowBeingDragged(bool arg__1)
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::setWindowBeingDragged(arg__1);
}
void DropIndicatorOverlay_wrapper::updateVisibility()
{
    if (m_updateVisibilityCallback) {
        const void *thisPtr = this;
        m_updateVisibilityCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::Core::DropIndicatorOverlay::updateVisibility();
    }
}
void DropIndicatorOverlay_wrapper::updateVisibility_nocallback()
{
    ::KDDockWidgets::Core::DropIndicatorOverlay::updateVisibility();
}
DropIndicatorOverlay_wrapper::~DropIndicatorOverlay_wrapper()
{
}

}
static KDDockWidgets::Core::DropIndicatorOverlay *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::DropIndicatorOverlay *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__DropIndicatorOverlay_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea(void *dropArea_)
{
    auto dropArea = reinterpret_cast<KDDockWidgets::Core::DropArea *>(dropArea_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper(dropArea);
    return reinterpret_cast<void *>(ptr);
}
void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea_View(void *dropArea_, void *view_)
{
    auto dropArea = reinterpret_cast<KDDockWidgets::Core::DropArea *>(dropArea_);
    auto view = reinterpret_cast<KDDockWidgets::Core::View *>(view_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper(dropArea, view);
    return reinterpret_cast<void *>(ptr);
}
// currentDropLocation() const
int c_KDDockWidgets__Core__DropIndicatorOverlay__currentDropLocation(void *thisObj)
{
    const auto &result = int(fromPtr(thisObj)->currentDropLocation());
    return result;
}
// dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
bool c_KDDockWidgets__Core__DropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->dropIndicatorVisible_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->dropIndicatorVisible(static_cast<KDDockWidgets::DropLocation>(arg__1));} }();
    return result;
}
// hover(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_Point(void *thisObj, void *globalPos_)
{
    assert(globalPos_);
    auto &globalPos = *reinterpret_cast<KDDockWidgets::Point *>(globalPos_);
    const auto &result = int(fromPtr(thisObj)->hover(globalPos));
    return result;
}
// hover_impl(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_)
{
    assert(globalPos_);
    auto &globalPos = *reinterpret_cast<KDDockWidgets::Point *>(globalPos_);
    const auto &result = fromWrapperPtr(thisObj)->hover_impl_nocallback(globalPos);
    return result;
}
// hoveredGroup() const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroup(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hoveredGroup();
    return result;
}
// hoveredGroupRect() const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroupRect(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->hoveredGroupRect() };
    return result;
}
// isHovered() const
bool c_KDDockWidgets__Core__DropIndicatorOverlay__isHovered(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isHovered();
    return result;
}
// multisplitterLocationFor(KDDockWidgets::DropLocation arg__1)
int c_static_KDDockWidgets__Core__DropIndicatorOverlay__multisplitterLocationFor_DropLocation(int arg__1)
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::multisplitterLocationFor(static_cast<KDDockWidgets::DropLocation>(arg__1));
    return result;
}
// onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    fromWrapperPtr(thisObj)->onHoveredGroupChanged_nocallback(arg__1);
}
// posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->posForIndicator_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->posForIndicator(static_cast<KDDockWidgets::DropLocation>(arg__1));} }() };
    return result;
}
// removeHover()
void c_KDDockWidgets__Core__DropIndicatorOverlay__removeHover(void *thisObj)
{
    fromPtr(thisObj)->removeHover();
}
// setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setCurrentDropLocation_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->setCurrentDropLocation(static_cast<KDDockWidgets::DropLocation>(arg__1));} }();
}
// setHoveredGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setHoveredGroup_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    fromPtr(thisObj)->setHoveredGroup(arg__1);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// setWindowBeingDragged(bool arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setWindowBeingDragged_bool(void *thisObj, bool arg__1)
{
    fromPtr(thisObj)->setWindowBeingDragged(arg__1);
}
// updateVisibility()
void c_KDDockWidgets__Core__DropIndicatorOverlay__updateVisibility(void *thisObj)
{
    fromWrapperPtr(thisObj)->updateVisibility_nocallback();
}
void c_KDDockWidgets__Core__DropIndicatorOverlay__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__DropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 1042:
        wrapper->m_dropIndicatorVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_dropIndicatorVisible>(callback);
        break;
    case 1046:
        wrapper->m_hover_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_hover_impl>(callback);
        break;
    case 1058:
        wrapper->m_onHoveredGroupChangedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_onHoveredGroupChanged>(callback);
        break;
    case 1060:
        wrapper->m_posForIndicatorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_posForIndicator>(callback);
        break;
    case 1063:
        wrapper->m_setCurrentDropLocationCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_setCurrentDropLocation>(callback);
        break;
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_setParentView_impl>(callback);
        break;
    case 1072:
        wrapper->m_updateVisibilityCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::DropIndicatorOverlay_wrapper::Callback_updateVisibility>(callback);
        break;
    }
}
}
