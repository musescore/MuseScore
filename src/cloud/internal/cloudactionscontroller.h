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
#ifndef MU_CLOUD_CLOUDACTIONSCONTROLLER_H
#define MU_CLOUD_CLOUDACTIONSCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "ui/iuiactionsregister.h"
#include "async/asyncable.h"

namespace mu::cloud {
class CloudActionsController : public actions::Actionable, public async::Asyncable
{
    INJECT(cloud, context::IGlobalContext, globalContext)
    INJECT(cloud, actions::IActionsDispatcher, dispatcher)

public:
    void init();

private:
    void uploadScore();
};
}

#endif // MU_CLOUD_CLOUDACTIONSCONTROLLER_H
