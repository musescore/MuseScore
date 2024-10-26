/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <FloatingWindow.h>
#include <geometry_helpers_p.h>
#include "core/MainWindow.h"
#include <core/Group.h>
#include <core/DockWidget.h>
#include "core/DropArea.h"
#include "core/TitleBar.h"
#include <core/Layout.h>
#include <KDDockWidgets.h>
#include <core/View.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class FloatingWindow_wrapper : public ::KDDockWidgets::Core::FloatingWindow
{
public:
    ~FloatingWindow_wrapper();
    FloatingWindow_wrapper(KDDockWidgets::Core::Group *group, KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow *parent = nullptr);
    FloatingWindow_wrapper(KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow *parent = nullptr);
    void addDockWidget(KDDockWidgets::Core::DockWidget *arg__1, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget *relativeTo, KDDockWidgets::InitialOption arg__4 = {});
    bool allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const;
    bool allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const;
    bool anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const;
    bool anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const;
    bool anyNonClosable() const;
    bool anyNonDockable() const;
    bool beingDeleted() const;
    KDDockWidgets::Margins contentMargins() const;
    KDDockWidgets::Rect dragRect() const;
    KDDockWidgets::Core::DropArea *dropArea() const;
    static void ensureRectIsOnScreen(KDDockWidgets::Rect &geometry);
    bool hasSingleDockWidget() const;
    bool hasSingleGroup() const;
    bool isInDragArea(KDDockWidgets::Point globalPoint) const;
    virtual bool isMDI() const;
    virtual bool isMDI_nocallback() const;
    bool isUtilityWindow() const;
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
    KDDockWidgets::Core::Layout *layout() const;
    KDDockWidgets::Core::MainWindow *mainWindow() const;
    void maybeCreateResizeHandler();
    KDDockWidgets::Core::DropArea *multiSplitter() const;
    void scheduleDeleteLater();
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setSuggestedGeometry(KDDockWidgets::Rect suggestedRect);
    KDDockWidgets::Core::Group *singleFrame() const;
    bool supportsMaximizeButton() const;
    bool supportsMinimizeButton() const;
    KDDockWidgets::Core::TitleBar *titleBar() const;
    void updateTitleAndIcon();
    void updateTitleBarVisibility();
    int userType() const;
    typedef bool (*Callback_isMDI)(void *);
    Callback_isMDI m_isMDICallback = nullptr;
    typedef bool (*Callback_isWindow)(void *);
    Callback_isWindow m_isWindowCallback = nullptr;
    typedef void (*Callback_setParentView_impl)(void *, KDDockWidgets::Core::View *parent);
    Callback_setParentView_impl m_setParentView_implCallback = nullptr;
    typedef KDDockWidgets::Core::DockWidget *(*Callback_singleDockWidget)(void *);
    Callback_singleDockWidget m_singleDockWidgetCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::FloatingWindow::FloatingWindow(KDDockWidgets::Core::Group * group, KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__constructor_Group_Rect_MainWindow(void *group_, void *suggestedGeometry_, void *parent_);
// KDDockWidgets::Core::FloatingWindow::FloatingWindow(KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__constructor_Rect_MainWindow(void *suggestedGeometry_, void *parent_);
// KDDockWidgets::Core::FloatingWindow::addDockWidget(KDDockWidgets::Core::DockWidget * arg__1, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption arg__4)
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *arg__1_, int location, void *relativeTo_, void *arg__4_);
// KDDockWidgets::Core::FloatingWindow::allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__allDockWidgetsHave_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__allDockWidgetsHave_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__anyDockWidgetsHas_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__anyDockWidgetsHas_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyNonClosable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__anyNonClosable(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::anyNonDockable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__anyNonDockable(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::beingDeleted() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__beingDeleted(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::contentMargins() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__contentMargins(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::dragRect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__dragRect(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::dropArea() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__dropArea(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::ensureRectIsOnScreen(KDDockWidgets::Rect & geometry)
DOCKS_EXPORT void c_static_KDDockWidgets__Core__FloatingWindow__ensureRectIsOnScreen_Rect(void *geometry_);
// KDDockWidgets::Core::FloatingWindow::hasSingleDockWidget() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::hasSingleGroup() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__hasSingleGroup(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isInDragArea(KDDockWidgets::Point globalPoint) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__isInDragArea_Point(void *thisObj, void *globalPoint_);
// KDDockWidgets::Core::FloatingWindow::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__isMDI(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isUtilityWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__isUtilityWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__isWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::layout() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__layout(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::mainWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__mainWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::maybeCreateResizeHandler()
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__maybeCreateResizeHandler(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::multiSplitter() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__multiSplitter(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::scheduleDeleteLater()
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__scheduleDeleteLater(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::FloatingWindow::setSuggestedGeometry(KDDockWidgets::Rect suggestedRect)
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__setSuggestedGeometry_Rect(void *thisObj, void *suggestedRect_);
// KDDockWidgets::Core::FloatingWindow::singleDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::singleFrame() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__singleFrame(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::supportsMaximizeButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__supportsMaximizeButton(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::supportsMinimizeButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__FloatingWindow__supportsMinimizeButton(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::titleBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__FloatingWindow__titleBar(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::updateTitleAndIcon()
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__updateTitleAndIcon(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::updateTitleBarVisibility()
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__updateTitleBarVisibility(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::userType() const
DOCKS_EXPORT int c_KDDockWidgets__Core__FloatingWindow__userType(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__FloatingWindow_Finalizer(void *cppObj);
}
