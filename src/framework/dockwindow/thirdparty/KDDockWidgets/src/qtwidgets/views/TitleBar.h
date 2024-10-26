/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_TITLEBAR_QTWIDGETS_H
#define KD_TITLEBAR_QTWIDGETS_H

#pragma once

#include "View.h"
#include "kddockwidgets/core/views/TitleBarViewInterface.h"

#include <QToolButton>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QLabel;
QT_END_NAMESPACE


class TestQtWidgets;

namespace KDDockWidgets::QtWidgets {

class DOCKS_EXPORT TitleBar : public View<QWidget>,
                              public Core::TitleBarViewInterface
{
    Q_OBJECT
public:
    /// Constructor called by the framework automatically
    /// Use it also if you're inheriting from QtWidgets::TitleBar while creating something custom
    explicit TitleBar(Core::TitleBar *controller, Core::View *parent = nullptr);

    /// Special ctor for the rare use case of using QtWidgets::TitleBar in a non-dock widget.
    /// For example, just to have a title bar in QMessageBox popups under EGLFS, so you don't have to
    /// style things twice.
    explicit TitleBar(QWidget *parent = nullptr);

    ~TitleBar() override;

    /// Returns the controller
    Core::TitleBar *titleBar() const;

#ifdef DOCKS_DEVELOPER_MODE
    // The following are needed for the unit-tests
    bool isCloseButtonVisible() const override;
    bool isCloseButtonEnabled() const override;
    bool isFloatButtonVisible() const override;
#endif

Q_SIGNALS:
    void isFocusedChanged();

protected:
    void init() override;
    void paintEvent(QPaintEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    QSize sizeHint() const override;
    void focusInEvent(QFocusEvent *) override;

    friend class KDDockWidgets::Core::TitleBar;
    int buttonAreaWidth() const;
    QRect iconRect() const;
    void updateMargins();

    void updateMaximizeButton(bool visible, bool enabled, TitleBarButtonType);
    void updateAutoHideButton(bool visible, bool enabled, TitleBarButtonType);
    void updateMinimizeButton(bool visible, bool enabled);

    virtual bool hasCustomLayout() const
    {
        return false;
    }

    QHBoxLayout *const m_layout;
    QAbstractButton *m_closeButton = nullptr;
    QAbstractButton *m_floatButton = nullptr;
    QAbstractButton *m_maximizeButton = nullptr;
    QAbstractButton *m_minimizeButton = nullptr;
    QAbstractButton *m_autoHideButton = nullptr;
    QLabel *m_dockWidgetIcon = nullptr;

private:
    friend class ::TestQtWidgets;
    // Private class just to hide KDBindings usage
    class Private;
    Private *const d;
};

/// @brief Button widget to be used in the TitleBar.
/// These are the KDDockWidget default buttons. Users can replace with their own and are not
/// forced to use these.
class Button : public QToolButton
{
    Q_OBJECT
public:
    explicit Button(QWidget *parent)
        : QToolButton(parent)
    {
        setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    }

    ~Button() override;

protected:
    friend class QtWidgets::TitleBar;

    bool event(QEvent *ev) override;
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *) override;

    bool m_inEventHandler = false;
};

}

#endif
