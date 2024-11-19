/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "ClassicDropIndicatorOverlay_c.h"


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
namespace KDDWBindingsCore {
ClassicDropIndicatorOverlay_wrapper::ClassicDropIndicatorOverlay_wrapper(KDDockWidgets::Core::DropArea *dropArea)
    : ::KDDockWidgets::Core::ClassicDropIndicatorOverlay(dropArea)
{
}
bool ClassicDropIndicatorOverlay_wrapper::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
{
    if (m_dropIndicatorVisibleCallback) {
        const void *thisPtr = this;
        return m_dropIndicatorVisibleCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::dropIndicatorVisible(arg__1);
    }
}
bool ClassicDropIndicatorOverlay_wrapper::dropIndicatorVisible_nocallback(KDDockWidgets::DropLocation arg__1) const
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::dropIndicatorVisible(arg__1);
}
KDDockWidgets::DropLocation ClassicDropIndicatorOverlay_wrapper::hover_impl(KDDockWidgets::Point globalPos)
{
    if (m_hover_implCallback) {
        const void *thisPtr = this;
        return m_hover_implCallback(const_cast<void *>(thisPtr), &globalPos);
    } else {
        return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::hover_impl(globalPos);
    }
}
KDDockWidgets::DropLocation ClassicDropIndicatorOverlay_wrapper::hover_impl_nocallback(KDDockWidgets::Point globalPos)
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::hover_impl(globalPos);
}
KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *ClassicDropIndicatorOverlay_wrapper::indicatorWindow() const
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::indicatorWindow();
}
void ClassicDropIndicatorOverlay_wrapper::onHoveredGroupChanged(KDDockWidgets::Core::Group *arg__1)
{
    if (m_onHoveredGroupChangedCallback) {
        const void *thisPtr = this;
        m_onHoveredGroupChangedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::onHoveredGroupChanged(arg__1);
    }
}
void ClassicDropIndicatorOverlay_wrapper::onHoveredGroupChanged_nocallback(KDDockWidgets::Core::Group *arg__1)
{
    ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::onHoveredGroupChanged(arg__1);
}
bool ClassicDropIndicatorOverlay_wrapper::onResize(KDDockWidgets::Size newSize)
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::onResize(newSize);
}
KDDockWidgets::Point ClassicDropIndicatorOverlay_wrapper::posForIndicator(KDDockWidgets::DropLocation arg__1) const
{
    if (m_posForIndicatorCallback) {
        const void *thisPtr = this;
        return *m_posForIndicatorCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::posForIndicator(arg__1);
    }
}
KDDockWidgets::Point ClassicDropIndicatorOverlay_wrapper::posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::posForIndicator(arg__1);
}
KDDockWidgets::Core::View *ClassicDropIndicatorOverlay_wrapper::rubberBand() const
{
    return ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::rubberBand();
}
void ClassicDropIndicatorOverlay_wrapper::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
{
    if (m_setCurrentDropLocationCallback) {
        const void *thisPtr = this;
        m_setCurrentDropLocationCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::setCurrentDropLocation(arg__1);
    }
}
void ClassicDropIndicatorOverlay_wrapper::setCurrentDropLocation_nocallback(KDDockWidgets::DropLocation arg__1)
{
    ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::setCurrentDropLocation(arg__1);
}
void ClassicDropIndicatorOverlay_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::setParentView_impl(parent);
    }
}
void ClassicDropIndicatorOverlay_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::setParentView_impl(parent);
}
void ClassicDropIndicatorOverlay_wrapper::updateVisibility()
{
    if (m_updateVisibilityCallback) {
        const void *thisPtr = this;
        m_updateVisibilityCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::updateVisibility();
    }
}
void ClassicDropIndicatorOverlay_wrapper::updateVisibility_nocallback()
{
    ::KDDockWidgets::Core::ClassicDropIndicatorOverlay::updateVisibility();
}
ClassicDropIndicatorOverlay_wrapper::~ClassicDropIndicatorOverlay_wrapper()
{
}

}
}
static KDDockWidgets::Core::ClassicDropIndicatorOverlay *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::ClassicDropIndicatorOverlay *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__constructor_DropArea(void *dropArea_)
{
    auto dropArea = reinterpret_cast<KDDockWidgets::Core::DropArea *>(dropArea_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper(dropArea);
    return reinterpret_cast<void *>(ptr);
}
// dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->dropIndicatorVisible_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->dropIndicatorVisible(static_cast<KDDockWidgets::DropLocation>(arg__1));} }();
    return result;
}
// hover_impl(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_)
{
    assert(globalPos_);
    auto &globalPos = *reinterpret_cast<KDDockWidgets::Point *>(globalPos_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hover_impl_nocallback(globalPos);} else {    return targetPtr->hover_impl(globalPos);} }();
    return result;
}
// indicatorWindow() const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__indicatorWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->indicatorWindow();
    return result;
}
// onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::Group *>(arg__1_);
    fromWrapperPtr(thisObj)->onHoveredGroupChanged_nocallback(arg__1);
}
// onResize(KDDockWidgets::Size newSize)
bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onResize_Size(void *thisObj, void *newSize_)
{
    assert(newSize_);
    auto &newSize = *reinterpret_cast<KDDockWidgets::Size *>(newSize_);
    const auto &result = fromPtr(thisObj)->onResize(newSize);
    return result;
}
// posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->posForIndicator_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->posForIndicator(static_cast<KDDockWidgets::DropLocation>(arg__1));} }() };
    return result;
}
// rubberBand() const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__rubberBand(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->rubberBand();
    return result;
}
// setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setCurrentDropLocation_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->setCurrentDropLocation(static_cast<KDDockWidgets::DropLocation>(arg__1));} }();
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// updateVisibility()
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__updateVisibility(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->updateVisibility_nocallback();} else {    return targetPtr->updateVisibility();} }();
}
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 1042:
        wrapper->m_dropIndicatorVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_dropIndicatorVisible>(callback);
        break;
    case 1046:
        wrapper->m_hover_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_hover_impl>(callback);
        break;
    case 1058:
        wrapper->m_onHoveredGroupChangedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_onHoveredGroupChanged>(callback);
        break;
    case 1060:
        wrapper->m_posForIndicatorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_posForIndicator>(callback);
        break;
    case 1063:
        wrapper->m_setCurrentDropLocationCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_setCurrentDropLocation>(callback);
        break;
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_setParentView_impl>(callback);
        break;
    case 1072:
        wrapper->m_updateVisibilityCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicDropIndicatorOverlay_wrapper::Callback_updateVisibility>(callback);
        break;
    }
}
}
