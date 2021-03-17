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
using namespace mu::ui;

//! NOTE Only actions processed by notation

const ActionList NotationActions::m_actions = {
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
               QT_TRANSLATE_NOOP("action", "Go to next measure or move text right")
               ),
    ActionItem("prev-measure",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous Measure"),
               QT_TRANSLATE_NOOP("action", "Go to previous measure or move text left")
               ),
    ActionItem("next-track",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Next staff or voice"),
               QT_TRANSLATE_NOOP("action", "Next staff or voice")
               ),
    ActionItem("prev-track",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Previous staff or voice"),
               QT_TRANSLATE_NOOP("action", "Previous staff or voice")
               ),
    ActionItem("select-next-chord",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add next chord to selection"),
               QT_TRANSLATE_NOOP("action", "Add next chord to selection")
               ),
    ActionItem("select-prev-chord",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Add previous chord to selection"),
               QT_TRANSLATE_NOOP("action", "Add previous chord to selection")
               ),
    ActionItem("pitch-up",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Up"),
               QT_TRANSLATE_NOOP("action", "Pitch up or move text or articulation up")
               ),
    ActionItem("pitch-down",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Down"),
               QT_TRANSLATE_NOOP("action", "Pitch down or move text or articulation down")
               ),
    ActionItem("pitch-down-octave",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Down Octave"),
               QT_TRANSLATE_NOOP("action", "Pitch down by an octave or move text or articulation down")
               ),
    ActionItem("pitch-up-octave",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Up Octave"),
               QT_TRANSLATE_NOOP("action", "Pitch up by an octave or move text or articulation up")
               ),
    ActionItem("cut",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Cut")
               ),
    ActionItem("copy",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Copy")
               ),
    ActionItem("paste",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Paste")
               ),
    ActionItem("paste-half",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Paste Half Duration"),
               QT_TRANSLATE_NOOP("action", "Paste half duration")
               ),
    ActionItem("paste-double",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Paste Double Duration"),
               QT_TRANSLATE_NOOP("action", "Paste double duration")
               ),
    ActionItem("paste-special",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Paste Special"),
               QT_TRANSLATE_NOOP("action", "Paste special")
               ),
    ActionItem("swap",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Swap with Clipboard"),
               QT_TRANSLATE_NOOP("action", "Swap with clipboard")
               ),
    ActionItem("select-all",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select All"),
               QT_TRANSLATE_NOOP("action", "Select all")
               ),
    ActionItem("select-section",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Select Section"),
               QT_TRANSLATE_NOOP("action", "Select section")
               ),
    ActionItem("select-similar",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Select: similar"),
               QT_TRANSLATE_NOOP("action", "Select all similar elements")
               ),
    ActionItem("select-similar-staff",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Select: in same staff"),
               QT_TRANSLATE_NOOP("action", "Select all similar elements in same staff")
               ),
    ActionItem("select-similar-range",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Select: in the range"),
               QT_TRANSLATE_NOOP("action", "Select all similar elements in the range selection")
               ),
    ActionItem("select-dialog",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Select Dialog"),
               QT_TRANSLATE_NOOP("action", "Select all similar elements with more options")
               ),
    ActionItem("delete",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Delete"),
               QT_TRANSLATE_NOOP("action", "Delete the selected element(s)"),
               IconCode::Code::DELETE_TANK
               ),
    ActionItem("edit-style",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Style..."),
               QT_TRANSLATE_NOOP("action", "Edit style")
               ),
    ActionItem("page-settings",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Page Settings..."),
               QT_TRANSLATE_NOOP("action", "Page settings")
               ),
    ActionItem("load-style",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action","Load Style..."),
               QT_TRANSLATE_NOOP("action","Load style")
               ),
    ActionItem("transpose",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "&Transpose..."),
               QT_TRANSLATE_NOOP("action", "Transpose")
               ),
    ActionItem("explode",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Explode"),
               QT_TRANSLATE_NOOP("action", "Explode contents of top selected staff into staves below")
               ),
    ActionItem("implode",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Implode"),
               QT_TRANSLATE_NOOP("action", "Implode contents of selected staves into top selected staff")
               ),
    ActionItem("realize-chord-symbols",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Realize Chord Symbols"),
               QT_TRANSLATE_NOOP("action", "Convert chord symbols into notes")
               ),
    ActionItem("time-delete",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Remove Selected Range"),
               QT_TRANSLATE_NOOP("action", "Remove selected range")
               ),
    ActionItem("slash-fill",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Fill With Slashes"),
               QT_TRANSLATE_NOOP("action", "Fill with slashes")
               ),
    ActionItem("slash-rhythm",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Toggle Rhythmic Slash Notation"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Rhythmic Slash Notation'")
               ),
    ActionItem("pitch-spell",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Respell Pitches"),
               QT_TRANSLATE_NOOP("action", "Respell pitches")
               ),
    ActionItem("reset-groupings",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Regroup Rhythms"),
               QT_TRANSLATE_NOOP("action", "Combine rests and tied notes from selection and resplit at rhythmical")
               ),
    ActionItem("resequence-rehearsal-marks",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Resequence Rehearsal Marks"),
               QT_TRANSLATE_NOOP("action", "Resequence rehearsal marks")
               ),
    ActionItem("unroll-repeats",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Unroll Repeats"),
               QT_TRANSLATE_NOOP("action", "Unroll Repeats")
               ),
    ActionItem("copy-lyrics-to-clipboard",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Copy Lyrics to Clipboard"),
               QT_TRANSLATE_NOOP("action", "Copy lyrics to clipboard")
               ),
    ActionItem("del-empty-measures",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Remove Empty Trailing Measures"),
               QT_TRANSLATE_NOOP("action", "Remove empty trailing measures")
               ),
    ActionItem("parts",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Parts"),
               QT_TRANSLATE_NOOP("action", "Manage parts"),
               IconCode::Code::PAGE
               ),
    ActionItem("view-mode-page",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Page View"),
               IconCode::Code::PAGE_VIEW
               ),
    ActionItem("view-mode-continuous",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Continuous View (horizontal)"),
               IconCode::Code::CONTINUOUS_VIEW
               ),
    ActionItem("view-mode-single",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Continuous View (vertical)"),
               IconCode::Code::CONTINUOUS_VIEW_VERTICAL
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
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add/Remove System Breaks…"),
               QT_TRANSLATE_NOOP("action", "Add/remove system breaks")
               ),
    ActionItem("edit-info",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Score Properties…"),
               QT_TRANSLATE_NOOP("action", "Edit score properties")
               ),
    ActionItem("undo",
               ShortcutContext::NotationUndoRedo,
               QT_TRANSLATE_NOOP("action", "Undo"),
               QT_TRANSLATE_NOOP("action", "Undo last change"),
               IconCode::Code::UNDO
               ),
    ActionItem("redo",
               ShortcutContext::NotationUndoRedo,
               QT_TRANSLATE_NOOP("action", "Redo"),
               QT_TRANSLATE_NOOP("action", "Redo last undo"),
               IconCode::Code::REDO
               ),
    ActionItem("voice-x12",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-2"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 1-2")
               ),
    ActionItem("voice-x13",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-3"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 1-3")
               ),
    ActionItem("voice-x14",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 1-4"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 1-4")
               ),
    ActionItem("voice-x23",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 2-3"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 2-3")
               ),
    ActionItem("voice-x24",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 2-4"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 2-4")
               ),
    ActionItem("voice-x34",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Exchange Voice 3-4"),
               QT_TRANSLATE_NOOP("action", "Exchange voice 3-4")
               ),
    ActionItem("split-measure",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Split Measure Before Selected Note/Rest"),
               QT_TRANSLATE_NOOP("action", "Split measure before selected note/rest")
               ),
    ActionItem("join-measures",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Join Selected Measures"),
               QT_TRANSLATE_NOOP("action", "Join selected measures")
               ),
    ActionItem("insert-measure",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert One Measure"),
               QT_TRANSLATE_NOOP("action", "Insert one measure")
               ),
    ActionItem("insert-measures",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert Measures"),
               QT_TRANSLATE_NOOP("action", "Insert measures")
               ),
    ActionItem("append-measure",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Append One Measure"),
               QT_TRANSLATE_NOOP("action", "Append one measure")
               ),
    ActionItem("append-measures",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Append Measures"),
               QT_TRANSLATE_NOOP("action", "Append measures")
               ),
    ActionItem("insert-hbox",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert Horizontal Frame"),
               QT_TRANSLATE_NOOP("action", "Insert horizontal frame"),
               IconCode::Code::HORIZONTAL_FRAME
               ),
    ActionItem("insert-vbox",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert Vertical Frame"),
               QT_TRANSLATE_NOOP("action", "Insert vertical frame"),
               IconCode::Code::VERTICAL_FRAME
               ),
    ActionItem("insert-textframe",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert Text Frame"),
               QT_TRANSLATE_NOOP("action", "Insert text frame"),
               IconCode::Code::TEXT_FRAME
               ),
    ActionItem("append-hbox",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Append Horizontal Frame"),
               QT_TRANSLATE_NOOP("action", "Append horizontal frame")
               ),
    ActionItem("append-vbox",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Append Vertical Frame"),
               QT_TRANSLATE_NOOP("action", "Append vertical frame")
               ),
    ActionItem("append-textframe",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Append Text Frame"),
               QT_TRANSLATE_NOOP("action", "Append text frame")
               ),
    ActionItem("acciaccatura",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Acciaccatura")
               ),
    ActionItem("appoggiatura",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Appoggiatura")
               ),
    ActionItem("grace4",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: quarter")
               ),
    ActionItem("grace16",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: 16th")
               ),
    ActionItem("grace32",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: 32nd")
               ),
    ActionItem("grace8after",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: 8th after")
               ),
    ActionItem("grace16after",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: 16th after")
               ),
    ActionItem("grace32after",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Grace: 32nd after")
               ),
    ActionItem("beam-start",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Beam Start"),
               QT_TRANSLATE_NOOP("action", "Beam start"),
               IconCode::Code::BEAM_START
               ),
    ActionItem("beam-mid",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Beam Middle"),
               QT_TRANSLATE_NOOP("action", "Beam middle"),
               IconCode::Code::BEAM_MIDDLE
               ),
    ActionItem("no-beam",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "No Beam"),
               QT_TRANSLATE_NOOP("action", "No beam"),
               IconCode::Code::NOTE_HEAD_EIGHTH
               ),
    ActionItem("beam-32",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Beam 16th Sub"),
               QT_TRANSLATE_NOOP("action", "Beam 16th sub"),
               IconCode::Code::BEAM_32
               ),
    ActionItem("beam-64",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Beam 32th Sub"),
               QT_TRANSLATE_NOOP("action", "Beam 32th sub"),
               IconCode::Code::BEAM_64
               ),
    ActionItem("auto-beam",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Auto beam"),
               QT_TRANSLATE_NOOP("action", "Auto beam"),
               IconCode::Code::AUTO_TEXT
               ),
    ActionItem("add-brackets",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add Brackets to Accidental"),
               QT_TRANSLATE_NOOP("action", "Add brackets to accidental"),
               IconCode::Code::BRACKET
               ),
    ActionItem("add-braces",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add Braces to Element"),
               QT_TRANSLATE_NOOP("action", "Add Braces to element"),
               IconCode::Code::BRACE
               ),
    ActionItem("add-parentheses",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add Parentheses to Element"),
               QT_TRANSLATE_NOOP("action", "Add parentheses to element"),
               IconCode::Code::BRACKET_PARENTHESES
               ),
    ActionItem("interval1",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Unison Above"),
               QT_TRANSLATE_NOOP("action", "Enter unison above")
               ),
    ActionItem("interval2",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Second Above"),
               QT_TRANSLATE_NOOP("action", "Enter second above")
               ),
    ActionItem("interval3",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Third Above"),
               QT_TRANSLATE_NOOP("action", "Enter third above")
               ),
    ActionItem("interval4",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Fourth Above"),
               QT_TRANSLATE_NOOP("action", "Enter fourth above")
               ),
    ActionItem("interval5",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Fifth Above"),
               QT_TRANSLATE_NOOP("action", "Enter fifth above")
               ),
    ActionItem("interval6",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Sixth Above"),
               QT_TRANSLATE_NOOP("action", "Enter sixth above")
               ),
    ActionItem("interval7",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Seventh Above"),
               QT_TRANSLATE_NOOP("action", "Enter seventh above")
               ),
    ActionItem("interval8",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Octave Above"),
               QT_TRANSLATE_NOOP("action", "Enter octave above")
               ),
    ActionItem("interval9",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Ninth Above"),
               QT_TRANSLATE_NOOP("action", "Enter ninth above")
               ),
    ActionItem("interval-2",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Second Below"),
               QT_TRANSLATE_NOOP("action", "Enter second below")
               ),
    ActionItem("interval-3",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Third Below"),
               QT_TRANSLATE_NOOP("action", "Enter third below")
               ),
    ActionItem("interval-4",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Fourth Below"),
               QT_TRANSLATE_NOOP("action", "Enter fourth below")
               ),
    ActionItem("interval-5",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Fifth Below"),
               QT_TRANSLATE_NOOP("action", "Enter fifth below")
               ),
    ActionItem("interval-6",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Sixth Below"),
               QT_TRANSLATE_NOOP("action", "Enter sixth below")
               ),
    ActionItem("interval-7",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Seventh Below"),
               QT_TRANSLATE_NOOP("action", "Enter seventh below")
               ),
    ActionItem("interval-8",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Octave Below"),
               QT_TRANSLATE_NOOP("action", "Enter octave below")
               ),
    ActionItem("interval-9",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Ninth Below"),
               QT_TRANSLATE_NOOP("action", "Enter ninth below")
               ),
    ActionItem("note-c",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "C"),
               QT_TRANSLATE_NOOP("action", "Enter note C")
               ),
    ActionItem("note-d",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "D"),
               QT_TRANSLATE_NOOP("action", "Enter note D")
               ),
    ActionItem("note-e",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "E"),
               QT_TRANSLATE_NOOP("action", "Enter note E")
               ),
    ActionItem("note-f",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "F"),
               QT_TRANSLATE_NOOP("action", "Enter note F")
               ),
    ActionItem("note-g",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "G"),
               QT_TRANSLATE_NOOP("action", "Enter note G")
               ),
    ActionItem("note-a",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "A"),
               QT_TRANSLATE_NOOP("action", "Enter note A")
               ),
    ActionItem("note-b",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "B"),
               QT_TRANSLATE_NOOP("action", "Enter note B")
               ),
    ActionItem("chord-c",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add C to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note C to chord")
               ),
    ActionItem("chord-d",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add D to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note D to chord")
               ),
    ActionItem("chord-e",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add E to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note E to chord")
               ),
    ActionItem("chord-f",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add F to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note F to chord")
               ),
    ActionItem("chord-g",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add G to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note G to chord")
               ),
    ActionItem("chord-a",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add A to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note A to chord")
               ),
    ActionItem("chord-b",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Add B to Chord"),
               QT_TRANSLATE_NOOP("action", "Add note B to chord")
               ),
    ActionItem("insert-c",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert C"),
               QT_TRANSLATE_NOOP("action", "Insert note C")
               ),
    ActionItem("insert-d",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert D"),
               QT_TRANSLATE_NOOP("action", "Insert note D")
               ),
    ActionItem("insert-e",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert E"),
               QT_TRANSLATE_NOOP("action", "Insert note E")
               ),
    ActionItem("insert-f",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert F"),
               QT_TRANSLATE_NOOP("action", "Insert note F")
               ),
    ActionItem("insert-g",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert G"),
               QT_TRANSLATE_NOOP("action", "Insert note G")
               ),
    ActionItem("insert-a",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert A"),
               QT_TRANSLATE_NOOP("action", "Insert note A")
               ),
    ActionItem("insert-b",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Insert B"),
               QT_TRANSLATE_NOOP("action", "Insert note B")
               ),
    ActionItem("add-8va",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Ottava 8va alta"),
               QT_TRANSLATE_NOOP("action", "Add ottava 8va alta")
               ),
    ActionItem("add-8vb",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Ottava 8va bassa"),
               QT_TRANSLATE_NOOP("action", "Add ottava 8va bassa")
               ),
    ActionItem("add-hairpin",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Crescendo"),
               QT_TRANSLATE_NOOP("action", "Add crescendo")
               ),
    ActionItem("add-hairpin-reverse",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Decrescendo"),
               QT_TRANSLATE_NOOP("action", "Add decrescendo")
               ),
    ActionItem("add-noteline",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action","Note Anchored Line"),
               QT_TRANSLATE_NOOP("action","Note anchored line")
               ),
    ActionItem("title-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Title"),
               QT_TRANSLATE_NOOP("action", "Add title text")
               ),
    ActionItem("subtitle-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Subtitle"),
               QT_TRANSLATE_NOOP("action", "Add subtitle text")
               ),
    ActionItem("composer-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Composer"),
               QT_TRANSLATE_NOOP("action", "Add composer text")
               ),
    ActionItem("poet-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Lirycist"),
               QT_TRANSLATE_NOOP("action", "Add lirycist text")
               ),
    ActionItem("part-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Part Name"),
               QT_TRANSLATE_NOOP("action", "Add part name")
               ),
    ActionItem("system-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "System Text"),
               QT_TRANSLATE_NOOP("action", "Add system text")
               ),
    ActionItem("staff-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Staff Text"),
               QT_TRANSLATE_NOOP("action", "Add staff text")
               ),
    ActionItem("expression-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Expression Text"),
               QT_TRANSLATE_NOOP("action", "Add expression text")
               ),
    ActionItem("rehearsalmark-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Rehearsal Mark"),
               QT_TRANSLATE_NOOP("action", "Add rehearsal mark")
               ),
    ActionItem("instrument-change-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Instrument Change"),
               QT_TRANSLATE_NOOP("action", "Add instrument change")
               ),
    ActionItem("fingering-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Fingering"),
               QT_TRANSLATE_NOOP("action", "Add fingering")
               ),
    ActionItem("sticking-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Sticking"),
               QT_TRANSLATE_NOOP("action", "Add sticking")
               ),
    ActionItem("chord-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Chord Symbol"),
               QT_TRANSLATE_NOOP("action", "Add chord symbol")
               ),
    ActionItem("roman-numeral-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Roman Numeral Analysis"),
               QT_TRANSLATE_NOOP("action", "Add Roman numeral analysis")
               ),
    ActionItem("nashville-number-text",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Nashville Number"),
               QT_TRANSLATE_NOOP("action", "Add Nashville number")
               ),
    ActionItem("lyrics",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Lyrics"),
               QT_TRANSLATE_NOOP("action", "Add lyrics")
               ),
    ActionItem("figured-bass",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Figured Bass"),
               QT_TRANSLATE_NOOP("action", "Add figured bass")
               ),
    ActionItem("tempo",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Tempo Marking"),
               QT_TRANSLATE_NOOP("action", "Add tempo marking")
               ),
    ActionItem("duplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Duplet"),
               QT_TRANSLATE_NOOP("action", "Add duplet")
               ),
    ActionItem("triplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Triplet"),
               QT_TRANSLATE_NOOP("action", "Add triplet")
               ),
    ActionItem("quadruplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Quadruplet"),
               QT_TRANSLATE_NOOP("action", "Add quadruplet")
               ),
    ActionItem("quintuplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Quintuplet"),
               QT_TRANSLATE_NOOP("action", "Add quintuplet")
               ),
    ActionItem("sextuplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Sextuplet"),
               QT_TRANSLATE_NOOP("action", "Add sextuplet")
               ),
    ActionItem("septuplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Septuplet"),
               QT_TRANSLATE_NOOP("action", "Add septuplet")
               ),
    ActionItem("octuplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Octuplet"),
               QT_TRANSLATE_NOOP("action", "Add octuplet")
               ),
    ActionItem("nonuplet",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Nontuplet"),
               QT_TRANSLATE_NOOP("action", "Add nontuplet")
               ),
    ActionItem("tuplet-dialog",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Other…"),
               QT_TRANSLATE_NOOP("action", "Other tuplets")
               ),
    ActionItem("stretch-",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Increase Layout Stretch"),
               QT_TRANSLATE_NOOP("action", "Increase layout stretch factor of selected measures")
               ),
    ActionItem("stretch+",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Decrease Layout Stretch"),
               QT_TRANSLATE_NOOP("action", "Decrease layout stretch factor of selected measures")
               ),
    ActionItem("reset-stretch",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Reset Layout Stretch"),
               QT_TRANSLATE_NOOP("action", "Reset layout stretch factor of selected measures or entire score")
               ),
    ActionItem("reset-text-style-overrides",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Reset Text Style Overrides"),
               QT_TRANSLATE_NOOP("action", "Reset all text style overrides to default")
               ),
    ActionItem("reset-beammode",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Reset Beams"),
               QT_TRANSLATE_NOOP("action", "Reset beams of selected measures")
               ),
    ActionItem("reset",
               ShortcutContext::NotationHasSelection,
               QT_TRANSLATE_NOOP("action", "Reset Shapes and Positions"),
               QT_TRANSLATE_NOOP("action", "Reset shapes and positions of selected elements to their defaults")
               ),
    ActionItem("show-invisible",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Show Invisible"),
               QT_TRANSLATE_NOOP("action", "Show invisible")
               ),
    ActionItem("show-unprintable",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Show Unprintable"),
               QT_TRANSLATE_NOOP("action", "Show unprintable")
               ),
    ActionItem("show-frames",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Show Frames"),
               QT_TRANSLATE_NOOP("action", "Show frames")
               ),
    ActionItem("show-pageborders",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Show Page Margins"),
               QT_TRANSLATE_NOOP("action", "Show page margins")
               ),
    ActionItem("show-irregular",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Mark Irregular Measures"),
               QT_TRANSLATE_NOOP("action", "Mark irregular measures")
               )
};

