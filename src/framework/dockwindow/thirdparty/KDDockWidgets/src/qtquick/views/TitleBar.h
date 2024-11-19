/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_TITLEBARQUICK_P_H
#define KD_TITLEBARQUICK_P_H
#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/core/views/TitleBarViewInterface.h"
#include "View.h"

namespace KDDockWidgets {


namespace Core {
class TitleBar;
}

namespace QtQuick {

class DOCKS_EXPORT TitleBar : public QtQuick::View, public Core::TitleBarViewInterface
{
    Q_OBJECT
    // These properties is just for the unit-tests
    Q_PROPERTY(QQuickItem *titleBarQmlItem READ titleBarQmlItem WRITE setTitleBarQmlItem NOTIFY
                   titleBarQmlItemChanged)
    Q_PROPERTY(QQuickItem *titleBarMouseArea READ titleBarMouseArea CONSTANT)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool hasIcon READ hasIcon NOTIFY iconChanged)
    Q_PROPERTY(bool closeButtonEnabled READ closeButtonEnabled WRITE setCloseButtonEnabled NOTIFY
                   closeButtonEnabledChanged)
    Q_PROPERTY(bool floatButtonVisible READ floatButtonVisible WRITE setFloatButtonVisible NOTIFY
                   floatButtonVisibleChanged)
    Q_PROPERTY(bool maximizeButtonVisible READ maximizeButtonVisible NOTIFY
                   maximizeButtonVisibleChanged)
    Q_PROPERTY(bool minimizeButtonVisible READ minimizeButtonVisible NOTIFY
                   minimizeButtonVisibleChanged)
    Q_PROPERTY(QString floatButtonToolTip READ floatButtonToolTip NOTIFY floatButtonToolTipChanged)
    Q_PROPERTY(bool isFocused READ isFocused NOTIFY isFocusedChanged)
    Q_PROPERTY(bool maximizeUsesRestoreIcon READ maximizeUsesRestoreIcon NOTIFY maximizeButtonVisibleChanged)


public:
    explicit TitleBar(Core::TitleBar *controller, QQuickItem *parent = nullptr);
    ~TitleBar() override;

protected:
#ifdef DOCKS_DEVELOPER_MODE
    // These 3 just for unit-tests
    bool isCloseButtonEnabled() const override;
    bool isCloseButtonVisible() const override;
    bool isFloatButtonVisible() const override;
#endif

    QQuickItem *titleBarQmlItem() const;
    QQuickItem *titleBarMouseArea() const;
    void setTitleBarQmlItem(QQuickItem *);

    // QML interface
    bool isFocused() const;
    bool floatButtonVisible() const;
    bool minimizeButtonVisible() const;
    bool maximizeButtonVisible() const;
    bool maximizeUsesRestoreIcon() const;
    bool closeButtonEnabled() const;
    QString floatButtonToolTip() const;
    bool hasIcon() const;
    QString title() const;
    void setCloseButtonEnabled(bool);
    void setFloatButtonVisible(bool);

    Q_INVOKABLE bool onDoubleClicked();
    Q_INVOKABLE void onCloseClicked();
    Q_INVOKABLE void onFloatClicked();

    Q_INVOKABLE void onMinimizeClicked();
    Q_INVOKABLE void onAutoHideClicked();
    Q_INVOKABLE bool isFloating() const;
    Q_INVOKABLE bool isMaximized() const override;

    /// These 2 are the same, both just toggle window maximization
    Q_INVOKABLE void toggleMaximized();
    Q_INVOKABLE void onMaximizeClicked();

Q_SIGNALS:
    void titleBarQmlItemChanged();
    void titleChanged();
    void iconChanged();
    void isFocusedChanged();
    void closeButtonEnabledChanged(bool);
    void floatButtonVisibleChanged(bool);
    void maximizeButtonVisibleChanged(bool);
    void minimizeButtonVisibleChanged(bool);
    void floatButtonToolTipChanged(const QString &);

    /// Emitted when the number of dock widgets under this titlebar changes
    void numDockWidgetsChanged();

protected:
    void init() override;
    bool event(QEvent *) override;

private:
    QQuickItem *floatButton() const;
    QQuickItem *closeButton() const;

    QPointer<QQuickItem> m_titleBarQmlItem;
};

}

}

#endif
