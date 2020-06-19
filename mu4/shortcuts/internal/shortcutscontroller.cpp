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
#include "shortcutscontroller.h"

#include "log.h"

using namespace mu::shortcuts;
using namespace mu::actions;

void ShortcutsController::activate(const std::string& sequence)
{
    LOGD() << "activate: " << sequence;

    std::list<Shortcut> shortcuts = shortcutsRegister()->shortcutsForSequence(sequence);
    IF_ASSERT_FAILED(!shortcuts.empty()) {
        return;
    }

    //! NOTE One shortcut for one action
    if (shortcuts.size() == 1) {
        dispatcher()->dispatch(shortcuts.front().action);
        return;
    }

    //! NOTE One shortcut for several action
    for (const Shortcut& sc: shortcuts) {
        LOGI() << sc.action;
    }
}
