//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_SHORTCUTS_SHORTCUTSCONTROLLER_H
#define MU_SHORTCUTS_SHORTCUTSCONTROLLER_H

#include "../ishortcutscontroller.h"
#include "modularity/ioc.h"
#include "ishortcutsregister.h"
#include "actions/iactionsdispatcher.h"
#include "actions/iactionsregister.h"

namespace mu::shortcuts {
class ShortcutsController : public IShortcutsController
{
    INJECT(shortcuts, IShortcutsRegister, shortcutsRegister)
    INJECT(shortcuts, actions::IActionsDispatcher, dispatcher)
    INJECT(shortcuts, actions::IActionsRegister, aregister)

public:
    ShortcutsController() = default;

    void activate(const std::string& sequence) override;
};
}

#endif // MU_SHORTCUTS_SHORTCUTSCONTROLLER_H
