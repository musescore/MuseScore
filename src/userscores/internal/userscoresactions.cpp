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
#include "userscoresactions.h"

using namespace mu::userscores;
using namespace mu::actions;
using namespace mu::shortcuts;

const ActionList UserScoresActions::m_actions = {
    ActionItem("file-open",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Open..."),
               QT_TRANSLATE_NOOP("action", "Load score from file")
               ),
    ActionItem("file-new",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "New..."),
               QT_TRANSLATE_NOOP("action", "Create new score")
               ),
    ActionItem("file-close",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Close"),
               QT_TRANSLATE_NOOP("action", "Close current score")
               ),
    ActionItem("file-save",
               ShortcutContext::NotationNeedSave,
               QT_TRANSLATE_NOOP("action", "Save"),
               QT_TRANSLATE_NOOP("action", "Save score to file")
               ),
    ActionItem("file-save-online",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Save Online..."),
               QT_TRANSLATE_NOOP("action", "Save score on musescore.com")
               ),
    ActionItem("file-save-as",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Save As..."),
               QT_TRANSLATE_NOOP("action", "Save score under a new file name")
               ),
    ActionItem("file-save-a-copy",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Save a Copy..."),
               QT_TRANSLATE_NOOP("action", "Save a copy of the score in addition to the current file")
               ),
    ActionItem("file-save-selection",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Save Selection..."),
               QT_TRANSLATE_NOOP("action", "Save current selection as new score")
               ),
    ActionItem("file-export",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Export..."),
               QT_TRANSLATE_NOOP("action", "Save a copy of the score in various formats")
               ),
    ActionItem("file-import",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Import PDF..."),
               QT_TRANSLATE_NOOP("action", "Import a PDF file with an experimental service on musescore.com")
               ),
    ActionItem("print",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Print..."),
               QT_TRANSLATE_NOOP("action", "Print score/part")
               ),
    ActionItem("clear-recent",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Clear Recent Files")
               )
};

const ActionItem& UserScoresActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}

const ActionCodeList UserScoresActions::actionCodes(ShortcutContext context)
{
    ActionCodeList codes;
    for (const ActionItem& action : m_actions) {
        if (action.shortcutContext == context) {
            codes.push_back(action.code);
        }
    }

    return codes;
}
