/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DropArea_c.h"


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
DropArea_wrapper::DropArea_wrapper(KDDockWidgets::Core::View *parent, QFlags<KDDockWidgets::MainWindowOption> options, bool isMDIWrapper)
    : ::KDDockWidgets::Core::DropArea(parent, options, isMDIWrapper)
{
}
void DropArea_wrapper::_addDockWidget(KDDockWidgets::Core::DockWidget *dw, KDDockWidgets::Location location, KDDockWidgets::Core::Item *relativeTo, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::DropArea::_addDockWidget(dw, location, relativeTo, initialOption);
}
void DropArea_wrapper::addDockWidget(KDDockWidgets::Core::DockWidget *dw, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget *relativeTo, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::DropArea::addDockWidget(dw, location, relativeTo, initialOption);
}
void DropArea_wrapper::addMultiSplitter(KDDockWidgets::Core::DropArea *splitter, KDDockWidgets::Location location, KDDockWidgets::Core::Group *relativeToGroup, KDDockWidgets::InitialOption option)
{
    ::KDDockWidgets::Core::DropArea::addMultiSplitter(splitter, location, relativeToGroup, option);
}
void DropArea_wrapper::addWidget(KDDockWidgets::Core::View *widget, KDDockWidgets::Location location, KDDockWidgets::Core::Item *relativeToItem, KDDockWidgets::InitialOption option)
{
    ::KDDockWidgets::Core::DropArea::addWidget(widget, location, relativeToItem, option);
}
KDDockWidgets::Core::Item *DropArea_wrapper::centralFrame() const
{
    return ::KDDockWidgets::Core::DropArea::centralFrame();
}
bool DropArea_wrapper::containsDockWidget(KDDockWidgets::Core::DockWidget *arg__1) const
{
    return ::KDDockWidgets::Core::DropArea::containsDockWidget(arg__1);
}
KDDockWidgets::Core::Group *DropArea_wrapper::createCentralGroup(QFlags<KDDockWidgets::MainWindowOption> options)
{
    return ::KDDockWidgets::Core::DropArea::createCentralGroup(options);
}
KDDockWidgets::DropLocation DropArea_wrapper::currentDropLocation() const
{
    return ::KDDockWidgets::Core::DropArea::currentDropLocation();
}
KDDockWidgets::Core::DropIndicatorOverlay *DropArea_wrapper::dropIndicatorOverlay() const
{
    return ::KDDockWidgets::Core::DropArea::dropIndicatorOverlay();
}
bool DropArea_wrapper::hasSingleFloatingGroup() const
{
    return ::KDDockWidgets::Core::DropArea::hasSingleFloatingGroup();
}
bool DropArea_wrapper::hasSingleGroup() const
{
    return ::KDDockWidgets::Core::DropArea::hasSingleGroup();
}
bool DropArea_wrapper::isMDIWrapper() const
{
    return ::KDDockWidgets::Core::DropArea::isMDIWrapper();
}
void DropArea_wrapper::layoutEqually()
{
    ::KDDockWidgets::Core::DropArea::layoutEqually();
}
void DropArea_wrapper::layoutParentContainerEqually(KDDockWidgets::Core::DockWidget *arg__1)
{
    ::KDDockWidgets::Core::DropArea::layoutParentContainerEqually(arg__1);
}
KDDockWidgets::Core::DockWidget *DropArea_wrapper::mdiDockWidgetWrapper() const
{
    return ::KDDockWidgets::Core::DropArea::mdiDockWidgetWrapper();
}
void DropArea_wrapper::removeHover()
{
    ::KDDockWidgets::Core::DropArea::removeHover();
}
void DropArea_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::DropArea::setParentView_impl(parent);
    }
}
void DropArea_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::DropArea::setParentView_impl(parent);
}
DropArea_wrapper::~DropArea_wrapper()
{
}

}
}
static KDDockWidgets::Core::DropArea *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::DropArea *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__DropArea_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__DropArea__constructor_View_MainWindowOptions_bool(void *parent_, int options_, bool isMDIWrapper)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    auto options = static_cast<QFlags<KDDockWidgets::MainWindowOption>>(options_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper(parent, options, isMDIWrapper);
    return reinterpret_cast<void *>(ptr);
}
//_addDockWidget(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Location location, KDDockWidgets::Core::Item * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DropArea___addDockWidget_DockWidget_Location_Item_InitialOption(void *thisObj, void *dw_, int location, void *relativeTo_, void *initialOption_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    auto relativeTo = reinterpret_cast<KDDockWidgets::Core::Item *>(relativeTo_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->_addDockWidget(dw, static_cast<KDDockWidgets::Location>(location), relativeTo, initialOption);
}
// addDockWidget(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DropArea__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *dw_, int location, void *relativeTo_, void *initialOption_)
{
    auto dw = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(dw_);
    auto relativeTo = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(relativeTo_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->addDockWidget(dw, static_cast<KDDockWidgets::Location>(location), relativeTo, initialOption);
}
// addMultiSplitter(KDDockWidgets::Core::DropArea * splitter, KDDockWidgets::Location location, KDDockWidgets::Core::Group * relativeToGroup, KDDockWidgets::InitialOption option)
void c_KDDockWidgets__Core__DropArea__addMultiSplitter_DropArea_Location_Group_InitialOption(void *thisObj, void *splitter_, int location, void *relativeToGroup_, void *option_)
{
    auto splitter = reinterpret_cast<KDDockWidgets::Core::DropArea *>(splitter_);
    auto relativeToGroup = reinterpret_cast<KDDockWidgets::Core::Group *>(relativeToGroup_);
    assert(option_);
    auto &option = *reinterpret_cast<KDDockWidgets::InitialOption *>(option_);
    fromPtr(thisObj)->addMultiSplitter(splitter, static_cast<KDDockWidgets::Location>(location), relativeToGroup, option);
}
// addWidget(KDDockWidgets::Core::View * widget, KDDockWidgets::Location location, KDDockWidgets::Core::Item * relativeToItem, KDDockWidgets::InitialOption option)
void c_KDDockWidgets__Core__DropArea__addWidget_View_Location_Item_InitialOption(void *thisObj, void *widget_, int location, void *relativeToItem_, void *option_)
{
    auto widget = reinterpret_cast<KDDockWidgets::Core::View *>(widget_);
    auto relativeToItem = reinterpret_cast<KDDockWidgets::Core::Item *>(relativeToItem_);
    assert(option_);
    auto &option = *reinterpret_cast<KDDockWidgets::InitialOption *>(option_);
    fromPtr(thisObj)->addWidget(widget, static_cast<KDDockWidgets::Location>(location), relativeToItem, option);
}
// centralFrame() const
void *c_KDDockWidgets__Core__DropArea__centralFrame(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->centralFrame();
    return result;
}
// containsDockWidget(KDDockWidgets::Core::DockWidget * arg__1) const
bool c_KDDockWidgets__Core__DropArea__containsDockWidget_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    const auto &result = fromPtr(thisObj)->containsDockWidget(arg__1);
    return result;
}
// createCentralGroup(QFlags<KDDockWidgets::MainWindowOption> options)
void *c_static_KDDockWidgets__Core__DropArea__createCentralGroup_MainWindowOptions(int options_)
{
    auto options = static_cast<QFlags<KDDockWidgets::MainWindowOption>>(options_);
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper::createCentralGroup(options);
    return result;
}
// currentDropLocation() const
int c_KDDockWidgets__Core__DropArea__currentDropLocation(void *thisObj)
{
    const auto &result = int(fromPtr(thisObj)->currentDropLocation());
    return result;
}
// dropIndicatorOverlay() const
void *c_KDDockWidgets__Core__DropArea__dropIndicatorOverlay(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->dropIndicatorOverlay();
    return result;
}
// hasSingleFloatingGroup() const
bool c_KDDockWidgets__Core__DropArea__hasSingleFloatingGroup(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hasSingleFloatingGroup();
    return result;
}
// hasSingleGroup() const
bool c_KDDockWidgets__Core__DropArea__hasSingleGroup(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hasSingleGroup();
    return result;
}
// isMDIWrapper() const
bool c_KDDockWidgets__Core__DropArea__isMDIWrapper(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isMDIWrapper();
    return result;
}
// layoutEqually()
void c_KDDockWidgets__Core__DropArea__layoutEqually(void *thisObj)
{
    fromPtr(thisObj)->layoutEqually();
}
// layoutParentContainerEqually(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__DropArea__layoutParentContainerEqually_DockWidget(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(arg__1_);
    fromPtr(thisObj)->layoutParentContainerEqually(arg__1);
}
// mdiDockWidgetWrapper() const
void *c_KDDockWidgets__Core__DropArea__mdiDockWidgetWrapper(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->mdiDockWidgetWrapper();
    return result;
}
// removeHover()
void c_KDDockWidgets__Core__DropArea__removeHover(void *thisObj)
{
    fromPtr(thisObj)->removeHover();
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DropArea__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
void c_KDDockWidgets__Core__DropArea__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__DropArea__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DropArea_wrapper::Callback_setParentView_impl>(callback);
        break;
    }
}
}
