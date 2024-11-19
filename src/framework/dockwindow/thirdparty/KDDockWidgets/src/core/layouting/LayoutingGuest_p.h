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

#include <kdbindings/signal.h>

namespace KDDockWidgets {

namespace Core {

class Item;
class LayoutingHost;

/// The interface graphical components need to implement in order to be hosted by a layout
/// The layout engine doesn't know about any GUI, only about LayoutingHost and LayoutingGuest
/// This allows to keep the layouting engine separate from the rest of KDDW and even
/// reused by non-KDDW projects

class DOCKS_EXPORT LayoutingGuest
{
public:
    LayoutingGuest();
    virtual ~LayoutingGuest();
    virtual Size minSize() const = 0;
    virtual Size maxSizeHint() const = 0;
    virtual void setGeometry(Rect r) = 0;
    virtual void setVisible(bool is) = 0;
    virtual Rect geometry() const = 0;
    virtual void setHost(LayoutingHost *parent) = 0;
    virtual LayoutingHost *host() const = 0;
    virtual QString id() const = 0;

    virtual bool freed() const
    {
        return false;
    }

    Core::Item *layoutItem() const;
    void setLayoutItem(Item *);
    virtual void setLayoutItem_impl(Core::Item *)
    {
    }

    virtual std::string toDebugString() const
    {
        return {};
    }

    KDBindings::Signal<LayoutingHost *> hostChanged;
    KDBindings::Signal<> beingDestroyed;
    KDBindings::Signal<> layoutInvalidated;

private:
    LayoutingGuest(const LayoutingGuest &) = delete;
    LayoutingGuest &operator=(const LayoutingGuest &) = delete;
    class Private;
    Private *const d;
};

}

}
