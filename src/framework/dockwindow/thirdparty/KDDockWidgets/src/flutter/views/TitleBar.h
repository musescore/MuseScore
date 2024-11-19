/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "core/views/TitleBarViewInterface.h"
#include "View.h"

#include <kdbindings/signal.h>

namespace KDDockWidgets {


namespace Core {
class TitleBar;
}

namespace flutter {

class DOCKS_EXPORT TitleBar : public View, public Core::TitleBarViewInterface
{
public:
    explicit TitleBar(Core::TitleBar *controller, Core::View *parent = nullptr);
    ~TitleBar() override;

    /// implemented by flutter
    virtual void onTitleBarChanged(const QString &);


protected:
#ifdef DOCKS_TESTING_METHODS
    // These 3 just for unit-tests
    bool isCloseButtonEnabled() const override;
    bool isCloseButtonVisible() const override;
    bool isFloatButtonVisible() const override;
#endif
protected:
    void init() override;

private:
    KDBindings::ScopedConnection m_titleChangedConnection;
};

}

}
