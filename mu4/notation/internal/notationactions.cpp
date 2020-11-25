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

const ActionList NotationActions::m_actions = {
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
    Action("paste-half",
           QT_TRANSLATE_NOOP("action", "Paste Half Duration"),
           ShortcutContext::NotationActive
           ),
    Action("paste-double",
           QT_TRANSLATE_NOOP("action", "Paste Double Duration"),
           ShortcutContext::NotationActive
           ),
    Action("paste-special",
           QT_TRANSLATE_NOOP("action", "Paste Special"),
           ShortcutContext::NotationActive
           ),
    Action("swap",
           QT_TRANSLATE_NOOP("action", "Swap with Clipboard"),
           ShortcutContext::NotationActive
           ),
    Action("select-all",
           QT_TRANSLATE_NOOP("action", "Select All"),
           ShortcutContext::NotationActive
           ),
    Action("delete",
           QT_TRANSLATE_NOOP("action", "Delete"),
           ShortcutContext::NotationActive,
           IconCode::Code::DELETE_TANK
           ),
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
    Action("transpose",
           QT_TRANSLATE_NOOP("action", "Transpose"),
           ShortcutContext::NotationActive
           ),
    Action("parts",
           QT_TRANSLATE_NOOP("action", "Parts"),
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
           ),
    Action("add-remove-breaks",
           QT_TRANSLATE_NOOP("action", "Add/remove system breaks"),
           ShortcutContext::NotationActive
           ),
    Action("edit-info",
           QT_TRANSLATE_NOOP("action", "Score Properties"),
           ShortcutContext::NotationActive
           ),
    Action("undo",
           QT_TRANSLATE_NOOP("action", "Undo"),
           ShortcutContext::NotationActive
           ),
    Action("redo",
           QT_TRANSLATE_NOOP("action", "Redo"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x12",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 1-2"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x13",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 1-3"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x14",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 1-4"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x23",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 2-3"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x24",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 2-4"),
           ShortcutContext::NotationActive
           ),
    Action("voice-x34",
           QT_TRANSLATE_NOOP("action", "Exchange Voice 3-4"),
           ShortcutContext::NotationActive
           )
};

