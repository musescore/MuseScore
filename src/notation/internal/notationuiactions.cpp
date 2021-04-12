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
#include "notationuiactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::notation;
using namespace mu::ui;
using namespace mu::actions;

static const ActionCode NOTE_INPUT_ACTION_CODE("note-input");

static const ActionCode SHOW_INVISIBLE_CODE("show-invisible");
static const ActionCode SHOW_UNPRINTABLE_CODE("show-unprintable");
static const ActionCode SHOW_FRAMES_CODE("show-frames");
static const ActionCode SHOW_PAGEBORDERS_CODE("show-pageborders");
static const ActionCode SHOW_IRREGULAR_CODE("show-irregular");

const UiActionList NotationUiActions::m_actions = {
    UiAction("escape",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Esc")
             ),
    UiAction("put-note", // args: QPoint pos, bool replace, bool insert
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Put Note")
             ),
    UiAction("next-element",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Next Element"),
             QT_TRANSLATE_NOOP("action", "Accessibility: Next element")
             ),
    UiAction("prev-element",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Previous Element"),
             QT_TRANSLATE_NOOP("action", "Accessibility: Previous element")
             ),
    UiAction("next-chord",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Next Chord"),
             QT_TRANSLATE_NOOP("action", "Go to next chord or move text right")
             ),
    UiAction("prev-chord",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Previous Chord"),
             QT_TRANSLATE_NOOP("action", "Go to previous chord or move text left")
             ),
    UiAction("next-measure",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Next Measure"),
             QT_TRANSLATE_NOOP("action", "Go to next measure or move text right")
             ),
    UiAction("prev-measure",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Previous Measure"),
             QT_TRANSLATE_NOOP("action", "Go to previous measure or move text left")
             ),
    UiAction("next-track",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Next staff or voice"),
             QT_TRANSLATE_NOOP("action", "Next staff or voice")
             ),
    UiAction("prev-track",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Previous staff or voice"),
             QT_TRANSLATE_NOOP("action", "Previous staff or voice")
             ),
    UiAction("select-next-chord",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Add next chord to selection"),
             QT_TRANSLATE_NOOP("action", "Add next chord to selection")
             ),
    UiAction("select-prev-chord",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Add previous chord to selection"),
             QT_TRANSLATE_NOOP("action", "Add previous chord to selection")
             ),
    UiAction("pitch-up",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Up"),
             QT_TRANSLATE_NOOP("action", "Pitch up or move text or articulation up")
             ),
    UiAction("pitch-down",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Down"),
             QT_TRANSLATE_NOOP("action", "Pitch down or move text or articulation down")
             ),
    UiAction("pitch-down-octave",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Down Octave"),
             QT_TRANSLATE_NOOP("action", "Pitch down by an octave or move text or articulation down")
             ),
    UiAction("pitch-up-octave",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Up Octave"),
             QT_TRANSLATE_NOOP("action", "Pitch up by an octave or move text or articulation up")
             ),
    UiAction("cut",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Cut")
             ),
    UiAction("copy",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Copy")
             ),
    UiAction("paste",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Paste")
             ),
    UiAction("paste-half",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Paste Half Duration"),
             QT_TRANSLATE_NOOP("action", "Paste half duration")
             ),
    UiAction("paste-double",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Paste Double Duration"),
             QT_TRANSLATE_NOOP("action", "Paste double duration")
             ),
    UiAction("paste-special",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Paste Special"),
             QT_TRANSLATE_NOOP("action", "Paste special")
             ),
    UiAction("swap",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Swap with Clipboard"),
             QT_TRANSLATE_NOOP("action", "Swap with clipboard")
             ),
    UiAction("toggle-visible",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Toggle visibility of elements"),
             QT_TRANSLATE_NOOP("action", "Toggle visibility of elements")
             ),
    UiAction("select-all",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Select All"),
             QT_TRANSLATE_NOOP("action", "Select all")
             ),
    UiAction("select-section",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Select Section"),
             QT_TRANSLATE_NOOP("action", "Select section")
             ),
    UiAction("select-similar",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Select: similar"),
             QT_TRANSLATE_NOOP("action", "Select all similar elements")
             ),
    UiAction("select-similar-staff",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Select: in same staff"),
             QT_TRANSLATE_NOOP("action", "Select all similar elements in same staff")
             ),
    UiAction("select-similar-range",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Select: in the range"),
             QT_TRANSLATE_NOOP("action", "Select all similar elements in the range selection")
             ),
    UiAction("select-dialog",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Select Dialog"),
             QT_TRANSLATE_NOOP("action", "Select all similar elements with more options")
             ),
    UiAction("delete",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Delete"),
             QT_TRANSLATE_NOOP("action", "Delete the selected element(s)"),
             IconCode::Code::DELETE_TANK
             ),
    UiAction("edit-style",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Style..."),
             QT_TRANSLATE_NOOP("action", "Edit style")
             ),
    UiAction("page-settings",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Page Settings..."),
             QT_TRANSLATE_NOOP("action", "Page settings")
             ),
    UiAction("load-style",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action","Load Style..."),
             QT_TRANSLATE_NOOP("action","Load style")
             ),
    UiAction("transpose",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "&Transpose..."),
             QT_TRANSLATE_NOOP("action", "Transpose")
             ),
    UiAction("explode",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Explode"),
             QT_TRANSLATE_NOOP("action", "Explode contents of top selected staff into staves below")
             ),
    UiAction("implode",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Implode"),
             QT_TRANSLATE_NOOP("action", "Implode contents of selected staves into top selected staff")
             ),
    UiAction("realize-chord-symbols",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Realize Chord Symbols"),
             QT_TRANSLATE_NOOP("action", "Convert chord symbols into notes")
             ),
    UiAction("time-delete",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Remove Selected Range"),
             QT_TRANSLATE_NOOP("action", "Remove selected range")
             ),
    UiAction("slash-fill",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Fill With Slashes"),
             QT_TRANSLATE_NOOP("action", "Fill with slashes")
             ),
    UiAction("slash-rhythm",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Toggle Rhythmic Slash Notation"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Rhythmic Slash Notation'")
             ),
    UiAction("pitch-spell",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Respell Pitches"),
             QT_TRANSLATE_NOOP("action", "Respell pitches")
             ),
    UiAction("reset-groupings",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Regroup Rhythms"),
             QT_TRANSLATE_NOOP("action", "Combine rests and tied notes from selection and resplit at rhythmical")
             ),
    UiAction("resequence-rehearsal-marks",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Resequence Rehearsal Marks"),
             QT_TRANSLATE_NOOP("action", "Resequence rehearsal marks")
             ),
    UiAction("unroll-repeats",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Unroll Repeats"),
             QT_TRANSLATE_NOOP("action", "Unroll Repeats")
             ),
    UiAction("copy-lyrics-to-clipboard",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Copy Lyrics to Clipboard"),
             QT_TRANSLATE_NOOP("action", "Copy lyrics to clipboard")
             ),
    UiAction("del-empty-measures",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Remove Empty Trailing Measures"),
             QT_TRANSLATE_NOOP("action", "Remove empty trailing measures")
             ),
    UiAction("parts",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Parts"),
             QT_TRANSLATE_NOOP("action", "Manage parts"),
             IconCode::Code::PAGE
             ),
    UiAction("view-mode-page",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Page View"),
             IconCode::Code::PAGE_VIEW
             ),
    UiAction("view-mode-continuous",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Continuous View (horizontal)"),
             IconCode::Code::CONTINUOUS_VIEW
             ),
    UiAction("view-mode-single",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Continuous View (vertical)"),
             IconCode::Code::CONTINUOUS_VIEW_VERTICAL
             ),
    UiAction("find",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Find")
             ),
    UiAction("staff-properties",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Staff/Part Properties")
             ),
    UiAction("add-remove-breaks",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add/Remove System Breaks…"),
             QT_TRANSLATE_NOOP("action", "Add/remove system breaks")
             ),
    UiAction("edit-info",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Score Properties…"),
             QT_TRANSLATE_NOOP("action", "Edit score properties")
             ),
    UiAction("undo",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Undo"),
             QT_TRANSLATE_NOOP("action", "Undo last change"),
             IconCode::Code::UNDO
             ),
    UiAction("redo",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Redo"),
             QT_TRANSLATE_NOOP("action", "Redo last undo"),
             IconCode::Code::REDO
             ),
    UiAction("voice-x12",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 1-2"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 1-2")
             ),
    UiAction("voice-x13",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 1-3"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 1-3")
             ),
    UiAction("voice-x14",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 1-4"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 1-4")
             ),
    UiAction("voice-x23",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 2-3"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 2-3")
             ),
    UiAction("voice-x24",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 2-4"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 2-4")
             ),
    UiAction("voice-x34",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Exchange Voice 3-4"),
             QT_TRANSLATE_NOOP("action", "Exchange voice 3-4")
             ),
    UiAction("split-measure",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Split Measure Before Selected Note/Rest"),
             QT_TRANSLATE_NOOP("action", "Split measure before selected note/rest")
             ),
    UiAction("join-measures",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Join Selected Measures"),
             QT_TRANSLATE_NOOP("action", "Join selected measures")
             ),
    UiAction("insert-measure",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert One Measure"),
             QT_TRANSLATE_NOOP("action", "Insert one measure")
             ),
    UiAction("insert-measures",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert Measures"),
             QT_TRANSLATE_NOOP("action", "Insert measures")
             ),
    UiAction("append-measure",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Append One Measure"),
             QT_TRANSLATE_NOOP("action", "Append one measure")
             ),
    UiAction("append-measures",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Append Measures"),
             QT_TRANSLATE_NOOP("action", "Append measures")
             ),
    UiAction("insert-hbox",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert Horizontal Frame"),
             QT_TRANSLATE_NOOP("action", "Insert horizontal frame"),
             IconCode::Code::HORIZONTAL_FRAME
             ),
    UiAction("insert-vbox",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert Vertical Frame"),
             QT_TRANSLATE_NOOP("action", "Insert vertical frame"),
             IconCode::Code::VERTICAL_FRAME
             ),
    UiAction("insert-textframe",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert Text Frame"),
             QT_TRANSLATE_NOOP("action", "Insert text frame"),
             IconCode::Code::TEXT_FRAME
             ),
    UiAction("append-hbox",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Append Horizontal Frame"),
             QT_TRANSLATE_NOOP("action", "Append horizontal frame")
             ),
    UiAction("append-vbox",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Append Vertical Frame"),
             QT_TRANSLATE_NOOP("action", "Append vertical frame")
             ),
    UiAction("append-textframe",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Append Text Frame"),
             QT_TRANSLATE_NOOP("action", "Append text frame")
             ),
    UiAction("acciaccatura",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Acciaccatura")
             ),
    UiAction("appoggiatura",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Appoggiatura")
             ),
    UiAction("grace4",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: quarter")
             ),
    UiAction("grace16",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: 16th")
             ),
    UiAction("grace32",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: 32nd")
             ),
    UiAction("grace8after",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: 8th after")
             ),
    UiAction("grace16after",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: 16th after")
             ),
    UiAction("grace32after",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Grace: 32nd after")
             ),
    UiAction("beam-start",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Beam Start"),
             QT_TRANSLATE_NOOP("action", "Beam start"),
             IconCode::Code::BEAM_START
             ),
    UiAction("beam-mid",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Beam Middle"),
             QT_TRANSLATE_NOOP("action", "Beam middle"),
             IconCode::Code::BEAM_MIDDLE
             ),
    UiAction("no-beam",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "No Beam"),
             QT_TRANSLATE_NOOP("action", "No beam"),
             IconCode::Code::NOTE_HEAD_EIGHTH
             ),
    UiAction("beam-32",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Beam 16th Sub"),
             QT_TRANSLATE_NOOP("action", "Beam 16th sub"),
             IconCode::Code::BEAM_32
             ),
    UiAction("beam-64",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Beam 32th Sub"),
             QT_TRANSLATE_NOOP("action", "Beam 32th sub"),
             IconCode::Code::BEAM_64
             ),
    UiAction("auto-beam",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Auto beam"),
             QT_TRANSLATE_NOOP("action", "Auto beam"),
             IconCode::Code::AUTO_TEXT
             ),
    UiAction("add-brackets",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add Brackets to Accidental"),
             QT_TRANSLATE_NOOP("action", "Add brackets to accidental"),
             IconCode::Code::BRACKET
             ),
    UiAction("add-braces",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add Braces to Element"),
             QT_TRANSLATE_NOOP("action", "Add Braces to element"),
             IconCode::Code::BRACE
             ),
    UiAction("add-parentheses",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add Parentheses to Element"),
             QT_TRANSLATE_NOOP("action", "Add parentheses to element"),
             IconCode::Code::BRACKET_PARENTHESES
             ),
    UiAction("interval1",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Unison Above"),
             QT_TRANSLATE_NOOP("action", "Enter unison above")
             ),
    UiAction("interval2",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Second Above"),
             QT_TRANSLATE_NOOP("action", "Enter second above")
             ),
    UiAction("interval3",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Third Above"),
             QT_TRANSLATE_NOOP("action", "Enter third above")
             ),
    UiAction("interval4",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Fourth Above"),
             QT_TRANSLATE_NOOP("action", "Enter fourth above")
             ),
    UiAction("interval5",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Fifth Above"),
             QT_TRANSLATE_NOOP("action", "Enter fifth above")
             ),
    UiAction("interval6",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Sixth Above"),
             QT_TRANSLATE_NOOP("action", "Enter sixth above")
             ),
    UiAction("interval7",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Seventh Above"),
             QT_TRANSLATE_NOOP("action", "Enter seventh above")
             ),
    UiAction("interval8",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Octave Above"),
             QT_TRANSLATE_NOOP("action", "Enter octave above")
             ),
    UiAction("interval9",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Ninth Above"),
             QT_TRANSLATE_NOOP("action", "Enter ninth above")
             ),
    UiAction("interval-2",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Second Below"),
             QT_TRANSLATE_NOOP("action", "Enter second below")
             ),
    UiAction("interval-3",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Third Below"),
             QT_TRANSLATE_NOOP("action", "Enter third below")
             ),
    UiAction("interval-4",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Fourth Below"),
             QT_TRANSLATE_NOOP("action", "Enter fourth below")
             ),
    UiAction("interval-5",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Fifth Below"),
             QT_TRANSLATE_NOOP("action", "Enter fifth below")
             ),
    UiAction("interval-6",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Sixth Below"),
             QT_TRANSLATE_NOOP("action", "Enter sixth below")
             ),
    UiAction("interval-7",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Seventh Below"),
             QT_TRANSLATE_NOOP("action", "Enter seventh below")
             ),
    UiAction("interval-8",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Octave Below"),
             QT_TRANSLATE_NOOP("action", "Enter octave below")
             ),
    UiAction("interval-9",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Ninth Below"),
             QT_TRANSLATE_NOOP("action", "Enter ninth below")
             ),
    UiAction("note-c",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "C"),
             QT_TRANSLATE_NOOP("action", "Enter note C")
             ),
    UiAction("note-d",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "D"),
             QT_TRANSLATE_NOOP("action", "Enter note D")
             ),
    UiAction("note-e",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "E"),
             QT_TRANSLATE_NOOP("action", "Enter note E")
             ),
    UiAction("note-f",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "F"),
             QT_TRANSLATE_NOOP("action", "Enter note F")
             ),
    UiAction("note-g",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "G"),
             QT_TRANSLATE_NOOP("action", "Enter note G")
             ),
    UiAction("note-a",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "A"),
             QT_TRANSLATE_NOOP("action", "Enter note A")
             ),
    UiAction("note-b",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "B"),
             QT_TRANSLATE_NOOP("action", "Enter note B")
             ),
    UiAction("chord-c",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add C to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note C to chord")
             ),
    UiAction("chord-d",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add D to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note D to chord")
             ),
    UiAction("chord-e",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add E to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note E to chord")
             ),
    UiAction("chord-f",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add F to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note F to chord")
             ),
    UiAction("chord-g",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add G to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note G to chord")
             ),
    UiAction("chord-a",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add A to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note A to chord")
             ),
    UiAction("chord-b",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Add B to Chord"),
             QT_TRANSLATE_NOOP("action", "Add note B to chord")
             ),
    UiAction("insert-c",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert C"),
             QT_TRANSLATE_NOOP("action", "Insert note C")
             ),
    UiAction("insert-d",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert D"),
             QT_TRANSLATE_NOOP("action", "Insert note D")
             ),
    UiAction("insert-e",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert E"),
             QT_TRANSLATE_NOOP("action", "Insert note E")
             ),
    UiAction("insert-f",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert F"),
             QT_TRANSLATE_NOOP("action", "Insert note F")
             ),
    UiAction("insert-g",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert G"),
             QT_TRANSLATE_NOOP("action", "Insert note G")
             ),
    UiAction("insert-a",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert A"),
             QT_TRANSLATE_NOOP("action", "Insert note A")
             ),
    UiAction("insert-b",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Insert B"),
             QT_TRANSLATE_NOOP("action", "Insert note B")
             ),
    UiAction("add-8va",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Ottava 8va alta"),
             QT_TRANSLATE_NOOP("action", "Add ottava 8va alta")
             ),
    UiAction("add-8vb",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Ottava 8va bassa"),
             QT_TRANSLATE_NOOP("action", "Add ottava 8va bassa")
             ),
    UiAction("add-hairpin",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Crescendo"),
             QT_TRANSLATE_NOOP("action", "Add crescendo")
             ),
    UiAction("add-hairpin-reverse",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Decrescendo"),
             QT_TRANSLATE_NOOP("action", "Add decrescendo")
             ),
    UiAction("add-noteline",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action","Note Anchored Line"),
             QT_TRANSLATE_NOOP("action","Note anchored line")
             ),
    UiAction("title-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Title"),
             QT_TRANSLATE_NOOP("action", "Add title text")
             ),
    UiAction("subtitle-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Subtitle"),
             QT_TRANSLATE_NOOP("action", "Add subtitle text")
             ),
    UiAction("composer-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Composer"),
             QT_TRANSLATE_NOOP("action", "Add composer text")
             ),
    UiAction("poet-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Lirycist"),
             QT_TRANSLATE_NOOP("action", "Add lirycist text")
             ),
    UiAction("part-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Part Name"),
             QT_TRANSLATE_NOOP("action", "Add part name")
             ),
    UiAction("system-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "System Text"),
             QT_TRANSLATE_NOOP("action", "Add system text")
             ),
    UiAction("staff-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Staff Text"),
             QT_TRANSLATE_NOOP("action", "Add staff text")
             ),
    UiAction("expression-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Expression Text"),
             QT_TRANSLATE_NOOP("action", "Add expression text")
             ),
    UiAction("rehearsalmark-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Rehearsal Mark"),
             QT_TRANSLATE_NOOP("action", "Add rehearsal mark")
             ),
    UiAction("instrument-change-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Instrument Change"),
             QT_TRANSLATE_NOOP("action", "Add instrument change")
             ),
    UiAction("fingering-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Fingering"),
             QT_TRANSLATE_NOOP("action", "Add fingering")
             ),
    UiAction("sticking-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Sticking"),
             QT_TRANSLATE_NOOP("action", "Add sticking")
             ),
    UiAction("chord-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Chord Symbol"),
             QT_TRANSLATE_NOOP("action", "Add chord symbol")
             ),
    UiAction("roman-numeral-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Roman Numeral Analysis"),
             QT_TRANSLATE_NOOP("action", "Add Roman numeral analysis")
             ),
    UiAction("nashville-number-text",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Nashville Number"),
             QT_TRANSLATE_NOOP("action", "Add Nashville number")
             ),
    UiAction("lyrics",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Lyrics"),
             QT_TRANSLATE_NOOP("action", "Add lyrics")
             ),
    UiAction("figured-bass",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Figured Bass"),
             QT_TRANSLATE_NOOP("action", "Add figured bass")
             ),
    UiAction("tempo",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Tempo Marking"),
             QT_TRANSLATE_NOOP("action", "Add tempo marking")
             ),
    UiAction("duplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Duplet"),
             QT_TRANSLATE_NOOP("action", "Add duplet")
             ),
    UiAction("triplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Triplet"),
             QT_TRANSLATE_NOOP("action", "Add triplet")
             ),
    UiAction("quadruplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Quadruplet"),
             QT_TRANSLATE_NOOP("action", "Add quadruplet")
             ),
    UiAction("quintuplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Quintuplet"),
             QT_TRANSLATE_NOOP("action", "Add quintuplet")
             ),
    UiAction("sextuplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Sextuplet"),
             QT_TRANSLATE_NOOP("action", "Add sextuplet")
             ),
    UiAction("septuplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Septuplet"),
             QT_TRANSLATE_NOOP("action", "Add septuplet")
             ),
    UiAction("octuplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Octuplet"),
             QT_TRANSLATE_NOOP("action", "Add octuplet")
             ),
    UiAction("nonuplet",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Nontuplet"),
             QT_TRANSLATE_NOOP("action", "Add nontuplet")
             ),
    UiAction("tuplet-dialog",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Other…"),
             QT_TRANSLATE_NOOP("action", "Other tuplets")
             ),
    UiAction("stretch-",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Increase Layout Stretch"),
             QT_TRANSLATE_NOOP("action", "Increase layout stretch factor of selected measures")
             ),
    UiAction("stretch+",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Decrease Layout Stretch"),
             QT_TRANSLATE_NOOP("action", "Decrease layout stretch factor of selected measures")
             ),
    UiAction("reset-stretch",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Reset Layout Stretch"),
             QT_TRANSLATE_NOOP("action", "Reset layout stretch factor of selected measures or entire score")
             ),
    UiAction("reset-text-style-overrides",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Reset Text Style Overrides"),
             QT_TRANSLATE_NOOP("action", "Reset all text style overrides to default")
             ),
    UiAction("reset-beammode",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Reset Beams"),
             QT_TRANSLATE_NOOP("action", "Reset beams of selected measures")
             ),
    UiAction("reset",
             mu::context::UiCtxNotationHasSelection,
             QT_TRANSLATE_NOOP("action", "Reset Shapes and Positions"),
             QT_TRANSLATE_NOOP("action", "Reset shapes and positions of selected elements to their defaults")
             ),
    UiAction("zoomin",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Zoom In"),
             QT_TRANSLATE_NOOP("action", "Zoom in")
             ),
    UiAction("zoomout",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Zoom Out"),
             QT_TRANSLATE_NOOP("action", "Zoom out")
             )
};

