/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "../qtcommon/Window_p.h"

namespace KDDockWidgets::QtQuick {

class DOCKS_EXPORT Window : public QtCommon::Window
{
public:
    using QtCommon::Window::Window;
    ~Window();
    std::shared_ptr<Core::View> rootView() const override;
    Window::Ptr transientParent() const override;
    void setVisible(bool) override;
    bool supportsHonouringLayoutMinSize() const override;

    Q_DISABLE_COPY(Window)
};

}
