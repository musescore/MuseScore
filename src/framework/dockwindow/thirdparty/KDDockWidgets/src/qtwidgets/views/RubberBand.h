/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_RUBBERBAND_QTWIDGETS_H
#define KD_RUBBERBAND_QTWIDGETS_H

#pragma once

#include "View.h"

#include <QRubberBand>

namespace KDDockWidgets {

namespace QtWidgets {

class DOCKS_EXPORT RubberBand : public View<QRubberBand>
{
    Q_OBJECT
public:
    explicit RubberBand(QWidget *parent = nullptr);
    ~RubberBand() override;
};

}

}

#endif