const UiActionList NotationUiActions::m_noteInputActions = {
    UiAction("note-input",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Default (Step time)"),
             QT_TRANSLATE_NOOP("action", "Enter notes with a mouse or keyboard"),
             IconCode::Code::EDIT,
             Checkable::Yes
             ),
    UiAction("note-input-rhythm",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Rhythm only (not pitch)"),
             QT_TRANSLATE_NOOP("action", "Enter durations with a single click or keypress"),
             IconCode::Code::RHYTHM_ONLY
             ),
    UiAction("note-input-repitch",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Re-pitch existing notes"),
             QT_TRANSLATE_NOOP("action", "Replace pitches without changing rhythms"),
             IconCode::Code::RE_PITH
             ),
    UiAction("note-input-realtime-auto",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Real-time (metronome)"),
             QT_TRANSLATE_NOOP("action", "Enter notes at a fixed tempo indicated by a metronome beat"),
             IconCode::Code::METRONOME
             ),
    UiAction("note-input-realtime-manual",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Real-time (foot pedal)"),
             QT_TRANSLATE_NOOP("action", "Enter notes while tapping a key or pedal to set the tempo"),
             IconCode::Code::FOOT_PEDAL
             ),
    UiAction("note-input-timewise",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Insert"),
             QT_TRANSLATE_NOOP("action", "Insert notes by increasing measure duration"),
             IconCode::Code::NOTE_PLUS
             ),
    UiAction("note-longa",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Longo"),
             QT_TRANSLATE_NOOP("action", "Note duration: Longa"),
             IconCode::Code::LONGO
             ),
    UiAction("note-breve",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Double whole note"),
             QT_TRANSLATE_NOOP("action", "Note duration: double whole note"),
             IconCode::Code::NOTE_WHOLE_DOUBLE
             ),
    UiAction("pad-note-1",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Whole note"),
             QT_TRANSLATE_NOOP("action", "Note duration: whole note"),
             IconCode::Code::NOTE_WHOLE
             ),
    UiAction("pad-note-2",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Half note"),
             QT_TRANSLATE_NOOP("action", "Note duration: half note"),
             IconCode::Code::NOTE_HALF
             ),
    UiAction("pad-note-4",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Quarter note"),
             QT_TRANSLATE_NOOP("action", "Note duration: quarter note"),
             IconCode::Code::NOTE_QUARTER
             ),
    UiAction("pad-note-8",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "8th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 8th note"),
             IconCode::Code::NOTE_8TH
             ),
    UiAction("pad-note-16",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "16th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 16th note"),
             IconCode::Code::NOTE_16TH
             ),
    UiAction("pad-note-32",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "32th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 32th note"),
             IconCode::Code::NOTE_32TH
             ),
    UiAction("pad-note-64",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "64th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 64th note"),
             IconCode::Code::NOTE_64TH
             ),
    UiAction("pad-note-128",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "128th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 128th note"),
             IconCode::Code::NOTE_128TH
             ),
    UiAction("pad-note-256",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "256th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 256th note"),
             IconCode::Code::NOTE_256TH
             ),
    UiAction("pad-note-512",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "512th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 512th note"),
             IconCode::Code::NOTE_512TH
             ),
    UiAction("pad-note-1024",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "1024th note"),
             QT_TRANSLATE_NOOP("action", "Note duration: 1024th note"),
             IconCode::Code::NOTE_1024TH
             ),
    UiAction("pad-dot",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Augmentation Dot"),
             QT_TRANSLATE_NOOP("action", "Note duration: augmentation dot"),
             IconCode::Code::NOTE_DOTTED
             ),
    UiAction("pad-dotdot",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Double Augmentation Dot"),
             QT_TRANSLATE_NOOP("action", "Note duration: double augmentation dot"),
             IconCode::Code::NOTE_DOTTED_2
             ),
    UiAction("pad-dot3",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Triple Augmentation Dot"),
             QT_TRANSLATE_NOOP("action", "Note duration: triple augmentation dot"),
             IconCode::Code::NOTE_DOTTED_3
             ),
    UiAction("pad-dot4",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Quadruple Augmentation Dot"),
             QT_TRANSLATE_NOOP("action", "Note duration: quadruple augmentation dot"),
             IconCode::Code::NOTE_DOTTED_4
             ),
    UiAction("pad-rest",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Rest"),
             QT_TRANSLATE_NOOP("action", "Note input: Rest"),
             IconCode::Code::REST
             ),
    UiAction("flat",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "♭"),
             QT_TRANSLATE_NOOP("action", "Note input: ♭"),
             IconCode::Code::FLAT
             ),
    UiAction("flat2",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Double ♭"),
             QT_TRANSLATE_NOOP("action", "Note input: Double ♭"),
             IconCode::Code::FLAT_DOUBLE
             ),
    UiAction("nat",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "♮"),
             QT_TRANSLATE_NOOP("action", "Note input: ♮"),
             IconCode::Code::NATURAL
             ),
    UiAction("sharp",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "♯"),
             QT_TRANSLATE_NOOP("action", "Note input: ♯"),
             IconCode::Code::SHARP
             ),
    UiAction("sharp2",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Double ♯"),
             QT_TRANSLATE_NOOP("action", "Note input: Double ♯"),
             IconCode::Code::SHARP_DOUBLE
             ),
    UiAction("tie",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Tie"),
             QT_TRANSLATE_NOOP("action", "Note duration: Tie"),
             IconCode::Code::NOTE_TIE
             ),
    UiAction("add-slur",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Slur"),
             QT_TRANSLATE_NOOP("action", "Add slur"),
             IconCode::Code::NOTE_SLUR
             ),
    UiAction("add-marcato",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Marcato"),
             QT_TRANSLATE_NOOP("action", "Toggle marcato"),
             IconCode::Code::MARCATO
             ),
    UiAction("add-sforzato",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Accent"),
             QT_TRANSLATE_NOOP("action", "Toggle accent"),
             IconCode::Code::ACCENT
             ),
    UiAction("add-tenuto",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Tenuto"),
             QT_TRANSLATE_NOOP("action", "Toggle tenuto"),
             IconCode::Code::TENUTO
             ),
    UiAction("add-staccato",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Staccato"),
             QT_TRANSLATE_NOOP("action", "Toggle staccato"),
             IconCode::Code::STACCATO
             ),
    UiAction("tuplet",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Tuplet"),
             QT_TRANSLATE_NOOP("action", "Add tuplet"),
             IconCode::Code::NOTE_TUPLET
             ),
    UiAction("voice-1",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Voice 1"),
             QT_TRANSLATE_NOOP("action", "Voice 1"),
             IconCode::Code::VOICE_1
             ),
    UiAction("voice-2",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Voice 2"),
             QT_TRANSLATE_NOOP("action", "Voice 2"),
             IconCode::Code::VOICE_2
             ),
    UiAction("voice-3",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Voice 3"),
             QT_TRANSLATE_NOOP("action", "Voice 3"),
             IconCode::Code::VOICE_3
             ),
    UiAction("voice-4",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Voice 4"),
             QT_TRANSLATE_NOOP("action", "Voice 4"),
             IconCode::Code::VOICE_4
             ),
    UiAction("flip",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Flip Direction"),
             QT_TRANSLATE_NOOP("action", "Flip direction"),
             IconCode::Code::NOTE_FLIP
             ),
};

