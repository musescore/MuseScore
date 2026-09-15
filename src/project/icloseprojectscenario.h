/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include "modularity/imoduleinterface.h"
#include "async/notification.h"
#include "async/promise.h"
#include "types/ret.h"

#include "types/projecttypes.h"

namespace mu::project {
class ICloseProjectScenario : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(ICloseProjectScenario)

public:
    virtual ~ICloseProjectScenario() = default;

    //! NOTE Resolves once the score is gone, or with an error if it is still there:
    //! the user kept it, the save it needed failed, or a close is already under way
    virtual muse::async::Promise<muse::Ret> closeOpenedProject(bool goToHome = true) = 0;

    virtual bool isBusy(BusyStatus status) const = 0;
    virtual muse::async::Notification busyChanged() const = 0;
};
}
