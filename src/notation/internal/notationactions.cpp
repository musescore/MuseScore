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
    ActionItem("file-open",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Open..."),
               QT_TRANSLATE_NOOP("action", "Load score from file")
               ),
    ActionItem("file-new",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "New Score"),
               QT_TRANSLATE_NOOP("action", "Create new score")
               ),
    ActionItem("file-save",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Save Score"),
               QT_TRANSLATE_NOOP("action", "Save score to file")
               ),
    ActionItem("file-save-as",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Save Score As"),
               QT_TRANSLATE_NOOP("action", "Save score under a new file name")
               ),
    ActionItem("escape",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Esc")
               ),
    ActionItem("put-note", // args: QPoint pos, bool replace, bool insert
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Put Note")
               ),
    ActionItem("next-element",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Next Element"),
               QT_TRANSLATE_NOOP("action", "Accessibility: Next element")
               ),
    ActionItem("prev-element",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous Element"),
               QT_TRANSLATE_NOOP("action", "Accessibility: Previous element")
               ),
    ActionItem("next-chord",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Next Chord"),
               QT_TRANSLATE_NOOP("action", "Go to next chord or move text right")
               ),
    ActionItem("prev-chord",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous Chord"),
               QT_TRANSLATE_NOOP("action", "Go to previous chord or move text left")
               ),
    ActionItem("next-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Next Measure"),
               QT_TRANSLATE_NOOP("action","Go to next measure or move text right")
               ),
    ActionItem("prev-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous Measure"),
               QT_TRANSLATE_NOOP("action","Go to previous measure or move text left")

               ),
    ActionItem("next-track",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Next staff or voice"),
               QT_TRANSLATE_NOOP("action","Next staff or voice")

               ),
    ActionItem("prev-track",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous staff or voice"),
               QT_TRANSLATE_NOOP("action","Previous staff or voice")

               ),
    ActionItem("pitch-up",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Up"),
               QT_TRANSLATE_NOOP("action","Pitch up or move text or articulation up")

               ),
    ActionItem("pitch-down",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Down"),
               QT_TRANSLATE_NOOP("action","Pitch down or move text or articulation down")
               ),
    ActionItem("pitch-down-octave",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Down Octave"),
               QT_TRANSLATE_NOOP("action","Pitch down by an octave or move text or articulation down")
               ),
    ActionItem("pitch-up-octave",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Up Octave"),
               QT_TRANSLATE_NOOP("action","Pitch up by an octave or move text or articulation up")
               ),
    ActionItem("cut",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Cut")
               ),
    ActionItem("copy",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Copy")
               ),
    ActionItem("paste",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Paste")
               ),
    ActionItem("paste-half",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Paste Half Duration")
               ),
    ActionItem("paste-double",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Paste Double Duration")
               ),
    ActionItem("paste-special",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Paste Special")
               ),
    ActionItem("swap",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Swap with Clipboard")
               ),
    ActionItem("select-all",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select All")
               ),
    ActionItem("delete",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Delete"),
               IconCode::Code::DELETE_TANK
               ),
    ActionItem("select-similar",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select: similar")
               ),
    ActionItem("select-similar-staff",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select: in same staff")
               ),
    ActionItem("select-similar-range",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select: in the range")
               ),
    ActionItem("select-dialog",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select Dialog")
               ),
    ActionItem("edit-style",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Style")
               ),
    ActionItem("page-settings",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Page Settings")
               ),
    ActionItem("load-style",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Load Style")
               ),
    ActionItem("transpose",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Transpose")
               ),
    ActionItem("parts",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Parts"),
               QT_TRANSLATE_NOOP("action","Manage parts")
               ),
    ActionItem("view-mode-page",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Page View")
               ),
    ActionItem("view-mode-continuous",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Continuous View")
               ),
    ActionItem("view-mode-single",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Single Page")
               ),
    ActionItem("find",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Find")
               ),
    ActionItem("staff-properties",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Staff/Part Properties")
               ),
    ActionItem("add-remove-breaks",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add/remove system breaks")
               ),
    ActionItem("edit-info",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Score Properties")
               ),
    ActionItem("undo",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Undo"),
               QT_TRANSLATE_NOOP("action","Undo last change")
               ),
    ActionItem("redo",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Redo"),
               QT_TRANSLATE_NOOP("action","Redo last undo")
               ),
    ActionItem("voice-x12",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-2")
               ),
    ActionItem("voice-x13",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-3")
               ),
    ActionItem("voice-x14",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-4")
               ),
    ActionItem("voice-x23",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 2-3")
               ),
    ActionItem("voice-x24",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 2-4")
               ),
    ActionItem("voice-x34",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 3-4")
               ),
    ActionItem("split-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Split measure"),
               QT_TRANSLATE_NOOP("action","Split measure before selected note/rest")
               ),
    ActionItem("join-measures",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Join Selected Measures")
               ),
    ActionItem("insert-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert Measure")
               ),
    ActionItem("insert-measures",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert Measures")
               ),
    ActionItem("append-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Append Measure")
               ),
    ActionItem("append-measures",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Append Measures")
               ),
    ActionItem("insert-hbox",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert Horizontal Frame")
               ),
    ActionItem("insert-vbox",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert Vertical Frame")
               ),
    ActionItem("insert-textframe",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert Text Frame")
               ),
    ActionItem("append-hbox",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Append Horizontal Frame")
               ),
    ActionItem("append-vbox",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Append Vertical Frame")
               ),
    ActionItem("append-textframe",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Append Text Frame")
               ),
    ActionItem("interval1",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Unison Above")
               ),
    ActionItem("interval2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Second Above")
               ),
    ActionItem("interval3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Third Above")
               ),
    ActionItem("interval4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fourth Above")
               ),
    ActionItem("interval5",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fifth Above")
               ),
    ActionItem("interval6",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Sixth Above")
               ),
    ActionItem("interval7",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Seventh Above")
               ),
    ActionItem("interval8",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Octave Above")
               ),
    ActionItem("interval9",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Ninth Above")
               ),
    ActionItem("interval-2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Second Below")
               ),
    ActionItem("interval-3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Third Below")
               ),
    ActionItem("interval-4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fourth Below")
               ),
    ActionItem("interval-5",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fifth Below")
               ),
    ActionItem("interval-6",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Sixth Below")
               ),
    ActionItem("interval-7",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Seventh Below")
               ),
    ActionItem("interval-8",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Octave Below")
               ),
    ActionItem("interval-9",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Ninth Below")
               ),
    ActionItem("note-c",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "C")
               ),
    ActionItem("note-d",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "D")
               ),
    ActionItem("note-e",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "E")
               ),
    ActionItem("note-f",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "F")
               ),
    ActionItem("note-g",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "G")
               ),
    ActionItem("note-a",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "A")
               ),
    ActionItem("note-b",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "B")
               ),
    ActionItem("chord-c",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add C to Chord")
               ),
    ActionItem("chord-d",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add D to Chord")
               ),
    ActionItem("chord-e",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add E to Chord")
               ),
    ActionItem("chord-f",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add F to Chord")
               ),
    ActionItem("chord-g",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add G to Chord")
               ),
    ActionItem("chord-a",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add A to Chord")
               ),
    ActionItem("chord-b",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add B to Chord")
               ),
    ActionItem("insert-c",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert C")
               ),
    ActionItem("insert-d",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert D")
               ),
    ActionItem("insert-e",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert E")
               ),
    ActionItem("insert-f",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert F")
               ),
    ActionItem("insert-g",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert G")
               ),
    ActionItem("insert-a",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert A")
               ),
    ActionItem("insert-b",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert B")
               ),
    ActionItem("add-8va",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Ottava 8va alta")
               ),
    ActionItem("add-8vb",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Ottava 8va bassa")
               ),
    ActionItem("add-hairpin",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Crescendo")
               ),
    ActionItem("add-hairpin-reverse",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Decrescendo")
               ),
    ActionItem("add-noteline",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Note Anchored Line")
               ),
    ActionItem("title-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Title")
               ),
    ActionItem("subtitle-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Subtitle")
               ),
    ActionItem("composer-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Composer")
               ),
    ActionItem("poet-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Lirycist")
               ),
    ActionItem("part-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Part Name")
               ),
    ActionItem("system-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "System Text")
               ),
    ActionItem("staff-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Staff Text")
               ),
    ActionItem("expression-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Expression Text")
               ),
    ActionItem("rehearsalmark-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Rehearsal Mark")
               ),
    ActionItem("instrument-change-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Instrument Change")
               ),
    ActionItem("fingering-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fingering")
               ),
    ActionItem("sticking-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Sticking")
               ),
    ActionItem("chord-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Chord Symbol")
               ),
    ActionItem("roman-numeral-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Roman Numeral Analysis")
               ),
    ActionItem("nashville-number-text",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Nashville Number")
               ),
    ActionItem("lyrics",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Lyrics")
               ),
    ActionItem("figured-bass",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Figured Bass")
               ),
    ActionItem("tempo",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tempo Marking")
               ),
    ActionItem("duplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Duplet")
               ),
    ActionItem("triplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Triplet")
               ),
    ActionItem("quadruplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quadruplet")
               ),
    ActionItem("quintuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quintuplet")
               ),
    ActionItem("sextuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "sextuplet")
               ),
    ActionItem("septuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Septuplet")
               ),
    ActionItem("octuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Octuplet")
               ),
    ActionItem("nonuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Nontuplet")
               ),
    ActionItem("tuplet-dialog",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Other")
               )
};

