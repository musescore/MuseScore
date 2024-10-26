/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/core/Platform.h"

#include <optional>

#ifdef KDDW_FLUTTER_HAS_COROUTINES
#include "CoRoutines.h"
#endif

namespace KDDockWidgets {

namespace Core {
class MainWindow;
}

namespace flutter {

class IndicatorWindow;

/// @brief implements functions specific to a particular platform
/// A platform can be for example qtwidgets, qtquick, etc.
class DOCKS_EXPORT Platform : public Core::Platform
{
public:
    Platform();
    ~Platform() override;

    static Platform *platformFlutter()
    {
        return static_cast<Platform *>(Platform::instance());
    }

    /// Flutter doesn't have a better way of retrieving the cursor position
    /// so save it whenever we get a move event
    static Point s_lastCursorPosition;

    const char *name() const override;
    bool hasActivePopup() const override;
    Core::ViewFactory *createDefaultViewFactory() override;
    std::shared_ptr<Core::Window> windowAt(Point globalPos) const override;

    int screenNumberForView(Core::View *) const override;
    Size screenSizeFor(Core::View *) const override;

    Core::View *createView(Core::Controller *controller, Core::View *parent = nullptr) const override;
    bool inDisallowedDragView(Point globalPos) const override;
    bool usesFallbackMouseGrabber() const override;
    void ungrabMouse() override;
    Vector<std::shared_ptr<Core::Screen>> screens() const override;
    std::shared_ptr<Core::Screen> primaryScreen() const override;

    void onFloatingWindowCreated(Core::FloatingWindow *) override;
    void onFloatingWindowDestroyed(Core::FloatingWindow *) override;
    void onMainWindowCreated(Core::MainWindow *) override;
    void onMainWindowDestroyed(Core::MainWindow *) override;
    virtual void onDropIndicatorOverlayCreated(flutter::IndicatorWindow *);
    virtual void onDropIndicatorOverlayDestroyed(flutter::IndicatorWindow *);
    virtual void rebuildWindowOverlay();

    void runDelayed(int ms, Core::DelayedCall *c) override;

#ifdef KDDW_FLUTTER_HAS_COROUTINES
    // Stuff required by the tests only and not used by dart bindings.
    KDDW_QCORO_TASK tests_wait(int ms) const override;
    KDDW_QCORO_TASK tests_waitForResize(Core::View *, int timeout) const override;
    KDDW_QCORO_TASK tests_waitForResize(Core::Controller *, int timeout) const override;
    KDDW_QCORO_TASK tests_waitForDeleted(Core::Controller *, int timeout = 5000) const override;
    KDDW_QCORO_TASK tests_waitForDeleted(Core::View *, int timeout = 5000) const override;
    KDDW_QCORO_TASK tests_waitForWindowActive(std::shared_ptr<Core::Window>, int timeout) const override;
    KDDW_QCORO_TASK tests_waitForEvent(Core::Object *w, Event::Type type, int timeout) const override;
    KDDW_QCORO_TASK tests_waitForEvent(Core::View *, Event::Type type, int timeout) const override;
    KDDW_QCORO_TASK tests_waitForEvent(std::shared_ptr<Core::Window>, Event::Type type, int timeout) const override;

    void tests_doubleClickOn(Point globalPos, Core::View *receiver) override;
    void tests_doubleClickOn(Point globalPos, std::shared_ptr<Core::Window> receiver) override;
    void tests_pressOn(Point globalPos, Core::View *receiver) override;
    void tests_pressOn(Point globalPos, std::shared_ptr<Core::Window> receiver) override;
    KDDW_QCORO_TASK tests_releaseOn(Point globalPos, Core::View *receiver) override;
    KDDW_QCORO_TASK tests_mouseMove(Point globalPos, Core::View *receiver) override;
    std::shared_ptr<Core::Window> tests_createWindow() override;

    mutable CoRoutines m_coRoutines;

    typedef KDDW_QCORO_TASK (*RunTestsFunc)();
    static RunTestsFunc s_runTestsFunc;
#endif

#ifdef DOCKS_TESTING_METHODS
    void tests_initPlatform_impl() override;
    void tests_deinitPlatform_impl() override;
    Core::View *tests_createView(Core::CreateViewOptions, Core::View *parent = nullptr) override;
    Core::View *tests_createFocusableView(Core::CreateViewOptions, Core::View *parent = nullptr) override;
    Core::View *tests_createNonClosableView(Core::View *parent = nullptr) override;
    Core::MainWindow *
    createMainWindow(const QString &uniqueName, Core::CreateViewOptions viewOpts,
                     MainWindowOptions options = MainWindowOption_HasCentralFrame,
                     Core::View *parent = nullptr, Qt::WindowFlags flags = {}) const override;

    void installMessageHandler() override;
    void uninstallMessageHandler() override;

    void pauseForDebugger() override;

    /// Pauses execution, so we can attach Dart's debugger
    virtual void pauseForDartDebugger() { };

    // Called by unit-test's main.dart. Runs the tests.
    // The tests are in C++, as they are the same ones for QtWidgets and QtQuick
    void runTests();

    // Called by Dart's event loop, to wake up paused C++ unit-tests
    void resumeCoRoutines();

    /// Implemented in Dart, resums C++ coroutines after ms
    virtual void scheduleResumeCoRoutines(int ms) const;

    std::optional<int> testsResult() const;
#endif
protected:
    void init();

    // Platform interface
public:
    std::shared_ptr<Core::View> focusedView() const override;
    Vector<std::shared_ptr<Core::Window>> windows() const override;
    void sendEvent(Core::View *, Event *) const override;
    int screenNumberForWindow(std::shared_ptr<Core::Window>) const override;
    bool isProcessingAppQuitEvent() const override;
    QString applicationName() const override;
    void setMouseCursor(Qt::CursorShape, bool discardLast = false) override;
    void restoreMouseCursor() override;
    DisplayType displayType() const override;
    bool isLeftMouseButtonPressed() const override;
    Point cursorPos() const override;
    void setCursorPos(Point) override;
    void setFocusedView(std::shared_ptr<Core::View>);

    class Private;
    Private *const d;
};

}

}
