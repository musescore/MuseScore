/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDDW_VIEW_GUARD_H
#define KDDW_VIEW_GUARD_H

#include "kddockwidgets/docks_export.h"

#include "kdbindings/signal.h"

namespace KDDockWidgets {

namespace Core {
class View;

/// @brief This class provides a weak reference to a view
/// i.e., it becomes null automatically once a View is destroyed
class DOCKS_EXPORT ViewGuard
{
public:
    ViewGuard(View *v);
    ViewGuard(const ViewGuard &);
    ~ViewGuard();

    operator bool() const;
    View *operator->();
    const View *operator->() const;
    void clear();
    bool isNull() const;
    View *view() const;

    ViewGuard &operator=(View *);
    ViewGuard &operator=(const ViewGuard &);

private:
    void setView(View *);
    View *v = nullptr;
    KDBindings::ConnectionHandle m_onDestroy;
};

}
}

#endif
