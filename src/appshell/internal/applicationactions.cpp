//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "applicationactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::appshell;
using namespace mu::actions;
using namespace mu::shortcuts;

const ActionList ApplicationActions::m_actions = {
    ActionItem("quit",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Quit")
               ),
    ActionItem("online-handbook",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Online Handbook")
               ),
    ActionItem("ask-help",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Ask for Help")
               ),
    ActionItem("report-bug",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Report a Bug"),
               QT_TRANSLATE_NOOP("action", "Report a bug")
               ),
    ActionItem("leave-feedback",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Feedback"),
               QT_TRANSLATE_NOOP("action", "Leave feedback")
               )
};

const ActionItem& ApplicationActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}