const ActionList NotationActions::m_noteInputActions = {
    ActionItem("note-input",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Default (Step time)"),
               QT_TRANSLATE_NOOP("action","Enter notes with a mouse or keyboard"),
               IconCode::Code::EDIT
               ),
    ActionItem("note-input-rhythm",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Rhythm only (not pitch)"),
               QT_TRANSLATE_NOOP("action","Enter durations with a single click or keypress"),
               IconCode::Code::RHYTHM_ONLY
               ),
    ActionItem("note-input-repitch",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Re-pitch existing notes"),
               QT_TRANSLATE_NOOP("action","Replace pitches without changing rhythms"),
               IconCode::Code::RE_PITH
               ),
    ActionItem("note-input-realtime-auto",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Real-time (metronome)"),
               QT_TRANSLATE_NOOP("action","Enter notes at a fixed tempo indicated by a metronome beat"),
               IconCode::Code::METRONOME
               ),
    ActionItem("note-input-realtime-manual",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Real-time (foot pedal)"),
               QT_TRANSLATE_NOOP("action","Enter notes while tapping a key or pedal to set the tempo"),
               IconCode::Code::FOOT_PEDAL
               ),
    ActionItem("note-input-timewise",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert"),
               QT_TRANSLATE_NOOP("action","Insert notes by increasing measure duration"),
               IconCode::Code::NOTE_PLUS
               ),
    ActionItem("note-longa",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Longo"),
               IconCode::Code::LONGO
               ),
    ActionItem("note-breve",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double whole note"),
               IconCode::Code::NOTE_WHOLE_DOUBLE
               ),
    ActionItem("pad-note-1",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Whole note"),
               IconCode::Code::NOTE_WHOLE
               ),
    ActionItem("pad-note-2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Half note"),
               IconCode::Code::NOTE_HALF
               ),
    ActionItem("pad-note-4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quarter note"),
               IconCode::Code::NOTE_QUARTER
               ),
    ActionItem("pad-note-8",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "8th note"),
               IconCode::Code::NOTE_8TH
               ),
    ActionItem("pad-note-16",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "16th note"),
               IconCode::Code::NOTE_16TH
               ),
    ActionItem("pad-note-32",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "32th note"),
               IconCode::Code::NOTE_32TH
               ),
    ActionItem("pad-note-64",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "64th note"),
               IconCode::Code::NOTE_64TH
               ),
    ActionItem("pad-note-128",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "128th note"),
               IconCode::Code::NOTE_128TH
               ),
    ActionItem("pad-note-256",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "256th note"),
               IconCode::Code::NOTE_256TH
               ),
    ActionItem("pad-note-512",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "512th note"),
               IconCode::Code::NOTE_512TH
               ),
    ActionItem("pad-note-1024",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "1024th note"),
               IconCode::Code::NOTE_1024TH
               ),
    ActionItem("pad-dot",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Dotted note"),
               IconCode::Code::NOTE_DOTTED
               ),
    ActionItem("pad-dotdot",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double dotted note"),
               IconCode::Code::NOTE_DOTTED_2
               ),
    ActionItem("pad-dot3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Triple dotted note"),
               IconCode::Code::NOTE_DOTTED_3
               ),
    ActionItem("pad-dot4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quadruple dotted note"),
               IconCode::Code::NOTE_DOTTED_4
               ),
    ActionItem("pad-rest",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Rest"),
               IconCode::Code::REST
               ),
    ActionItem("flat",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Flat"),
               IconCode::Code::FLAT
               ),
    ActionItem("flat2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double flat"),
               IconCode::Code::FLAT_DOUBLE
               ),
    ActionItem("nat",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Natural"),
               IconCode::Code::NATURAL
               ),
    ActionItem("sharp",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Sharp"),
               IconCode::Code::SHARP
               ),
    ActionItem("sharp2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double sharp"),
               IconCode::Code::SHARP_DOUBLE
               ),
    ActionItem("tie",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tie"),
               IconCode::Code::NOTE_TIE
               ),
    ActionItem("add-slur",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Slur"),
               IconCode::Code::NOTE_SLUR
               ),
    ActionItem("add-marcato",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Marcato"),
               IconCode::Code::MARCATO
               ),
    ActionItem("add-sforzato", // TODO
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Accent"),
               IconCode::Code::ACCENT
               ),
    ActionItem("add-tenuto",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tenuto"),
               IconCode::Code::TENUTO
               ),
    ActionItem("add-staccato",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Staccato"),
               IconCode::Code::STACCATO
               ),
    ActionItem("tuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tuplet"),
               IconCode::Code::NOTE_TUPLET
               ),
    ActionItem("voice-1",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 1"),
               IconCode::Code::VOICE_1
               ),
    ActionItem("voice-2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 2"),
               IconCode::Code::VOICE_2
               ),
    ActionItem("voice-3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 3"),
               IconCode::Code::VOICE_3
               ),
    ActionItem("voice-4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 4"),
               IconCode::Code::VOICE_4
               ),
    ActionItem("flip",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Flip"),
               IconCode::Code::NOTE_FLIP
               ),
};

