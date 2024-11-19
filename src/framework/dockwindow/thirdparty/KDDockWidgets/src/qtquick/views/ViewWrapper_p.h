/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_VIEWWRAPPER_QTQUICK_H
#define KD_VIEWWRAPPER_QTQUICK_H

#pragma once

#include "../../qtcommon/ViewWrapper_p.h"

#include <QQuickItem>
#include <QPointer>

namespace KDDockWidgets::QtQuick {

/// @brief A View that doesn't own its QQuickItem
/// Implements a View API around an existing QQuickItem
/// Useful for items that are not created by KDDW.
class DOCKS_EXPORT ViewWrapper : public QtCommon::ViewWrapper
{
public:
    QRect geometry() const override;
    void setGeometry(QRect) override;
    void move(int x, int y) override;
    QPoint mapToGlobal(QPoint) const override;
    QPoint mapFromGlobal(QPoint) const override;
    bool isRootView() const override;
    bool isVisible() const override;
    bool isExplicitlyHidden() const override;
    void setVisible(bool) override;
    void activateWindow() override;
    bool isMaximized() const override;
    bool isMinimized() const override;
    QSize maxSizeHint() const override;
    void setSize(int width, int height) override;
    bool is(Core::ViewType) const override;
    std::shared_ptr<Core::View> childViewAt(QPoint) const override;
    QVector<std::shared_ptr<Core::View>> childViews() const override;
    std::shared_ptr<Core::Window> window() const override;
    std::shared_ptr<Core::View> rootView() const override;
    std::shared_ptr<Core::View> parentView() const override;
    void setParent(Core::View *) override;
    void grabMouse() override;
    void releaseMouse() override;
    void setFocus(Qt::FocusReason) override;
    void setFocusPolicy(Qt::FocusPolicy) override;
    QString viewName() const override;
    bool isNull() const override;
    void setWindowTitle(const QString &title) override;
    QPoint mapTo(Core::View *someAncestor, QPoint pos) const override;
    bool hasAttribute(Qt::WidgetAttribute) const override;
    void setCursor(Qt::CursorShape) override;
    QSize minSize() const override;
    bool close() override;
    Qt::FocusPolicy focusPolicy() const override;
    bool hasFocus() const override;
    void raiseAndActivate() override;

    const Core::View *unwrap() const;
    Core::View *unwrap();

    static std::shared_ptr<Core::View> create(QObject *widget);
    static std::shared_ptr<Core::View> create(QQuickItem *widget);

private:
    explicit ViewWrapper(QObject *widget);
    explicit ViewWrapper(QQuickItem *widget);
    QPointer<QQuickItem> m_item;
};

}

#endif
