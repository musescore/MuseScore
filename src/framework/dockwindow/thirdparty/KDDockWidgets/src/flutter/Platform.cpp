/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Platform.h"
#include "kddockwidgets/KDDockWidgets.h"

#include "Window_p.h"
#include "Screen_p.h"
#include "views/View.h"
#include "core/Platform_p.h"
#include "core/Logging_p.h"
#include "core/ObjectGuard_p.h"
#include "core/ViewGuard.h"
#include "ViewFactory.h"
#include "kddockwidgets/core/MainWindow.h"

#include <mutex>
#include <memory.h>

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

Point Platform::s_lastCursorPosition = { -1, -1 };

class flutter::Platform::Private
{
public:
    std::shared_ptr<Core::View> m_focusedView;
    std::optional<int> m_testsResult;
};

Platform::Platform()
    : d(new Private())
{
    init();
}

Platform::~Platform()
{
    delete d;
}

void Platform::init()
{
}

std::shared_ptr<Core::View> Platform::focusedView() const
{
    return d->m_focusedView;
}

Vector<std::shared_ptr<Core::Window>> Platform::windows() const
{
    KDDW_WARN("Platform::windows: Not implemented");
    return {};
}

void Platform::sendEvent(Core::View *, Event *) const
{
    KDDW_WARN("Platform::sendEvent: Not implemented");
}

const char *Platform::name() const
{
    KDDW_WARN("Platform::name: Not implemented");
    return nullptr;
}

bool Platform::hasActivePopup() const
{
    return false;
}

Core::ViewFactory *Platform::createDefaultViewFactory()
{
    return new ViewFactory();
}

Core::Window::Ptr Platform::windowAt(Point) const
{
    KDDW_WARN("Platform::windowAt: Not implemented");
    return {};
}

int Platform::screenNumberForView(Core::View *) const
{
    KDDW_WARN("Platform::screenNumberFor: Not implemented");
    return -1;
}

int Platform::screenNumberForWindow(std::shared_ptr<Core::Window>) const
{
    KDDW_WARN("Platform::screenNumberFor: Not implemented");
    return {};
}

bool Platform::isProcessingAppQuitEvent() const
{
    KDDW_WARN("Platform::isProcessingAppQuitEven: Not implemented");
    return {};
}

QString Platform::applicationName() const
{
    // FLUTTER_TODO
    // KDDW_WARN("Platform::applicationName: Not implemented");
    return {};
}

void Platform::setMouseCursor(Qt::CursorShape, bool)
{
    KDDW_WARN("Platform::setMouseCursor: Not implemented");
}

void Platform::restoreMouseCursor()
{
    KDDW_WARN("Platform::restoreMouseCursor: Not implemented");
}

Core::Platform::DisplayType Platform::displayType() const
{
    /// FLUTTER_TODO: Improve this
    return DisplayType::Other;
}

bool Platform::isLeftMouseButtonPressed() const
{
    KDDW_WARN("Platform::isLeftMouseButtonPressed: Not implemented");
    return {};
}

Size Platform::screenSizeFor(Core::View *) const
{
    KDDW_WARN("Platform::screenSizeFor: Not implemented");
    return {};
}

Core::View *Platform::createView(Core::Controller *, Core::View *) const
{
    KDDW_WARN("Platform::createView: Not implemented");
    return nullptr;
}

bool Platform::usesFallbackMouseGrabber() const
{
    return false;
}

bool Platform::inDisallowedDragView(Point) const
{
    return false;
}

void Platform::ungrabMouse()
{
    KDDW_WARN("Platform::ungrabMouse: Not implemented");
}

Core::Screen::List Platform::screens() const
{
    /// FLUTTER_TODO, find the actual screens
    return { primaryScreen() };
}

Core::Screen::Ptr Platform::primaryScreen() const
{
    return std::make_shared<flutter::Screen>();
}

void Platform::onFloatingWindowCreated(Core::FloatingWindow *)
{
    KDDW_WARN("Platform::onFloatingWindowCreated: Implemented in dart");
}

void Platform::onFloatingWindowDestroyed(Core::FloatingWindow *)
{
    KDDW_WARN("Platform::onFloatingWindowDestroyed: Implemented in dart");
}

