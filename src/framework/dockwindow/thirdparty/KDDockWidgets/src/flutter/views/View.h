/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/core/Controller.h"
#include "kddockwidgets/core/View.h"

#include <memory>
#include <optional>

namespace KDDockWidgets::flutter {

class DOCKS_EXPORT View : public Core::View
{
public:
    using Core::View::close;
    using Core::View::resize;

    explicit View(Core::Controller *controller, Core::ViewType type, Core::View *,
                  Qt::WindowFlags windowFlags = {});

    ~View() override;

    Size minSize() const override;
    Size maxSizeHint() const override;
    Rect geometry() const override;
    Rect normalGeometry() const override;
    void setNormalGeometry(Rect geo);
    void setGeometry(Rect geometry) override;
    void setMaximumSize(Size sz) override;

    bool isVisible() const override;
    void setVisible(bool visible) override;
    bool isExplicitlyHidden() const override;

    void move(int x, int y) override;
    void setSize(int w, int h) override;

    void setWidth(int w) override;
    void setHeight(int h) override;
    void setFixedWidth(int w) override;
    void setFixedHeight(int h) override;
    void show() override;
    void hide() override;
    void updateGeometry();
    void update() override;
    void setParent(Core::View *parent) override;
    void raiseAndActivate() override;
    void activateWindow() override;
    void raise() override;
    bool isRootView() const override;
    Point mapToGlobal(Point localPt) const override;
    Point mapFromGlobal(Point globalPt) const override;
    Point mapTo(Core::View *parent, Point pos) const override;
    void setWindowOpacity(double v) override;

    /// Called when the flutter widget was resized by its own initiative (and not kddw)
    /// Usually it's kddw driving geometry, but there's 2 cases where flutter might trigger it
    ///     - Window is resized by user with mouse
    ///     - Widget has m_fillsParent=true and its parent was resized
    bool onFlutterWidgetResized(int w, int h);

    bool close() override;
    void setFlag(Qt::WindowType f, bool on = true) override;
    void enableAttribute(Qt::WidgetAttribute attr, bool enable = true) override;
    bool hasAttribute(Qt::WidgetAttribute attr) const override;
    Qt::WindowFlags flags() const override;

    void setWindowTitle(const QString &title) override;
    void setWindowIcon(const Icon &icon) override;
    bool isActiveWindow() const override;

    void showNormal() override;
    void showMinimized() override;
    void showMaximized() override;

    bool isMinimized() const override;
    bool isMaximized() const override;

    std::shared_ptr<Core::Window> window() const override;
    std::shared_ptr<Core::View> childViewAt(Point p) const override;
    std::shared_ptr<Core::View> rootView() const override;
    std::shared_ptr<Core::View> parentView() const override;
    std::shared_ptr<Core::View> asWrapper() override;

    void setViewName(const QString &name) override;
    void grabMouse() override;
    void releaseMouse() override;
    void releaseKeyboard() override;
    void setFocus(Qt::FocusReason reason) override;
    Qt::FocusPolicy focusPolicy() const override;
    bool hasFocus() const override;
    void setFocusPolicy(Qt::FocusPolicy policy) override;
    QString viewName() const override;
    void setMinimumSize(Size sz) override;
    void render(QPainter *) override;
    void setCursor(Qt::CursorShape shape) override;
    void setMouseTracking(bool enable) override;
    Vector<std::shared_ptr<Core::View>> childViews() const override;
    void setZOrder(int z) override;
    Core::HANDLE handle() const override;

    virtual void onChildAdded(Core::View *childView);
    virtual void onChildRemoved(Core::View *childView);
    virtual void onChildVisibilityChanged(Core::View *childView);
    virtual void raiseChild(Core::View *childView);
    virtual void raiseWindow(Core::View *rootView);
    virtual void onGeometryChanged();

    /// Returns whether the flutter Widget associated with this view is mounted
    /// ie, it has a render object
    virtual bool isMounted() const;

    /// Called by flutter when a mouse event is received
    void onMouseEvent(Event::Type eventType, Point localPos, Point globalPos, bool leftIsPressed);

    /// View can override if it's interested in events which the event filter rejected
    virtual void onMousePress(MouseEvent *)
    {
    }

    /// Implemented in Dart
    virtual void onRebuildRequested();

private:
    View *m_parentView = nullptr;
    QString m_name;
    Size m_minSize;
    Size m_maxSize;
    Rect m_geometry;
    std::optional<bool> m_visible;
    bool m_inCtor = true;
    KDDW_DELETE_COPY_CTOR(View)
};

inline View *asView_flutter(Core::View *view)
{
    if (!view)
        return nullptr;
    return static_cast<View *>(view);
}

inline View *asView_flutter(Core::Controller *controller)
{
    if (!controller)
        return nullptr;

    return static_cast<View *>(controller->view());
}

} // namespace KDDockWidgets::flutter
