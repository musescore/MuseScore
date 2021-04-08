/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "DropAreaWithCentralFrame_p.h"
#include "Config.h"
#include "FrameworkWidgetFactory.h"

using namespace KDDockWidgets;

DropAreaWithCentralFrame::DropAreaWithCentralFrame(QWidgetOrQuick *parent, MainWindowOptions options)
    : DropArea(parent)
    , m_centralFrame(createCentralFrame(options))
{
    if (m_centralFrame)
        addWidget(m_centralFrame, KDDockWidgets::Location_OnTop, {});
}

DropAreaWithCentralFrame::~DropAreaWithCentralFrame()
{
}

Frame* DropAreaWithCentralFrame::createCentralFrame(MainWindowOptions options)
{
    Frame *frame = nullptr;
    if (options & MainWindowOption_HasCentralFrame) {
        frame = Config::self().frameworkWidgetFactory()->createFrame(nullptr, FrameOptions() | FrameOption_IsCentralFrame | FrameOption_AlwaysShowsTabs);
        frame->setObjectName(QStringLiteral("central frame"));
    }

    return frame;
}
