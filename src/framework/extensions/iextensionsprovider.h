/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#pragma once

#include "modularity/imoduleinterface.h"

#include "global/types/ret.h"
#include "global/async/notification.h"
#include "global/async/channel.h"

#include "extensionstypes.h"

namespace muse::extensions {
class IExtensionsProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsProvider)
public:

    virtual ~IExtensionsProvider() = default;

    virtual void reloadExtensions() = 0;
    virtual Ret removeExtension(const Uri& uri) = 0;

    virtual ManifestList manifestList(Filter filter = Filter::All) const = 0;
    virtual async::Notification manifestListChanged() const = 0;

    virtual bool exists(const Uri& uri) const = 0;
    virtual const Manifest& manifest(const Uri& uri) const = 0;
    virtual async::Channel<Manifest> manifestChanged() const = 0;
    virtual Action action(const UriQuery& q) const = 0;

    virtual KnownCategories knownCategories() const = 0;

    virtual Ret perform(const UriQuery& uri) = 0;
    virtual Ret run(const UriQuery& uri) = 0;

    virtual Ret setExecPoint(const Uri& uri, const ExecPointName& name) = 0;
    virtual std::vector<ExecPoint> execPoints(const Uri& uri) const = 0;
    virtual Ret performPoint(const ExecPointName& name) = 0;
    virtual void performPointAsync(const ExecPointName& name) = 0;
};
}
