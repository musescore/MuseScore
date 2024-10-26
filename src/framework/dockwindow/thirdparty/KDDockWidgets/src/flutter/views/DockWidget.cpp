/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DockWidget.h"
#include "core/ViewFactory.h"

#include "kddockwidgets/core/TitleBar.h"
#include "kddockwidgets/core/DockWidget.h"
#include "core/DockWidget_p.h"
#include "core/Logging_p.h"
#include "kddockwidgets/core/Group.h"
#include "flutter/Platform.h"
#include "flutter/views/TitleBar.h"
#include "flutter/views/Group.h"
#include "flutter/ViewFactory.h"
#include "flutter/views/ViewWrapper_p.h"

#include <Config.h>

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;


DockWidget::DockWidget(const QString &uniqueName, DockWidgetOptions options,
                       LayoutSaverOptions layoutSaverOptions)
    : View(new Core::DockWidget(this, uniqueName, options, layoutSaverOptions), Core::ViewType::DockWidget,
           nullptr)
    , Core::DockWidgetViewInterface(asDockWidgetController())
{
    init();
    m_dockWidget->init();

    m_dockWidget->dptr()->guestViewChanged.connect([this] {
        if (auto guest = m_dockWidget->guestView()) {
            guest->setVisible(true);
        }
    });
}

DockWidget::~DockWidget()
{
}

void DockWidget::init()
{
}

Size DockWidget::minSize() const
{
    if (auto guestWidget = dockWidget()->guestView()) {
        // The guests min-size is the same as the widget's, there's no spacing or margins.
        return guestWidget->minSize();
    }

    return View::minSize();
}

Size DockWidget::maxSizeHint() const
{
    if (auto guestWidget = dockWidget()->guestView()) {
        // The guests max-size is the same as the widget's, there's no spacing or margins.
        return guestWidget->maxSizeHint();
    }

    return View::maxSizeHint();
}

Core::DockWidget *DockWidget::dockWidget() const
{
    return m_dockWidget;
}

std::shared_ptr<Core::View> DockWidget::focusCandidate() const
{
    return ViewWrapper::create(const_cast<flutter::DockWidget *>(this));
}