const UiActionList NotationUiActions::m_scoreConfigActions = {
    UiAction(SHOW_INVISIBLE_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Show Invisible"),
             QT_TRANSLATE_NOOP("action", "Show invisible"),
             Checkable::Yes
             ),
    UiAction(SHOW_UNPRINTABLE_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Show Unprintable"),
             QT_TRANSLATE_NOOP("action", "Show unprintable"),
             Checkable::Yes
             ),
    UiAction(SHOW_FRAMES_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Show Frames"),
             QT_TRANSLATE_NOOP("action", "Show frames"),
             Checkable::Yes
             ),
    UiAction(SHOW_PAGEBORDERS_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Show Page Margins"),
             QT_TRANSLATE_NOOP("action", "Show page margins"),
             Checkable::Yes
             ),
    UiAction(SHOW_IRREGULAR_CODE,
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Mark Irregular Measures"),
             QT_TRANSLATE_NOOP("action", "Mark irregular measures"),
             Checkable::Yes
             ),
};

NotationUiActions::NotationUiActions(std::shared_ptr<NotationActionController> controller)
    : m_controller(controller)
{
}

void NotationUiActions::init()
{
    m_controller->currentNotationNoteInputChanged().onNotify(this, [this]() {
        m_actionCheckedChanged.send({ NOTE_INPUT_ACTION_CODE });
    });

    m_controller->currentNotationChanged().onNotify(this, [this]() {
        actions::ActionCodeList alist;
        for (const UiAction& a : m_scoreConfigActions) {
            alist.push_back(a.code);
        }
        m_actionCheckedChanged.send(alist);
    });
}

