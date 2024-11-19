/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/DropIndicatorOverlay.h"

namespace KDDockWidgets {

namespace Core {

/**
 * @brief A dummy DropIndicatorOverlay implementation which doesn't do anything.
 *
 * Used for debugging purposes or if someone doesn't want the drop indicators.
 */
class DOCKS_EXPORT NullDropIndicatorOverlay : public DropIndicatorOverlay
{
    Q_OBJECT
public:
    explicit NullDropIndicatorOverlay(Core::DropArea *);
    ~NullDropIndicatorOverlay() override;
    DropLocation hover_impl(Point) override
    {
        return {};
    }

    DropLocation dropLocationForPos(Point) const
    {
        return {};
    }

protected:
    Point posForIndicator(DropLocation) const override
    {
        return {};
    }
};

}

}
