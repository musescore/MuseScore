/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <core/Platform.h>
#include <ViewFactory.h>
#include <core/View.h>
#include <QtCompat_p.h>
#include <geometry_helpers_p.h>
#include <core/Controller.h>
#include <DelayedCall_p.h>
#include <string_p.h>
#include <FloatingWindow.h>
#include "core/MainWindow.h"
#include <vector>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsCore {
class Platform_wrapper : public ::KDDockWidgets::Core::Platform
{
public:
    ~Platform_wrapper();
    Platform_wrapper();
    virtual QString applicationName() const;
    virtual QString applicationName_nocallback() const;
    virtual KDDockWidgets::Core::ViewFactory *createDefaultViewFactory();
    virtual KDDockWidgets::Core::ViewFactory *createDefaultViewFactory_nocallback();
    virtual KDDockWidgets::Core::MainWindow *createMainWindow(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions arg__2, QFlags<KDDockWidgets::MainWindowOption> options = KDDockWidgets::MainWindowOption::MainWindowOption_HasCentralFrame, KDDockWidgets::Core::View *parent = nullptr, Qt::WindowFlags arg__5 = {}) const;
    virtual KDDockWidgets::Core::MainWindow *createMainWindow_nocallback(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions arg__2, QFlags<KDDockWidgets::MainWindowOption> options = KDDockWidgets::MainWindowOption::MainWindowOption_HasCentralFrame, KDDockWidgets::Core::View *parent = nullptr, Qt::WindowFlags arg__5 = {}) const;
    virtual KDDockWidgets::Core::View *createView(KDDockWidgets::Core::Controller *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createView_nocallback(KDDockWidgets::Core::Controller *arg__1, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Point cursorPos() const;
    virtual KDDockWidgets::Point cursorPos_nocallback() const;
    virtual void dumpManagedBacktrace();
    virtual void dumpManagedBacktrace_nocallback();
    virtual bool hasActivePopup() const;
    virtual bool hasActivePopup_nocallback() const;
    virtual bool inDisallowedDragView(KDDockWidgets::Point globalPos) const;
    virtual bool inDisallowedDragView_nocallback(KDDockWidgets::Point globalPos) const;
    virtual void installMessageHandler();
    virtual void installMessageHandler_nocallback();
    static KDDockWidgets::Core::Platform *instance();
    static bool isInitialized();
    virtual bool isLeftMouseButtonPressed() const;
    virtual bool isLeftMouseButtonPressed_nocallback() const;
    virtual bool isProcessingAppQuitEvent() const;
    virtual bool isProcessingAppQuitEvent_nocallback() const;
    bool isQt() const;
    bool isQtQuick() const;
    bool isQtWidgets() const;
    virtual const char *name() const;
    virtual const char *name_nocallback() const;
    virtual void onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowCreated_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowDestroyed_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onMainWindowCreated(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowCreated_nocallback(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowDestroyed(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowDestroyed_nocallback(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void pauseForDebugger();
    virtual void pauseForDebugger_nocallback();
    virtual void restoreMouseCursor();
    virtual void restoreMouseCursor_nocallback();
    virtual void runDelayed(int ms, KDDockWidgets::Core::DelayedCall *c);
    virtual void runDelayed_nocallback(int ms, KDDockWidgets::Core::DelayedCall *c);
    virtual int screenNumberForView(KDDockWidgets::Core::View *arg__1) const;
    virtual int screenNumberForView_nocallback(KDDockWidgets::Core::View *arg__1) const;
    virtual KDDockWidgets::Size screenSizeFor(KDDockWidgets::Core::View *arg__1) const;
    virtual KDDockWidgets::Size screenSizeFor_nocallback(KDDockWidgets::Core::View *arg__1) const;
    virtual void sendEvent(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Event *arg__2) const;
    virtual void sendEvent_nocallback(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Event *arg__2) const;
    virtual void setCursorPos(KDDockWidgets::Point arg__1);
    virtual void setCursorPos_nocallback(KDDockWidgets::Point arg__1);
    virtual void setMouseCursor(Qt::CursorShape arg__1, bool discardLast = false);
    virtual void setMouseCursor_nocallback(Qt::CursorShape arg__1, bool discardLast = false);
    int startDragDistance() const;
    virtual int startDragDistance_impl() const;
    virtual int startDragDistance_impl_nocallback() const;
    virtual bool supportsAeroSnap() const;
    virtual bool supportsAeroSnap_nocallback() const;
    virtual KDDockWidgets::Core::View *tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent = nullptr);
    virtual KDDockWidgets::Core::View *tests_createFocusableView_nocallback(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent = nullptr);
    virtual KDDockWidgets::Core::View *tests_createNonClosableView(KDDockWidgets::Core::View *parent = nullptr);
    virtual KDDockWidgets::Core::View *tests_createNonClosableView_nocallback(KDDockWidgets::Core::View *parent = nullptr);
    virtual KDDockWidgets::Core::View *tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent = nullptr);
    virtual KDDockWidgets::Core::View *tests_createView_nocallback(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent = nullptr);
    virtual void tests_deinitPlatform_impl();
    virtual void tests_deinitPlatform_impl_nocallback();
    virtual void tests_initPlatform_impl();
    virtual void tests_initPlatform_impl_nocallback();
    virtual void ungrabMouse();
    virtual void ungrabMouse_nocallback();
    virtual void uninstallMessageHandler();
    virtual void uninstallMessageHandler_nocallback();
    virtual bool usesFallbackMouseGrabber() const;
    virtual bool usesFallbackMouseGrabber_nocallback() const;
    typedef QString *(*Callback_applicationName)(void *);
    Callback_applicationName m_applicationNameCallback = nullptr;
    typedef KDDockWidgets::Core::ViewFactory *(*Callback_createDefaultViewFactory)(void *);
    Callback_createDefaultViewFactory m_createDefaultViewFactoryCallback = nullptr;
    typedef KDDockWidgets::Core::MainWindow *(*Callback_createMainWindow)(void *, const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions *arg__2, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View *parent, Qt::WindowFlags arg__5);
    Callback_createMainWindow m_createMainWindowCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createView)(void *, KDDockWidgets::Core::Controller *arg__1, KDDockWidgets::Core::View *parent);
    Callback_createView m_createViewCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_cursorPos)(void *);
    Callback_cursorPos m_cursorPosCallback = nullptr;
    typedef void (*Callback_dumpManagedBacktrace)(void *);
    Callback_dumpManagedBacktrace m_dumpManagedBacktraceCallback = nullptr;
    typedef bool (*Callback_hasActivePopup)(void *);
    Callback_hasActivePopup m_hasActivePopupCallback = nullptr;
    typedef bool (*Callback_inDisallowedDragView)(void *, KDDockWidgets::Point *globalPos);
    Callback_inDisallowedDragView m_inDisallowedDragViewCallback = nullptr;
    typedef void (*Callback_installMessageHandler)(void *);
    Callback_installMessageHandler m_installMessageHandlerCallback = nullptr;
    typedef bool (*Callback_isLeftMouseButtonPressed)(void *);
    Callback_isLeftMouseButtonPressed m_isLeftMouseButtonPressedCallback = nullptr;
    typedef bool (*Callback_isProcessingAppQuitEvent)(void *);
    Callback_isProcessingAppQuitEvent m_isProcessingAppQuitEventCallback = nullptr;
    typedef const char *(*Callback_name)(void *);
    Callback_name m_nameCallback = nullptr;
    typedef void (*Callback_onFloatingWindowCreated)(void *, KDDockWidgets::Core::FloatingWindow *arg__1);
    Callback_onFloatingWindowCreated m_onFloatingWindowCreatedCallback = nullptr;
    typedef void (*Callback_onFloatingWindowDestroyed)(void *, KDDockWidgets::Core::FloatingWindow *arg__1);
    Callback_onFloatingWindowDestroyed m_onFloatingWindowDestroyedCallback = nullptr;
    typedef void (*Callback_onMainWindowCreated)(void *, KDDockWidgets::Core::MainWindow *arg__1);
    Callback_onMainWindowCreated m_onMainWindowCreatedCallback = nullptr;
    typedef void (*Callback_onMainWindowDestroyed)(void *, KDDockWidgets::Core::MainWindow *arg__1);
    Callback_onMainWindowDestroyed m_onMainWindowDestroyedCallback = nullptr;
    typedef void (*Callback_pauseForDebugger)(void *);
    Callback_pauseForDebugger m_pauseForDebuggerCallback = nullptr;
    typedef void (*Callback_restoreMouseCursor)(void *);
    Callback_restoreMouseCursor m_restoreMouseCursorCallback = nullptr;
    typedef void (*Callback_runDelayed)(void *, int ms, KDDockWidgets::Core::DelayedCall *c);
    Callback_runDelayed m_runDelayedCallback = nullptr;
    typedef int (*Callback_screenNumberForView)(void *, KDDockWidgets::Core::View *arg__1);
    Callback_screenNumberForView m_screenNumberForViewCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_screenSizeFor)(void *, KDDockWidgets::Core::View *arg__1);
    Callback_screenSizeFor m_screenSizeForCallback = nullptr;
    typedef void (*Callback_sendEvent)(void *, KDDockWidgets::Core::View *arg__1, KDDockWidgets::Event *arg__2);
    Callback_sendEvent m_sendEventCallback = nullptr;
    typedef void (*Callback_setCursorPos)(void *, KDDockWidgets::Point *arg__1);
    Callback_setCursorPos m_setCursorPosCallback = nullptr;
    typedef void (*Callback_setMouseCursor)(void *, Qt::CursorShape arg__1, bool discardLast);
    Callback_setMouseCursor m_setMouseCursorCallback = nullptr;
    typedef int (*Callback_startDragDistance_impl)(void *);
    Callback_startDragDistance_impl m_startDragDistance_implCallback = nullptr;
    typedef bool (*Callback_supportsAeroSnap)(void *);
    Callback_supportsAeroSnap m_supportsAeroSnapCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_tests_createFocusableView)(void *, KDDockWidgets::Core::CreateViewOptions *arg__1, KDDockWidgets::Core::View *parent);
    Callback_tests_createFocusableView m_tests_createFocusableViewCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_tests_createNonClosableView)(void *, KDDockWidgets::Core::View *parent);
    Callback_tests_createNonClosableView m_tests_createNonClosableViewCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_tests_createView)(void *, KDDockWidgets::Core::CreateViewOptions *arg__1, KDDockWidgets::Core::View *parent);
    Callback_tests_createView m_tests_createViewCallback = nullptr;
    typedef void (*Callback_tests_deinitPlatform_impl)(void *);
    Callback_tests_deinitPlatform_impl m_tests_deinitPlatform_implCallback = nullptr;
    typedef void (*Callback_tests_initPlatform_impl)(void *);
    Callback_tests_initPlatform_impl m_tests_initPlatform_implCallback = nullptr;
    typedef void (*Callback_ungrabMouse)(void *);
    Callback_ungrabMouse m_ungrabMouseCallback = nullptr;
    typedef void (*Callback_uninstallMessageHandler)(void *);
    Callback_uninstallMessageHandler m_uninstallMessageHandlerCallback = nullptr;
    typedef bool (*Callback_usesFallbackMouseGrabber)(void *);
    Callback_usesFallbackMouseGrabber m_usesFallbackMouseGrabberCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::Core::Platform::Platform()
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__constructor();
// KDDockWidgets::Core::Platform::applicationName() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__applicationName(void *thisObj);
// KDDockWidgets::Core::Platform::createDefaultViewFactory()
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__createDefaultViewFactory(void *thisObj);
// KDDockWidgets::Core::Platform::createMainWindow(const QString & uniqueName, KDDockWidgets::Core::CreateViewOptions arg__2, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View * parent, Qt::WindowFlags arg__5) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__createMainWindow_QString_CreateViewOptions_MainWindowOptions_View_WindowFlags(void *thisObj, const char *uniqueName_, void *arg__2_, int options_, void *parent_, int arg__5);
// KDDockWidgets::Core::Platform::createView(KDDockWidgets::Core::Controller * arg__1, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__createView_Controller_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::cursorPos() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__cursorPos(void *thisObj);
// KDDockWidgets::Core::Platform::dumpManagedBacktrace()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__dumpManagedBacktrace(void *thisObj);
// KDDockWidgets::Core::Platform::hasActivePopup() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__hasActivePopup(void *thisObj);
// KDDockWidgets::Core::Platform::inDisallowedDragView(KDDockWidgets::Point globalPos) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__inDisallowedDragView_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::Platform::installMessageHandler()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__installMessageHandler(void *thisObj);
// KDDockWidgets::Core::Platform::instance()
DOCKS_EXPORT void *c_static_KDDockWidgets__Core__Platform__instance();
// KDDockWidgets::Core::Platform::isInitialized()
DOCKS_EXPORT bool c_static_KDDockWidgets__Core__Platform__isInitialized();
// KDDockWidgets::Core::Platform::isLeftMouseButtonPressed() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__isLeftMouseButtonPressed(void *thisObj);
// KDDockWidgets::Core::Platform::isProcessingAppQuitEvent() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__isProcessingAppQuitEvent(void *thisObj);
// KDDockWidgets::Core::Platform::isQt() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__isQt(void *thisObj);
// KDDockWidgets::Core::Platform::isQtQuick() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__isQtQuick(void *thisObj);
// KDDockWidgets::Core::Platform::isQtWidgets() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__isQtWidgets(void *thisObj);
// KDDockWidgets::Core::Platform::name() const
DOCKS_EXPORT const char *c_KDDockWidgets__Core__Platform__name(void *thisObj);
// KDDockWidgets::Core::Platform::onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__onFloatingWindowCreated_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__onFloatingWindowDestroyed_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onMainWindowCreated(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__onMainWindowCreated_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onMainWindowDestroyed(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__onMainWindowDestroyed_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::pauseForDebugger()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__pauseForDebugger(void *thisObj);
// KDDockWidgets::Core::Platform::restoreMouseCursor()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__restoreMouseCursor(void *thisObj);
// KDDockWidgets::Core::Platform::runDelayed(int ms, KDDockWidgets::Core::DelayedCall * c)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__runDelayed_int_DelayedCall(void *thisObj, int ms, void *c_);
// KDDockWidgets::Core::Platform::screenNumberForView(KDDockWidgets::Core::View * arg__1) const
DOCKS_EXPORT int c_KDDockWidgets__Core__Platform__screenNumberForView_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::screenSizeFor(KDDockWidgets::Core::View * arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__screenSizeFor_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::sendEvent(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Event * arg__2) const
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__sendEvent_View_Event(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Platform::setCursorPos(KDDockWidgets::Point arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__setCursorPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__setMouseCursor_CursorShape_bool(void *thisObj, int arg__1, bool discardLast);
// KDDockWidgets::Core::Platform::startDragDistance() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Platform__startDragDistance(void *thisObj);
// KDDockWidgets::Core::Platform::startDragDistance_impl() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Platform__startDragDistance_impl(void *thisObj);
// KDDockWidgets::Core::Platform::supportsAeroSnap() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__supportsAeroSnap(void *thisObj);
// KDDockWidgets::Core::Platform::tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__tests_createFocusableView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::tests_createNonClosableView(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__tests_createNonClosableView_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Platform::tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__Core__Platform__tests_createView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::tests_deinitPlatform_impl()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__tests_deinitPlatform_impl(void *thisObj);
// KDDockWidgets::Core::Platform::tests_initPlatform_impl()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__tests_initPlatform_impl(void *thisObj);
// KDDockWidgets::Core::Platform::ungrabMouse()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__ungrabMouse(void *thisObj);
// KDDockWidgets::Core::Platform::uninstallMessageHandler()
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__uninstallMessageHandler(void *thisObj);
// KDDockWidgets::Core::Platform::usesFallbackMouseGrabber() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Platform__usesFallbackMouseGrabber(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Platform_Finalizer(void *cppObj);
}
