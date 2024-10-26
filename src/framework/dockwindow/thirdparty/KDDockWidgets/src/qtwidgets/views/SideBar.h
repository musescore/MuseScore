/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_SIDEBARWIDGET_P_H
#define KD_SIDEBARWIDGET_P_H

#include "View.h"
#include "kddockwidgets/docks_export.h"
#include <kddockwidgets/core/views/SideBarViewInterface.h>

#include <QToolButton>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QBoxLayout;
class QAbstractButton;
QT_END_NAMESPACE

namespace KDDockWidgets {

namespace Core {
class SideBar;
}

namespace QtWidgets {
class SideBar;
}

class DOCKS_EXPORT SideBarButton : public QToolButton
{
    Q_OBJECT
public:
    explicit SideBarButton(Core::DockWidget *dw, QtWidgets::SideBar *parent);
    ~SideBarButton() override;

protected:
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;

private:
    bool isVertical() const;

    friend class QtWidgets::SideBar;
    class Private;
    Private *const d;
};

namespace QtWidgets {

class DOCKS_EXPORT SideBar : public View<QWidget>, public Core::SideBarViewInterface
{
    Q_OBJECT
public:
    explicit SideBar(Core::SideBar *, QWidget *parent);

protected:
    void addDockWidget_Impl(Core::DockWidget *dock) override;
    void removeDockWidget_Impl(Core::DockWidget *dock) override;

    // virtual so users can provide their own buttons
    virtual SideBarButton *createButton(Core::DockWidget *dw,
                                        SideBar *parent) const;

private:
    void init() override;

    QBoxLayout *m_layout = nullptr;
};
}

}

#endif
