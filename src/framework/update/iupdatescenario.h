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
#ifndef MUSE_UPDATE_IUPDATESCENARIO_H
#define MUSE_UPDATE_IUPDATESCENARIO_H

#include "types/ret.h"
#include "async/promise.h"

#include "modularity/imoduleinterface.h"

namespace muse::update {
class IUpdateScenario : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUpdateScenario)

public:
    using CheckForUpdateCompleteCallback = std::function<void ()>;

    virtual ~IUpdateScenario() = default;
    virtual bool needCheckForUpdate() const = 0;
    virtual muse::async::Promise<Ret> checkForUpdate(bool manual) = 0;

    virtual bool hasUpdate() const = 0;
    virtual muse::Ret showUpdate() = 0;
};
}

#endif // MUSE_UPDATE_IUPDATESCENARIO_H
