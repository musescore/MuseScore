/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_DROPAREA_QTQUICK_H
#define KD_DROPAREA_QTQUICK_H

#pragma once

#include "View.h"

namespace KDDockWidgets {

namespace Core {
class DropArea;
}

namespace QtQuick {

class DOCKS_EXPORT DropArea : public QtQuick::View
{
    Q_OBJECT
public:
    explicit DropArea(Core::DropArea *, Core::View *parent);
    ~DropArea();

private:
    Core::DropArea *const m_dropArea;
};

}

}

#endif
