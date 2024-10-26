/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_SIDEBAR_QTQUICK_H
#define KD_SIDEBAR_QTQUICK_H

#pragma once

#include "View.h"
#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/core/views/SideBarViewInterface.h"

#include <QToolButton>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QBoxLayout;
class QAbstractButton;
QT_END_NAMESPACE

namespace KDDockWidgets {

class Group;

namespace Core {
class SideBar;
}

namespace QtQuick {
class SideBar;
}

class SideBarButton : public QToolButton
{
    Q_OBJECT
public:
    explicit SideBarButton(Core::DockWidget *dw, QtQuick::SideBar *parent);
    bool isVertical() const;
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    QtQuick::SideBar *const m_sideBar;
    const QPointer<Core::DockWidget> m_dockWidget;
};

namespace QtQuick {

class DOCKS_EXPORT SideBar : public QtQuick::View<QQuickItem>, public Core::SideBarViewInterface
{
    Q_OBJECT
public:
    explicit SideBar(Core::SideBar *, QQuickItem *parent);

    void init() override;
    bool isVertical() const;

    void addDockWidget_Impl(Core::DockWidget *) override;
    void removeDockWidget_Impl(Core::DockWidget *) override;

    // virtual so users can provide their own buttons
    virtual SideBarButton *createButton(Core::DockWidget *dw, SideBar *parent) const;

private:
    QBoxLayout *m_layout = nullptr;
};
}

}

#endif
