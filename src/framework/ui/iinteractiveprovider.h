/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MUSE_UI_IINTERACTIVEPROVIDER_H
#define MUSE_UI_IINTERACTIVEPROVIDER_H

#include "global/modularity/imoduleinterface.h"

#include "global/types/uri.h"
#include "global/types/retval.h"
#include "global/progress.h"
#include "global/async/promise.h"

class QWindow;

namespace muse::ui {
class IInteractiveProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ILaunchProvider)

public:
    virtual ~IInteractiveProvider() = default;

    // color
    virtual RetVal<QColor> selectColor(const QColor& color = Qt::white, const QString& title = "") = 0;
    virtual bool isSelectColorOpened() const = 0;

    virtual RetVal<Val> open(const UriQuery& uri) = 0;
    virtual RetVal<Val> openSync(const UriQuery& uri) = 0;
    virtual async::Promise<Val> openAsync(const UriQuery& uri) = 0;
    virtual RetVal<bool> isOpened(const Uri& uri) const = 0;
    virtual RetVal<bool> isOpened(const UriQuery& uri) const = 0;
    virtual async::Channel<Uri> opened() const = 0;

    virtual void raise(const UriQuery& uri) = 0;

    virtual void close(const Uri& uri) = 0;
    virtual void close(const UriQuery& uri) = 0;
    virtual void closeAllDialogs() = 0;

    virtual ValCh<Uri> currentUri() const = 0;
    virtual RetVal<bool> isCurrentUriDialog() const = 0;
    virtual async::Notification currentUriAboutToBeChanged() const = 0;
    virtual std::vector<Uri> stack() const = 0;

    virtual QWindow* topWindow() const = 0;
    virtual bool topWindowIsWidget() const = 0;
};
}

#endif // MUSE_UI_IINTERACTIVEPROVIDER_H
