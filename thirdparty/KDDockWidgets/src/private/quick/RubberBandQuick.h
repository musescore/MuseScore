/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDDOCKWIDGETS_RUBBERBANDQUICK_H
#define KDDOCKWIDGETS_RUBBERBANDQUICK_H

#include "QWidgetAdapter.h"

namespace KDDockWidgets
{

class RubberBandQuick : public QWidgetAdapter
{
    Q_OBJECT
public:
    explicit RubberBandQuick(QQuickItem *parent = nullptr);
};

}

#endif
