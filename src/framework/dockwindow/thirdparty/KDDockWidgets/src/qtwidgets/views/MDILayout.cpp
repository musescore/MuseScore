/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MDILayout.h"
#include "kddockwidgets/core/MDILayout.h"
#include "core/View_p.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

MDILayout::MDILayout(Core::MDILayout *controller, Core::View *parent)
    : QtWidgets::View<QWidget>(controller, Core::ViewType::MDILayout, QtCommon::View_qt::asQWidget(parent))
    , m_controller(controller)
{
    Q_ASSERT(controller);
}

MDILayout::~MDILayout()
{
    if (!d->freed())
        m_controller->viewAboutToBeDeleted();
}
