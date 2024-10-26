/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Config_c.h"


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
KDDockWidgets::Size Config_wrapper::absoluteWidgetMaxSize() const
{
    return ::KDDockWidgets::Config::absoluteWidgetMaxSize();
}
KDDockWidgets::Size Config_wrapper::absoluteWidgetMinSize() const
{
    return ::KDDockWidgets::Config::absoluteWidgetMinSize();
}
double Config_wrapper::draggedWindowOpacity() const
{
    return ::KDDockWidgets::Config::draggedWindowOpacity();
}
bool Config_wrapper::dropIndicatorsInhibited() const
{
    return ::KDDockWidgets::Config::dropIndicatorsInhibited();
}
bool Config_wrapper::layoutSaverUsesStrictMode() const
{
    return ::KDDockWidgets::Config::layoutSaverUsesStrictMode();
}
int Config_wrapper::layoutSpacing() const
{
    return ::KDDockWidgets::Config::layoutSpacing();
}
int Config_wrapper::mdiPopupThreshold() const
{
    return ::KDDockWidgets::Config::mdiPopupThreshold();
}
bool Config_wrapper::onlyProgrammaticDrag() const
{
    return ::KDDockWidgets::Config::onlyProgrammaticDrag();
}
void Config_wrapper::printDebug()
{
    ::KDDockWidgets::Config::printDebug();
}
KDDockWidgets::Config &Config_wrapper::self()
{
    return ::KDDockWidgets::Config::self();
}
int Config_wrapper::separatorThickness() const
{
    return ::KDDockWidgets::Config::separatorThickness();
}
void Config_wrapper::setAbsoluteWidgetMaxSize(KDDockWidgets::Size size)
{
    ::KDDockWidgets::Config::setAbsoluteWidgetMaxSize(size);
}
void Config_wrapper::setAbsoluteWidgetMinSize(KDDockWidgets::Size size)
{
    ::KDDockWidgets::Config::setAbsoluteWidgetMinSize(size);
}
void Config_wrapper::setDraggedWindowOpacity(double opacity)
{
    ::KDDockWidgets::Config::setDraggedWindowOpacity(opacity);
}
void Config_wrapper::setDropIndicatorsInhibited(bool inhibit) const
{
    ::KDDockWidgets::Config::setDropIndicatorsInhibited(inhibit);
}
void Config_wrapper::setLayoutSaverStrictMode(bool arg__1)
{
    ::KDDockWidgets::Config::setLayoutSaverStrictMode(arg__1);
}
void Config_wrapper::setLayoutSpacing(int arg__1)
{
    ::KDDockWidgets::Config::setLayoutSpacing(arg__1);
}
void Config_wrapper::setMDIPopupThreshold(int arg__1)
{
    ::KDDockWidgets::Config::setMDIPopupThreshold(arg__1);
}
void Config_wrapper::setOnlyProgrammaticDrag(bool arg__1)
{
    ::KDDockWidgets::Config::setOnlyProgrammaticDrag(arg__1);
}
void Config_wrapper::setSeparatorThickness(int value)
{
    ::KDDockWidgets::Config::setSeparatorThickness(value);
}
void Config_wrapper::setStartDragDistance(int arg__1)
{
    ::KDDockWidgets::Config::setStartDragDistance(arg__1);
}
void Config_wrapper::setTransparencyOnlyOverDropIndicator(bool only)
{
    ::KDDockWidgets::Config::setTransparencyOnlyOverDropIndicator(only);
}
void Config_wrapper::setViewFactory(KDDockWidgets::Core::ViewFactory *arg__1)
{
    ::KDDockWidgets::Config::setViewFactory(arg__1);
}
int Config_wrapper::startDragDistance() const
{
    return ::KDDockWidgets::Config::startDragDistance();
}
bool Config_wrapper::transparencyOnlyOverDropIndicator() const
{
    return ::KDDockWidgets::Config::transparencyOnlyOverDropIndicator();
}
KDDockWidgets::Core::ViewFactory *Config_wrapper::viewFactory() const
{
    return ::KDDockWidgets::Config::viewFactory();
}
Config_wrapper::~Config_wrapper()
{
}

}
static KDDockWidgets::Config *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Config *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::Config_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Config_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Config_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::Config_wrapper *>(cppObj);
} // absoluteWidgetMaxSize() const
void *c_KDDockWidgets__Config__absoluteWidgetMaxSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->absoluteWidgetMaxSize() };
    return result;
}
// absoluteWidgetMinSize() const
void *c_KDDockWidgets__Config__absoluteWidgetMinSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->absoluteWidgetMinSize() };
    return result;
}
// draggedWindowOpacity() const
double c_KDDockWidgets__Config__draggedWindowOpacity(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->draggedWindowOpacity();
    return result;
}
// dropIndicatorsInhibited() const
bool c_KDDockWidgets__Config__dropIndicatorsInhibited(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->dropIndicatorsInhibited();
    return result;
}
// layoutSaverUsesStrictMode() const
bool c_KDDockWidgets__Config__layoutSaverUsesStrictMode(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layoutSaverUsesStrictMode();
    return result;
}
// layoutSpacing() const
int c_KDDockWidgets__Config__layoutSpacing(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layoutSpacing();
    return result;
}
// mdiPopupThreshold() const
int c_KDDockWidgets__Config__mdiPopupThreshold(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->mdiPopupThreshold();
    return result;
}
// onlyProgrammaticDrag() const
bool c_KDDockWidgets__Config__onlyProgrammaticDrag(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->onlyProgrammaticDrag();
    return result;
}
// printDebug()
void c_KDDockWidgets__Config__printDebug(void *thisObj)
{
    fromPtr(thisObj)->printDebug();
}
// self()
void *c_static_KDDockWidgets__Config__self()
{
    const auto &result = &KDDockWidgetsBindings_wrappersNS::Config_wrapper::self();
    return result;
}
// separatorThickness() const
int c_KDDockWidgets__Config__separatorThickness(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->separatorThickness();
    return result;
}
// setAbsoluteWidgetMaxSize(KDDockWidgets::Size size)
void c_KDDockWidgets__Config__setAbsoluteWidgetMaxSize_Size(void *thisObj, void *size_)
{
    assert(size_);
    auto &size = *reinterpret_cast<KDDockWidgets::Size *>(size_);
    fromPtr(thisObj)->setAbsoluteWidgetMaxSize(size);
}
// setAbsoluteWidgetMinSize(KDDockWidgets::Size size)
void c_KDDockWidgets__Config__setAbsoluteWidgetMinSize_Size(void *thisObj, void *size_)
{
    assert(size_);
    auto &size = *reinterpret_cast<KDDockWidgets::Size *>(size_);
    fromPtr(thisObj)->setAbsoluteWidgetMinSize(size);
}
// setDraggedWindowOpacity(double opacity)
void c_KDDockWidgets__Config__setDraggedWindowOpacity_double(void *thisObj, double opacity)
{
    fromPtr(thisObj)->setDraggedWindowOpacity(opacity);
}
// setDropIndicatorsInhibited(bool inhibit) const
void c_KDDockWidgets__Config__setDropIndicatorsInhibited_bool(void *thisObj, bool inhibit)
{
    fromPtr(thisObj)->setDropIndicatorsInhibited(inhibit);
}
// setLayoutSaverStrictMode(bool arg__1)
void c_KDDockWidgets__Config__setLayoutSaverStrictMode_bool(void *thisObj, bool arg__1)
{
    fromPtr(thisObj)->setLayoutSaverStrictMode(arg__1);
}
// setLayoutSpacing(int arg__1)
void c_KDDockWidgets__Config__setLayoutSpacing_int(void *thisObj, int arg__1)
{
    fromPtr(thisObj)->setLayoutSpacing(arg__1);
}
// setMDIPopupThreshold(int arg__1)
void c_KDDockWidgets__Config__setMDIPopupThreshold_int(void *thisObj, int arg__1)
{
    fromPtr(thisObj)->setMDIPopupThreshold(arg__1);
}
// setOnlyProgrammaticDrag(bool arg__1)
void c_KDDockWidgets__Config__setOnlyProgrammaticDrag_bool(void *thisObj, bool arg__1)
{
    fromPtr(thisObj)->setOnlyProgrammaticDrag(arg__1);
}
// setSeparatorThickness(int value)
void c_KDDockWidgets__Config__setSeparatorThickness_int(void *thisObj, int value)
{
    fromPtr(thisObj)->setSeparatorThickness(value);
}
// setStartDragDistance(int arg__1)
void c_KDDockWidgets__Config__setStartDragDistance_int(void *thisObj, int arg__1)
{
    fromPtr(thisObj)->setStartDragDistance(arg__1);
}
// setTransparencyOnlyOverDropIndicator(bool only)
void c_KDDockWidgets__Config__setTransparencyOnlyOverDropIndicator_bool(void *thisObj, bool only)
{
    fromPtr(thisObj)->setTransparencyOnlyOverDropIndicator(only);
}
// setViewFactory(KDDockWidgets::Core::ViewFactory * arg__1)
void c_KDDockWidgets__Config__setViewFactory_ViewFactory(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::ViewFactory *>(arg__1_);
    fromPtr(thisObj)->setViewFactory(arg__1);
}
// startDragDistance() const
int c_KDDockWidgets__Config__startDragDistance(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->startDragDistance();
    return result;
}
// transparencyOnlyOverDropIndicator() const
bool c_KDDockWidgets__Config__transparencyOnlyOverDropIndicator(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->transparencyOnlyOverDropIndicator();
    return result;
}
// viewFactory() const
void *c_KDDockWidgets__Config__viewFactory(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->viewFactory();
    return result;
}
void c_KDDockWidgets__Config__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
