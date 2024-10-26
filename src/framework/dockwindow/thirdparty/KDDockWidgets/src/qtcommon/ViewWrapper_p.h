/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/qtcommon/View.h"
#include "kddockwidgets/docks_export.h"

namespace KDDockWidgets {

namespace Core {
class DropArea;
}

namespace QtCommon {

/// @brief The base class for view wrappers
/// A view wrapper is a view that doesn't own the native GUI element(QWidget, QQuickItem etc.)
/// It just adds View API to an existing GUI element. Useful for GUI elements that are not created
/// by KDDW. this is optional
class DOCKS_EXPORT ViewWrapper : public QtCommon::View_qt
{
public:
    using Ptr = std::shared_ptr<View>;

    explicit ViewWrapper(Core::Controller *controller, QObject *thisObj);
    ~ViewWrapper() override;

    void setMinimumSize(QSize) override;
    QSize maxSizeHint() const override;
    QRect normalGeometry() const override;
    void setWidth(int width) override;
    void setHeight(int height) override;
    void show() override;
    void hide() override;
    void update() override;
    void raiseAndActivate() override;
    void raise() override;
    void setFlag(Qt::WindowType, bool = true) override;
    void enableAttribute(Qt::WidgetAttribute, bool enable = true) override;
    Qt::WindowFlags flags() const override;
    void setWindowIcon(const QIcon &) override;
    void showNormal() override;
    void showMinimized() override;
    void showMaximized() override;
    void setMaximumSize(QSize sz) override;
    bool isActiveWindow() const override;
    void setFixedWidth(int) override;
    void setFixedHeight(int) override;
    void setWindowOpacity(double) override;
    void releaseKeyboard() override;
    void render(QPainter *p) override;
    void setMouseTracking(bool) override;
    std::shared_ptr<View> asWrapper() override;

private:
    Q_DISABLE_COPY(ViewWrapper)
    const bool m_ownsController;
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // In Qt6 we can't delete it
bool operator==(ViewWrapper::Ptr, ViewWrapper::Ptr) = delete;
#endif

bool operator!=(ViewWrapper::Ptr, ViewWrapper::Ptr) = delete;

}

}
