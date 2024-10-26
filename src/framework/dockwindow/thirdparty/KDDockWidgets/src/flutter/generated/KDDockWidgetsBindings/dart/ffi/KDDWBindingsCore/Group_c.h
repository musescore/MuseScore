/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/Group.h>
#include <core/View.h>
#include <core/DockWidget.h>
#include <KDDockWidgets.h>
#include <FloatingWindow.h>
#include "core/Stack.h"
#include <TabBar.h>
#include <geometry_helpers_p.h>
#include "core/TitleBar.h"
#include <string_p.h>
#include "core/MainWindow.h"
#include <Item_p.h>
#include "core/DropArea.h"
#include <core/Layout.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Group_wrapper : public ::KDDockWidgets::Core::Group
{
public:
    ~Group_wrapper();
    Group_wrapper(KDDockWidgets::Core::View *parent = nullptr);
    KDDockWidgets::Core::TitleBar *actualTitleBar() const;
    void addTab(KDDockWidgets::Core::DockWidget *arg__1, KDDockWidgets::InitialOption arg__2 = {});
    void addTab(KDDockWidgets::Core::FloatingWindow *floatingWindow, KDDockWidgets::InitialOption arg__2 = {});
    void addTab(KDDockWidgets::Core::Group *arg__1, KDDockWidgets::InitialOption arg__2 = {});
    bool allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const;
    bool allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const;
    bool alwaysShowsTabs() const;
    bool anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const;
    bool anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const;
    bool anyNonClosable() const;
    bool anyNonDockable() const;
    bool beingDeletedLater() const;
    KDDockWidgets::Size biggestDockWidgetMaxSize() const;
    bool containsDockWidget(KDDockWidgets::Core::DockWidget *w) const;
    bool containsMouse(KDDockWidgets::Point globalPos) const;
    KDDockWidgets::Core::DockWidget *currentDockWidget() const;
    int currentIndex() const;
    int currentTabIndex() const;
    static int dbg_numFrames();
    KDDockWidgets::Core::FloatingWindow *detachTab(KDDockWidgets::Core::DockWidget *arg__1);
    KDDockWidgets::Core::DockWidget *dockWidgetAt(int index) const;
    int dockWidgetCount() const;
    KDDockWidgets::Size dockWidgetsMinSize() const;
    virtual KDDockWidgets::Rect dragRect() const;
    virtual KDDockWidgets::Rect dragRect_nocallback() const;
    KDDockWidgets::Core::FloatingWindow *floatingWindow() const;
    virtual void focusedWidgetChangedCallback();
    virtual void focusedWidgetChangedCallback_nocallback();
    static KDDockWidgets::Core::Group *fromItem(const KDDockWidgets::Core::Item *arg__1);
    bool hasNestedMDIDockWidgets() const;
    bool hasSingleDockWidget() const;
    bool hasTabsVisible() const;
    int indexOfDockWidget(const KDDockWidgets::Core::DockWidget *arg__1);
    void insertDockWidget(KDDockWidgets::Core::DockWidget *arg__1, int index);
    void insertWidget(KDDockWidgets::Core::DockWidget *arg__1, int index, KDDockWidgets::InitialOption arg__3 = {});
    bool isCentralGroup() const;
    bool isDockable() const;
    bool isEmpty() const;
    bool isFloating() const;
    virtual void isFocusedChangedCallback();
    virtual void isFocusedChangedCallback_nocallback();
    bool isInFloatingWindow() const;
    bool isInMainWindow() const;
    bool isMDI() const;
    bool isMDIWrapper() const;
    bool isOverlayed() const;
    bool isTheOnlyGroup() const;
    KDDockWidgets::Core::Item *layoutItem() const;
    KDDockWidgets::Core::MainWindow *mainWindow() const;
    KDDockWidgets::Core::DockWidget *mdiDockWidgetWrapper() const;
    KDDockWidgets::Core::DropArea *mdiDropAreaWrapper() const;
    KDDockWidgets::Core::Group *mdiFrame() const;
    int nonContentsHeight() const;
    void onDockWidgetCountChanged();
    void onDockWidgetTitleChanged(KDDockWidgets::Core::DockWidget *arg__1);
    void removeWidget(KDDockWidgets::Core::DockWidget *arg__1);
    void renameTab(int index, const QString &arg__2);
    void restoreToPreviousPosition();
    void setCurrentDockWidget(KDDockWidgets::Core::DockWidget *arg__1);
    void setCurrentTabIndex(int index);
    void setLayout(KDDockWidgets::Core::Layout *arg__1);
    void setLayoutItem(KDDockWidgets::Core::Item *item);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    KDDockWidgets::Core::Stack *stack() const;
    KDDockWidgets::Core::TabBar *tabBar() const;
    QString title() const;
    KDDockWidgets::Core::TitleBar *titleBar() const;
    void unoverlay();
    void updateFloatingActions();
    void updateTitleAndIcon();
    void updateTitleBarVisibility();
    int userType() const;
    typedef KDDockWidgets::Rect *(*Callback_dragRect)(void *);
    Callback_dragRect m_dragRectCallback = nullptr;
    typedef void (*Callback_focusedWidgetChangedCallback)(void *);
    Callback_focusedWidgetChangedCallback m_focusedWidgetChangedCallbackCallback = nullptr;
    typedef void (*Callback_isFocusedChangedCallback)(void *);
    Callback_isFocusedChangedCallback m_isFocusedChangedCallbackCallback = nullptr;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::Group::Group(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__constructor_View(void *parent_);
// KDDockWidgets::Core::Group::actualTitleBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__actualTitleBar(void *thisObj);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::DockWidget * arg__1, KDDockWidgets::InitialOption arg__2)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__addTab_DockWidget_InitialOption(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::FloatingWindow * floatingWindow, KDDockWidgets::InitialOption arg__2)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__addTab_FloatingWindow_InitialOption(void *thisObj, void *floatingWindow_, void *arg__2_);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::InitialOption arg__2)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__addTab_Group_InitialOption(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Group::allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__allDockWidgetsHave_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__allDockWidgetsHave_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::alwaysShowsTabs() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__alwaysShowsTabs(void *thisObj);
// KDDockWidgets::Core::Group::anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__anyDockWidgetsHas_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__anyDockWidgetsHas_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::anyNonClosable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__anyNonClosable(void *thisObj);
// KDDockWidgets::Core::Group::anyNonDockable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__anyNonDockable(void *thisObj);
// KDDockWidgets::Core::Group::beingDeletedLater() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__beingDeletedLater(void *thisObj);
// KDDockWidgets::Core::Group::biggestDockWidgetMaxSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__biggestDockWidgetMaxSize(void *thisObj);
// KDDockWidgets::Core::Group::containsDockWidget(KDDockWidgets::Core::DockWidget * w) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__containsDockWidget_DockWidget(void *thisObj, void *w_);
// KDDockWidgets::Core::Group::containsMouse(KDDockWidgets::Point globalPos) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__containsMouse_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::Group::currentDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__currentDockWidget(void *thisObj);
// KDDockWidgets::Core::Group::currentIndex() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__currentIndex(void *thisObj);
// KDDockWidgets::Core::Group::currentTabIndex() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__currentTabIndex(void *thisObj);
// KDDockWidgets::Core::Group::dbg_numFrames()
DOCKS_EXPORT int c_static_KDDockWidgets__Core__Group__dbg_numFrames();
// KDDockWidgets::Core::Group::detachTab(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__detachTab_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::dockWidgetAt(int index) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__dockWidgetAt_int(void *thisObj, int index);
// KDDockWidgets::Core::Group::dockWidgetCount() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__dockWidgetCount(void *thisObj);
// KDDockWidgets::Core::Group::dockWidgetsMinSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__dockWidgetsMinSize(void *thisObj);
// KDDockWidgets::Core::Group::dragRect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__dragRect(void *thisObj);
// KDDockWidgets::Core::Group::floatingWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__floatingWindow(void *thisObj);
// KDDockWidgets::Core::Group::focusedWidgetChangedCallback()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__focusedWidgetChangedCallback(void *thisObj);
// KDDockWidgets::Core::Group::fromItem(const KDDockWidgets::Core::Item * arg__1)
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__Group__fromItem_Item(void *arg__1_);
// KDDockWidgets::Core::Group::hasNestedMDIDockWidgets() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__hasNestedMDIDockWidgets(void *thisObj);
// KDDockWidgets::Core::Group::hasSingleDockWidget() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::Group::hasTabsVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__hasTabsVisible(void *thisObj);
// KDDockWidgets::Core::Group::indexOfDockWidget(const KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__indexOfDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::insertDockWidget(KDDockWidgets::Core::DockWidget * arg__1, int index)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__insertDockWidget_DockWidget_int(void *thisObj, void *arg__1_, int index);
// KDDockWidgets::Core::Group::insertWidget(KDDockWidgets::Core::DockWidget * arg__1, int index, KDDockWidgets::InitialOption arg__3)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__insertWidget_DockWidget_int_InitialOption(void *thisObj, void *arg__1_, int index, void *arg__3_);
// KDDockWidgets::Core::Group::isCentralGroup() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isCentralGroup(void *thisObj);
// KDDockWidgets::Core::Group::isDockable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isDockable(void *thisObj);
// KDDockWidgets::Core::Group::isEmpty() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isEmpty(void *thisObj);
// KDDockWidgets::Core::Group::isFloating() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isFloating(void *thisObj);
// KDDockWidgets::Core::Group::isFocusedChangedCallback()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__isFocusedChangedCallback(void *thisObj);
// KDDockWidgets::Core::Group::isInFloatingWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isInFloatingWindow(void *thisObj);
// KDDockWidgets::Core::Group::isInMainWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isInMainWindow(void *thisObj);
// KDDockWidgets::Core::Group::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isMDI(void *thisObj);
// KDDockWidgets::Core::Group::isMDIWrapper() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isMDIWrapper(void *thisObj);
// KDDockWidgets::Core::Group::isOverlayed() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isOverlayed(void *thisObj);
// KDDockWidgets::Core::Group::isTheOnlyGroup() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Group__isTheOnlyGroup(void *thisObj);
// KDDockWidgets::Core::Group::layoutItem() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__layoutItem(void *thisObj);
// KDDockWidgets::Core::Group::mainWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__mainWindow(void *thisObj);
// KDDockWidgets::Core::Group::mdiDockWidgetWrapper() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__mdiDockWidgetWrapper(void *thisObj);
// KDDockWidgets::Core::Group::mdiDropAreaWrapper() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__mdiDropAreaWrapper(void *thisObj);
// KDDockWidgets::Core::Group::mdiFrame() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__mdiFrame(void *thisObj);
// KDDockWidgets::Core::Group::nonContentsHeight() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__nonContentsHeight(void *thisObj);
// KDDockWidgets::Core::Group::onDockWidgetCountChanged()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__onDockWidgetCountChanged(void *thisObj);
// KDDockWidgets::Core::Group::onDockWidgetTitleChanged(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__onDockWidgetTitleChanged_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::removeWidget(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__removeWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::renameTab(int index, const QString & arg__2)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__renameTab_int_QString(void *thisObj, int index, const char *arg__2_);
// KDDockWidgets::Core::Group::restoreToPreviousPosition()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__restoreToPreviousPosition(void *thisObj);
// KDDockWidgets::Core::Group::setCurrentDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__setCurrentDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::setCurrentTabIndex(int index)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__setCurrentTabIndex_int(void *thisObj, int index);
// KDDockWidgets::Core::Group::setLayout(KDDockWidgets::Core::Layout * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__setLayout_Layout(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::setLayoutItem(KDDockWidgets::Core::Item * item)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__setLayoutItem_Item(void *thisObj, void *item_);
// KDDockWidgets::Core::Group::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Group::stack() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__stack(void *thisObj);
// KDDockWidgets::Core::Group::tabBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__tabBar(void *thisObj);
// KDDockWidgets::Core::Group::title() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__title(void *thisObj);
// KDDockWidgets::Core::Group::titleBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Group__titleBar(void *thisObj);
// KDDockWidgets::Core::Group::unoverlay()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__unoverlay(void *thisObj);
// KDDockWidgets::Core::Group::updateFloatingActions()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__updateFloatingActions(void *thisObj);
// KDDockWidgets::Core::Group::updateTitleAndIcon()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__updateTitleAndIcon(void *thisObj);
// KDDockWidgets::Core::Group::updateTitleBarVisibility()
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__updateTitleBarVisibility(void *thisObj);
// KDDockWidgets::Core::Group::userType() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Group__userType(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Group__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Group_Finalizer(void *cppObj);
}
