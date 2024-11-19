/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_FLOATING_WINDOWQUICK_P_H
#define KD_FLOATING_WINDOWQUICK_P_H

#include "View.h"

QT_BEGIN_NAMESPACE
class QQuickView;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {
class FloatingWindow;
}

namespace QtQuick {

class MainWindow;
class TitleBar;
class DropArea;

class DOCKS_EXPORT FloatingWindow : public QtQuick::View
{
    Q_OBJECT
    Q_PROPERTY(QObject *titleBar READ titleBar CONSTANT)
    Q_PROPERTY(QObject *dropArea READ dropArea CONSTANT)
public:
    explicit FloatingWindow(Core::FloatingWindow *controller,
                            QtQuick::MainWindow *parent = nullptr,
                            Qt::WindowFlags flags = {});
    ~FloatingWindow();

    QSize minSize() const override;

    // QML interface
    QObject *titleBar() const;
    QObject *dropArea() const;

    Core::Item *rootItem() const;

protected:
    void setGeometry(QRect) override;

private:
    void onWindowStateChanged(Qt::WindowState);
    int contentsMargins() const;
    int titleBarHeight() const;
    QWindow *candidateParentWindow() const;
    void init() override final;
    QQuickView *const m_quickWindow;
    QQuickItem *m_visualItem = nullptr;
    Core::FloatingWindow *const m_controller;
    Q_DISABLE_COPY(FloatingWindow)
};

}

}

#endif
