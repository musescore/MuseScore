/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MDILayout.h"
#include "core/View_p.h"
#include "kddockwidgets/core/MDILayout.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

MDILayout::MDILayout(Core::MDILayout *controller, Core::View *parent)
    : QtQuick::View(controller, Core::ViewType::MDILayout, asQQuickItem(parent))
    , m_controller(controller)
{
    Q_ASSERT(controller);
}

MDILayout::~MDILayout()
{
    if (!Core::View::d->freed())
        m_controller->viewAboutToBeDeleted();
}

void MDILayout::setParent(Core::View *parent)
{
    QtQuick::View::setParent(parent);
    if (parent) {
        makeItemFillParent(this);
    }
}
