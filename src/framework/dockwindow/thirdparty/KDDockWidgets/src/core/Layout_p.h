/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "Layout.h"
#include "layouting/LayoutingHost_p.h"
#include "kdbindings/signal.h"

namespace KDDockWidgets::Core {

class Layout::Private : public LayoutingHost
{
public:
    explicit Private(Layout *);
    ~Private() override;
    bool supportsHonouringLayoutMinSize() const override;

    Layout *const q;
    bool m_inResizeEvent = false;
    KDBindings::ConnectionHandle m_minSizeChangedHandler;

    /// @brief Emitted when the count of visible widgets changes
    KDBindings::Signal<int> visibleWidgetCountChanged;

    bool m_viewDeleted = false;
};

}
