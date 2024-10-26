/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "Platform_c.h"


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
namespace KDDWBindingsFlutter {
Platform_wrapper::Platform_wrapper()
    : ::KDDockWidgets::flutter::Platform()
{
}
QString Platform_wrapper::applicationName() const
{
    if (m_applicationNameCallback) {
        const void *thisPtr = this;
        return *m_applicationNameCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::applicationName();
    }
}
QString Platform_wrapper::applicationName_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::applicationName();
}
KDDockWidgets::Core::ViewFactory *Platform_wrapper::createDefaultViewFactory()
{
    if (m_createDefaultViewFactoryCallback) {
        const void *thisPtr = this;
        return m_createDefaultViewFactoryCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::createDefaultViewFactory();
    }
}
KDDockWidgets::Core::ViewFactory *Platform_wrapper::createDefaultViewFactory_nocallback()
{
    return ::KDDockWidgets::flutter::Platform::createDefaultViewFactory();
}
KDDockWidgets::Core::MainWindow *Platform_wrapper::createMainWindow(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View *parent, Qt::WindowFlags flags) const
{
    if (m_createMainWindowCallback) {
        const void *thisPtr = this;
        return m_createMainWindowCallback(const_cast<void *>(thisPtr), uniqueName, &viewOpts, options, parent, flags);
    } else {
        return ::KDDockWidgets::flutter::Platform::createMainWindow(uniqueName, viewOpts, options, parent, flags);
    }
}
KDDockWidgets::Core::MainWindow *Platform_wrapper::createMainWindow_nocallback(const QString &uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View *parent, Qt::WindowFlags flags) const
{
    return ::KDDockWidgets::flutter::Platform::createMainWindow(uniqueName, viewOpts, options, parent, flags);
}
KDDockWidgets::Core::View *Platform_wrapper::createView(KDDockWidgets::Core::Controller *controller, KDDockWidgets::Core::View *parent) const
{
    if (m_createViewCallback) {
        const void *thisPtr = this;
        return m_createViewCallback(const_cast<void *>(thisPtr), controller, parent);
    } else {
        return ::KDDockWidgets::flutter::Platform::createView(controller, parent);
    }
}
KDDockWidgets::Core::View *Platform_wrapper::createView_nocallback(KDDockWidgets::Core::Controller *controller, KDDockWidgets::Core::View *parent) const
{
    return ::KDDockWidgets::flutter::Platform::createView(controller, parent);
}
KDDockWidgets::Point Platform_wrapper::cursorPos() const
{
    if (m_cursorPosCallback) {
        const void *thisPtr = this;
        return *m_cursorPosCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::cursorPos();
    }
}
KDDockWidgets::Point Platform_wrapper::cursorPos_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::cursorPos();
}
void Platform_wrapper::dumpManagedBacktrace()
{
    if (m_dumpManagedBacktraceCallback) {
        const void *thisPtr = this;
        m_dumpManagedBacktraceCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::dumpManagedBacktrace();
    }
}
void Platform_wrapper::dumpManagedBacktrace_nocallback()
{
    ::KDDockWidgets::flutter::Platform::dumpManagedBacktrace();
}
bool Platform_wrapper::hasActivePopup() const
{
    if (m_hasActivePopupCallback) {
        const void *thisPtr = this;
        return m_hasActivePopupCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::hasActivePopup();
    }
}
bool Platform_wrapper::hasActivePopup_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::hasActivePopup();
}
bool Platform_wrapper::inDisallowedDragView(KDDockWidgets::Point globalPos) const
{
    if (m_inDisallowedDragViewCallback) {
        const void *thisPtr = this;
        return m_inDisallowedDragViewCallback(const_cast<void *>(thisPtr), &globalPos);
    } else {
        return ::KDDockWidgets::flutter::Platform::inDisallowedDragView(globalPos);
    }
}
bool Platform_wrapper::inDisallowedDragView_nocallback(KDDockWidgets::Point globalPos) const
{
    return ::KDDockWidgets::flutter::Platform::inDisallowedDragView(globalPos);
}
void Platform_wrapper::init()
{
    ::KDDockWidgets::flutter::Platform::init();
}
void Platform_wrapper::installMessageHandler()
{
    if (m_installMessageHandlerCallback) {
        const void *thisPtr = this;
        m_installMessageHandlerCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::installMessageHandler();
    }
}
void Platform_wrapper::installMessageHandler_nocallback()
{
    ::KDDockWidgets::flutter::Platform::installMessageHandler();
}
bool Platform_wrapper::isLeftMouseButtonPressed() const
{
    if (m_isLeftMouseButtonPressedCallback) {
        const void *thisPtr = this;
        return m_isLeftMouseButtonPressedCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::isLeftMouseButtonPressed();
    }
}
bool Platform_wrapper::isLeftMouseButtonPressed_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::isLeftMouseButtonPressed();
}
bool Platform_wrapper::isProcessingAppQuitEvent() const
{
    if (m_isProcessingAppQuitEventCallback) {
        const void *thisPtr = this;
        return m_isProcessingAppQuitEventCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::isProcessingAppQuitEvent();
    }
}
bool Platform_wrapper::isProcessingAppQuitEvent_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::isProcessingAppQuitEvent();
}
const char *Platform_wrapper::name() const
{
    if (m_nameCallback) {
        const void *thisPtr = this;
        return m_nameCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::name();
    }
}
const char *Platform_wrapper::name_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::name();
}
void Platform_wrapper::onDropIndicatorOverlayCreated(KDDockWidgets::flutter::IndicatorWindow *arg__1)
{
    if (m_onDropIndicatorOverlayCreatedCallback) {
        const void *thisPtr = this;
        m_onDropIndicatorOverlayCreatedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onDropIndicatorOverlayCreated(arg__1);
    }
}
void Platform_wrapper::onDropIndicatorOverlayCreated_nocallback(KDDockWidgets::flutter::IndicatorWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onDropIndicatorOverlayCreated(arg__1);
}
void Platform_wrapper::onDropIndicatorOverlayDestroyed(KDDockWidgets::flutter::IndicatorWindow *arg__1)
{
    if (m_onDropIndicatorOverlayDestroyedCallback) {
        const void *thisPtr = this;
        m_onDropIndicatorOverlayDestroyedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onDropIndicatorOverlayDestroyed(arg__1);
    }
}
void Platform_wrapper::onDropIndicatorOverlayDestroyed_nocallback(KDDockWidgets::flutter::IndicatorWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onDropIndicatorOverlayDestroyed(arg__1);
}
void Platform_wrapper::onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    if (m_onFloatingWindowCreatedCallback) {
        const void *thisPtr = this;
        m_onFloatingWindowCreatedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onFloatingWindowCreated(arg__1);
    }
}
void Platform_wrapper::onFloatingWindowCreated_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onFloatingWindowCreated(arg__1);
}
void Platform_wrapper::onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    if (m_onFloatingWindowDestroyedCallback) {
        const void *thisPtr = this;
        m_onFloatingWindowDestroyedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onFloatingWindowDestroyed(arg__1);
    }
}
void Platform_wrapper::onFloatingWindowDestroyed_nocallback(KDDockWidgets::Core::FloatingWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onFloatingWindowDestroyed(arg__1);
}
void Platform_wrapper::onMainWindowCreated(KDDockWidgets::Core::MainWindow *arg__1)
{
    if (m_onMainWindowCreatedCallback) {
        const void *thisPtr = this;
        m_onMainWindowCreatedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onMainWindowCreated(arg__1);
    }
}
void Platform_wrapper::onMainWindowCreated_nocallback(KDDockWidgets::Core::MainWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onMainWindowCreated(arg__1);
}
void Platform_wrapper::onMainWindowDestroyed(KDDockWidgets::Core::MainWindow *arg__1)
{
    if (m_onMainWindowDestroyedCallback) {
        const void *thisPtr = this;
        m_onMainWindowDestroyedCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::onMainWindowDestroyed(arg__1);
    }
}
void Platform_wrapper::onMainWindowDestroyed_nocallback(KDDockWidgets::Core::MainWindow *arg__1)
{
    ::KDDockWidgets::flutter::Platform::onMainWindowDestroyed(arg__1);
}
void Platform_wrapper::pauseForDartDebugger()
{
    if (m_pauseForDartDebuggerCallback) {
        const void *thisPtr = this;
        m_pauseForDartDebuggerCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::pauseForDartDebugger();
    }
}
void Platform_wrapper::pauseForDartDebugger_nocallback()
{
    ::KDDockWidgets::flutter::Platform::pauseForDartDebugger();
}
void Platform_wrapper::pauseForDebugger()
{
    if (m_pauseForDebuggerCallback) {
        const void *thisPtr = this;
        m_pauseForDebuggerCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::pauseForDebugger();
    }
}
void Platform_wrapper::pauseForDebugger_nocallback()
{
    ::KDDockWidgets::flutter::Platform::pauseForDebugger();
}
KDDockWidgets::flutter::Platform *Platform_wrapper::platformFlutter()
{
    return ::KDDockWidgets::flutter::Platform::platformFlutter();
}
void Platform_wrapper::rebuildWindowOverlay()
{
    if (m_rebuildWindowOverlayCallback) {
        const void *thisPtr = this;
        m_rebuildWindowOverlayCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::rebuildWindowOverlay();
    }
}
void Platform_wrapper::rebuildWindowOverlay_nocallback()
{
    ::KDDockWidgets::flutter::Platform::rebuildWindowOverlay();
}
void Platform_wrapper::restoreMouseCursor()
{
    if (m_restoreMouseCursorCallback) {
        const void *thisPtr = this;
        m_restoreMouseCursorCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::restoreMouseCursor();
    }
}
void Platform_wrapper::restoreMouseCursor_nocallback()
{
    ::KDDockWidgets::flutter::Platform::restoreMouseCursor();
}
void Platform_wrapper::resumeCoRoutines()
{
    ::KDDockWidgets::flutter::Platform::resumeCoRoutines();
}
void Platform_wrapper::runDelayed(int ms, KDDockWidgets::Core::DelayedCall *c)
{
    if (m_runDelayedCallback) {
        const void *thisPtr = this;
        m_runDelayedCallback(const_cast<void *>(thisPtr), ms, c);
    } else {
        ::KDDockWidgets::flutter::Platform::runDelayed(ms, c);
    }
}
void Platform_wrapper::runDelayed_nocallback(int ms, KDDockWidgets::Core::DelayedCall *c)
{
    ::KDDockWidgets::flutter::Platform::runDelayed(ms, c);
}
void Platform_wrapper::runTests()
{
    ::KDDockWidgets::flutter::Platform::runTests();
}
void Platform_wrapper::scheduleResumeCoRoutines(int ms) const
{
    if (m_scheduleResumeCoRoutinesCallback) {
        const void *thisPtr = this;
        m_scheduleResumeCoRoutinesCallback(const_cast<void *>(thisPtr), ms);
    } else {
        ::KDDockWidgets::flutter::Platform::scheduleResumeCoRoutines(ms);
    }
}
void Platform_wrapper::scheduleResumeCoRoutines_nocallback(int ms) const
{
    ::KDDockWidgets::flutter::Platform::scheduleResumeCoRoutines(ms);
}
int Platform_wrapper::screenNumberForView(KDDockWidgets::Core::View *arg__1) const
{
    if (m_screenNumberForViewCallback) {
        const void *thisPtr = this;
        return m_screenNumberForViewCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        return ::KDDockWidgets::flutter::Platform::screenNumberForView(arg__1);
    }
}
int Platform_wrapper::screenNumberForView_nocallback(KDDockWidgets::Core::View *arg__1) const
{
    return ::KDDockWidgets::flutter::Platform::screenNumberForView(arg__1);
}
KDDockWidgets::Size Platform_wrapper::screenSizeFor(KDDockWidgets::Core::View *arg__1) const
{
    if (m_screenSizeForCallback) {
        const void *thisPtr = this;
        return *m_screenSizeForCallback(const_cast<void *>(thisPtr), arg__1);
    } else {
        return ::KDDockWidgets::flutter::Platform::screenSizeFor(arg__1);
    }
}
KDDockWidgets::Size Platform_wrapper::screenSizeFor_nocallback(KDDockWidgets::Core::View *arg__1) const
{
    return ::KDDockWidgets::flutter::Platform::screenSizeFor(arg__1);
}
void Platform_wrapper::sendEvent(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Event *arg__2) const
{
    if (m_sendEventCallback) {
        const void *thisPtr = this;
        m_sendEventCallback(const_cast<void *>(thisPtr), arg__1, arg__2);
    } else {
        ::KDDockWidgets::flutter::Platform::sendEvent(arg__1, arg__2);
    }
}
void Platform_wrapper::sendEvent_nocallback(KDDockWidgets::Core::View *arg__1, KDDockWidgets::Event *arg__2) const
{
    ::KDDockWidgets::flutter::Platform::sendEvent(arg__1, arg__2);
}
void Platform_wrapper::setCursorPos(KDDockWidgets::Point arg__1)
{
    if (m_setCursorPosCallback) {
        const void *thisPtr = this;
        m_setCursorPosCallback(const_cast<void *>(thisPtr), &arg__1);
    } else {
        ::KDDockWidgets::flutter::Platform::setCursorPos(arg__1);
    }
}
void Platform_wrapper::setCursorPos_nocallback(KDDockWidgets::Point arg__1)
{
    ::KDDockWidgets::flutter::Platform::setCursorPos(arg__1);
}
void Platform_wrapper::setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
{
    if (m_setMouseCursorCallback) {
        const void *thisPtr = this;
        m_setMouseCursorCallback(const_cast<void *>(thisPtr), arg__1, discardLast);
    } else {
        ::KDDockWidgets::flutter::Platform::setMouseCursor(arg__1, discardLast);
    }
}
void Platform_wrapper::setMouseCursor_nocallback(Qt::CursorShape arg__1, bool discardLast)
{
    ::KDDockWidgets::flutter::Platform::setMouseCursor(arg__1, discardLast);
}
int Platform_wrapper::startDragDistance_impl() const
{
    if (m_startDragDistance_implCallback) {
        const void *thisPtr = this;
        return m_startDragDistance_implCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::startDragDistance_impl();
    }
}
int Platform_wrapper::startDragDistance_impl_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::startDragDistance_impl();
}
bool Platform_wrapper::supportsAeroSnap() const
{
    if (m_supportsAeroSnapCallback) {
        const void *thisPtr = this;
        return m_supportsAeroSnapCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::supportsAeroSnap();
    }
}
bool Platform_wrapper::supportsAeroSnap_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::supportsAeroSnap();
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent)
{
    if (m_tests_createFocusableViewCallback) {
        const void *thisPtr = this;
        return m_tests_createFocusableViewCallback(const_cast<void *>(thisPtr), &arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::Platform::tests_createFocusableView(arg__1, parent);
    }
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createFocusableView_nocallback(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent)
{
    return ::KDDockWidgets::flutter::Platform::tests_createFocusableView(arg__1, parent);
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createNonClosableView(KDDockWidgets::Core::View *parent)
{
    if (m_tests_createNonClosableViewCallback) {
        const void *thisPtr = this;
        return m_tests_createNonClosableViewCallback(const_cast<void *>(thisPtr), parent);
    } else {
        return ::KDDockWidgets::flutter::Platform::tests_createNonClosableView(parent);
    }
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createNonClosableView_nocallback(KDDockWidgets::Core::View *parent)
{
    return ::KDDockWidgets::flutter::Platform::tests_createNonClosableView(parent);
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent)
{
    if (m_tests_createViewCallback) {
        const void *thisPtr = this;
        return m_tests_createViewCallback(const_cast<void *>(thisPtr), &arg__1, parent);
    } else {
        return ::KDDockWidgets::flutter::Platform::tests_createView(arg__1, parent);
    }
}
KDDockWidgets::Core::View *Platform_wrapper::tests_createView_nocallback(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View *parent)
{
    return ::KDDockWidgets::flutter::Platform::tests_createView(arg__1, parent);
}
void Platform_wrapper::tests_deinitPlatform_impl()
{
    if (m_tests_deinitPlatform_implCallback) {
        const void *thisPtr = this;
        m_tests_deinitPlatform_implCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::tests_deinitPlatform_impl();
    }
}
void Platform_wrapper::tests_deinitPlatform_impl_nocallback()
{
    ::KDDockWidgets::flutter::Platform::tests_deinitPlatform_impl();
}
void Platform_wrapper::tests_initPlatform_impl()
{
    if (m_tests_initPlatform_implCallback) {
        const void *thisPtr = this;
        m_tests_initPlatform_implCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::tests_initPlatform_impl();
    }
}
void Platform_wrapper::tests_initPlatform_impl_nocallback()
{
    ::KDDockWidgets::flutter::Platform::tests_initPlatform_impl();
}
void Platform_wrapper::ungrabMouse()
{
    if (m_ungrabMouseCallback) {
        const void *thisPtr = this;
        m_ungrabMouseCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::ungrabMouse();
    }
}
void Platform_wrapper::ungrabMouse_nocallback()
{
    ::KDDockWidgets::flutter::Platform::ungrabMouse();
}
void Platform_wrapper::uninstallMessageHandler()
{
    if (m_uninstallMessageHandlerCallback) {
        const void *thisPtr = this;
        m_uninstallMessageHandlerCallback(const_cast<void *>(thisPtr));
    } else {
        ::KDDockWidgets::flutter::Platform::uninstallMessageHandler();
    }
}
void Platform_wrapper::uninstallMessageHandler_nocallback()
{
    ::KDDockWidgets::flutter::Platform::uninstallMessageHandler();
}
bool Platform_wrapper::usesFallbackMouseGrabber() const
{
    if (m_usesFallbackMouseGrabberCallback) {
        const void *thisPtr = this;
        return m_usesFallbackMouseGrabberCallback(const_cast<void *>(thisPtr));
    } else {
        return ::KDDockWidgets::flutter::Platform::usesFallbackMouseGrabber();
    }
}
bool Platform_wrapper::usesFallbackMouseGrabber_nocallback() const
{
    return ::KDDockWidgets::flutter::Platform::usesFallbackMouseGrabber();
}
Platform_wrapper::~Platform_wrapper()
{
}

}
}
static KDDockWidgets::flutter::Platform *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::flutter::Platform *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__flutter__Platform_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper *>(cppObj);
}
void *c_KDDockWidgets__flutter__Platform__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// applicationName() const
void *c_KDDockWidgets__flutter__Platform__applicationName(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<QString> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->applicationName_nocallback();} else {    return targetPtr->applicationName();} }() };
    return result;
}
// createDefaultViewFactory()
void *c_KDDockWidgets__flutter__Platform__createDefaultViewFactory(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createDefaultViewFactory_nocallback();} else {    return targetPtr->createDefaultViewFactory();} }();
    return result;
}
// createMainWindow(const QString & uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View * parent, Qt::WindowFlags flags) const
void *c_KDDockWidgets__flutter__Platform__createMainWindow_QString_CreateViewOptions_MainWindowOptions_View_WindowFlags(void *thisObj, const char *uniqueName_, void *viewOpts_, int options_, void *parent_, int flags)
{
    const auto uniqueName = QString::fromUtf8(uniqueName_);
    assert(viewOpts_);
    auto &viewOpts = *reinterpret_cast<KDDockWidgets::Core::CreateViewOptions *>(viewOpts_);
    auto options = static_cast<QFlags<KDDockWidgets::MainWindowOption>>(options_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createMainWindow_nocallback(uniqueName,viewOpts,options,parent,static_cast<Qt::WindowFlags>(flags));} else {    return targetPtr->createMainWindow(uniqueName,viewOpts,options,parent,static_cast<Qt::WindowFlags>(flags));} }();
    free(( char * )uniqueName_);
    return result;
}
// createView(KDDockWidgets::Core::Controller * controller, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__Platform__createView_Controller_View(void *thisObj, void *controller_, void *parent_)
{
    auto controller = reinterpret_cast<KDDockWidgets::Core::Controller *>(controller_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->createView_nocallback(controller,parent);} else {    return targetPtr->createView(controller,parent);} }();
    return result;
}
// cursorPos() const
void *c_KDDockWidgets__flutter__Platform__cursorPos(void *thisObj)
{
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Point> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->cursorPos_nocallback();} else {    return targetPtr->cursorPos();} }() };
    return result;
}
// dumpManagedBacktrace()
void c_KDDockWidgets__flutter__Platform__dumpManagedBacktrace(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->dumpManagedBacktrace_nocallback();} else {    return targetPtr->dumpManagedBacktrace();} }();
}
// hasActivePopup() const
bool c_KDDockWidgets__flutter__Platform__hasActivePopup(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->hasActivePopup_nocallback();} else {    return targetPtr->hasActivePopup();} }();
    return result;
}
// inDisallowedDragView(KDDockWidgets::Point globalPos) const
bool c_KDDockWidgets__flutter__Platform__inDisallowedDragView_Point(void *thisObj, void *globalPos_)
{
    assert(globalPos_);
    auto &globalPos = *reinterpret_cast<KDDockWidgets::Point *>(globalPos_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->inDisallowedDragView_nocallback(globalPos);} else {    return targetPtr->inDisallowedDragView(globalPos);} }();
    return result;
}
// init()
void c_KDDockWidgets__flutter__Platform__init(void *thisObj)
{
    fromWrapperPtr(thisObj)->init();
}
// installMessageHandler()
void c_KDDockWidgets__flutter__Platform__installMessageHandler(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->installMessageHandler_nocallback();} else {    return targetPtr->installMessageHandler();} }();
}
// isLeftMouseButtonPressed() const
bool c_KDDockWidgets__flutter__Platform__isLeftMouseButtonPressed(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isLeftMouseButtonPressed_nocallback();} else {    return targetPtr->isLeftMouseButtonPressed();} }();
    return result;
}
// isProcessingAppQuitEvent() const
bool c_KDDockWidgets__flutter__Platform__isProcessingAppQuitEvent(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->isProcessingAppQuitEvent_nocallback();} else {    return targetPtr->isProcessingAppQuitEvent();} }();
    return result;
}
// name() const
const char *c_KDDockWidgets__flutter__Platform__name(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->name_nocallback();} else {    return targetPtr->name();} }();
    return result;
}
// onDropIndicatorOverlayCreated(KDDockWidgets::flutter::IndicatorWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayCreated_IndicatorWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::flutter::IndicatorWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onDropIndicatorOverlayCreated_nocallback(arg__1);} else {    return targetPtr->onDropIndicatorOverlayCreated(arg__1);} }();
}
// onDropIndicatorOverlayDestroyed(KDDockWidgets::flutter::IndicatorWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayDestroyed_IndicatorWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::flutter::IndicatorWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onDropIndicatorOverlayDestroyed_nocallback(arg__1);} else {    return targetPtr->onDropIndicatorOverlayDestroyed(arg__1);} }();
}
// onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onFloatingWindowCreated_FloatingWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onFloatingWindowCreated_nocallback(arg__1);} else {    return targetPtr->onFloatingWindowCreated(arg__1);} }();
}
// onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onFloatingWindowDestroyed_FloatingWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::FloatingWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onFloatingWindowDestroyed_nocallback(arg__1);} else {    return targetPtr->onFloatingWindowDestroyed(arg__1);} }();
}
// onMainWindowCreated(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onMainWindowCreated_MainWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onMainWindowCreated_nocallback(arg__1);} else {    return targetPtr->onMainWindowCreated(arg__1);} }();
}
// onMainWindowDestroyed(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onMainWindowDestroyed_MainWindow(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::MainWindow *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->onMainWindowDestroyed_nocallback(arg__1);} else {    return targetPtr->onMainWindowDestroyed(arg__1);} }();
}
// pauseForDartDebugger()
void c_KDDockWidgets__flutter__Platform__pauseForDartDebugger(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->pauseForDartDebugger_nocallback();} else {    return targetPtr->pauseForDartDebugger();} }();
}
// pauseForDebugger()
void c_KDDockWidgets__flutter__Platform__pauseForDebugger(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->pauseForDebugger_nocallback();} else {    return targetPtr->pauseForDebugger();} }();
}
// platformFlutter()
void *c_static_KDDockWidgets__flutter__Platform__platformFlutter()
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::platformFlutter();
    return result;
}
// rebuildWindowOverlay()
void c_KDDockWidgets__flutter__Platform__rebuildWindowOverlay(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->rebuildWindowOverlay_nocallback();} else {    return targetPtr->rebuildWindowOverlay();} }();
}
// restoreMouseCursor()
void c_KDDockWidgets__flutter__Platform__restoreMouseCursor(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->restoreMouseCursor_nocallback();} else {    return targetPtr->restoreMouseCursor();} }();
}
// resumeCoRoutines()
void c_KDDockWidgets__flutter__Platform__resumeCoRoutines(void *thisObj)
{
    fromPtr(thisObj)->resumeCoRoutines();
}
// runDelayed(int ms, KDDockWidgets::Core::DelayedCall * c)
void c_KDDockWidgets__flutter__Platform__runDelayed_int_DelayedCall(void *thisObj, int ms, void *c_)
{
    auto c = reinterpret_cast<KDDockWidgets::Core::DelayedCall *>(c_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->runDelayed_nocallback(ms,c);} else {    return targetPtr->runDelayed(ms,c);} }();
}
// runTests()
void c_KDDockWidgets__flutter__Platform__runTests(void *thisObj)
{
    fromPtr(thisObj)->runTests();
}
// scheduleResumeCoRoutines(int ms) const
void c_KDDockWidgets__flutter__Platform__scheduleResumeCoRoutines_int(void *thisObj, int ms)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->scheduleResumeCoRoutines_nocallback(ms);} else {    return targetPtr->scheduleResumeCoRoutines(ms);} }();
}
// screenNumberForView(KDDockWidgets::Core::View * arg__1) const
int c_KDDockWidgets__flutter__Platform__screenNumberForView_View(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__1_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->screenNumberForView_nocallback(arg__1);} else {    return targetPtr->screenNumberForView(arg__1);} }();
    return result;
}
// screenSizeFor(KDDockWidgets::Core::View * arg__1) const
void *c_KDDockWidgets__flutter__Platform__screenSizeFor_View(void *thisObj, void *arg__1_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__1_);
    const auto &result = new Dartagnan::ValueWrapper<KDDockWidgets::Size> { [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->screenSizeFor_nocallback(arg__1);} else {    return targetPtr->screenSizeFor(arg__1);} }() };
    return result;
}
// sendEvent(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Event * arg__2) const
void c_KDDockWidgets__flutter__Platform__sendEvent_View_Event(void *thisObj, void *arg__1_, void *arg__2_)
{
    auto arg__1 = reinterpret_cast<KDDockWidgets::Core::View *>(arg__1_);
    auto arg__2 = reinterpret_cast<KDDockWidgets::Event *>(arg__2_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->sendEvent_nocallback(arg__1,arg__2);} else {    return targetPtr->sendEvent(arg__1,arg__2);} }();
}
// setCursorPos(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__flutter__Platform__setCursorPos_Point(void *thisObj, void *arg__1_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Point *>(arg__1_);
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setCursorPos_nocallback(arg__1);} else {    return targetPtr->setCursorPos(arg__1);} }();
}
// setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
void c_KDDockWidgets__flutter__Platform__setMouseCursor_CursorShape_bool(void *thisObj, int arg__1, bool discardLast)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->setMouseCursor_nocallback(static_cast<Qt::CursorShape>(arg__1),discardLast);} else {    return targetPtr->setMouseCursor(static_cast<Qt::CursorShape>(arg__1),discardLast);} }();
}
// startDragDistance_impl() const
int c_KDDockWidgets__flutter__Platform__startDragDistance_impl(void *thisObj)
{
    const auto &result = fromWrapperPtr(thisObj)->startDragDistance_impl_nocallback();
    return result;
}
// supportsAeroSnap() const
bool c_KDDockWidgets__flutter__Platform__supportsAeroSnap(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->supportsAeroSnap_nocallback();} else {    return targetPtr->supportsAeroSnap();} }();
    return result;
}
// tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createFocusableView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Core::CreateViewOptions *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->tests_createFocusableView_nocallback(arg__1,parent);} else {    return targetPtr->tests_createFocusableView(arg__1,parent);} }();
    return result;
}
// tests_createNonClosableView(KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createNonClosableView_View(void *thisObj, void *parent_)
{
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->tests_createNonClosableView_nocallback(parent);} else {    return targetPtr->tests_createNonClosableView(parent);} }();
    return result;
}
// tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_)
{
    assert(arg__1_);
    auto &arg__1 = *reinterpret_cast<KDDockWidgets::Core::CreateViewOptions *>(arg__1_);
    auto parent = reinterpret_cast<KDDockWidgets::Core::View *>(parent_);
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->tests_createView_nocallback(arg__1,parent);} else {    return targetPtr->tests_createView(arg__1,parent);} }();
    return result;
}
// tests_deinitPlatform_impl()
void c_KDDockWidgets__flutter__Platform__tests_deinitPlatform_impl(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->tests_deinitPlatform_impl_nocallback();} else {    return targetPtr->tests_deinitPlatform_impl();} }();
}
// tests_initPlatform_impl()
void c_KDDockWidgets__flutter__Platform__tests_initPlatform_impl(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->tests_initPlatform_impl_nocallback();} else {    return targetPtr->tests_initPlatform_impl();} }();
}
// ungrabMouse()
void c_KDDockWidgets__flutter__Platform__ungrabMouse(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->ungrabMouse_nocallback();} else {    return targetPtr->ungrabMouse();} }();
}
// uninstallMessageHandler()
void c_KDDockWidgets__flutter__Platform__uninstallMessageHandler(void *thisObj)
{
    [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->uninstallMessageHandler_nocallback();} else {    return targetPtr->uninstallMessageHandler();} }();
}
// usesFallbackMouseGrabber() const
bool c_KDDockWidgets__flutter__Platform__usesFallbackMouseGrabber(void *thisObj)
{
    const auto &result = [&] {auto targetPtr = fromPtr(thisObj);auto wrapperPtr = dynamic_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper*>(targetPtr);if (wrapperPtr) {    return wrapperPtr->usesFallbackMouseGrabber_nocallback();} else {    return targetPtr->usesFallbackMouseGrabber();} }();
    return result;
}
void c_KDDockWidgets__flutter__Platform__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
void c_KDDockWidgets__flutter__Platform__registerVirtualMethodCallback(void *ptr, void *callback, int methodId)
{
    auto wrapper = fromWrapperPtr(ptr);
    switch (methodId) {
    case 133:
        wrapper->m_applicationNameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_applicationName>(callback);
        break;
    case 134:
        wrapper->m_createDefaultViewFactoryCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_createDefaultViewFactory>(callback);
        break;
    case 135:
        wrapper->m_createMainWindowCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_createMainWindow>(callback);
        break;
    case 136:
        wrapper->m_createViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_createView>(callback);
        break;
    case 137:
        wrapper->m_cursorPosCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_cursorPos>(callback);
        break;
    case 138:
        wrapper->m_dumpManagedBacktraceCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_dumpManagedBacktrace>(callback);
        break;
    case 140:
        wrapper->m_hasActivePopupCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_hasActivePopup>(callback);
        break;
    case 141:
        wrapper->m_inDisallowedDragViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_inDisallowedDragView>(callback);
        break;
    case 142:
        wrapper->m_installMessageHandlerCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_installMessageHandler>(callback);
        break;
    case 145:
        wrapper->m_isLeftMouseButtonPressedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_isLeftMouseButtonPressed>(callback);
        break;
    case 146:
        wrapper->m_isProcessingAppQuitEventCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_isProcessingAppQuitEvent>(callback);
        break;
    case 150:
        wrapper->m_nameCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_name>(callback);
        break;
    case 182:
        wrapper->m_onDropIndicatorOverlayCreatedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onDropIndicatorOverlayCreated>(callback);
        break;
    case 183:
        wrapper->m_onDropIndicatorOverlayDestroyedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onDropIndicatorOverlayDestroyed>(callback);
        break;
    case 151:
        wrapper->m_onFloatingWindowCreatedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onFloatingWindowCreated>(callback);
        break;
    case 152:
        wrapper->m_onFloatingWindowDestroyedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onFloatingWindowDestroyed>(callback);
        break;
    case 153:
        wrapper->m_onMainWindowCreatedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onMainWindowCreated>(callback);
        break;
    case 154:
        wrapper->m_onMainWindowDestroyedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_onMainWindowDestroyed>(callback);
        break;
    case 184:
        wrapper->m_pauseForDartDebuggerCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_pauseForDartDebugger>(callback);
        break;
    case 155:
        wrapper->m_pauseForDebuggerCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_pauseForDebugger>(callback);
        break;
    case 186:
        wrapper->m_rebuildWindowOverlayCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_rebuildWindowOverlay>(callback);
        break;
    case 156:
        wrapper->m_restoreMouseCursorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_restoreMouseCursor>(callback);
        break;
    case 157:
        wrapper->m_runDelayedCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_runDelayed>(callback);
        break;
    case 189:
        wrapper->m_scheduleResumeCoRoutinesCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_scheduleResumeCoRoutines>(callback);
        break;
    case 158:
        wrapper->m_screenNumberForViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_screenNumberForView>(callback);
        break;
    case 159:
        wrapper->m_screenSizeForCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_screenSizeFor>(callback);
        break;
    case 160:
        wrapper->m_sendEventCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_sendEvent>(callback);
        break;
    case 161:
        wrapper->m_setCursorPosCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_setCursorPos>(callback);
        break;
    case 162:
        wrapper->m_setMouseCursorCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_setMouseCursor>(callback);
        break;
    case 164:
        wrapper->m_startDragDistance_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_startDragDistance_impl>(callback);
        break;
    case 165:
        wrapper->m_supportsAeroSnapCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_supportsAeroSnap>(callback);
        break;
    case 166:
        wrapper->m_tests_createFocusableViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_tests_createFocusableView>(callback);
        break;
    case 167:
        wrapper->m_tests_createNonClosableViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_tests_createNonClosableView>(callback);
        break;
    case 168:
        wrapper->m_tests_createViewCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_tests_createView>(callback);
        break;
    case 169:
        wrapper->m_tests_deinitPlatform_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_tests_deinitPlatform_impl>(callback);
        break;
    case 170:
        wrapper->m_tests_initPlatform_implCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_tests_initPlatform_impl>(callback);
        break;
    case 171:
        wrapper->m_ungrabMouseCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_ungrabMouse>(callback);
        break;
    case 172:
        wrapper->m_uninstallMessageHandlerCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_uninstallMessageHandler>(callback);
        break;
    case 173:
        wrapper->m_usesFallbackMouseGrabberCallback = reinterpret_cast<KDDockWidgetsBindings_wrappersNS::KDDWBindingsFlutter::Platform_wrapper::Callback_usesFallbackMouseGrabber>(callback);
        break;
    }
}
}
