/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "core/Screen_p.h"


namespace KDDockWidgets::flutter {

/// Class to represent a hardware screen. For now only returns dummy values
/// We'll see what to do after flutter has multi-window
class Screen : public Core::Screen
{
public:
    ~Screen() override;
    QString name() const override;
    Size size() const override;
    Rect geometry() const override;
    double devicePixelRatio() const override;
    Size availableSize() const override;
    Rect availableGeometry() const override;
    Size virtualSize() const override;
    Rect virtualGeometry() const override;
    bool equals(std::shared_ptr<Core::Screen> other) const override;
};

}
