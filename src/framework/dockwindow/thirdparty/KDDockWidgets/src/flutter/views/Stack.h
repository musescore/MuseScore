/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/


#pragma once

#include "View.h"
#include "core/views/StackViewInterface.h"

namespace KDDockWidgets {
namespace flutter {

class DOCKS_EXPORT Stack : public View, public Core::StackViewInterface
{
public:
    explicit Stack(Core::Stack *controller, Core::View *parent = nullptr);

    bool isPositionDraggable(Point p) const override;
    void setDocumentMode(bool) override;

private:
    KDDW_DELETE_COPY_CTOR(Stack)
};

}
}