const ActionList NotationActions::m_noteInputActions = {
    Action("note-input",
           QT_TRANSLATE_NOOP("action", "Default (Step time)"),
           ShortcutContext::NotationActive,
           IconCode::Code::EDIT
           ),
    Action("note-input-rhythm",
           QT_TRANSLATE_NOOP("action", "Rhythm only (not pitch)"),
           ShortcutContext::NotationActive,
           IconCode::Code::RHYTHM_ONLY
           ),
    Action("note-input-repitch",
           QT_TRANSLATE_NOOP("action", "Re-pitch existing notes"),
           ShortcutContext::NotationActive,
           IconCode::Code::RE_PITH
           ),
    Action("note-input-realtime-auto",
           QT_TRANSLATE_NOOP("action", "Real-time (metronome)"),
           ShortcutContext::NotationActive,
           IconCode::Code::METRONOME
           ),
    Action("note-input-realtime-manual",
           QT_TRANSLATE_NOOP("action", "Real-time (foot pedal)"),
           ShortcutContext::NotationActive,
           IconCode::Code::FOOT_PEDAL
           ),
//    Action("note-input-timewise", // TODO Insert (shifts notation to the right)
//           QT_TRANSLATE_NOOP("action", "Re-pitch existing notes "),
//           ShortcutContext::NotationActive,
//           IconCode::Code::NOTE_TO_RIGHT
//           ),
//    Action("note-input-repitch", // TODO Insert (extends measure)
//           QT_TRANSLATE_NOOP("action", "Re-pitch existing notes "),
//           ShortcutContext::NotationActive,
//           IconCode::Code::NOTE_PLUS
//           ),
    Action("note-longa",
           QT_TRANSLATE_NOOP("action", "Longo"),
           ShortcutContext::NotationActive,
           IconCode::Code::LONGO
           ),
    Action("note-breve",
           QT_TRANSLATE_NOOP("action", "Double whole note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_WHOLE_DOUBLE
           ),
    Action("pad-note-1",
           QT_TRANSLATE_NOOP("action", "Whole note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_WHOLE
           ),
    Action("pad-note-2",
           QT_TRANSLATE_NOOP("action", "Half note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_HALF
           ),
    Action("pad-note-4",
           QT_TRANSLATE_NOOP("action", "Quarter note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_QUARTER
           ),
    Action("pad-note-8",
           QT_TRANSLATE_NOOP("action", "8th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_8TH
           ),
    Action("pad-note-16",
           QT_TRANSLATE_NOOP("action", "16th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_16TH
           ),
    Action("pad-note-32",
           QT_TRANSLATE_NOOP("action", "32th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_32TH
           ),
    Action("pad-note-64",
           QT_TRANSLATE_NOOP("action", "64th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_64TH
           ),
    Action("pad-note-128",
           QT_TRANSLATE_NOOP("action", "128th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_128TH
           ),
    Action("pad-note-256",
           QT_TRANSLATE_NOOP("action", "256th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_256TH
           ),
    Action("pad-note-512",
           QT_TRANSLATE_NOOP("action", "512th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_512TH
           ),
    Action("pad-note-1024",
           QT_TRANSLATE_NOOP("action", "1024th note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_1024TH
           ),
    Action("pad-dot",
           QT_TRANSLATE_NOOP("action", "Dotted note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_DOTTED
           ),
    Action("pad-dotdot",
           QT_TRANSLATE_NOOP("action", "Double dotted note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_DOTTED_2
           ),
    Action("pad-dot3",
           QT_TRANSLATE_NOOP("action", "Triple dotted note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_DOTTED_3
           ),
    Action("pad-dot4",
           QT_TRANSLATE_NOOP("action", "Quadruple dotted note"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_DOTTED_4
           ),
    Action("pad-rest",
           QT_TRANSLATE_NOOP("action", "Rest"),
           ShortcutContext::NotationActive,
           IconCode::Code::REST
           ),
    Action("flat2",
           QT_TRANSLATE_NOOP("action", "Double flat"),
           ShortcutContext::NotationActive,
           IconCode::Code::FLAT_DOUBLE
           ),
    Action("flat",
           QT_TRANSLATE_NOOP("action", "Flat"),
           ShortcutContext::NotationActive,
           IconCode::Code::FLAT
           ),
    Action("nat",
           QT_TRANSLATE_NOOP("action", "Natural"),
           ShortcutContext::NotationActive,
           IconCode::Code::NATURAL
           ),
    Action("sharp",
           QT_TRANSLATE_NOOP("action", "Sharp"),
           ShortcutContext::NotationActive,
           IconCode::Code::SHARP
           ),
    Action("sharp2",
           QT_TRANSLATE_NOOP("action", "Double sharp"),
           ShortcutContext::NotationActive,
           IconCode::Code::SHARP_DOUBLE
           ),
    Action("tie",
           QT_TRANSLATE_NOOP("action", "Tie"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_TIE
           ),
    Action("add-slur",
           QT_TRANSLATE_NOOP("action", "Slur"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_SLUR
           ),
    Action("add-marcato",
           QT_TRANSLATE_NOOP("action", "Marcato"),
           ShortcutContext::NotationActive,
           IconCode::Code::MARCATO
           ),
    Action("add-sforzato", // TODO
           QT_TRANSLATE_NOOP("action", "Accent"),
           ShortcutContext::NotationActive,
           IconCode::Code::ACCENT
           ),
    Action("add-tenuto",
           QT_TRANSLATE_NOOP("action", "Tenuto"),
           ShortcutContext::NotationActive,
           IconCode::Code::TENUTO
           ),
    Action("add-staccato",
           QT_TRANSLATE_NOOP("action", "Staccato"),
           ShortcutContext::NotationActive,
           IconCode::Code::STACCATO
           ),
    Action("tuplet",
           QT_TRANSLATE_NOOP("action", "Tuplet"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_TUPLET
           ),
    Action("voice-1",
           QT_TRANSLATE_NOOP("action", "Voice 1"),
           ShortcutContext::NotationActive,
           IconCode::Code::VOICE_1
           ),
    Action("voice-2",
           QT_TRANSLATE_NOOP("action", "Voice 2"),
           ShortcutContext::NotationActive,
           IconCode::Code::VOICE_2
           ),
    Action("voice-3",
           QT_TRANSLATE_NOOP("action", "Voice 3"),
           ShortcutContext::NotationActive,
           IconCode::Code::VOICE_3
           ),
    Action("voice-4",
           QT_TRANSLATE_NOOP("action", "Voice 4"),
           ShortcutContext::NotationActive,
           IconCode::Code::VOICE_4
           ),
    Action("flip",
           QT_TRANSLATE_NOOP("action", "Flip"),
           ShortcutContext::NotationActive,
           IconCode::Code::NOTE_FLIP
           )
};

const Action& NotationActions::action(const ActionName& name) const
{
    for (const Action& action : m_actions) {
        if (action.name == name) {
            return action;
        }
    }

    for (const Action& action : m_noteInputActions) {
        if (action.name == name) {
            return action;
        }
    }

    static Action null;
    return null;
}

ActionList NotationActions::defaultNoteInputActions()
{
    return m_noteInputActions;
}
