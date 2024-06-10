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

#ifndef MUSE_DOCK_DOCKWINDOWACTIONSCONTROLLER_H
#define MUSE_DOCK_DOCKWINDOWACTIONSCONTROLLER_H

#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "../idockwindowprovider.h"

namespace muse::dock {
class DockWindowActionsController : public muse::Injectable, public muse::actions::Actionable
{
    muse::Inject<IDockWindowProvider> dockWindowProvider = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    DockWindowActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

private:
    void setDockOpen(const muse::actions::ActionData& args);
    void toggleOpened(const muse::actions::ActionData& args);
    void toggleFloating(const muse::actions::ActionData& args);

    void restoreDefaultLayout();

    IDockWindow* window() const;
};
}

#endif // MUSE_DOCK_DOCKWINDOWACTIONSCONTROLLER_H
