/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Platform.h>
#include <ViewFactory.h>
#include <core/View.h>
#include <geometry_helpers_p.h>
#include <core/Controller.h>
#include <FloatingWindow.h>
#include "core/MainWindow.h"
#include <ClassicIndicatorsWindow.h>
#include <DelayedCall_p.h>
#include <core/Platform.h>
#include <string_p.h>
#include <QtCompat_p.h>
#include <vector>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class Platform_wrapper : public ::KDDockWidgets::flutter::Platform
{
public:
    ~Platform_wrapper();
    Platform_wrapper();
    virtual QString applicationName() const;
    virtual QString applicationName_nocallback() const;
    virtual KDDockWidgets::Core::ViewFactory *createDefaultViewFactory();
    virtual KDDockWidgets::Core::ViewFactory *createDefaultViewFactory_nocallback();
    virtual KDDockWidgets::Core::MainWindow *createMainWindow(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options = KDDockWidgets::MainWindowOption::MainWindowOption_HasCentralFrame, KDDockWidgets::Core::View *parent = nullptr, Qt::WindowFlags flags = {}) const;
    virtual KDDockWidgets::Core::MainWindow *createMainWindow_nocallback(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options = KDDockWidgets::MainWindowOption::MainWindowOption_HasCentralFrame, KDDockWidgets::Core::View *parent = nullptr, Qt::WindowFlags flags = {}) const;
    virtual KDDockWidgets::Core::View *createView(KDDockWidgets::Core::Controller *controller, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Core::View *createView_nocallback(KDDockWidgets::Core::Controller *controller, KDDockWidgets::Core::View *parent = nullptr) const;
    virtual KDDockWidgets::Point cursorPos() const;
    virtual KDDockWidgets::Point cursorPos_nocallback() const;
    virtual void dumpManagedBacktrace();
    virtual void dumpManagedBacktrace_nocallback();
    virtual bool hasActivePopup() const;
    virtual bool hasActivePopup_nocallback() const;
    virtual bool inDisallowedDragView(KDDockWidgets::Point globalPos) const;
    virtual bool inDisallowedDragView_nocallback(KDDockWidgets::Point globalPos) const;
    void init();
    virtual void installMessageHandler();
    virtual void installMessageHandler_nocallback();
    virtual bool isLeftMouseButtonPressed() const;
    virtual bool isLeftMouseButtonPressed_nocallback() const;
    virtual bool isProcessingAppQuitEvent() const;
    virtual bool isProcessingAppQuitEvent_nocallback() const;
    virtual const char *name() const;
    virtual const char *name_nocallback() const;
    virtual void onDropIndicatorOverlayCreated(KDDockWidgets::flutter::IndicatorWindow *arg__1);
    virtual void onDropIndicatorOverlayCreated_nocallback(KDDockWidgets::flutter::IndicatorWindow *arg__1);
    virtual void onDropIndicatorOverlayDestroyed(KDDockWidgets::flutter::IndicatorWindow *arg__1);
    virtual void onDropIndicatorOverlayDestroyed_nocallback(KDDockWidgets::flutter::IndicatorWindow *arg__1);
    virtual void onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowCreated_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onFloatingWindowDestroyed_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1);
    virtual void onMainWindowCreated(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowCreated_nocallback(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowDestroyed(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void onMainWindowDestroyed_nocallback(KDDockWidgets::Core::MainWindow *arg__1);
    virtual void pauseForDartDebugger();
    virtual void pauseForDartDebugger_nocallback();
    virtual void pauseForDebugger();
    virtual void pauseForDebugger_nocallback();
    static KDDockWidgets::flutter::Platform *platformFlutter();
    virtual void rebuildWindowOverlay();
    virtual void rebuildWindowOverlay_nocallback();
    virtual void restoreMouseCursor();
    virtual void restoreMouseCursor_nocallback();
    void resumeCoRoutines();
    virtual void runDelayed(int ms, KDDockWidgets::Core::DelayedCall *c);
    virtual void runDelayed_nocallback(int ms, KDDockWidgets::Core::DelayedCall *c);
    void runTests();
    virtual void scheduleResumeCoRoutines(int ms) const;
    virtual void scheduleResumeCoRoutines_nocallback(int ms) const;
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
    typedef KDDockWidgets::Core::MainWindow *(*Callback_createMainWindow)(void *, const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions *viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View *parent, Qt::WindowFlags flags);
    Callback_createMainWindow m_createMainWindowCallback = nullptr;
    typedef KDDockWidgets::Core::View *(*Callback_createView)(void *, KDDockWidgets::Core::Controller *controller, KDDockWidgets::Core::View *parent);
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
    typedef void (*Callback_onDropIndicatorOverlayCreated)(void *, KDDockWidgets::flutter::IndicatorWindow *arg__1);
    Callback_onDropIndicatorOverlayCreated m_onDropIndicatorOverlayCreatedCallback = nullptr;
    typedef void (*Callback_onDropIndicatorOverlayDestroyed)(void *, KDDockWidgets::flutter::IndicatorWindow *arg__1);
    Callback_onDropIndicatorOverlayDestroyed m_onDropIndicatorOverlayDestroyedCallback = nullptr;
    typedef void (*Callback_onFloatingWindowCreated)(void *, KDDockWidgets::Core::FloatingWindow *arg__1);
    Callback_onFloatingWindowCreated m_onFloatingWindowCreatedCallback = nullptr;
    typedef void (*Callback_onFloatingWindowDestroyed)(void *, KDDockWidgets::Core::FloatingWindow *arg__1);
    Callback_onFloatingWindowDestroyed m_onFloatingWindowDestroyedCallback = nullptr;
    typedef void (*Callback_onMainWindowCreated)(void *, KDDockWidgets::Core::MainWindow *arg__1);
    Callback_onMainWindowCreated m_onMainWindowCreatedCallback = nullptr;
    typedef void (*Callback_onMainWindowDestroyed)(void *, KDDockWidgets::Core::MainWindow *arg__1);
    Callback_onMainWindowDestroyed m_onMainWindowDestroyedCallback = nullptr;
    typedef void (*Callback_pauseForDartDebugger)(void *);
    Callback_pauseForDartDebugger m_pauseForDartDebuggerCallback = nullptr;
    typedef void (*Callback_pauseForDebugger)(void *);
    Callback_pauseForDebugger m_pauseForDebuggerCallback = nullptr;
    typedef void (*Callback_rebuildWindowOverlay)(void *);
    Callback_rebuildWindowOverlay m_rebuildWindowOverlayCallback = nullptr;
    typedef void (*Callback_restoreMouseCursor)(void *);
    Callback_restoreMouseCursor m_restoreMouseCursorCallback = nullptr;
    typedef void (*Callback_runDelayed)(void *, int ms, KDDockWidgets::Core::DelayedCall *c);
    Callback_runDelayed m_runDelayedCallback = nullptr;
    typedef void (*Callback_scheduleResumeCoRoutines)(void *, int ms);
    Callback_scheduleResumeCoRoutines m_scheduleResumeCoRoutinesCallback = nullptr;
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
// KDDockWidgets::flutter::Platform::Platform()
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__constructor();
// KDDockWidgets::flutter::Platform::applicationName() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__applicationName(void *thisObj);
// KDDockWidgets::flutter::Platform::createDefaultViewFactory()
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__createDefaultViewFactory(void *thisObj);
// KDDockWidgets::flutter::Platform::createMainWindow(const QString & uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View * parent, Qt::WindowFlags flags) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__createMainWindow_QString_CreateViewOptions_MainWindowOptions_View_WindowFlags(void *thisObj, const char *uniqueName_, void *viewOpts_, int options_, void *parent_, int flags);
// KDDockWidgets::flutter::Platform::createView(KDDockWidgets::Core::Controller * controller, KDDockWidgets::Core::View * parent) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__createView_Controller_View(void *thisObj, void *controller_, void *parent_);
// KDDockWidgets::flutter::Platform::cursorPos() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__cursorPos(void *thisObj);
// KDDockWidgets::flutter::Platform::dumpManagedBacktrace()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__dumpManagedBacktrace(void *thisObj);
// KDDockWidgets::flutter::Platform::hasActivePopup() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__hasActivePopup(void *thisObj);
// KDDockWidgets::flutter::Platform::inDisallowedDragView(KDDockWidgets::Point globalPos) const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__inDisallowedDragView_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::Platform::init()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__init(void *thisObj);
// KDDockWidgets::flutter::Platform::installMessageHandler()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__installMessageHandler(void *thisObj);
// KDDockWidgets::flutter::Platform::isLeftMouseButtonPressed() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__isLeftMouseButtonPressed(void *thisObj);
// KDDockWidgets::flutter::Platform::isProcessingAppQuitEvent() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__isProcessingAppQuitEvent(void *thisObj);
// KDDockWidgets::flutter::Platform::name() const
DOCKS_EXPORT const char *c_KDDockWidgets__flutter__Platform__name(void *thisObj);
// KDDockWidgets::flutter::Platform::onDropIndicatorOverlayCreated(KDDockWidgets::flutter::IndicatorWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayCreated_IndicatorWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onDropIndicatorOverlayDestroyed(KDDockWidgets::flutter::IndicatorWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayDestroyed_IndicatorWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onFloatingWindowCreated_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onFloatingWindowDestroyed_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onMainWindowCreated(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onMainWindowCreated_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onMainWindowDestroyed(KDDockWidgets::Core::MainWindow * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__onMainWindowDestroyed_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::pauseForDartDebugger()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__pauseForDartDebugger(void *thisObj);
// KDDockWidgets::flutter::Platform::pauseForDebugger()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__pauseForDebugger(void *thisObj);
// KDDockWidgets::flutter::Platform::platformFlutter()
DOCKS_EXPORT void *c_static_KDDockWidgets__flutter__Platform__platformFlutter();
// KDDockWidgets::flutter::Platform::rebuildWindowOverlay()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__rebuildWindowOverlay(void *thisObj);
// KDDockWidgets::flutter::Platform::restoreMouseCursor()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__restoreMouseCursor(void *thisObj);
// KDDockWidgets::flutter::Platform::resumeCoRoutines()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__resumeCoRoutines(void *thisObj);
// KDDockWidgets::flutter::Platform::runDelayed(int ms, KDDockWidgets::Core::DelayedCall * c)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__runDelayed_int_DelayedCall(void *thisObj, int ms, void *c_);
// KDDockWidgets::flutter::Platform::runTests()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__runTests(void *thisObj);
// KDDockWidgets::flutter::Platform::scheduleResumeCoRoutines(int ms) const
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__scheduleResumeCoRoutines_int(void *thisObj, int ms);
// KDDockWidgets::flutter::Platform::screenNumberForView(KDDockWidgets::Core::View * arg__1) const
DOCKS_EXPORT int c_KDDockWidgets__flutter__Platform__screenNumberForView_View(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::screenSizeFor(KDDockWidgets::Core::View * arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__screenSizeFor_View(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::sendEvent(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Event * arg__2) const
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__sendEvent_View_Event(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::flutter::Platform::setCursorPos(KDDockWidgets::Point arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__setCursorPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__setMouseCursor_CursorShape_bool(void *thisObj, int arg__1, bool discardLast);
// KDDockWidgets::flutter::Platform::startDragDistance_impl() const
DOCKS_EXPORT int c_KDDockWidgets__flutter__Platform__startDragDistance_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::supportsAeroSnap() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__supportsAeroSnap(void *thisObj);
// KDDockWidgets::flutter::Platform::tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__tests_createFocusableView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::Platform::tests_createNonClosableView(KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__tests_createNonClosableView_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::Platform::tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Platform__tests_createView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::Platform::tests_deinitPlatform_impl()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__tests_deinitPlatform_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::tests_initPlatform_impl()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__tests_initPlatform_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::ungrabMouse()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__ungrabMouse(void *thisObj);
// KDDockWidgets::flutter::Platform::uninstallMessageHandler()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__uninstallMessageHandler(void *thisObj);
// KDDockWidgets::flutter::Platform::usesFallbackMouseGrabber() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Platform__usesFallbackMouseGrabber(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Platform_Finalizer(void *cppObj);
}