const ActionItem& NotationActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    for (const ActionItem& action : m_noteInputActions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}

ActionList NotationActions::defaultNoteInputActions()
{
    return m_noteInputActions;
}

DurationType NotationActions::actionDurationType(const ActionCode& actionCode)
{
    static QMap<actions::ActionCode, DurationType> durations = {
        { "note-longa", DurationType::V_LONG },
        { "note-breve", DurationType::V_BREVE },
        { "pad-note-1", DurationType::V_WHOLE },
        { "pad-note-2", DurationType::V_HALF },
        { "pad-note-4", DurationType::V_QUARTER },
        { "pad-note-8", DurationType::V_EIGHTH },
        { "pad-note-16", DurationType::V_16TH },
        { "pad-note-32", DurationType::V_32ND },
        { "pad-note-64", DurationType::V_64TH },
        { "pad-note-128", DurationType::V_128TH },
        { "pad-note-256", DurationType::V_256TH },
        { "pad-note-512", DurationType::V_512TH },
        { "pad-note-1024", DurationType::V_1024TH }
    };

    DurationType type = DurationType::V_INVALID;
    if (durations.contains(actionCode)) {
        type = durations[actionCode];
    }

    return type;
}

AccidentalType NotationActions::actionAccidentalType(const ActionCode& actionCode)
{
    static QMap<actions::ActionCode, AccidentalType> accidentals = {
        { "flat2", AccidentalType::FLAT2 },
        { "flat", AccidentalType::FLAT },
        { "nat", AccidentalType::NATURAL },
        { "sharp", AccidentalType::SHARP },
        { "sharp2", AccidentalType::SHARP2 }
    };

    AccidentalType type = AccidentalType::NONE;
    if (accidentals.contains(actionCode)) {
        type = accidentals[actionCode];
    }

    return type;
}

