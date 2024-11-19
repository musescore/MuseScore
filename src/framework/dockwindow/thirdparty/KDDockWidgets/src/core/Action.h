/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/QtCompat_p.h"

/// Class to abstract QAction, so code still works with QtQuick and Flutter

namespace KDDockWidgets {

namespace Core {
class DockWidget;

class DOCKS_EXPORT Action
{
public:
    explicit Action(Core::DockWidget *, const char *debugName = "");
    virtual ~Action();

    virtual void setIcon(const KDDockWidgets::Icon &) = 0;
    virtual KDDockWidgets::Icon icon() const = 0;

    virtual void setText(const QString &text) = 0;

    virtual void setToolTip(const QString &text) = 0;
    virtual QString toolTip() const = 0;

    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;

    virtual bool isChecked() const = 0;
    virtual void setChecked(bool checked) = 0;

    virtual bool blockSignals(bool) = 0;

#ifdef DOCKS_DEVELOPER_MODE
    // Only used by QtWidget tests
    virtual void trigger()
    {
    }
#endif

    bool enabled() const;
    void toggle();

    class Private;
    Private *const d;

    Action(const Action &) = delete;
    Action &operator=(const Action &) = delete;
};

}

}
