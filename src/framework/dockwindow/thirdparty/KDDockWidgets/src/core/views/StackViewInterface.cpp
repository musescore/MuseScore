/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "StackViewInterface.h"
#include "kddockwidgets/core/Stack.h"


namespace KDDockWidgets::Core {

StackViewInterface::StackViewInterface(Stack *controller)
    : m_stack(controller)
{
}

StackViewInterface::~StackViewInterface() = default;

bool StackViewInterface::isPositionDraggable(Point) const
{
    return true;
}

void StackViewInterface::setDocumentMode(bool)
{
    /// Only QtWidgets reimplements this
}

}
