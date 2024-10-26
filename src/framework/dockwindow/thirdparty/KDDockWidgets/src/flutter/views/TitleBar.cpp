/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "TitleBar.h"

#include "core/DragController_p.h"
#include "core/Logging_p.h"
#include "core/TitleBar_p.h"
#include "kddockwidgets/core/Group.h"
#include "kddockwidgets/core/FloatingWindow.h"
#include "kddockwidgets/core/TitleBar.h"


using namespace KDDockWidgets;
using namespace KDDockWidgets::flutter;


TitleBar::TitleBar(Core::TitleBar *controller, Core::View *parent)
    : View(controller, Core::ViewType::TitleBar, parent)
    , Core::TitleBarViewInterface(controller)
{
}

TitleBar::~TitleBar()
{
}

void TitleBar::init()
{
    setFixedHeight(30);
    m_titleChangedConnection = m_titleBar->dptr()->titleChanged.connect([this] { onTitleBarChanged(m_titleBar->title()); });
}

void TitleBar::onTitleBarChanged(const QString &)
{
    KDDW_WARN("TitleBar::onTitleBarChanged: Implemented in dart instead");
}

#ifdef DOCKS_TESTING_METHODS
bool TitleBar::isCloseButtonEnabled() const
{
    return true;
}

bool TitleBar::isCloseButtonVisible() const
{
    return true;
}

bool TitleBar::isFloatButtonVisible() const
{
    return true;
}
#endif
