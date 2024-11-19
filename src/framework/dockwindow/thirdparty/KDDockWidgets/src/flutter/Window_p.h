/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/Screen_p.h"
#include "core/Window_p.h"

namespace KDDockWidgets::flutter {

class DOCKS_EXPORT Window : public Core::Window
{
public:
    /// For flutter, we identify windows with a sequential ID
    explicit Window(std::shared_ptr<Core::View> rootView);

    ~Window() override;
    std::shared_ptr<Core::View> rootView() const override;
    Window::Ptr transientParent() const override;
    void setGeometry(Rect) override;
    void setVisible(bool) override;
    bool supportsHonouringLayoutMinSize() const override;

    void setWindowState(WindowState) override;
    Rect geometry() const override;
    bool isVisible() const override;
    Core::WId handle() const override;
    bool equals(std::shared_ptr<Core::Window> other) const override;
    void setFramePosition(Point targetPos) override;
    Rect frameGeometry() const override;
    void resize(int width, int height) override;
    bool isActive() const override;
    WindowState windowState() const override;
    Point mapFromGlobal(Point globalPos) const override;
    Point mapToGlobal(Point localPos) const override;
    Core::Screen::Ptr screen() const override;
    void destroy() override;
    Size minSize() const override;
    Size maxSize() const override;
    Point fromNativePixels(Point) const override;
    bool isFullScreen() const override;
    void onScreenChanged(Core::Object *context, WindowScreenChangedCallback) override;

private:
    std::shared_ptr<Core::View> m_rootView;
    bool m_isVisible = true;
    Rect m_geometry;
};
}
