/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief A MultiSplitter with support for drop indicators when hovering over.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#ifndef KD_DROPAREA_QTWIDGETS_H
#define KD_DROPAREA_QTWIDGETS_H
#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

#include "kddockwidgets/qtwidgets/views/View.h"

namespace KDDockWidgets {

namespace Core {
class DropArea;
}

namespace QtWidgets {

class DOCKS_EXPORT DropArea : public QtWidgets::View<QWidget>
{
    Q_OBJECT
public:
    explicit DropArea(Core::DropArea *, Core::View *parent);
    ~DropArea();

private:
    Core::DropArea *const m_dropArea;
};

}

}

#endif
