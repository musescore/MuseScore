/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "Controller.h"

#include <memory>

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {

using HANDLE = const void *;
using WId = quintptr;

class Item;
class EventFilterInterface;
class Controller;
class Screen;
class Window;
class MDILayout;
class DropArea;
class DockWidget;
class FloatingWindow;
class Group;
class Layout;
class Stack;
class TabBar;
class TitleBar;
class MainWindow;

class DOCKS_EXPORT View
{
public:
    explicit View(Controller *controller, ViewType);
    virtual ~View();

    /// init method to solve cyclic ctor dependencies between view and controllers
    /// Called by the controller
    virtual void init();

    /// @brief Returns a handle for the GUI element
    /// This value only makes sense to the frontend. For example, for QtQuick it might be a
    /// QQuickItem, while for QtWidgets it's a QWidget. Can be whatever the frontend developer
    /// wants, as long as it uniquely identifies the GUI element. KDDW backend only uses it for
    /// comparison purposes
    virtual HANDLE handle() const = 0;

    /// @brief Returns whether the gui item represented by this view was already deleted
    /// Usually false, as KDDW internal gui elements inherit View, and nobody will access them after
    /// destruction. However, ViewWrapper derived classes, wrap an existing gui element, which might
    /// get deleted. Override isNull() in our ViewWrapper subclasses and return true if the wrapped
    /// gui element was already deleted
    virtual bool isNull() const;

    virtual void setParent(View *) = 0;
    virtual Size minSize() const = 0;
    virtual void setMinimumSize(Size) = 0;

    virtual Qt::FocusPolicy focusPolicy() const = 0;
    virtual bool hasFocus() const = 0;
    virtual Size maxSizeHint() const = 0;
    virtual Rect geometry() const = 0;
    virtual Rect normalGeometry() const = 0;
    virtual void setGeometry(Rect) = 0;
    virtual bool isVisible() const = 0;
    virtual bool isExplicitlyHidden() const = 0;
    virtual void setVisible(bool) = 0;
    virtual void move(int x, int y) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void setWidth(int width) = 0;
    virtual void setHeight(int height) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void update() = 0;
    virtual void raiseAndActivate() = 0;

    /// If this view is a root view, then raises its window, otherwise, raises itself
    /// relatively to its siblings and does not raise its window
    virtual void raise() = 0;

    virtual void activateWindow() = 0;
    virtual bool isRootView() const = 0;
    virtual Point mapToGlobal(Point) const = 0;
    virtual Point mapFromGlobal(Point) const = 0;
    virtual Point mapTo(View *, Point) const = 0;
    virtual bool close() = 0;
    virtual void setFlag(Qt::WindowType, bool = true) = 0;
    virtual Qt::WindowFlags flags() const = 0;
    virtual void setWindowTitle(const QString &title) = 0;
    virtual void setWindowIcon(const Icon &) = 0;

    /// Enable/disable attributes. This is mostly for QtWidget compatibility
    /// Do not use. We don't depend on Qt::WidgetAttribute in a future version.
    virtual void enableAttribute(Qt::WidgetAttribute, bool enable = true) = 0;
    virtual bool hasAttribute(Qt::WidgetAttribute) const = 0;

    /// @brief Installs an event filter in this view to intercept the event it receives
    /// Analogue to QObject::installEventFilter() in the Qt world
    /// @sa removeViewEventFilter
    void installViewEventFilter(EventFilterInterface *);

    /// @brief Removes the event filter
    void removeViewEventFilter(EventFilterInterface *);

    /// @brief Delivers mouse events and such to event filters
    bool deliverViewEventToFilters(Event *e);

    virtual void showNormal() = 0;
    virtual void showMinimized() = 0;
    virtual void showMaximized() = 0;
    virtual bool isMinimized() const = 0;
    virtual bool isMaximized() const = 0;

    virtual void createPlatformWindow();
    virtual void setMaximumSize(Size sz) = 0;
    virtual bool isActiveWindow() const = 0;
    virtual void setFixedWidth(int) = 0;
    virtual void setFixedHeight(int) = 0;
    virtual void grabMouse() = 0;
    virtual void releaseMouse() = 0;
    virtual void releaseKeyboard() = 0;
    virtual void setFocus(Qt::FocusReason) = 0;
    virtual void setFocusPolicy(Qt::FocusPolicy) = 0;
    virtual void setWindowOpacity(double) = 0;
    virtual void setCursor(Qt::CursorShape) = 0;
    virtual void setMouseTracking(bool) = 0;

    virtual bool onResize(int h, int w);
    bool onResize(Size);

    virtual bool onFocusInEvent(FocusEvent *)
    {
        return false;
    }

    /// Equivalent to Qt's QObject::objectName()
    virtual void setViewName(const QString &) = 0;
    virtual QString viewName() const = 0;

    virtual void render(QPainter *) = 0;

    virtual std::shared_ptr<View> childViewAt(Point localPos) const = 0;

    /// @brief Returns the top-level gui element which this view is inside
    /// It's the root view of the window.
    virtual std::shared_ptr<View> rootView() const = 0;

    /// @brief Returns the window this view is inside
    /// For the Qt frontend, this wraps a QWindow.
    /// Like QWidget::window()
    virtual std::shared_ptr<Core::Window> window() const = 0;

    /// @brief Returns the gui element's parent. Like QWidget::parentWidget()
    virtual std::shared_ptr<View> parentView() const = 0;

    /// @brief Returns this view, but as a wrapper
    virtual std::shared_ptr<View> asWrapper() = 0;

    /// @brief Returns whether the view is of the specified type
    /// Virtual so it can be overridden by ViewWrapper. When we're wrapping an existing GUI element
    /// only the specific frontend can know what's the actual type
    virtual bool is(ViewType) const;

    /// @brief Sets the z order
    /// Not supported on all platforms and only relevant for MDI mode.
    virtual void setZOrder(int);

    /// Returns the zOrder
    /// Not supported on all platforms and only relevant for MDI mode.
    virtual int zOrder() const;

    /// @Returns a list of child views
    virtual Vector<std::shared_ptr<View>> childViews() const = 0;

    /// @brief Returns whether the DTOR is currently running. freed() might be true while inDtor
    /// false, as the implementation of free() is free to delay it (with deleteLater() for example)
    bool inDtor() const;

    /// @brief Returns whether this view represents the same GUI element as the other
    bool equals(const View *other) const;
    bool equals(const std::shared_ptr<View> &) const;
    static bool equals(const View *one, const View *two);

    Point pos() const;
    Size size() const;
    Rect rect() const;
    int x() const;
    int y() const;
    int height() const;
    int width() const;
    void resize(Size);
    void resize(int w, int h);
    void move(Point);
    void setSize(Size);
    int minimumWidth() const;
    int minimumHeight() const;

    /// Returns the size of the screen that this view belongs to
    Size screenSize() const;

    /// The minimum minimum size a dock widget can have
    static Size hardcodedMinimumSize();

    /// @brief Returns the controller of the first parent view of the specified type
    /// Goes up the view hierarchy chain until it finds it. Returns nullptr otherwise.
    static Controller *firstParentOfType(View *view, ViewType);

    /// @brief Returns this view's controller
    Controller *controller() const;

    /// Returns the View's controller, casted as T
    template<typename T>
    T *asController()
    {
        if (m_inDtor)
            return nullptr;

        return object_cast<T *>(m_controller);
    }

    /// asFooController() are deprecated. Use asController<T>() instead
    Core::FloatingWindow *asFloatingWindowController() const;
    Core::Group *asGroupController() const;
    Core::TitleBar *asTitleBarController() const;
    Core::TabBar *asTabBarController() const;
    Core::Stack *asStackController() const;
    Core::DockWidget *asDockWidgetController() const;
    Core::MainWindow *asMainWindowController() const;
    Core::DropArea *asDropAreaController() const;
    Core::MDILayout *asMDILayoutController() const;
    Core::Layout *asLayout() const;

    /// Prints some debug to stderr
    void dumpDebug();

    bool isFixedWidth() const;
    bool isFixedHeight() const;

public:
    class Private;
    Private *const d;

protected:
    Controller *const m_controller;
    bool m_inDtor = false;

    View(const View &) = delete;
    View &operator=(const View &) = delete;

#ifdef KDDW_FRONTEND_FLUTTER
    // Little workaround so flutter has the same deletion order as Qt.
    // In Qt we have this order of deletion
    //    1. ~Core::View() deletes the controller
    //    2. ~QObject deletes children views
    // But in Flutter, we don't have ~QObject, so we were deleting children views inside ~flutter::View
    // which runs before ~Core::View() not after, causing different deletion ordering, thus different behaviour
    // and different bugs. Let's keep both frontends consistent predictable.

    // No shared pointers, as lifetime is managed by parent-children relationship (as in QObject)
    Vector<Core::View *> m_childViews;
#endif
};

}

}
