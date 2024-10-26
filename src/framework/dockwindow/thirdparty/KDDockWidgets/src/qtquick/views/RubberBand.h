/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_MULTISPLITTER_RUBBERBAND_QUICK_H
#define KD_MULTISPLITTER_RUBBERBAND_QUICK_H

#include "kddockwidgets/docks_export.h"
#include "View.h"

#include <QQuickItem>

namespace KDDockWidgets::QtQuick {

class DOCKS_EXPORT RubberBand : public View
{
    Q_OBJECT
public:
    explicit RubberBand(QQuickItem *parent);
};

}

#endif
