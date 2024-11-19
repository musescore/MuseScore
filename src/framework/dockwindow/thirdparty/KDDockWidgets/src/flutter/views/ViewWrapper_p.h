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

namespace KDDockWidgets::flutter {

class View;

class DOCKS_EXPORT ViewWrapper : public Core::View
{
public:
    using Core::View::close;
    using Core::View::resize;

    static std::shared_ptr<Core::View> create(flutter::View *wrapped);
    ~ViewWrapper() override;

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

    bool onResize(int w, int h) override;

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

    bool is(Core::ViewType) const override;

private:
    explicit ViewWrapper(flutter::View *wrapped);
    void setWeakPtr(std::weak_ptr<ViewWrapper> thisPtr);
    flutter::View *const m_wrappedView = nullptr;
    std::weak_ptr<ViewWrapper> m_thisWeakPtr;
    KDDW_DELETE_COPY_CTOR(ViewWrapper)
};

} // namespace KDDockWidgets::flutter
