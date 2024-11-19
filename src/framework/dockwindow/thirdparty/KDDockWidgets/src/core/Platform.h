/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "View.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/QtCompat_p.h"

#include <vector>
#include <memory.h>

namespace KDDockWidgets {

namespace Core {
class DelayedCall;
class EventFilterInterface;
struct CreateViewOptions;
class ClassicDropIndicatorOverlay;
class SegmentedDropIndicatorOverlay;
class ViewFactory;
class Window;

/// @brief implements functions specific to a particular platform
/// A platform can be for example qtwidgets, qtquick, etc.
class DOCKS_EXPORT Platform
{
public:
    /// @brief Enum describing the graphics stack type
    enum class DisplayType {
        Other = 0,
        X11 = 1,
        Wayland = 2,
        QtOffscreen = 3,
        QtEGLFS = 4,
        Windows = 5
    };

    virtual ~Platform();
    /// @brief Returns the name of the platform, only "qtwidgets" and "qtquick"
    virtual const char *name() const = 0;

    /// @brief Returns the platform singleton
    static Platform *instance();

    /// Returns whether a Platform instance exists
    // Will be false at start up right before using KDDW and at shutdown after dtor runs
    static bool hasInstance();

    /// @brief Returns whether a popup is open
    /// Usually not needed to override. Investigate further in case side bars aren't auto hiding
    virtual bool hasActivePopup() const;

    /// @brief Returns the focused view, if any
    virtual std::shared_ptr<View> focusedView() const = 0;

    /// @brief Returns all windows
    virtual Vector<std::shared_ptr<Core::Window>> windows() const = 0;

    /// @brief Creates and returns the default ViewFactory
    virtual ViewFactory *createDefaultViewFactory() = 0;

    /// @brief Returns the window at the specified global coordinates
    virtual std::shared_ptr<Core::Window> windowAt(Point globalPos) const = 0;

    /// @brief Sends the specified event to the specified view
    virtual void sendEvent(View *, Event *) const = 0;

    /// @brief Returns the screen index for the specified view or window.
    /// It's up to the platform to decide how screens are ordered, kddw won't care.
    virtual int screenNumberForView(View *) const = 0;
    virtual int screenNumberForWindow(std::shared_ptr<Core::Window>) const = 0;

    /// @brief Returns the size of the screen where this view is in
    virtual Size screenSizeFor(View *) const = 0;

    /// @brief Create an empty view
    /// For Qt this would just returns a empty QWidget or QQuickItem
    /// other frontends can return something as basic.
    virtual View *createView(Controller *, View *parent = nullptr) const = 0;

    /// @brief Returns whether this platform is QtWidgets
    bool isQtWidgets() const;

    /// @brief Returns whether this platform is QtQuick
    bool isQtQuick() const;

    /// @brief Returns whether this platform is Qt based
    bool isQt() const;

    /// @brief Returns how many pixels the mouse must move for a drag to start
    /// This is usually 4 by default (QApplication::startDragDistance() for QtWidgets)
    /// You can override by calling Config::setStartDragDistance(), so you don't need to create
    /// a new Platform class.
    int startDragDistance() const;

    /// @brief Return whether we use the global event filter based mouse grabber
    virtual bool usesFallbackMouseGrabber() const = 0;

    /// @brief Returns whether the specified global position is on top of a view
    /// that isn't draggable. This is needed since not the entire title bar is draggable.
    /// For example, clicking on the close button shouldn't start a drag.
    virtual bool inDisallowedDragView(Point globalPos) const = 0;

    /// @brief Releases the mouse grab, if any
    virtual void ungrabMouse() = 0;

    /// runs the specified all after ms
    /// Equivalent to QTimer::singleShot in Qt
    virtual void runDelayed(int ms, Core::DelayedCall *c) = 0;

    /**
     * @brief Returns whether we're processing a Event::Quit
     *
     * Used internally to know if we should let Qt close a NonClosable dock widget at shutdown time.
     */
    virtual bool isProcessingAppQuitEvent() const = 0;

    /// @brief Installs a global event filter
    /// Events will be forwarded to the specified EventFilterInterface
    void installGlobalEventFilter(EventFilterInterface *);

    /// @brief Removes a global event filter
    void removeGlobalEventFilter(EventFilterInterface *);

    /// @brief Returns the application name
    /// This name will be used as title of floating dock widgets which contain more than 1 group
    virtual QString applicationName() const = 0;

