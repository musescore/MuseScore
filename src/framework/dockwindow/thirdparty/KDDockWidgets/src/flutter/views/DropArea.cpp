/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DropArea.h"
#include "ClassicDropIndicatorOverlay.h"
#include "ClassicIndicatorWindowViewInterface.h"
#include "kddockwidgets/core/DropArea.h"
#include "core/View_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;

DropArea::DropArea(Core::DropArea *dropArea, Core::View *parent)
    : flutter::View(dropArea, Core::ViewType::DropArea, parent)
    , m_dropArea(dropArea)
{
    assert(dropArea);
}

DropArea::~DropArea()
{
    m_inDtor = true;
    if (!d->freed())
        m_dropArea->viewAboutToBeDeleted();
}

flutter::View *DropArea::indicatorWindow() const
{
    auto overlay = static_cast<Core::ClassicDropIndicatorOverlay *>(m_dropArea->dropIndicatorOverlay());
    return dynamic_cast<flutter::View *>(overlay->indicatorWindow());
}
