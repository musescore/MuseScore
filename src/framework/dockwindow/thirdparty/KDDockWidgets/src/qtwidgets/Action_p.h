/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KDDOCKWIDGETS_ACTION_QTWIDGETS_H
#define KDDOCKWIDGETS_ACTION_QTWIDGETS_H
#pragma once

#include "kddockwidgets/core/Action.h"

#include <QAction>

namespace KDDockWidgets {

namespace QtWidgets {

class DOCKS_EXPORT Action : public QAction, public Core::Action
{
    Q_OBJECT
public:
    explicit Action(Core::DockWidget *, const char *debugName = "");
    ~Action() override;

    void setIcon(const KDDockWidgets::Icon &) override;
    KDDockWidgets::Icon icon() const override;

    void setText(const QString &) override;

    void setToolTip(const QString &txt) override;
    QString toolTip() const override;

    void setEnabled(bool) override;
    bool isEnabled() const override;

    bool isChecked() const override;
    void setChecked(bool) override;

    bool blockSignals(bool) override;

#ifdef DOCKS_DEVELOPER_MODE
    void trigger() override;
#endif

private:
    bool m_lastCheckedState = false;
};

}

}

#endif
