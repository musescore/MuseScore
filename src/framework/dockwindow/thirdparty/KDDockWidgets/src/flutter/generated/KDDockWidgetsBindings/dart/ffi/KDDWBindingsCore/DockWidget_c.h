/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/DockWidget.h>
#include <core/View.h>
#include <string_p.h>
#include <KDDockWidgets.h>
#include <FloatingWindow.h>
#include <geometry_helpers_p.h>
#include "core/TitleBar.h"
#include "core/MainWindow.h"

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class DockWidget_wrapper : public ::KDDockWidgets::Core::DockWidget
{
public:
    ~DockWidget_wrapper();
    DockWidget_wrapper(KDDockWidgets::Core::View *view, const QString &uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options = {}, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions = {});
    void addDockWidgetAsTab(KDDockWidgets::Core::DockWidget *other, KDDockWidgets::InitialOption initialOption = {});
    void addDockWidgetToContainingWindow(KDDockWidgets::Core::DockWidget *other, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget *relativeTo = nullptr, KDDockWidgets::InitialOption initialOption = {});
    static KDDockWidgets::Core::DockWidget *byName(const QString &uniqueName);
    int currentTabIndex() const;
    KDDockWidgets::Core::FloatingWindow *floatingWindow() const;
    void forceClose();
    KDDockWidgets::Rect groupGeometry() const;
    bool hasPreviousDockedLocation() const;
    void init();
    bool isCurrentTab() const;
    bool isFloating() const;
    bool isFocused() const;
    bool isInMainWindow() const;
    bool isInSideBar() const;
    bool isMainWindow() const;
    bool isOpen() const;
    bool isOverlayed() const;
    bool isPersistentCentralDockWidget() const;
    bool isTabbed() const;
    KDDockWidgets::Size lastOverlayedSize() const;
    QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions() const;
    KDDockWidgets::Core::MainWindow *mainWindow() const;
    int mdiZ() const;
    void moveToSideBar();
    void onResize(KDDockWidgets::Size newSize);
    void open();
    QFlags<KDDockWidgets::DockWidgetOption> options() const;
    void raise();
    void removeFromSideBar();
    void resizeInLayout(int left, int top, int right, int bottom);
    void setAffinityName(const QString &name);
    void setAsCurrentTab();
    bool setFloating(bool floats);
    void setFloatingGeometry(KDDockWidgets::Rect geo);
    void setMDIPosition(KDDockWidgets::Point pos);
    void setMDISize(KDDockWidgets::Size size);
    void setMDIZ(int z);
    void setOptions(QFlags<KDDockWidgets::DockWidgetOption> arg__1);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setTitle(const QString &title);
    void setUniqueName(const QString &arg__1);
    void setUserType(int userType);
    void show();
    KDDockWidgets::Size sizeInLayout() const;
    bool skipsRestore() const;
    bool startDragging(bool singleTab = false);
    int tabIndex() const;
    QString title() const;
    KDDockWidgets::Core::TitleBar *titleBar() const;
    QString uniqueName() const;
    int userType() const;
    bool wasRestored() const;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::DockWidget::DockWidget(KDDockWidgets::Core::View * view, const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions)
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__constructor_View_QString_DockWidgetOptions_LayoutSaverOptions(void *view_, const char *uniqueName_, int options_, int layoutSaverOptions_);
// KDDockWidgets::Core::DockWidget::addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::InitialOption initialOption)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__addDockWidgetAsTab_DockWidget_InitialOption(void *thisObj, void *other_, void *initialOption_);
// KDDockWidgets::Core::DockWidget::addDockWidgetToContainingWindow(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__addDockWidgetToContainingWindow_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *other_, int location, void *relativeTo_, void *initialOption_);
// KDDockWidgets::Core::DockWidget::byName(const QString & uniqueName)
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__DockWidget__byName_QString(const char *uniqueName_);
// KDDockWidgets::Core::DockWidget::currentTabIndex() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__currentTabIndex(void *thisObj);
// KDDockWidgets::Core::DockWidget::floatingWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__floatingWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::forceClose()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__forceClose(void *thisObj);
// KDDockWidgets::Core::DockWidget::groupGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__groupGeometry(void *thisObj);
// KDDockWidgets::Core::DockWidget::hasPreviousDockedLocation() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__hasPreviousDockedLocation(void *thisObj);
// KDDockWidgets::Core::DockWidget::init()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__init(void *thisObj);
// KDDockWidgets::Core::DockWidget::isCurrentTab() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isCurrentTab(void *thisObj);
// KDDockWidgets::Core::DockWidget::isFloating() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isFloating(void *thisObj);
// KDDockWidgets::Core::DockWidget::isFocused() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isFocused(void *thisObj);
// KDDockWidgets::Core::DockWidget::isInMainWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isInMainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::isInSideBar() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isInSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::isMainWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isMainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::isOpen() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isOpen(void *thisObj);
// KDDockWidgets::Core::DockWidget::isOverlayed() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isOverlayed(void *thisObj);
// KDDockWidgets::Core::DockWidget::isPersistentCentralDockWidget() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isPersistentCentralDockWidget(void *thisObj);
// KDDockWidgets::Core::DockWidget::isTabbed() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__isTabbed(void *thisObj);
// KDDockWidgets::Core::DockWidget::lastOverlayedSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__lastOverlayedSize(void *thisObj);
// KDDockWidgets::Core::DockWidget::layoutSaverOptions() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__layoutSaverOptions(void *thisObj);
// KDDockWidgets::Core::DockWidget::mainWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__mainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::mdiZ() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__mdiZ(void *thisObj);
// KDDockWidgets::Core::DockWidget::moveToSideBar()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__moveToSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::onResize(KDDockWidgets::Size newSize)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__onResize_Size(void *thisObj, void *newSize_);
// KDDockWidgets::Core::DockWidget::open()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__open(void *thisObj);
// KDDockWidgets::Core::DockWidget::options() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__options(void *thisObj);
// KDDockWidgets::Core::DockWidget::raise()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__raise(void *thisObj);
// KDDockWidgets::Core::DockWidget::removeFromSideBar()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__removeFromSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::resizeInLayout(int left, int top, int right, int bottom)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__resizeInLayout_int_int_int_int(void *thisObj, int left, int top, int right, int bottom);
// KDDockWidgets::Core::DockWidget::setAffinityName(const QString & name)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setAffinityName_QString(void *thisObj, const char *name_);
// KDDockWidgets::Core::DockWidget::setAsCurrentTab()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setAsCurrentTab(void *thisObj);
// KDDockWidgets::Core::DockWidget::setFloating(bool floats)
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__setFloating_bool(void *thisObj, bool floats);
// KDDockWidgets::Core::DockWidget::setFloatingGeometry(KDDockWidgets::Rect geo)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setFloatingGeometry_Rect(void *thisObj, void *geo_);
// KDDockWidgets::Core::DockWidget::setMDIPosition(KDDockWidgets::Point pos)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setMDIPosition_Point(void *thisObj, void *pos_);
// KDDockWidgets::Core::DockWidget::setMDISize(KDDockWidgets::Size size)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setMDISize_Size(void *thisObj, void *size_);
// KDDockWidgets::Core::DockWidget::setMDIZ(int z)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setMDIZ_int(void *thisObj, int z);
// KDDockWidgets::Core::DockWidget::setOptions(QFlags<KDDockWidgets::DockWidgetOption> arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setOptions_DockWidgetOptions(void *thisObj, int arg__1_);
// KDDockWidgets::Core::DockWidget::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::DockWidget::setTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::DockWidget::setUniqueName(const QString & arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setUniqueName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::DockWidget::setUserType(int userType)
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__setUserType_int(void *thisObj, int userType);
// KDDockWidgets::Core::DockWidget::show()
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__show(void *thisObj);
// KDDockWidgets::Core::DockWidget::sizeInLayout() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__sizeInLayout(void *thisObj);
// KDDockWidgets::Core::DockWidget::skipsRestore() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__skipsRestore(void *thisObj);
// KDDockWidgets::Core::DockWidget::startDragging(bool singleTab)
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__startDragging_bool(void *thisObj, bool singleTab);
// KDDockWidgets::Core::DockWidget::tabIndex() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__tabIndex(void *thisObj);
// KDDockWidgets::Core::DockWidget::title() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__title(void *thisObj);
// KDDockWidgets::Core::DockWidget::titleBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__titleBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::uniqueName() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__DockWidget__uniqueName(void *thisObj);
// KDDockWidgets::Core::DockWidget::userType() const
DOCKS_EXPORT int c_KDDockWidgets__Core__DockWidget__userType(void *thisObj);
// KDDockWidgets::Core::DockWidget::wasRestored() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__DockWidget__wasRestored(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__DockWidget_Finalizer(void *cppObj);
}
