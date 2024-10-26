/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "kddockwidgets/docks_export.h"
#include "kddockwidgets/KDDockWidgets.h"

namespace KDDockWidgets {

namespace Core {

class LayoutingGuest;
class ItemContainer;

/// The interface graphical components need to implement in order to host a layout
/// The layout engine doesn't know about any GUI, only about LayoutingHost.
/// This allows to keep the layouting engine separate from the rest of KDDW and even
/// reused by non-KDDW projects.
/// For a normal KDDW project, the LayoutingHost is the Core::DropArea while the LayoutingGuest
/// is the Core::Group. Group contains tabs, each being a Core::DockWidget.
/// Each of these Core::* classes are rendered by their View::* counterparts, which for QtWidgets
/// frontend are a QWidget derived class.

class DOCKS_EXPORT LayoutingHost
{
public:
    LayoutingHost() = default;
    virtual ~LayoutingHost();

    /// Weather this layout host supports min size constraints or not
    virtual bool supportsHonouringLayoutMinSize() const = 0;

    void insertItem(Core::LayoutingGuest *guest, KDDockWidgets::Location loc,
                    const InitialOption &initialOption = {});

    void insertItemRelativeTo(Core::LayoutingGuest *guest, Core::LayoutingGuest *relativeTo, Location loc,
                              const InitialOption &initialOption = {});

    Core::ItemContainer *m_rootItem = nullptr;

private:
    LayoutingHost(const LayoutingHost &) = delete;
    LayoutingHost &operator=(const LayoutingHost &) = delete;
};

}

}