    /// @brief Sets the mouse cursor to the specified shape, this has an application-wide effect
    /// Call restoreMouseCursor() to set the previous cursor shape
    /// @param discardLast If true, then the previous shape is discarded instead of
    /// being saved into a stack for restoreMouseCursor()
    virtual void setMouseCursor(Qt::CursorShape, bool discardLast = false) = 0;

    /// @brief Undoes the call to setMouseCursor()
    virtual void restoreMouseCursor() = 0;

    /// @brief Returns the type of graphics stack being used
    virtual DisplayType displayType() const = 0;

    /// @brief Returns whether the left mouse button is pressed
    virtual bool isLeftMouseButtonPressed() const = 0;

    /// @brief Returns all available screens
    virtual Vector<std::shared_ptr<Screen>> screens() const = 0;

    virtual std::shared_ptr<Screen> primaryScreen() const = 0;

    /// @brief For non-C++, managed languages (having a VM) prints a non-native back-trace
    /// For example, the flutter frontend implements this to get a dart backtrace
    /// Used for debugging only. Can be called by gdb.
    virtual void dumpManagedBacktrace()
    {
    }

    /// @brief Called when a floating window is created.
    /// Overridden by flutter, so it can create a window
    virtual void onFloatingWindowCreated(Core::FloatingWindow *);

    /// @brief Called when a floating window is created.
    /// Overridden by flutter, so it can destroy the window
    virtual void onFloatingWindowDestroyed(Core::FloatingWindow *);

    /// @brief Called when a main window is created.
    /// Overridden by flutter, so it can create a window
    /// Used by tests only. In real life users will instantiate a MainWindow in dart directly.
    virtual void onMainWindowCreated(Core::MainWindow *);

    /// @brief Called when a main window is created.
    /// Overridden by flutter, so it can destroy the window
    virtual void onMainWindowDestroyed(Core::MainWindow *);

    /// Returns the mouse cursor position in screen coordinates
    virtual Point cursorPos() const = 0;

    /// Sets the mouse cursor position in screen coordinates
    virtual void setCursorPos(Point) = 0;

    /// Reads the specified and returns its content
    /// The default implementation uses std::ifstream while the Qt implementation
    /// uses QFile, as it needs to support QRC
    virtual QByteArray readFile(const QString &, bool &ok) const;

    /// Only supported on Qt, for windows
    virtual bool supportsAeroSnap() const;

    /// @brief list the list of frontend types supported by this build
    static std::vector<KDDockWidgets::FrontendType> frontendTypes();

    /// @brief Returns whether the Platform was already initialized
    static bool isInitialized();

#if defined(DOCKS_DEVELOPER_MODE) && !defined(DARTAGNAN_BINDINGS_RUN)
    /// Big timeout since lsan on github actions is very slow
#define DEFAULT_TIMEOUT 20000

    // Stuff required by the tests only and not used by dart bindings.

    /// @brief halts the test during the specified number of milliseconds
    /// The event loop keeps running. Use this for debugging purposes so you can interact with your
    /// test and see what's going on
    virtual KDDW_QCORO_TASK tests_wait(int ms) const = 0;

    /// @brief Waits for the specified view to receive a resize event
    /// Returns true if the view was resized until timeout was reached
    virtual KDDW_QCORO_TASK tests_waitForResize(View *, int timeout = DEFAULT_TIMEOUT) const = 0;
    virtual KDDW_QCORO_TASK tests_waitForResize(Controller *, int timeout = DEFAULT_TIMEOUT) const = 0;
    virtual KDDW_QCORO_TASK tests_waitForDeleted(View *, int timeout = DEFAULT_TIMEOUT) const = 0;
    virtual KDDW_QCORO_TASK tests_waitForDeleted(Controller *, int timeout = DEFAULT_TIMEOUT) const = 0;

    /// @brief Waits for the specified window to be active (have the keyboard focus)
    /// Window::isActive() should return true
    /// @sa Window::isActive()
    virtual KDDW_QCORO_TASK tests_waitForWindowActive(std::shared_ptr<Core::Window>, int timeout = DEFAULT_TIMEOUT) const = 0;

    /// @brief Waits for the specified view to receive the specified event
    /// Returns true if the view received said event until timeout was reached
    virtual KDDW_QCORO_TASK tests_waitForEvent(Core::Object *w, Event::Type type, int timeout = DEFAULT_TIMEOUT) const = 0;
    virtual KDDW_QCORO_TASK tests_waitForEvent(View *, Event::Type type, int timeout = DEFAULT_TIMEOUT) const = 0;
    virtual KDDW_QCORO_TASK tests_waitForEvent(std::shared_ptr<Core::Window>, Event::Type type,
                                               int timeout = DEFAULT_TIMEOUT) const = 0;

