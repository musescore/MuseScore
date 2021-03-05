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

#include "paletteactions.h"

#include "shortcuts/shortcutstypes.h"

using namespace mu::palette;
using namespace mu::actions;
using namespace mu::shortcuts;

const mu::actions::ActionList PaletteActions::m_actions = {
    ActionItem("masterpalette",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Master Palette")
               ),
};

const ActionItem& PaletteActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}

const ActionCodeList PaletteActions::actionCodes(ShortcutContext context)
{
    ActionCodeList codes;
    for (const ActionItem& action : m_actions) {
        if (action.shortcutContext == context) {
            codes.push_back(action.code);
        }
    }

    return codes;
}
