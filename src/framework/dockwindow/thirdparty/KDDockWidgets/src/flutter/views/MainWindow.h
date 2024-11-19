/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "View.h"
#include "kddockwidgets/core/views/MainWindowViewInterface.h"

namespace KDDockWidgets {

namespace flutter {

class DOCKS_EXPORT MainWindow : public flutter::View, public Core::MainWindowViewInterface
{
public:
    explicit MainWindow(const QString &uniqueName, MainWindowOptions options = {},
                        flutter::View *parent = nullptr, Qt::WindowFlags flags = {});

    ~MainWindow() override;

protected:
    Margins centerWidgetMargins() const override;
    Rect centralAreaGeometry() const override;
    void setContentsMargins(int left, int top, int right, int bottom) override;
};

}

}
