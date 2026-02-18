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

DropAreaWithCentralFrame::DropAreaWithCentralFrame(int ctx, QWidgetOrQuick *parent, MainWindowOptions options)
    : DropArea(ctx, parent)
    , m_centralFrame(createCentralFrame(ctx, options))
{
    if (m_centralFrame)
        addWidget(m_centralFrame, KDDockWidgets::Location_OnTop, {});
}

DropAreaWithCentralFrame::~DropAreaWithCentralFrame()
{
}

Frame *DropAreaWithCentralFrame::createCentralFrame(int ctx, MainWindowOptions options)
{
    Frame *frame = nullptr;
    if (options & MainWindowOption_HasCentralFrame) {
        FrameOptions frameOptions = FrameOption_IsCentralFrame;
        const bool hasPersistentCentralWidget = (options & MainWindowOption_HasCentralWidget) == MainWindowOption_HasCentralWidget;
        if (hasPersistentCentralWidget) {
            frameOptions |= FrameOption_NonDockable;
        } else {
            // With a persistent central widget we don't allow detaching it
            frameOptions |= FrameOption_AlwaysShowsTabs;
        }

        frame = Config::self(ctx).frameworkWidgetFactory()->createFrame(nullptr, frameOptions);
        frame->setObjectName(QStringLiteral("central frame"));
    }

    return frame;
}