int NotationActions::actionDotCount(const ActionCode& actionCode)
{
    static QMap<actions::ActionCode, int> dots = {
        { "pad-dot", 1 },
        { "pad-dotdot", 2 },
        { "pad-dot3", 3 },
        { "pad-dot4", 4 }
    };

    int dotCount = 0;
    if (dots.contains(actionCode)) {
        dotCount = dots[actionCode];
    }

    return dotCount;
}

int NotationActions::actionVoice(const ActionCode& actionCode)
{
    QMap<actions::ActionCode, int> voices {
        { "voice-1", 0 },
        { "voice-2", 1 },
        { "voice-3", 2 },
        { "voice-4", 3 }
    };

    int voice = 0;
    if (voices.contains(actionCode)) {
        voice = voices[actionCode];
    }

    return voice;
}

SymbolId NotationActions::actionArticulationSymbolId(const ActionCode& actionCode)
{
    static QMap<actions::ActionCode, SymbolId> articulations {
        { "add-marcato", SymbolId::articMarcatoAbove },
        { "add-sforzato", SymbolId::articAccentAbove },
        { "add-tenuto", SymbolId::articTenutoAbove },
        { "add-staccato", SymbolId::articStaccatoAbove }
    };

    SymbolId symbolId = SymbolId::noSym;
    if (articulations.contains(actionCode)) {
        symbolId = articulations[actionCode];
    }

    return symbolId;
}
