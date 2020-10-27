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
#include "notationactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::shortcuts;
using namespace mu::framework;

//! NOTE Only actions processed by notation

const std::vector<Action> NotationActions::m_actions = {
    Action("file-open",
           QT_TRANSLATE_NOOP("action", "Open..."),
           ShortcutContext::Any
           ),
    Action("file-new",
           QT_TRANSLATE_NOOP("action", "New Score"),
           ShortcutContext::Any
           ),
    Action("file-save",
           QT_TRANSLATE_NOOP("action", "Save Score"),
           ShortcutContext::Any
           ),
    Action("file-save-as",
           QT_TRANSLATE_NOOP("action", "Save Score As"),
           ShortcutContext::Any
           ),
    Action("note-input",
           QT_TRANSLATE_NOOP("action", "Note Input"),
           ShortcutContext::NotationActive
           ),
    Action("pad-note-4",
           QT_TRANSLATE_NOOP("action", "4th"),
           ShortcutContext::NotationActive
           ),
    Action("pad-note-8",
           QT_TRANSLATE_NOOP("action", "8th"),
           ShortcutContext::NotationActive
           ),
    Action("pad-note-16",
           QT_TRANSLATE_NOOP("action", "16th"),
           ShortcutContext::NotationActive
           ),
    Action("put-note", // args: QPoint pos, bool replace, bool insert
           QT_TRANSLATE_NOOP("action", "Put Note"),
           ShortcutContext::NotationActive
           ),
    Action("next-element",
           QT_TRANSLATE_NOOP("action", "Next Element"),
           ShortcutContext::NotationActive
           ),
    Action("prev-element",
           QT_TRANSLATE_NOOP("action", "Previous Element"),
           ShortcutContext::NotationActive
           ),
    Action("next-chord",
           QT_TRANSLATE_NOOP("action", "Next Chord"),
           ShortcutContext::NotationActive
           ),
    Action("prev-chord",
           QT_TRANSLATE_NOOP("action", "Previous Chord"),
           ShortcutContext::NotationActive
           ),
    Action("next-measure",
           QT_TRANSLATE_NOOP("action", "Next Measure"),
           ShortcutContext::NotationActive
           ),
    Action("prev-measure",
           QT_TRANSLATE_NOOP("action", "Previous Measure"),
           ShortcutContext::NotationActive
           ),
    Action("next-track",
           QT_TRANSLATE_NOOP("action", "Next staff or voice"),
           ShortcutContext::NotationActive
           ),
    Action("prev-track",
           QT_TRANSLATE_NOOP("action", "Previous staff or voice"),
           ShortcutContext::NotationActive
           ),
    Action("pitch-up",
           QT_TRANSLATE_NOOP("action", "Up"),
           ShortcutContext::NotationActive
           ),
    Action("pitch-down",
           QT_TRANSLATE_NOOP("action", "Down"),
           ShortcutContext::NotationActive
           ),
    Action("pitch-down-octave",
           QT_TRANSLATE_NOOP("action", "Down Octave"),
           ShortcutContext::NotationActive
           ),
    Action("pitch-up-octave",
           QT_TRANSLATE_NOOP("action", "Up Octave"),
           ShortcutContext::NotationActive
           ),
    Action("cut",
           QT_TRANSLATE_NOOP("action", "Cut"),
           ShortcutContext::NotationActive
           ),
    Action("copy",
           QT_TRANSLATE_NOOP("action", "Copy"),
           ShortcutContext::NotationActive
           ),
    Action("paste",
           QT_TRANSLATE_NOOP("action", "Paste"),
           ShortcutContext::NotationActive
           ),
    Action("swap",
           QT_TRANSLATE_NOOP("action", "Swap"),
           ShortcutContext::NotationActive
           ),
    Action("delete",
           QT_TRANSLATE_NOOP("action", "Delete"),
           ShortcutContext::NotationActive,
           IconCode::Code::DELETE_TANK),
    Action("edit-style",
           QT_TRANSLATE_NOOP("action", "Style"),
           ShortcutContext::NotationActive
           ),
    Action("page-settings",
           QT_TRANSLATE_NOOP("action", "Page Settings"),
           ShortcutContext::NotationActive
           ),
    Action("load-style",
           QT_TRANSLATE_NOOP("action", "Load Style"),
           ShortcutContext::NotationActive
           ),
    Action("view-mode-page",
           QT_TRANSLATE_NOOP("action", "Page View"),
           ShortcutContext::NotationActive),
    Action("view-mode-continuous",
           QT_TRANSLATE_NOOP("action", "Continuous View"),
           ShortcutContext::NotationActive
           ),
    Action("view-mode-single",
           QT_TRANSLATE_NOOP("action", "Single Page"),
           ShortcutContext::NotationActive
           ),
    Action("find",
           QT_TRANSLATE_NOOP("action", "Find"),
           ShortcutContext::NotationActive
           ),
    Action("staff-properties",
           QT_TRANSLATE_NOOP("action", "Staff/Part Properties"),
           ShortcutContext::NotationActive
           )
};

const Action& NotationActions::action(const ActionName& name) const
{
    for (const Action& a : m_actions) {
        if (a.name == name) {
            return a;
        }
    }

    static Action null;
    return null;
}