void Platform::onMainWindowCreated(Core::MainWindow *)
{
    KDDW_WARN("Platform::onMainWindowCreated: Implemented in dart");
}

void Platform::onMainWindowDestroyed(Core::MainWindow *)
{
    KDDW_WARN("Platform::onMainWindowDestroyed: Implemented in dart");
}

void Platform::onDropIndicatorOverlayCreated(flutter::IndicatorWindow *)
{
    KDDW_WARN("Platform::onDropIndicatorOverlayCreated: Implemented in dart");
}

void Platform::onDropIndicatorOverlayDestroyed(flutter::IndicatorWindow *)
{
    KDDW_WARN("Platform::onDropIndicatorOverlayDestroyed: Implemented in dart");
}

void Platform::rebuildWindowOverlay()
{
    KDDW_WARN("Platform::rebuildWindowOverlay: Implemented in dart");
}

void Platform::runDelayed(int, Core::DelayedCall *)
{
    Q_UNREACHABLE(); // Platform.dart gets called instead
}

Point Platform::cursorPos() const
{
    return flutter::Platform::s_lastCursorPosition;
}

void Platform::setCursorPos(Point)
{
    KDDW_WARN("Platform::setCursorPos: Not implemented");
}


#ifdef DOCKS_TESTING_METHODS

void Platform::installMessageHandler()
{
}

void Platform::uninstallMessageHandler()
{
}

void Platform::pauseForDebugger()
{
    pauseForDartDebugger();
}


namespace KDDockWidgets::flutter {

class TestView_flutter : public flutter::View
{
public:
    explicit TestView_flutter(Core::CreateViewOptions opts, Core::View *)
        : flutter::View(nullptr, Core::ViewType::None, nullptr /* FLUTTER_TODO parent*/)
        , m_opts(opts)
    {
    }

    ~TestView_flutter();

    Size maxSizeHint() const override
    {
        return {};
    }

private:
    Core::CreateViewOptions m_opts;
};

class FocusableTestView_flutter : public flutter::View
{
public:
    explicit FocusableTestView_flutter(Core::CreateViewOptions opts, Core::View *)
        : flutter::View(nullptr, Core::ViewType::None, nullptr /* FLUTTER_TODO parent*/)
        , m_opts(opts)
    {
    }

    ~FocusableTestView_flutter();

    Size maxSizeHint() const override
    {
        return {};
    }

private:
    Core::CreateViewOptions m_opts;
};

class NonClosableTestView_flutter : public flutter::View
{
public:
    explicit NonClosableTestView_flutter(Core::View *)
        : flutter::View(nullptr, Core::ViewType::None, nullptr /* FLUTTER_TODO parent*/)
    {
    }

    ~NonClosableTestView_flutter();
};

TestView_flutter::~TestView_flutter() = default;
NonClosableTestView_flutter::~NonClosableTestView_flutter() = default;
FocusableTestView_flutter::~FocusableTestView_flutter() = default;

}

static std::mutex m_mutex;
void Platform::runTests()
{
#ifdef KDDW_FLUTTER_HAS_COROUTINES
    // Called from Flutter, so C++ tests run in the ui thread
    assert(s_runTestsFunc);

    // The tests run in a co-routine, meaning they can be interrupted (due to a C++ wait or deleteLater)
    // and the Flutter event loop keeps running. When they are actually finished, the "then()" block is run.
    s_runTestsFunc().then([this](auto result) {
        std::lock_guard<std::mutex> locker(m_mutex);
        assert(!d->m_testsResult.has_value());
        d->m_testsResult = result ? 0 : 1;
    });
#endif
}

void Platform::resumeCoRoutines()
{
#ifdef KDDW_FLUTTER_HAS_COROUTINES
    m_coRoutines.resume();
#endif
}

void Platform::scheduleResumeCoRoutines(int) const
{
    KDDW_WARN("Platform::scheduleResumeCoRoutines: Implemented in dart instead");
}

std::optional<int> Platform::testsResult() const
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return d->m_testsResult;
}

void Platform::tests_initPlatform_impl()
{
}

void Platform::tests_deinitPlatform_impl()
{
}

