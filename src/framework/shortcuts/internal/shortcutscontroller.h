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
#ifndef MUSE_SHORTCUTS_SHORTCUTSCONTROLLER_H
#define MUSE_SHORTCUTS_SHORTCUTSCONTROLLER_H

#include "../ishortcutscontroller.h"

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveprovider.h"
#include "ui/iuicontextresolver.h"
#include "ishortcutsregister.h"
#include "shortcutcontext.h"

namespace muse::shortcuts {
class ShortcutsController : public IShortcutsController, public async::Asyncable
{
    INJECT(IShortcutsRegister, shortcutsRegister)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(muse::ui::IUiActionsRegister, aregister)
    INJECT(muse::ui::IInteractiveProvider, interactiveProvider)
    INJECT(muse::ui::IUiContextResolver, uiContextResolver)

    //! NOTE May be missing because it must be implemented outside the framework
    INJECT(IShortcutContextPriority, shortcutContextPriority)

public:
    ShortcutsController() = default;

    void init();

    void activate(const std::string& sequence) override;
    bool isRegistered(const std::string& sequence) const override;

private:
    muse::actions::ActionCode resolveAction(const std::string& sequence) const;
};
}

#endif // MUSE_SHORTCUTS_SHORTCUTSCONTROLLER_H
