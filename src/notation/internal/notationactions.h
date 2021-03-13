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
#ifndef MU_NOTATION_NOTATIONACTIONS_H
#define MU_NOTATION_NOTATIONACTIONS_H

#include "actions/imoduleactions.h"
#include "notationtypes.h"
#include "shortcuts/shortcutstypes.h"

namespace mu::notation {
class NotationActions : public actions::IModuleActions
{
public:

    const actions::ActionItem& action(const actions::ActionCode& actionCode) const override;

    static const actions::ActionCodeList actionCodes(shortcuts::ShortcutContext context);
    static actions::ActionList defaultNoteInputActions();

    static DurationType actionDurationType(const actions::ActionCode& actionCode);
    static AccidentalType actionAccidentalType(const actions::ActionCode& actionCode);
    static int actionDotCount(const actions::ActionCode& actionCode);
    static int actionVoice(const actions::ActionCode& actionCode);
    static SymbolId actionArticulationSymbolId(const actions::ActionCode& actionCode);

private:
    static const actions::ActionList m_actions;
    static const actions::ActionList m_noteInputActions;
};
}

#endif // MU_NOTATION_NOTATIONACTIONS_H
