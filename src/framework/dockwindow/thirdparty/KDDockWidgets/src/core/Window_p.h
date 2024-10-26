/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "View.h"
#include "Screen_p.h"

namespace KDDockWidgets::Core {

/// @brief Represents a top-level window
/// In Qt for example, this would be equivalent to QWindow.
/// In doesn't take ownership, just gives a generic API to an existing window.

class DOCKS_EXPORT Window
{
public:
    using Ptr = std::shared_ptr<Core::Window>;
    using List = Vector<Ptr>;
    typedef void (*WindowScreenChangedCallback)(Core::Object *context, Ptr window);

    Window() = default;
    virtual ~Window();
    virtual void setWindowState(WindowState) = 0;

    /// @brief Returns the geometry of the client area of the window
    /// This excludes any native window frame and title bar.
    /// The top-left of this rect is in screen coordinates.
    /// @sa setGeometry, resize, size
    virtual Rect geometry() const = 0;

    /// @brief Sets the geometry of the client area of the window
    /// This excludes any native window frame and title bar.
    /// The top-left of this rect is in screen coordinates.
    virtual void setGeometry(Rect) = 0;

    /// @brief Sets the position of the window to pos
    /// Unlike setPosition, this position includes the native frame
    virtual void setFramePosition(Point pos) = 0;

    /// @brief Returns the geometry of the window including its frame (window decos)
    virtual Rect frameGeometry() const = 0;


    /// @brief Resizes the client-area of the window, excluding any native frame
    /// Should be equivalent of resizing via setGeometry()
    /// @sa size, setGeometry
    virtual void resize(int width, int height) = 0;

    /// @brief Returns whether the window is visible
    virtual bool isVisible() const = 0;

    /// @brief Sets the window visible or hidden;
    virtual void setVisible(bool) = 0;

    /// @brief Returns the handle that uniquely identifies the window within the window manager
    /// This can be simply the native platform window pointer
    virtual WId handle() const = 0;

    /// @brief Returns whether the two Window instances refer to the same underlying platform window
    virtual bool equals(std::shared_ptr<Core::Window> other) const = 0;

    /// @brief Returns whether a window is active
    /// An active window has keyboard focus, and might have its window decos
    /// highlighted. Windows are usually become active after you click on their
    /// title bar, but can also be done programmatically.
    virtual bool isActive() const = 0;

    /// @brief Returns the root view of this window
    /// For example, for QtWidgets, this would be the top-level QWidget
    /// represented by this QWindow
    virtual std::shared_ptr<View> rootView() const = 0;

    /// @brief Returns the window state
    virtual WindowState windowState() const = 0;

    /// @brief Returns the parent of this window, if any
    /// Popups and utility windows usually have a parent window
    virtual Window::Ptr transientParent() const = 0;

    /// @brief Receives a point in screen coordinates and returns it in window coordinates
    virtual Point mapFromGlobal(Point globalPos) const = 0;

    /// @brief Receives a point in local coordinates and returns it in global coordinates
    /// The top-left of the window's client area is 0,0 in local coordinates
    virtual Point mapToGlobal(Point localPos) const = 0;

    /// @brief Returns the screen this window is on
    virtual Screen::Ptr screen() const = 0;

    /// Deletes the underlying window. Only used during tests.
    virtual void destroy() = 0;

    /// @brief Returns whether this window is fullscreen currently
    virtual bool isFullScreen() const = 0;

    /// @brief Returns whether this window can't be shrunk to a size that would violate the layout's
    /// min size This is true for QtWidgets where the layout constraings propagate up to the window
    /// However, for QtQuick it difficult as there's no QLayout.
    //    - For QtQuick/FloatingWindow we try to not violate the min-size, which we have total
    //    control over
    //    - But for QtQuick/MainWindow it's more difficult, as we don't know how the user composed
    //    his
    //      main.qml. so this is false if the Window is not a FloatingWindow
    //
    // This method is only used to so we can suppress some warnings regarding layout being clipped
    // due to too small window.
    virtual bool supportsHonouringLayoutMinSize() const = 0;

    /// @brief Returns the window's minimum size
    virtual Size minSize() const = 0;

    /// @brief Returns the window's maximum size
    virtual Size maxSize() const = 0;

    /// @brief Receives a point in native global space and returns in logical global space.
    /// This is relevant only when there's HDPI scaling applied.
    /// By native it's meant that it corresponds to physical pixels, which is what win32 API deals
    /// with. By logical it's meant that there might be an HDPI factor applied. Both returned and
    /// received points are in global space (screen space). Implement if your frontend will run on
    /// Windows, otherwise it's unused.
    virtual Point fromNativePixels(Point) const = 0;

    /// @brief Starts a native window move
    /// Only needed on Windows. The difference between a a native move and a client move is that we
    /// can get aerosnap on the edges of the screen.
    virtual void startSystemMove();

    /// @brief Returns the client size of this window.
    Size size() const;

    /// @brif Returns the position of the top-left corner, including native frame
    Point framePosition() const;

    /// @brief Sets the position of the window at targetPos
    /// This ignores any frame (native decos the window has)
    void setPosition(Point targetPos);

    /// Convenience API
    /// @sa minSize, maxSize
    int minWidth() const;
    int minHeight() const;
    int maxWidth() const;
    int maxHeight() const;

    /// @brief Registers a callback to be called when window changes screen
    /// Multiple callbacks can be registered
    virtual void onScreenChanged(Core::Object *context, WindowScreenChangedCallback) = 0;

    /// Restoring a QWindow directly with showMinimized() is buggy on Qt on Windows, due to WM_NCCALCSIZE not handled correctly
    virtual void setHasBeenMinimizedDirectlyFromRestore(bool)
    {
    }
    virtual bool hasBeenMinimizedDirectlyFromRestore() const
    {
        return false;
    }

private:
    bool containsView(Controller *) const;
    bool containsView(View *) const;

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
};

inline bool operator==(Core::Window::Ptr w1, Window::Ptr w2)
{
    if (!w1 && !w2)
        return true;

    if (w1 && w2)
        return w1->equals(w2);

    return false;
}

bool operator!=(Core::Window::Ptr, Core::Window::Ptr) = delete;

}
