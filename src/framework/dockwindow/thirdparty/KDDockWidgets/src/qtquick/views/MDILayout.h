/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_MDILAYOUT_QTQUICK_H
#define KD_MDILAYOUT_QTQUICK_H

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

#include "View.h"

namespace KDDockWidgets {
namespace Core {
class MDILayout;
}

namespace QtQuick {

class DOCKS_EXPORT MDILayout : public QtQuick::View
{
    Q_OBJECT
public:
    explicit MDILayout(Core::MDILayout *controller, Core::View *parent);
    ~MDILayout();

private:
    void setParent(Core::View *parent) override;
    Core::MDILayout *const m_controller;
};

}

}

#endif
