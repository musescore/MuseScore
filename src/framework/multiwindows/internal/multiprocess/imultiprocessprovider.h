/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

 #include <string>
 #include <vector>

 #include "global/modularity/imoduleinterface.h"
 #include "global/async/notification.h"

namespace muse::mi {
struct InstanceMeta
{
    std::string id;
    bool isServer = false;
};

class IMultiProcessProvider : MODULE_GLOBAL_EXPORT_INTERFACE
{
    INTERFACE_ID(IMultiProcessProvider);
public:
    virtual ~IMultiProcessProvider() = default;

    virtual const std::string& selfID() const = 0;
    virtual bool isMainInstance() const = 0;
    virtual std::vector<InstanceMeta> instances() const = 0;
    virtual async::Notification instancesChanged() const = 0;
};
}
