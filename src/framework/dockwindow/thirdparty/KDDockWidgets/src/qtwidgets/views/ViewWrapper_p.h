/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_VIEWWRAPPER_QTWIDGETS_H
#define KD_VIEWWRAPPER_QTWIDGETS_H

#pragma once

#include "../../qtcommon/ViewWrapper_p.h"

#include <QWidget>
#include <QPointer>

namespace KDDockWidgets::QtWidgets {

/// @brief A View that doesn't own its QWidget
/// Implements a View API around an existing QWidget
/// Useful for widgets that are not created by KDDW.
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
    void setVisible(bool) override;
    bool isExplicitlyHidden() const override;
    void activateWindow() override;
    bool isMaximized() const override;
    bool isMinimized() const override;
    void setSize(int width, int height) override;
    bool is(Core::ViewType) const override;
    std::shared_ptr<Core::Window> window() const override;
    std::shared_ptr<View> rootView() const override;
    std::shared_ptr<View> parentView() const override;
    void setParent(View *) override;
    bool close() override;
    std::shared_ptr<View> childViewAt(QPoint localPos) const override;
    QVector<std::shared_ptr<View>> childViews() const override;
    void grabMouse() override;
    void releaseMouse() override;
    void setFocus(Qt::FocusReason) override;
    QString viewName() const override;
    bool isNull() const override;
    void setWindowTitle(const QString &title) override;
    QPoint mapTo(View *, QPoint) const override;
    bool hasAttribute(Qt::WidgetAttribute) const override;
    void setCursor(Qt::CursorShape) override;
    QSize minSize() const override;
    Qt::FocusPolicy focusPolicy() const override;
    void setFocusPolicy(Qt::FocusPolicy) override;
    bool hasFocus() const override;
    QSize maxSizeHint() const override;
    void raiseAndActivate() override;
    QWidget *widget() const;

    static std::shared_ptr<View> create(QObject *widget);
    static std::shared_ptr<View> create(QWidget *widget);

private:
    explicit ViewWrapper(QObject *widget);
    explicit ViewWrapper(QWidget *widget);
    QPointer<QWidget> m_widget;
};

}

#endif