const ActionList NotationActions::m_noteInputActions = {
    ActionItem("note-input",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Default (Step time)"),
               QT_TRANSLATE_NOOP("action", "Enter notes with a mouse or keyboard"),
               IconCode::Code::EDIT
               ),
    ActionItem("note-input-rhythm",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Rhythm only (not pitch)"),
               QT_TRANSLATE_NOOP("action", "Enter durations with a single click or keypress"),
               IconCode::Code::RHYTHM_ONLY
               ),
    ActionItem("note-input-repitch",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Re-pitch existing notes"),
               QT_TRANSLATE_NOOP("action", "Replace pitches without changing rhythms"),
               IconCode::Code::RE_PITH
               ),
    ActionItem("note-input-realtime-auto",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Real-time (metronome)"),
               QT_TRANSLATE_NOOP("action", "Enter notes at a fixed tempo indicated by a metronome beat"),
               IconCode::Code::METRONOME
               ),
    ActionItem("note-input-realtime-manual",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Real-time (foot pedal)"),
               QT_TRANSLATE_NOOP("action", "Enter notes while tapping a key or pedal to set the tempo"),
               IconCode::Code::FOOT_PEDAL
               ),
    ActionItem("note-input-timewise",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Insert"),
               QT_TRANSLATE_NOOP("action", "Insert notes by increasing measure duration"),
               IconCode::Code::NOTE_PLUS
               ),
    ActionItem("note-longa",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Longo"),
               QT_TRANSLATE_NOOP("action", "Note duration: Longa"),
               IconCode::Code::LONGO
               ),
    ActionItem("note-breve",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double whole note"),
               QT_TRANSLATE_NOOP("action", "Note duration: double whole note"),
               IconCode::Code::NOTE_WHOLE_DOUBLE
               ),
    ActionItem("pad-note-1",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Whole note"),
               QT_TRANSLATE_NOOP("action", "Note duration: whole note"),
               IconCode::Code::NOTE_WHOLE
               ),
    ActionItem("pad-note-2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Half note"),
               QT_TRANSLATE_NOOP("action", "Note duration: half note"),
               IconCode::Code::NOTE_HALF
               ),
    ActionItem("pad-note-4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quarter note"),
               QT_TRANSLATE_NOOP("action", "Note duration: quarter note"),
               IconCode::Code::NOTE_QUARTER
               ),
    ActionItem("pad-note-8",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "8th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 8th note"),
               IconCode::Code::NOTE_8TH
               ),
    ActionItem("pad-note-16",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "16th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 16th note"),
               IconCode::Code::NOTE_16TH
               ),
    ActionItem("pad-note-32",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "32th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 32th note"),
               IconCode::Code::NOTE_32TH
               ),
    ActionItem("pad-note-64",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "64th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 64th note"),
               IconCode::Code::NOTE_64TH
               ),
    ActionItem("pad-note-128",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "128th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 128th note"),
               IconCode::Code::NOTE_128TH
               ),
    ActionItem("pad-note-256",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "256th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 256th note"),
               IconCode::Code::NOTE_256TH
               ),
    ActionItem("pad-note-512",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "512th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 512th note"),
               IconCode::Code::NOTE_512TH
               ),
    ActionItem("pad-note-1024",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "1024th note"),
               QT_TRANSLATE_NOOP("action", "Note duration: 1024th note"),
               IconCode::Code::NOTE_1024TH
               ),
    ActionItem("pad-dot",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Augmentation Dot"),
               QT_TRANSLATE_NOOP("action", "Note duration: augmentation dot"),
               IconCode::Code::NOTE_DOTTED
               ),
    ActionItem("pad-dotdot",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double Augmentation Dot"),
               QT_TRANSLATE_NOOP("action", "Note duration: double augmentation dot"),
               IconCode::Code::NOTE_DOTTED_2
               ),
    ActionItem("pad-dot3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Triple Augmentation Dot"),
               QT_TRANSLATE_NOOP("action", "Note duration: triple augmentation dot"),
               IconCode::Code::NOTE_DOTTED_3
               ),
    ActionItem("pad-dot4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Quadruple Augmentation Dot"),
               QT_TRANSLATE_NOOP("action", "Note duration: quadruple augmentation dot"),
               IconCode::Code::NOTE_DOTTED_4
               ),
    ActionItem("pad-rest",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Rest"),
               QT_TRANSLATE_NOOP("action", "Note input: Rest"),
               IconCode::Code::REST
               ),
    ActionItem("flat",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "♭"),
               QT_TRANSLATE_NOOP("action", "Note input: ♭"),
               IconCode::Code::FLAT
               ),
    ActionItem("flat2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double ♭"),
               QT_TRANSLATE_NOOP("action", "Note input: Double ♭"),
               IconCode::Code::FLAT_DOUBLE
               ),
    ActionItem("nat",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "♮"),
               QT_TRANSLATE_NOOP("action", "Note input: ♮"),
               IconCode::Code::NATURAL
               ),
    ActionItem("sharp",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "♯"),
               QT_TRANSLATE_NOOP("action", "Note input: ♯"),
               IconCode::Code::SHARP
               ),
    ActionItem("sharp2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Double ♯"),
               QT_TRANSLATE_NOOP("action", "Note input: Double ♯"),
               IconCode::Code::SHARP_DOUBLE
               ),
    ActionItem("tie",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tie"),
               QT_TRANSLATE_NOOP("action", "Note duration: Tie"),
               IconCode::Code::NOTE_TIE
               ),
    ActionItem("add-slur",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Slur"),
               QT_TRANSLATE_NOOP("action", "Add slur"),
               IconCode::Code::NOTE_SLUR
               ),
    ActionItem("add-marcato",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Marcato"),
               QT_TRANSLATE_NOOP("action", "Toggle marcato"),
               IconCode::Code::MARCATO
               ),
    ActionItem("add-sforzato",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Accent"),
               QT_TRANSLATE_NOOP("action", "Toggle accent"),
               IconCode::Code::ACCENT
               ),
    ActionItem("add-tenuto",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tenuto"),
               QT_TRANSLATE_NOOP("action", "Toggle tenuto"),
               IconCode::Code::TENUTO
               ),
    ActionItem("add-staccato",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Staccato"),
               QT_TRANSLATE_NOOP("action", "Toggle staccato"),
               IconCode::Code::STACCATO
               ),
    ActionItem("tuplet",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Tuplet"),
               QT_TRANSLATE_NOOP("action", "Add tuplet"),
               IconCode::Code::NOTE_TUPLET
               ),
    ActionItem("voice-1",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 1"),
               QT_TRANSLATE_NOOP("action", "Voice 1"),
               IconCode::Code::VOICE_1
               ),
    ActionItem("voice-2",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 2"),
               QT_TRANSLATE_NOOP("action", "Voice 2"),
               IconCode::Code::VOICE_2
               ),
    ActionItem("voice-3",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 3"),
               QT_TRANSLATE_NOOP("action", "Voice 3"),
               IconCode::Code::VOICE_3
               ),
    ActionItem("voice-4",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Voice 4"),
               QT_TRANSLATE_NOOP("action", "Voice 4"),
               IconCode::Code::VOICE_4
               ),
    ActionItem("flip",
               ShortcutContext::NotationActive,
               QT_TRANSLATE_NOOP("action", "Flip Direction"),
               QT_TRANSLATE_NOOP("action", "Flip direction"),
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

const ActionCodeList NotationActions::actionCodes(ShortcutContext context)
{
    ActionCodeList codes;
    for (const ActionItem& action : m_actions) {
        if (action.shortcutContext == context) {
            codes.push_back(action.code);
        }
    }

    for (const ActionItem& action : m_noteInputActions) {
        if (action.shortcutContext == context) {
            codes.push_back(action.code);
        }
    }

    return codes;
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
