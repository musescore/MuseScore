/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "../qtcommon/Window_p.h"

namespace KDDockWidgets {

namespace QtWidgets {

class DOCKS_EXPORT Window : public QtCommon::Window
{
public:
    using QtCommon::Window::Window;

    explicit Window(QWidget *topLevel);
    ~Window() override;
    std::shared_ptr<Core::View> rootView() const override;
    Window::Ptr transientParent() const override;
    void setGeometry(QRect) override;
    void setVisible(bool) override;
    bool supportsHonouringLayoutMinSize() const override;
    void destroy() override;

    Q_DISABLE_COPY(Window)
};

}
}
