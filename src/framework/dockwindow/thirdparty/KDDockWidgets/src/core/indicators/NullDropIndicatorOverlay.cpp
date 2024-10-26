/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "NullDropIndicatorOverlay.h"

namespace KDDockWidgets::Core {

NullDropIndicatorOverlay::NullDropIndicatorOverlay(Core::DropArea *dropArea)
    : DropIndicatorOverlay(dropArea)
{
}

NullDropIndicatorOverlay::~NullDropIndicatorOverlay() = default;

}
