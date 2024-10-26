/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "ClassicIndicatorWindowViewInterface_c.h"


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
ClassicIndicatorWindowViewInterface_wrapper::ClassicIndicatorWindowViewInterface_wrapper()
    : ::KDDockWidgets::Core::ClassicIndicatorWindowViewInterface()
{
}
KDDockWidgets::DropLocation ClassicIndicatorWindowViewInterface_wrapper::hover(KDDockWidgets::Point arg__1)
{
    if (m_hoverCallback) {
        const void *thisPtr = this;
        return m_hoverCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "hover: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::DropLocation ClassicIndicatorWindowViewInterface_wrapper::hover_nocallback(KDDockWidgets::Point arg__1)
{
    std::cerr << "hover: Warning: Calling pure-virtual\n";
    return {};
}
bool ClassicIndicatorWindowViewInterface_wrapper::isWindow() const
{
    if (m_isWindowCallback) {
        const void *thisPtr = this;
        return m_isWindowCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "isWindow: Warning: Calling pure-virtual\n";
        return {};
    }
}
bool ClassicIndicatorWindowViewInterface_wrapper::isWindow_nocallback() const
{
    std::cerr << "isWindow: Warning: Calling pure-virtual\n";
    return {};
}
KDDockWidgets::Point ClassicIndicatorWindowViewInterface_wrapper::posForIndicator(KDDockWidgets::DropLocation arg__1) const
{
    if (m_posForIndicatorCallback) {
        const void *thisPtr = this;
        return *m_posForIndicatorCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "posForIndicator: Warning: Calling pure-virtual\n";
        return {};
    }
}
KDDockWidgets::Point ClassicIndicatorWindowViewInterface_wrapper::posForIndicator_nocallback(KDDockWidgets::DropLocation arg__1) const
{
    std::cerr << "posForIndicator: Warning: Calling pure-virtual\n";
    return {};
}
void ClassicIndicatorWindowViewInterface_wrapper::raise()
{
    if (m_raiseCallback) {
        const void *thisPtr = this;
        m_raiseCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "raise: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::raise_nocallback()
{
    std::cerr << "raise: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::resize(KDDockWidgets::Size arg__1)
{
    if (m_resizeCallback) {
        const void *thisPtr = this;
        m_resizeCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "resize: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::resize_nocallback(KDDockWidgets::Size arg__1)
{
    std::cerr << "resize: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::setGeometry(KDDockWidgets::Rect arg__1)
{
    if (m_setGeometryCallback) {
        const void *thisPtr = this;
        m_setGeometryCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        std::cerr << "setGeometry: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::setGeometry_nocallback(KDDockWidgets::Rect arg__1)
{
    std::cerr << "setGeometry: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::setObjectName(const QString &arg__1)
{
    if (m_setObjectNameCallback) {
        const void *thisPtr = this;
        m_setObjectNameCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setObjectName: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::setObjectName_nocallback(const QString &arg__1)
{
    std::cerr << "setObjectName: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::setVisible(bool arg__1)
{
    if (m_setVisibleCallback) {
        const void *thisPtr = this;
        m_setVisibleCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        std::cerr << "setVisible: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::setVisible_nocallback(bool arg__1)
{
    std::cerr << "setVisible: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::updateIndicatorVisibility()
{
    if (m_updateIndicatorVisibilityCallback) {
        const void *thisPtr = this;
        m_updateIndicatorVisibilityCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "updateIndicatorVisibility: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::updateIndicatorVisibility_nocallback()
{
    std::cerr << "updateIndicatorVisibility: Warning: Calling pure-virtual\n";
    return;
}
void ClassicIndicatorWindowViewInterface_wrapper::updatePositions()
{
    if (m_updatePositionsCallback) {
        const void *thisPtr = this;
        m_updatePositionsCallback(const_cast<void *>(thisPtr));
    } else {
        std::cerr << "updatePositions: Warning: Calling pure-virtual\n";
        return;
    }
}
void ClassicIndicatorWindowViewInterface_wrapper::updatePositions_nocallback()
{
    std::cerr << "updatePositions: Warning: Calling pure-virtual\n";
    return;
}
ClassicIndicatorWindowViewInterface_wrapper::~ClassicIndicatorWindowViewInterface_wrapper()
{
}

}
}
static KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::ClassicIndicatorWindowViewInterface *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// hover(KDDockWidgets::Point arg__1)
int c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__hover_Point(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Point *>(arg__1_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hover_nocallback(arg__1);} else {    return targetPtr->hover(arg__1);} }();
    return result;
}
// isWindow() const
bool c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__isWindow(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isWindow_nocallback();} else {    return targetPtr->isWindow();} }();
    return result;
}
// posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__posForIndicator_DropLocation(void *thisObj, int arg__1)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->posForIndicator_nocallback(static_cast<KDDockWidgets::DropLocation>(arg__1));} else {    return targetPtr->posForIndicator(static_cast<KDDockWidgets::DropLocation>(arg__1));} }() };
    return result;
}
// raise()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__raise(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->raise_nocallback();} else {    return targetPtr->raise();} }();
}
// resize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__resize_Size(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Size *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->resize_nocallback(arg__1);} else {    return targetPtr->resize(arg__1);} }();
}
// setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setGeometry_Rect(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Rect *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setGeometry_nocallback(arg__1);} else {    return targetPtr->setGeometry(arg__1);} }();
}
// setObjectName(const QString & arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setObjectName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setObjectName_nocallback(arg__1);} else {    return targetPtr->setObjectName(arg__1);} }();
    free(( char * )arg__1_);
}
// setVisible(bool arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setVisible_bool(void *thisObj, bool arg__1)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setVisible_nocallback(arg__1);} else {    return targetPtr->setVisible(arg__1);} }();
}
// updateIndicatorVisibility()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updateIndicatorVisibility(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->updateIndicatorVisibility_nocallback();} else {    return targetPtr->updateIndicatorVisibility();} }();
}
// updatePositions()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updatePositions(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->updatePositions_nocallback();} else {    return targetPtr->updatePositions();} }();
}
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 1150:
        wrapper->m_hoverCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_hover>(callback);
        break;
    case 1151:
        wrapper->m_isWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_isWindow>(callback);
        break;
    case 1152:
        wrapper->m_posForIndicatorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_posForIndicator>(callback);
        break;
    case 1153:
        wrapper->m_raiseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_raise>(callback);
        break;
    case 1154:
        wrapper->m_resizeCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_resize>(callback);
        break;
    case 1155:
        wrapper->m_setGeometryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_setGeometry>(callback);
        break;
    case 1156:
        wrapper->m_setObjectNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_setObjectName>(callback);
        break;
    case 1157:
        wrapper->m_setVisibleCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_setVisible>(callback);
        break;
    case 1158:
        wrapper->m_updateIndicatorVisibilityCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_updateIndicatorVisibility>(callback);
        break;
    case 1159:
        wrapper->m_updatePositionsCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::ClassicIndicatorWindowViewInterface_wrapper::Callback_updatePositions>(callback);
        break;
    }
}
}
