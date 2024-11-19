/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <TabBar.h>
#include "core/Stack.h"
#include <core/DockWidget.h>
#include <geometry_helpers_p.h>
#include <core/Group.h>
#include <string_p.h>
#include <core/View.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class TabBar_wrapper : public ::KDDockWidgets::Core::TabBar
{
public:
    ~TabBar_wrapper();
    TabBar_wrapper(KDDockWidgets::Core::Stack *tabWidget = nullptr);
    KDDockWidgets::Core::DockWidget *currentDockWidget() const;
    int currentIndex() const;
    KDDockWidgets::Core::DockWidget *dockWidgetAt(KDDockWidgets::Point localPos) const;
    KDDockWidgets::Core::DockWidget *dockWidgetAt(int index) const;
    virtual bool dragCanStart(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const;
    virtual bool dragCanStart_nocallback(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const;
    KDDockWidgets::Core::Group *group() const;
    bool hasSingleDockWidget() const;
    int indexOfDockWidget(const KDDockWidgets::Core::DockWidget *dw) const;
    virtual bool isMDI() const;
    virtual bool isMDI_nocallback() const;
    bool isMovingTab() const;
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
    void moveTabTo(int from, int to);
    int numDockWidgets() const;
    void onMouseDoubleClick(KDDockWidgets::Point localPos);
    void onMousePress(KDDockWidgets::Point localPos);
    KDDockWidgets::Rect rectForTab(int index) const;
    void removeDockWidget(KDDockWidgets::Core::DockWidget *dw);
    void renameTab(int index, const QString &arg__2);
    void setCurrentDockWidget(KDDockWidgets::Core::DockWidget *dw);
    void setCurrentIndex(int index);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    KDDockWidgets::Core::Stack *stack() const;
    bool tabsAreMovable() const;
    QString text(int index) const;
    typedef bool (*Callback_dragCanStart)(void *, KDDockWidgets::Point *pressPos, KDDockWidgets::Point *pos);
    Callback_dragCanStart m_dragCanStartCallback = nullptr;
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
// KDDockWidgets::Core::TabBar::TabBar(KDDockWidgets::Core::Stack * tabWidget)
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__constructor_Stack(void *tabWidget_);
// KDDockWidgets::Core::TabBar::currentDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__currentDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::currentIndex() const
DOCKS_EXPORT int c_KDDockWidgets__Core__TabBar__currentIndex(void *thisObj);
// KDDockWidgets::Core::TabBar::dockWidgetAt(KDDockWidgets::Point localPos) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::dockWidgetAt(int index) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::dragCanStart(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__dragCanStart_Point_Point(void *thisObj, void *pressPos_, void *pos_);
// KDDockWidgets::Core::TabBar::group() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__group(void *thisObj);
// KDDockWidgets::Core::TabBar::hasSingleDockWidget() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::indexOfDockWidget(const KDDockWidgets::Core::DockWidget * dw) const
DOCKS_EXPORT int c_KDDockWidgets__Core__TabBar__indexOfDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__isMDI(void *thisObj);
// KDDockWidgets::Core::TabBar::isMovingTab() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__isMovingTab(void *thisObj);
// KDDockWidgets::Core::TabBar::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__isWindow(void *thisObj);
// KDDockWidgets::Core::TabBar::moveTabTo(int from, int to)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__moveTabTo_int_int(void *thisObj, int from, int to);
// KDDockWidgets::Core::TabBar::numDockWidgets() const
DOCKS_EXPORT int c_KDDockWidgets__Core__TabBar__numDockWidgets(void *thisObj);
// KDDockWidgets::Core::TabBar::onMouseDoubleClick(KDDockWidgets::Point localPos)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__onMouseDoubleClick_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::onMousePress(KDDockWidgets::Point localPos)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__onMousePress_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::rectForTab(int index) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__rectForTab_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__removeDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::renameTab(int index, const QString & arg__2)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__renameTab_int_QString(void *thisObj, int index, const char *arg__2_);
// KDDockWidgets::Core::TabBar::setCurrentDockWidget(KDDockWidgets::Core::DockWidget * dw)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__setCurrentDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::setCurrentIndex(int index)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__setCurrentIndex_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::TabBar::singleDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::stack() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__stack(void *thisObj);
// KDDockWidgets::Core::TabBar::tabsAreMovable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TabBar__tabsAreMovable(void *thisObj);
// KDDockWidgets::Core::TabBar::text(int index) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TabBar__text_int(void *thisObj, int index);
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__TabBar_Finalizer(void *cppObj);
}
