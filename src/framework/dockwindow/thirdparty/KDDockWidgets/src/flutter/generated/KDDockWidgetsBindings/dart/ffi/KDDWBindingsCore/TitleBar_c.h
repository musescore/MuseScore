/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include "core/TitleBar.h"
#include <FloatingWindow.h>
#include <core/Group.h>
#include <core/View.h>
#include <core/DockWidget.h>
#include <string_p.h>
#include "core/MainWindow.h"
#include <TabBar.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class TitleBar_wrapper : public ::KDDockWidgets::Core::TitleBar
{
public:
    ~TitleBar_wrapper();
    TitleBar_wrapper(KDDockWidgets::Core::FloatingWindow *parent);
    TitleBar_wrapper(KDDockWidgets::Core::Group *parent);
    TitleBar_wrapper(KDDockWidgets::Core::View *arg__1);
    bool closeButtonEnabled() const;
    QString floatButtonToolTip() const;
    bool floatButtonVisible() const;
    KDDockWidgets::Core::FloatingWindow *floatingWindow() const;
    KDDockWidgets::Core::Group *group() const;
    bool hasIcon() const;
    bool isCloseButtonEnabled() const;
    bool isCloseButtonVisible() const;
    bool isFloatButtonVisible() const;
    bool isFloating() const;
    bool isFocused() const;
    virtual bool isMDI() const;
    virtual bool isMDI_nocallback() const;
    bool isOverlayed() const;
    bool isStandalone() const;
    virtual bool isWindow() const;
    virtual bool isWindow_nocallback() const;
    KDDockWidgets::Core::MainWindow *mainWindow() const;
    bool maximizeButtonVisible() const;
    void onAutoHideClicked();
    void onCloseClicked();
    bool onDoubleClicked();
    void onFloatClicked();
    void onMaximizeClicked();
    void onMinimizeClicked();
    void setCloseButtonEnabled(bool arg__1);
    void setCloseButtonVisible(bool arg__1);
    void setFloatButtonVisible(bool arg__1);
    virtual void setParentView_impl(KDDockWidgets::Core::View *parent);
    virtual void setParentView_impl_nocallback(KDDockWidgets::Core::View *parent);
    void setTitle(const QString &title);
    bool supportsAutoHideButton() const;
    bool supportsFloatingButton() const;
    bool supportsMaximizeButton() const;
    bool supportsMinimizeButton() const;
    KDDockWidgets::Core::TabBar *tabBar() const;
    QString title() const;
    bool titleBarIsFocusable() const;
    void toggleMaximized();
    void updateButtons();
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
// KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::FloatingWindow * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__constructor_FloatingWindow(void *parent_);
// KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::Group * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__constructor_Group(void *parent_);
// KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::View * arg__1)
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__constructor_View(void *arg__1_);
// KDDockWidgets::Core::TitleBar::closeButtonEnabled() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__closeButtonEnabled(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatButtonToolTip() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__floatButtonToolTip(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__floatButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatingWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__floatingWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::group() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__group(void *thisObj);
// KDDockWidgets::Core::TitleBar::hasIcon() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__hasIcon(void *thisObj);
// KDDockWidgets::Core::TitleBar::isCloseButtonEnabled() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isCloseButtonEnabled(void *thisObj);
// KDDockWidgets::Core::TitleBar::isCloseButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isCloseButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFloatButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isFloatButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFloating() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isFloating(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFocused() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isFocused(void *thisObj);
// KDDockWidgets::Core::TitleBar::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isMDI(void *thisObj);
// KDDockWidgets::Core::TitleBar::isOverlayed() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isOverlayed(void *thisObj);
// KDDockWidgets::Core::TitleBar::isStandalone() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isStandalone(void *thisObj);
// KDDockWidgets::Core::TitleBar::isWindow() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__isWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::mainWindow() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__mainWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::maximizeButtonVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__maximizeButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::onAutoHideClicked()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__onAutoHideClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onCloseClicked()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__onCloseClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onDoubleClicked()
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__onDoubleClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onFloatClicked()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__onFloatClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onMaximizeClicked()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__onMaximizeClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onMinimizeClicked()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__onMinimizeClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::setCloseButtonEnabled(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__setCloseButtonEnabled_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setCloseButtonVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__setCloseButtonVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setFloatButtonVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__setFloatButtonVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setParentView_impl(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::TitleBar::setTitle(const QString & title)
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__setTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::TitleBar::singleDockWidget() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsAutoHideButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__supportsAutoHideButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsFloatingButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__supportsFloatingButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsMaximizeButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__supportsMaximizeButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsMinimizeButton() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__supportsMinimizeButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::tabBar() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__tabBar(void *thisObj);
// KDDockWidgets::Core::TitleBar::title() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__TitleBar__title(void *thisObj);
// KDDockWidgets::Core::TitleBar::titleBarIsFocusable() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__TitleBar__titleBarIsFocusable(void *thisObj);
// KDDockWidgets::Core::TitleBar::toggleMaximized()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__toggleMaximized(void *thisObj);
// KDDockWidgets::Core::TitleBar::updateButtons()
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__updateButtons(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__TitleBar_Finalizer(void *cppObj);
}
