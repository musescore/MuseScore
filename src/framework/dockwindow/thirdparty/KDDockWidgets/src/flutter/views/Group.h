/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "View.h"
#include "core/views/GroupViewInterface.h"
#include "TitleBar.h"

namespace KDDockWidgets {

namespace Core {
class Group;
class DockWidget;
}

namespace flutter {

class Stack;

class DOCKS_EXPORT Group : public View, public Core::GroupViewInterface
{
public:
    explicit Group(Core::Group *controller, Core::View *parent = nullptr);
    ~Group() override;

    /// @reimp
    Size minSize() const override;

    /// @reimp
    Size maxSizeHint() const override;

    Rect dragRect() const override;
    int currentIndex() const;

protected:
    int nonContentsHeight() const override;
};

}
}
