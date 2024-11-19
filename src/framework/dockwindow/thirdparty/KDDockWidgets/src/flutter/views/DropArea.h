/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "View.h"

namespace KDDockWidgets {

namespace Core {
class DropArea;
}

namespace flutter {

class View;

class DOCKS_EXPORT DropArea : public flutter::View
{
public:
    explicit DropArea(Core::DropArea *, Core::View *parent);
    ~DropArea();

    flutter::View *indicatorWindow() const;

private:
    Core::DropArea *const m_dropArea;
};

}
}