    virtual void tests_doubleClickOn(Point globalPos, View *receiver) = 0;
    virtual void tests_doubleClickOn(Point globalPos, std::shared_ptr<Core::Window> receiver) = 0;
    virtual void tests_pressOn(Point globalPos, View *receiver) = 0;
    virtual void tests_pressOn(Point globalPos, std::shared_ptr<Core::Window> receiver) = 0;
    virtual KDDW_QCORO_TASK tests_releaseOn(Point globalPos, View *receiver) = 0;
    virtual KDDW_QCORO_TASK tests_mouseMove(Point globalPos, View *receiver) = 0;

    /// @brief Creates a Window. For the sole purpose of unit-testing Window.
    /// The created window should be visible.
    virtual std::shared_ptr<Core::Window> tests_createWindow() = 0;

    /// @brief Creates the platform. Called by the tests at startup.
    /// For any custom behaviour in your derived Platform override tests_initPlatform_impl()
    static void tests_initPlatform(int &argc, char *argv[], KDDockWidgets::FrontendType, bool defaultToOffscreenQPA = true);

    /// @brief Deletes the platform. Called at end of tests.
    /// For any custom behaviour in your derived Platform override tests_deinitPlatform_impl()
    static void tests_deinitPlatform();

    std::string m_expectedWarning;
#endif

#ifdef DOCKS_TESTING_METHODS

    class WarningObserver
    {
        KDDW_DELETE_COPY_CTOR(WarningObserver)
    public:
        WarningObserver() = default;
        virtual ~WarningObserver();
        virtual void onFatal() = 0;
    };

    /// @brief Creates a view with the specified parent
    /// If the parent is null then a new window is created and the returned view will be the root
    /// view
    virtual View *tests_createView(CreateViewOptions, View *parent = nullptr) = 0;

    /// @brief Returns a view that can have keyboard focus
    /// For example a line edit. This is used to for testing focus related features.
    virtual View *tests_createFocusableView(CreateViewOptions, View *parent = nullptr) = 0;

    /// @brief Returns a view that rejects close events
    virtual View *tests_createNonClosableView(View *parent = nullptr) = 0;

    virtual void installMessageHandler() = 0;
    virtual void uninstallMessageHandler() = 0;

    /// @brief Creates a main window. This is not API that the user will use, but used
    /// internally by some tools that need a main window
    virtual Core::MainWindow *
    createMainWindow(const QString &uniqueName, CreateViewOptions,
                     MainWindowOptions options = MainWindowOption_HasCentralFrame,
                     View *parent = nullptr, Qt::WindowFlags = {}) const = 0;

    virtual void pauseForDebugger();

protected:
    /// @brief Implement any needed initializations before tests starting to run, if any
    /// Override in derived classes for custom behavior.
    virtual void tests_initPlatform_impl()
    {
    }

    /// @brief Implement any needed cleanup after the tests runs, if any
    /// Override in derived classes for custom behavior.
    virtual void tests_deinitPlatform_impl()
    {
    }

#endif

public:
    class Private;
    Private *const d;

protected:
    virtual int startDragDistance_impl() const;
    Platform();

    Platform(const Platform &) = delete;
    Platform &operator=(const Platform &) = delete;
};

#if defined(DOCKS_DEVELOPER_MODE)

#if !defined(DARTAGNAN_BINDINGS_RUN)

struct SetExpectedWarning
{
    explicit SetExpectedWarning(const std::string &s)
    {
        if (!s.empty())
            Platform::instance()->m_expectedWarning = s;
    }

    ~SetExpectedWarning()
    {
        Platform::instance()->m_expectedWarning.clear();
    }

    KDDW_DELETE_COPY_CTOR(SetExpectedWarning)
};

#endif // bindings

#endif // dev-mode

#if defined(DOCKS_TESTING_METHODS)

struct CreateViewOptions
{
    bool isVisible = false;
    Size sizeHint = {};
    Size minSize = { 0, 0 };
    Size maxSize = Size(16777215, 16777215);
    Size size = { 1000, 1000 };
    bool createWindow = false;

    Size getMinSize() const
    {
        return minSize;
    }

    Size getMaxSize() const
    {
        return maxSize;
    }

    Size getSize() const
    {
        return size;
    }
};

#endif

} // Core

} // KDDockWidgets
