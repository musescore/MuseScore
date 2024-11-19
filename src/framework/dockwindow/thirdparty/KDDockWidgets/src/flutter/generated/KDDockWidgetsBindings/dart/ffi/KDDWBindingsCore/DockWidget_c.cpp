/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "DockWidget_c.h"


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
DockWidget_wrapper::DockWidget_wrapper(KDDockWidgets::Core::View *view, const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions)
    : ::KDDockWidgets::Core::DockWidget(view, uniqueName, options, layoutSaverOptions)
{
}
void DockWidget_wrapper::addDockWidgetAsTab(KDDockWidgets::Core::DockWidget *other, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::DockWidget::addDockWidgetAsTab(other, initialOption);
}
void DockWidget_wrapper::addDockWidgetToContainingWindow(KDDockWidgets::Core::DockWidget *other, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget *relativeTo, KDDockWidgets::InitialOption initialOption)
{
    ::KDDockWidgets::Core::DockWidget::addDockWidgetToContainingWindow(other, location, relativeTo, initialOption);
}
KDDockWidgets::Core::DockWidget *DockWidget_wrapper::byName(const QString &uniqueName)
{
    return ::KDDockWidgets::Core::DockWidget::byName(uniqueName);
}
int DockWidget_wrapper::currentTabIndex() const
{
    return ::KDDockWidgets::Core::DockWidget::currentTabIndex();
}
KDDockWidgets::Core::FloatingWindow *DockWidget_wrapper::floatingWindow() const
{
    return ::KDDockWidgets::Core::DockWidget::floatingWindow();
}
void DockWidget_wrapper::forceClose()
{
    ::KDDockWidgets::Core::DockWidget::forceClose();
}
KDDockWidgets::Rect DockWidget_wrapper::groupGeometry() const
{
    return ::KDDockWidgets::Core::DockWidget::groupGeometry();
}
bool DockWidget_wrapper::hasPreviousDockedLocation() const
{
    return ::KDDockWidgets::Core::DockWidget::hasPreviousDockedLocation();
}
void DockWidget_wrapper::init()
{
    ::KDDockWidgets::Core::DockWidget::init();
}
bool DockWidget_wrapper::isCurrentTab() const
{
    return ::KDDockWidgets::Core::DockWidget::isCurrentTab();
}
bool DockWidget_wrapper::isFloating() const
{
    return ::KDDockWidgets::Core::DockWidget::isFloating();
}
bool DockWidget_wrapper::isFocused() const
{
    return ::KDDockWidgets::Core::DockWidget::isFocused();
}
bool DockWidget_wrapper::isInMainWindow() const
{
    return ::KDDockWidgets::Core::DockWidget::isInMainWindow();
}
bool DockWidget_wrapper::isInSideBar() const
{
    return ::KDDockWidgets::Core::DockWidget::isInSideBar();
}
bool DockWidget_wrapper::isMainWindow() const
{
    return ::KDDockWidgets::Core::DockWidget::isMainWindow();
}
bool DockWidget_wrapper::isOpen() const
{
    return ::KDDockWidgets::Core::DockWidget::isOpen();
}
bool DockWidget_wrapper::isOverlayed() const
{
    return ::KDDockWidgets::Core::DockWidget::isOverlayed();
}
bool DockWidget_wrapper::isPersistentCentralDockWidget() const
{
    return ::KDDockWidgets::Core::DockWidget::isPersistentCentralDockWidget();
}
bool DockWidget_wrapper::isTabbed() const
{
    return ::KDDockWidgets::Core::DockWidget::isTabbed();
}
KDDockWidgets::Size DockWidget_wrapper::lastOverlayedSize() const
{
    return ::KDDockWidgets::Core::DockWidget::lastOverlayedSize();
}
QFlags<KDDockWidgets::LayoutSaverOption> DockWidget_wrapper::layoutSaverOptions() const
{
    return ::KDDockWidgets::Core::DockWidget::layoutSaverOptions();
}
KDDockWidgets::Core::MainWindow *DockWidget_wrapper::mainWindow() const
{
    return ::KDDockWidgets::Core::DockWidget::mainWindow();
}
int DockWidget_wrapper::mdiZ() const
{
    return ::KDDockWidgets::Core::DockWidget::mdiZ();
}
void DockWidget_wrapper::moveToSideBar()
{
    ::KDDockWidgets::Core::DockWidget::moveToSideBar();
}
void DockWidget_wrapper::onResize(KDDockWidgets::Size newSize)
{
    ::KDDockWidgets::Core::DockWidget::onResize(newSize);
}
void DockWidget_wrapper::open()
{
    ::KDDockWidgets::Core::DockWidget::open();
}
QFlags<KDDockWidgets::DockWidgetOption> DockWidget_wrapper::options() const
{
    return ::KDDockWidgets::Core::DockWidget::options();
}
void DockWidget_wrapper::raise()
{
    ::KDDockWidgets::Core::DockWidget::raise();
}
void DockWidget_wrapper::removeFromSideBar()
{
    ::KDDockWidgets::Core::DockWidget::removeFromSideBar();
}
void DockWidget_wrapper::resizeInLayout(int left, int top, int right, int bottom)
{
    ::KDDockWidgets::Core::DockWidget::resizeInLayout(left, top, right, bottom);
}
void DockWidget_wrapper::setAffinityName(const QString &name)
{
    ::KDDockWidgets::Core::DockWidget::setAffinityName(name);
}
void DockWidget_wrapper::setAsCurrentTab()
{
    ::KDDockWidgets::Core::DockWidget::setAsCurrentTab();
}
bool DockWidget_wrapper::setFloating(bool floats)
{
    return ::KDDockWidgets::Core::DockWidget::setFloating(floats);
}
void DockWidget_wrapper::setFloatingGeometry(KDDockWidgets::Rect geo)
{
    ::KDDockWidgets::Core::DockWidget::setFloatingGeometry(geo);
}
void DockWidget_wrapper::setMDIPosition(KDDockWidgets::Point pos)
{
    ::KDDockWidgets::Core::DockWidget::setMDIPosition(pos);
}
void DockWidget_wrapper::setMDISize(KDDockWidgets::Size size)
{
    ::KDDockWidgets::Core::DockWidget::setMDISize(size);
}
void DockWidget_wrapper::setMDIZ(int z)
{
    ::KDDockWidgets::Core::DockWidget::setMDIZ(z);
}
void DockWidget_wrapper::setOptions(QFlags<KDDockWidgets::DockWidgetOption> arg__1)
{
    ::KDDockWidgets::Core::DockWidget::setOptions(arg__1);
}
void DockWidget_wrapper::setParentView_impl(KDDockWidgets::Core::View *parent)
{
    if (m_setParentView_implCallback) {
        const void *thisPtr = this;
        m_setParentView_implCallback(const_cast<void *>(thisPtr), parent);
    } else {
        ::KDDockWidgets::Core::DockWidget::setParentView_impl(parent);
    }
}
void DockWidget_wrapper::setParentView_impl_nocallback(KDDockWidgets::Core::View *parent)
{
    ::KDDockWidgets::Core::DockWidget::setParentView_impl(parent);
}
void DockWidget_wrapper::setTitle(const QString &title)
{
    ::KDDockWidgets::Core::DockWidget::setTitle(title);
}
void DockWidget_wrapper::setUniqueName(const QString &arg__1)
{
    ::KDDockWidgets::Core::DockWidget::setUniqueName(arg__1);
}
void DockWidget_wrapper::setUserType(int userType)
{
    ::KDDockWidgets::Core::DockWidget::setUserType(userType);
}
void DockWidget_wrapper::show()
{
    ::KDDockWidgets::Core::DockWidget::show();
}
KDDockWidgets::Size DockWidget_wrapper::sizeInLayout() const
{
    return ::KDDockWidgets::Core::DockWidget::sizeInLayout();
}
bool DockWidget_wrapper::skipsRestore() const
{
    return ::KDDockWidgets::Core::DockWidget::skipsRestore();
}
bool DockWidget_wrapper::startDragging(bool singleTab)
{
    return ::KDDockWidgets::Core::DockWidget::startDragging(singleTab);
}
int DockWidget_wrapper::tabIndex() const
{
    return ::KDDockWidgets::Core::DockWidget::tabIndex();
}
QString DockWidget_wrapper::title() const
{
    return ::KDDockWidgets::Core::DockWidget::title();
}
KDDockWidgets::Core::TitleBar *DockWidget_wrapper::titleBar() const
{
    return ::KDDockWidgets::Core::DockWidget::titleBar();
}
QString DockWidget_wrapper::uniqueName() const
{
    return ::KDDockWidgets::Core::DockWidget::uniqueName();
}
int DockWidget_wrapper::userType() const
{
    return ::KDDockWidgets::Core::DockWidget::userType();
}
bool DockWidget_wrapper::wasRestored() const
{
    return ::KDDockWidgets::Core::DockWidget::wasRestored();
}
DockWidget_wrapper::~DockWidget_wrapper()
{
}

}
}
static KDDockWidgets::Core::DockWidget *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::Core::DockWidget *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__Core__DockWidget_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper *>(cppObj);
}
void *c_KDDockWidgets__Core__DockWidget__constructor_View_QString_DockWidgetOptions_LayoutSaverOptions(void *view_, const char *uniqueName_, int options_, int layoutSaverOptions_)
{
    auto view = reinterpret_cast<KDDockWidgets::Core::View *>(view_);
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    auto options = static_cast<QFlags<KDDockWidgets::DockWidgetOption>>(options_);
    auto layoutSaverOptions = static_cast<QFlags<KDDockWidgets::LayoutSaverOption>>(layoutSaverOptions_);
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper(view, uniqueName, options, layoutSaverOptions);
    return reinterpret_cast<void *>(ptr);
}
// addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DockWidget__addDockWidgetAsTab_DockWidget_InitialOption(void *thisObj, void *other_, void *initialOption_)
{
    auto other = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(other_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->addDockWidgetAsTab(other, initialOption);
}
// addDockWidgetToContainingWindow(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DockWidget__addDockWidgetToContainingWindow_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *other_, int location, void *relativeTo_, void *initialOption_)
{
    auto other = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(other_);
    auto relativeTo = reinterpret_cast<KDDockWidgets::Core::DockWidget *>(relativeTo_);
    assert(initialOption_);
    auto &initialOption = *reinterpret_cast<KDDockWidgets::InitialOption *>(initialOption_);
    fromPtr(thisObj)->addDockWidgetToContainingWindow(other, static_cast<KDDockWidgets::Location>(location), relativeTo, initialOption);
}
// byName(const QString & uniqueName)
void *c_static_KDDockWidgets__Core__DockWidget__byName_QString(const char *uniqueName_)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper::byName(uniqueName);
    free(( char * )uniqueName_);
    return result;
}
// currentTabIndex() const
int c_KDDockWidgets__Core__DockWidget__currentTabIndex(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->currentTabIndex();
    return result;
}
// floatingWindow() const
void *c_KDDockWidgets__Core__DockWidget__floatingWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->floatingWindow();
    return result;
}
// forceClose()
void c_KDDockWidgets__Core__DockWidget__forceClose(void *thisObj)
{
    fromPtr(thisObj)->forceClose();
}
// groupGeometry() const
void *c_KDDockWidgets__Core__DockWidget__groupGeometry(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Rect> { fromPtr(thisObj)->groupGeometry() };
    return result;
}
// hasPreviousDockedLocation() const
bool c_KDDockWidgets__Core__DockWidget__hasPreviousDockedLocation(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->hasPreviousDockedLocation();
    return result;
}
// init()
void c_KDDockWidgets__Core__DockWidget__init(void *thisObj)
{
    fromPtr(thisObj)->init();
}
// isCurrentTab() const
bool c_KDDockWidgets__Core__DockWidget__isCurrentTab(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isCurrentTab();
    return result;
}
// isFloating() const
bool c_KDDockWidgets__Core__DockWidget__isFloating(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isFloating();
    return result;
}
// isFocused() const
bool c_KDDockWidgets__Core__DockWidget__isFocused(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isFocused();
    return result;
}
// isInMainWindow() const
bool c_KDDockWidgets__Core__DockWidget__isInMainWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isInMainWindow();
    return result;
}
// isInSideBar() const
bool c_KDDockWidgets__Core__DockWidget__isInSideBar(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isInSideBar();
    return result;
}
// isMainWindow() const
bool c_KDDockWidgets__Core__DockWidget__isMainWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isMainWindow();
    return result;
}
// isOpen() const
bool c_KDDockWidgets__Core__DockWidget__isOpen(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isOpen();
    return result;
}
// isOverlayed() const
bool c_KDDockWidgets__Core__DockWidget__isOverlayed(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isOverlayed();
    return result;
}
// isPersistentCentralDockWidget() const
bool c_KDDockWidgets__Core__DockWidget__isPersistentCentralDockWidget(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isPersistentCentralDockWidget();
    return result;
}
// isTabbed() const
bool c_KDDockWidgets__Core__DockWidget__isTabbed(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->isTabbed();
    return result;
}
// lastOverlayedSize() const
void *c_KDDockWidgets__Core__DockWidget__lastOverlayedSize(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->lastOverlayedSize() };
    return result;
}
// layoutSaverOptions() const
int c_KDDockWidgets__Core__DockWidget__layoutSaverOptions(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->layoutSaverOptions();
    return result;
}
// mainWindow() const
void *c_KDDockWidgets__Core__DockWidget__mainWindow(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->mainWindow();
    return result;
}
// mdiZ() const
int c_KDDockWidgets__Core__DockWidget__mdiZ(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->mdiZ();
    return result;
}
// moveToSideBar()
void c_KDDockWidgets__Core__DockWidget__moveToSideBar(void *thisObj)
{
    fromPtr(thisObj)->moveToSideBar();
}
// onResize(KDDockWidgets::Size newSize)
void c_KDDockWidgets__Core__DockWidget__onResize_Size(void *thisObj, void *newSize_)
{
    assert(newSize_);
    auto &newSize = *reinterpret_cast<KDDockWidgets::Size *>(newSize_);
    fromPtr(thisObj)->onResize(newSize);
}
// open()
void c_KDDockWidgets__Core__DockWidget__open(void *thisObj)
{
    fromPtr(thisObj)->open();
}
// options() const
int c_KDDockWidgets__Core__DockWidget__options(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->options();
    return result;
}
// raise()
void c_KDDockWidgets__Core__DockWidget__raise(void *thisObj)
{
    fromPtr(thisObj)->raise();
}
// removeFromSideBar()
void c_KDDockWidgets__Core__DockWidget__removeFromSideBar(void *thisObj)
{
    fromPtr(thisObj)->removeFromSideBar();
}
// resizeInLayout(int left, int top, int right, int bottom)
void c_KDDockWidgets__Core__DockWidget__resizeInLayout_int_int_int_int(void *thisObj, int left, int top, int right, int bottom)
{
    fromPtr(thisObj)->resizeInLayout(left, top, right, bottom);
}
// setAffinityName(const QString & name)
void c_KDDockWidgets__Core__DockWidget__setAffinityName_QString(void *thisObj, const char *name_)
{
    const auto name = QString::fromUtf8(name_);
    fromPtr(thisObj)->setAffinityName(name);
    free(( char * )name_);
}
// setAsCurrentTab()
void c_KDDockWidgets__Core__DockWidget__setAsCurrentTab(void *thisObj)
{
    fromPtr(thisObj)->setAsCurrentTab();
}
// setFloating(bool floats)
bool c_KDDockWidgets__Core__DockWidget__setFloating_bool(void *thisObj, bool floats)
{
    const auto &result = fromPtr(thisObj)->setFloating(floats);
    return result;
}
// setFloatingGeometry(KDDockWidgets::Rect geo)
void c_KDDockWidgets__Core__DockWidget__setFloatingGeometry_Rect(void *thisObj, void *geo_)
{
    assert(geo_);
    auto &geo = *reinterpret_cast<KDDockWidgets::Rect *>(geo_);
    fromPtr(thisObj)->setFloatingGeometry(geo);
}
// setMDIPosition(KDDockWidgets::Point pos)
void c_KDDockWidgets__Core__DockWidget__setMDIPosition_Point(void *thisObj, void *pos_)
{
    assert(pos_);
    auto &pos = *reinterpret_cast<KDDockWidgets::Point *>(pos_);
    fromPtr(thisObj)->setMDIPosition(pos);
}
// setMDISize(KDDockWidgets::Size size)
void c_KDDockWidgets__Core__DockWidget__setMDISize_Size(void *thisObj, void *size_)
{
    assert(size_);
    auto &size = *reinterpret_cast<KDDockWidgets::Size *>(size_);
    fromPtr(thisObj)->setMDISize(size);
}
// setMDIZ(int z)
void c_KDDockWidgets__Core__DockWidget__setMDIZ_int(void *thisObj, int z)
{
    fromPtr(thisObj)->setMDIZ(z);
}
// setOptions(QFlags<KDDockWidgets::DockWidgetOption> arg__1)
void c_KDDockWidgets__Core__DockWidget__setOptions_DockWidgetOptions(void *thisObj, int arg__1_)
{
    auto arg__1 = static_cast<QFlags<KDDockWidgets::DockWidgetOption>>(arg__1_);
    fromPtr(thisObj)->setOptions(arg__1);
}
// setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DockWidget__setParentView_impl_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    fromWrapperPtr(thisObj)->setParentView_impl_nocallback(parent);
}
// setTitle(const QString & title)
void c_KDDockWidgets__Core__DockWidget__setTitle_QString(void *thisObj, const char *title_)
{
    const auto title = QString::fromUtf8(title_);
    fromPtr(thisObj)->setTitle(title);
    free(( char * )title_);
}
// setUniqueName(const QString & arg__1)
void c_KDDockWidgets__Core__DockWidget__setUniqueName_QString(void *thisObj, const char *arg__1_)
{
    const auto arg__1 = QString::fromUtf8(arg__1_);
    fromPtr(thisObj)->setUniqueName(arg__1);
    free(( char * )arg__1_);
}
// setUserType(int userType)
void c_KDDockWidgets__Core__DockWidget__setUserType_int(void *thisObj, int userType)
{
    fromPtr(thisObj)->setUserType(userType);
}
// show()
void c_KDDockWidgets__Core__DockWidget__show(void *thisObj)
{
    fromPtr(thisObj)->show();
}
// sizeInLayout() const
void *c_KDDockWidgets__Core__DockWidget__sizeInLayout(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { fromPtr(thisObj)->sizeInLayout() };
    return result;
}
// skipsRestore() const
bool c_KDDockWidgets__Core__DockWidget__skipsRestore(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->skipsRestore();
    return result;
}
// startDragging(bool singleTab)
bool c_KDDockWidgets__Core__DockWidget__startDragging_bool(void *thisObj, bool singleTab)
{
    const auto &result = fromPtr(thisObj)->startDragging(singleTab);
    return result;
}
// tabIndex() const
int c_KDDockWidgets__Core__DockWidget__tabIndex(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->tabIndex();
    return result;
}
// title() const
void *c_KDDockWidgets__Core__DockWidget__title(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->title() };
    return result;
}
// titleBar() const
void *c_KDDockWidgets__Core__DockWidget__titleBar(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->titleBar();
    return result;
}
// uniqueName() const
void *c_KDDockWidgets__Core__DockWidget__uniqueName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { fromPtr(thisObj)->uniqueName() };
    return result;
}
// userType() const
int c_KDDockWidgets__Core__DockWidget__userType(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->userType();
    return result;
}
// wasRestored() const
bool c_KDDockWidgets__Core__DockWidget__wasRestored(void *thisObj)
{
    const auto &result = fromPtr(thisObj)->wasRestored();
    return result;
}
void c_KDDockWidgets__Core__DockWidget__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__Core__DockWidget__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 331:
        wrapper->m_setParentView_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsCore::DockWidget_wrapper::Callback_setParentView_impl>(callback);
        break;
    }
}
}
