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
using namespace mu::ui;

const ActionList ApplicationActions::m_actions = {
    ActionItem("quit",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Quit")
               ),
    ActionItem("fullscreen",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Full Screen"),
               QT_TRANSLATE_NOOP("action", "Full screen")
               ),
    ActionItem("about",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "About...")
               ),
    ActionItem("about-qt",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "About Qt...")
               ),
    ActionItem("about-musicxml",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "About MusicXML...")
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
               ),
    ActionItem("revert-factory",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Revert to Factory Settings"),
               QT_TRANSLATE_NOOP("action", "Revert to factory settings")
               ),
    ActionItem("toggle-mixer",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Mixer"),
               QT_TRANSLATE_NOOP("action", "Toggle mixer"),
               IconCode::Code::MIXER
               ),
    ActionItem("toggle-navigator",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Navigator"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Navigator'")
               ),
    ActionItem("toggle-palette",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Palettes"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Palettes'")
               ),
    ActionItem("toggle-instruments",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Instruments"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Instruments'")
               ),
    ActionItem("inspector",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Inspector"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Inspector'")
               ),
    ActionItem("toggle-statusbar",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Status Bar"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Status Bar'")
               ),
    ActionItem("toggle-noteinput",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Note Input"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Note Input' toolbar")
               ),
    ActionItem("toggle-notationtoolbar",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Notation Toolbar"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Notation' toolbar")
               ),
    ActionItem("toggle-undoredo",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Undo/Redo Toolbar"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Undo/Redo' toolbar")
               ),
    ActionItem("toggle-transport",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Playback Controls"),
               QT_TRANSLATE_NOOP("action", "Toggle Playback Controls toolbar")
               ),
    ActionItem("preference-dialog",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Preferences"),
               QT_TRANSLATE_NOOP("action", "Open preferences dialog")
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

const ActionCodeList ApplicationActions::actionCodes(ShortcutContext context)
{
    ActionCodeList codes;
    for (const ActionItem& action : m_actions) {
        if (action.shortcutContext == context) {
            codes.push_back(action.code);
        }
    }

    return codes;
}
