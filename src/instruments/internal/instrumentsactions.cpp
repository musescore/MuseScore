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

#include "instrumentsactions.h"

using namespace mu::instruments;
using namespace mu::actions;
using namespace mu::shortcuts;

const ActionList InstrumentsActions::m_actions = {
    Action("instruments",
           QT_TRANSLATE_NOOP("action", "Instruments..."),
           ShortcutContext::Any
           )
};

const Action& InstrumentsActions::action(const ActionName& actionName) const
{
    for (const Action& action : m_actions) {
        if (action.name == actionName) {
            return action;
        }
    }

    static Action null;
    return null;
}
