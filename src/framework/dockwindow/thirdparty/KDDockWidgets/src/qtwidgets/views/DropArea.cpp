/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DropArea.h"
#include "core/Utils_p.h"
#include "core/View_p.h"
#include "kddockwidgets/core/DropArea.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

DropArea::DropArea(Core::DropArea *dropArea, Core::View *parent)
    : QtWidgets::View<QWidget>(dropArea, Core::ViewType::DropArea, View_qt::asQWidget(parent))
    , m_dropArea(dropArea)
{
    Q_ASSERT(dropArea);
    if (isWayland()) {
        setAcceptDrops(true);
    }
}

DropArea::~DropArea()
{
    if (!d->freed())
        m_dropArea->viewAboutToBeDeleted();
}