const UiActionList& NotationUiActions::actionsList() const
{
    static UiActionList alist;
    if (alist.empty()) {
        alist.insert(alist.end(), m_actions.begin(), m_actions.end());
        alist.insert(alist.end(), m_noteInputActions.begin(), m_noteInputActions.end());
        alist.insert(alist.end(), m_scoreConfigActions.begin(), m_scoreConfigActions.end());
    }
    return alist;
}

bool NotationUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool NotationUiActions::isScoreConfigAction(const actions::ActionCode& code) const
{
    for (const UiAction& a : m_scoreConfigActions) {
        if (a.code == code) {
            return true;
        }
    }
    return false;
}

bool NotationUiActions::isScoreConfigChecked(const actions::ActionCode& code, const ScoreConfig& cfg) const
{
    if (SHOW_INVISIBLE_CODE == code) {
        return cfg.isShowInvisibleElements;
    }
    if (SHOW_UNPRINTABLE_CODE == code) {
        return cfg.isShowUnprintableElements;
    }
    if (SHOW_FRAMES_CODE == code) {
        return cfg.isShowFrames;
    }
    if (SHOW_PAGEBORDERS_CODE == code) {
        return cfg.isShowPageMargins;
    }
    if (SHOW_IRREGULAR_CODE == code) {
        return cfg.isMarkIrregularMeasures;
    }

    return false;
}

bool NotationUiActions::actionChecked(const UiAction& act) const
{
    if (act.code == NOTE_INPUT_ACTION_CODE) {
        auto noteInput = m_controller->currentNotationNoteInput();
        if (noteInput) {
            return noteInput->isNoteInputMode();
        }
    }

    if (isScoreConfigAction(act.code)) {
        auto interaction = m_controller->currentNotationInteraction();
        if (interaction) {
            return isScoreConfigChecked(act.code, interaction->scoreConfig());
        }
    }

    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> NotationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> NotationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

UiActionList NotationUiActions::defaultNoteInputActions()
{
    return m_noteInputActions;
}

DurationType NotationUiActions::actionDurationType(const ActionCode& actionCode)
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

AccidentalType NotationUiActions::actionAccidentalType(const ActionCode& actionCode)
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

int NotationUiActions::actionDotCount(const ActionCode& actionCode)
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

int NotationUiActions::actionVoice(const ActionCode& actionCode)
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

SymbolId NotationUiActions::actionArticulationSymbolId(const ActionCode& actionCode)
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
