/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"

namespace KDDockWidgets {

namespace Core {

class TitleBar;

/// @brief The interface that TitleBar views should implement
class DOCKS_EXPORT TitleBarViewInterface
{
public:
    explicit TitleBarViewInterface(Core::TitleBar *);
    virtual ~TitleBarViewInterface();
#ifdef DOCKS_TESTING_METHODS
    virtual bool isFloatButtonVisible() const = 0;
    virtual bool isCloseButtonVisible() const = 0;
    virtual bool isCloseButtonEnabled() const = 0;
#endif
protected:
    Core::TitleBar *const m_titleBar;
    TitleBarViewInterface(const TitleBarViewInterface &) = delete;
    TitleBarViewInterface &operator=(const TitleBarViewInterface &) = delete;
};

}

}
