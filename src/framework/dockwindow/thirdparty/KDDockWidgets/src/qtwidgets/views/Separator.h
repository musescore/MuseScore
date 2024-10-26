/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_SEPARATOR_QTWIDGETS_H
#define KD_SEPARATOR_QTWIDGETS_H

#pragma once

#include "View.h"
#include "kddockwidgets/Qt5Qt6Compat_p.h"


namespace KDDockWidgets::Core {
class Separator;
}

namespace KDDockWidgets::QtWidgets {

class DOCKS_EXPORT Separator : public View<QWidget>
{
    Q_OBJECT
public:
    explicit Separator(Core::Separator *controller, Core::View *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(KDDockWidgets::Qt5Qt6Compat::QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

private:
    Core::Separator *const m_controller;
};

}

#endif
