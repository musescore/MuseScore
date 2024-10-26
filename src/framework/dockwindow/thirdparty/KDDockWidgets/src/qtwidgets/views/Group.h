/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_GROUP_QTWIDGETS_H
#define KD_GROUP_QTWIDGETS_H

#pragma once

#include "View.h"
#include <kddockwidgets/core/views/GroupViewInterface.h>

namespace KDDockWidgets::Core {
class Group;
}

namespace KDDockWidgets::QtWidgets {

class DOCKS_EXPORT Group : public View<QWidget>, public Core::GroupViewInterface
{
    Q_OBJECT
public:
    explicit Group(Core::Group *controller, QWidget *parent = nullptr);
    void init() override;

    int nonContentsHeight() const override;
    QRect dragRect() const override;

Q_SIGNALS:
    void numDockWidgetsChanged();
    void isInMainWindowChanged();
    void isFocusedChanged();

protected:
    void paintEvent(QPaintEvent *) override;
    QSize maxSizeHint() const override;
};

}

#endif