Core::View *Platform::tests_createView(Core::CreateViewOptions, Core::View *)
{
    Q_UNREACHABLE(); // Platform.dart gets called instead
    return {};
}

Core::View *Platform::tests_createFocusableView(Core::CreateViewOptions opts, Core::View *parent)
{
    // FLUTTER_TODO: It's not focusable. To fix when we get to those tests
    return tests_createView(opts, parent);
}

Core::View *Platform::tests_createNonClosableView(Core::View *parent)
{
    // FLUTTER_TODO: It's not non-closable. To fix when we get to those tests
    return tests_createView({ .isVisible = true }, parent);
}

Core::MainWindow *Platform::createMainWindow(const QString &, Core::CreateViewOptions,
                                             MainWindowOptions, Core::View *,
                                             Qt::WindowFlags) const
{
    Q_UNREACHABLE(); // Platform.dart gets called instead
    return {};
}

#endif


void Platform::setFocusedView(std::shared_ptr<Core::View> view)
{
    d->m_focusedView = view;
}

#ifdef KDDW_FLUTTER_HAS_COROUTINES

Platform::RunTestsFunc Platform::s_runTestsFunc = nullptr;

KDDW_QCORO_TASK Platform::tests_waitForWindowActive(std::shared_ptr<Core::Window>, int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_waitForEvent(Core::Object *, Event::Type, int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_waitForEvent(Core::View *, Event::Type, int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_waitForEvent(std::shared_ptr<Core::Window>, Event::Type,
                                             int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_wait(int ms) const
{
    /// Tell Dart to wake us up soon
    scheduleResumeCoRoutines(ms);

    /// Return to Dart's event loop
    co_await m_coRoutines.suspend();

    co_return true;
}

KDDW_QCORO_TASK Platform::tests_waitForResize(Core::View *, int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_waitForResize(Core::Controller *, int) const
{
    co_return co_await tests_wait(1000);
}

KDDW_QCORO_TASK Platform::tests_waitForDeleted(Core::Controller *obj, int timeout) const
{
    if (!obj)
        co_return true;

    Core::ObjectGuard<Core::Controller> guard = obj;
    int elapsed = 0;
    const int step = 100;

    while (guard && elapsed < timeout) {
        co_await tests_wait(step);
        elapsed += step;
    }

    co_return guard.isNull();
}

KDDW_QCORO_TASK Platform::tests_waitForDeleted(Core::View *view, int timeout) const
{
    if (!view)
        co_return true;

    Core::ViewGuard guard(view);

    int elapsed = 0;
    const int step = 100;

    while (guard && elapsed < timeout) {
        co_await tests_wait(step);
        elapsed += step;
    }

    co_return guard.isNull();
}

std::shared_ptr<Core::Window> Platform::tests_createWindow()
{
    auto window = new flutter::Window(tests_createView({}, nullptr)->asWrapper());
    return std::shared_ptr<Core::Window>(window);
}

void Platform::tests_doubleClickOn(Point, Core::View *)
{
    KDDW_WARN("Platform::tests_doubleClickOn: Not implemented yet");
}

void Platform::tests_doubleClickOn(Point, std::shared_ptr<Core::Window>)
{
    KDDW_WARN("Platform::tests_doubleClickOn: Not implemented yet");
}

QCoro::Task<bool> kddw_fakeMouseMove(Point globalPos);
QCoro::Task<bool> kddw_fakeMouseButton(Point globalPos, bool isPress);

void Platform::tests_pressOn(Point globalPos, Core::View *)
{
    kddw_fakeMouseButton(globalPos, /*isPress=*/true);
}

void Platform::tests_pressOn(Point globalPos, std::shared_ptr<Core::Window>)
{
    kddw_fakeMouseButton(globalPos, /*isPress=*/true);
}

KDDW_QCORO_TASK Platform::tests_releaseOn(Point globalPos, Core::View *)
{
    co_await kddw_fakeMouseButton(globalPos, /*isPress=*/false);
    co_return true;
}

KDDW_QCORO_TASK Platform::tests_mouseMove(Point globalPos, Core::View *)
{
    co_await kddw_fakeMouseMove(globalPos);
    co_return true;
}

#endif
