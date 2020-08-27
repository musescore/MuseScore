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

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::shortcuts;

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
