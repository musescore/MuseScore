/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "notationuiactions.h"

#include <unordered_map>

#include "types/translatablestring.h"
#include "ui/view/iconcodes.h"
#include "context/shortcutcontext.h"

using namespace mu;
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

static const ActionCode NOTE_INPUT_ACTION_CODE("note-input");

static const ActionCode SHOW_INVISIBLE_CODE("show-invisible");
static const ActionCode SHOW_UNPRINTABLE_CODE("show-unprintable");
static const ActionCode SHOW_FRAMES_CODE("show-frames");
static const ActionCode SHOW_PAGEBORDERS_CODE("show-pageborders");
static const ActionCode SHOW_SOUND_FLAGS("show-soundflags");
static const ActionCode SHOW_IRREGULAR_CODE("show-irregular");

static const ActionCode TOGGLE_CONCERT_PITCH_CODE("concert-pitch");

// avoid translation duplication

// //: This is comment for translator

//: Note
static const TranslatableString noteC = TranslatableString("action", "C");
//: Note
static const TranslatableString noteD = TranslatableString("action", "D");
//: Note
static const TranslatableString noteE = TranslatableString("action", "E");
//: Note
static const TranslatableString noteF = TranslatableString("action", "F");
//: Note
static const TranslatableString noteG = TranslatableString("action", "G");
//: Note
static const TranslatableString noteA = TranslatableString("action", "A");
//: Note
static const TranslatableString noteB = TranslatableString("action", "B");

static const TranslatableString Enter_note_X = TranslatableString("action", "Enter note %1");
static const TranslatableString Add_X_to_chord = TranslatableString("action", "Add %1 to chord");
static const TranslatableString Add_note_X_to_chord = TranslatableString("action", "Add note %1 to chord");
static const TranslatableString Insert_X = TranslatableString("action", "Insert %1");

static const TranslatableString fret_X_TAB = TranslatableString("action", "Fret %1 (TAB)");
static const TranslatableString enter_TAB_fret_X = TranslatableString("action", "Enter TAB: fret %1");

//: Addition to the name of an action to indicate that this action only applies to tablature notation.
//: '%1' is the name of the action.
static const TranslatableString X_TAB = TranslatableString("action", "%1 (TAB)");

//! NOTE Each notation actions should has context is UiCtxProjectOpened.
//! If you want what action to dispatch by shortcut only when notation is focused (ex notation-move-right by press Right key),
//! then you should set the shortcut context accordingly, not the action context.
//! Because actions can be dispatched not only shortcuts, but another way, ex by click Button, Menu and etc

const UiActionList NotationUiActions::m_actions = {
    UiAction("notation-escape",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED
             ),
    UiAction("put-note", // args: PointF pos, bool replace, bool insert
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Put note")
             ),
    UiAction("remove-note", // args: PointF pos
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Remove note")
             ),
    UiAction("next-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next element"),
             TranslatableString("action", "Select next element in score")
             ),
    UiAction("prev-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous element"),
             TranslatableString("action", "Select previous element in score")
             ),
    UiAction("notation-move-right",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next chord / Shift text right"),
             TranslatableString("action", "Select next chord / move text right")
             ),
    UiAction("notation-move-left",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous chord / Shift text left"),
             TranslatableString("action", "Select previous chord / move text left")
             ),
    UiAction("notation-move-right-quickly",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next measure / Shift text right quickly"),
             TranslatableString("action", "Go to next measure / move text right quickly")
             ),
    UiAction("notation-move-left-quickly",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous measure / Shift text left quickly"),
             TranslatableString("action", "Go to previous measure / move text left quickly")
             ),
    UiAction("up-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_LIST_SELECTION,
             TranslatableString("action", "Up note in chord"),
             TranslatableString("action", "Select note/rest above")
             ),
    UiAction("down-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_LIST_SELECTION,
             TranslatableString("action", "Down note in chord"),
             TranslatableString("action", "Select note/rest below")
             ),
    UiAction("top-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Top note in chord"),
             TranslatableString("action", "Select top note in chord")
             ),
    UiAction("bottom-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Bottom note in chord"),
             TranslatableString("action", "Select bottom note in chord")
             ),
    UiAction("first-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "First element"),
             TranslatableString("action", "Go to first element in score")
             ),
    UiAction("last-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Last element"),
             TranslatableString("action", "Go to last element in score")
             ),
    UiAction("move-up",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move to staff above"),
             TranslatableString("action", "Move selected note/rest to staff above")
             ),
    UiAction("move-down",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move to staff below"),
             TranslatableString("action", "Move selected note/rest to staff below")
             ),
    UiAction("next-track",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next staff or voice"),
             TranslatableString("action", "Go to next staff or voice")
             ),
    UiAction("prev-track",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous staff or voice"),
             TranslatableString("action", "Go to previous staff or voice")
             ),
    UiAction("next-frame",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next frame"),
             TranslatableString("action", "Go to next frame")
             ),
    UiAction("prev-frame",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous frame"),
             TranslatableString("action", "Go to previous frame")
             ),
    UiAction("next-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Next system"),
             TranslatableString("action", "Go to next system")
             ),
    UiAction("prev-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Previous system"),
             TranslatableString("action", "Go to previous system")
             ),
    UiAction("select-next-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add next chord to selection"),
             TranslatableString("action", "Add to selection: next note/rest")
             ),
    UiAction("select-prev-chord",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add previous chord to selection"),
             TranslatableString("action", "Add to selection: previous note/rest")
             ),
    UiAction("move-left",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move chord/rest left"),
             TranslatableString("action", "Move chord/rest left")
             ),
    UiAction("move-right",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move chord/rest right"),
             TranslatableString("action", "Move chord/rest right")
             ),
    UiAction("toggle-snap-to-previous",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Snap to &previous"),
             TranslatableString("action", "Snap to previous"),
             Checkable::Yes
             ),
    UiAction("toggle-snap-to-next",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Snap to &next"),
             TranslatableString("action", "Snap to next"),
             Checkable::Yes
             ),
    UiAction("pitch-up",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Up"),
             TranslatableString("action", "Move pitch/selection up")
             ),
    UiAction("pitch-down",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Down"),
             TranslatableString("action", "Move pitch/selection down")
             ),
    UiAction("pitch-down-octave",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Down octave"),
             TranslatableString("action", "Move pitch down an octave")
             ),
    UiAction("pitch-up-octave",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Up octave"),
             TranslatableString("action", "Move pitch up an octave")
             ),
    UiAction("double-duration",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Double duration"),
             TranslatableString("action", "Double selected duration")
             ),
    UiAction("half-duration",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Halve duration"),
             TranslatableString("action", "Halve selected duration")
             ),
    UiAction("inc-duration-dotted",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Double selected duration (dotted)"),
             TranslatableString("action", "Double selected duration (includes dotted values)")
             ),
    UiAction("dec-duration-dotted",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Halve selected duration (dotted)"),
             TranslatableString("action", "Halve selected duration (includes dotted values)")
             ),
    UiAction("notation-cut",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Cu&t"),
             TranslatableString("action", "Cut"),
             IconCode::Code::CUT
             ),
    UiAction("notation-copy",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Copy"),
             TranslatableString("action", "Copy"),
             IconCode::Code::COPY
             ),
    UiAction("notation-paste",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Past&e"),
             TranslatableString("action", "Paste"),
             IconCode::Code::PASTE
             ),
    UiAction("notation-paste-half",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Paste &half duration"),
             TranslatableString("action", "Paste half duration")
             ),
    UiAction("notation-paste-double",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Paste &double duration"),
             TranslatableString("action", "Paste double duration")
             ),
    UiAction("notation-paste-special",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Paste special"),
             TranslatableString("action", "Paste special")
             ),
    UiAction("notation-swap",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Swap with clipboard"),
             TranslatableString("action", "Copy/paste: swap with clipboard")
             ),
    UiAction("toggle-visible",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle visibility of elements"),
             TranslatableString("action", "Toggle visibility of elements")
             ),
    UiAction("notation-select-all",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select &all"),
             TranslatableString("action", "Select all")
             ),
    UiAction("notation-select-section",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select sectio&n"),
             TranslatableString("action", "Select section")
             ),
    UiAction("select-similar",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Similar"),
             TranslatableString("action", "Select similar elements")
             ),
    UiAction("select-similar-staff",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Similar on this staff"),
             TranslatableString("action", "Select similar elements on the same staff")
             ),
    UiAction("select-similar-range",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Similar in this range"),
             TranslatableString("action", "Select similar elements in the selected range")
             ),
    UiAction("select-dialog",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "More…"),
             TranslatableString("action", "Select similar elements with more options…")
             ),
    UiAction("notation-delete",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "De&lete"),
             TranslatableString("action", "Delete"),
             IconCode::Code::DELETE_TANK
             ),
    UiAction("edit-style",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Style…"),
             TranslatableString("action", "Format style…")
             ),
    UiAction("page-settings",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Page settings…"),
             TranslatableString("action", "Page settings…")
             ),
    UiAction("load-style",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Load style…"),
             TranslatableString("action", "Load style…")
             ),
    UiAction("save-style",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "S&ave style…"),
             TranslatableString("action", "Save style…")
             ),
    UiAction("transpose",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Transpose…"),
             TranslatableString("action", "Transpose…")
             ),
    UiAction("explode",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Explode"),
             TranslatableString("action", "Explode")
             ),
    UiAction("implode",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Implode"),
             TranslatableString("action", "Implode")
             ),
    UiAction("realize-chord-symbols",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Realize &chord symbols"),
             TranslatableString("action", "Realize chord symbols")
             ),
    UiAction("time-delete",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Remove selected ran&ge"),
             TranslatableString("action", "Delete selected measures"),
             IconCode::Code::DELETE_TANK
             ),
    UiAction("slash-fill",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Fill with slashes"),
             TranslatableString("action", "Fill with slashes")
             ),
    UiAction("slash-rhythm",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle rhythmic sl&ash notation"),
             TranslatableString("action", "Toggle rhythmic slash notation")
             ),
    UiAction("pitch-spell",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Respell &pitches"),
             TranslatableString("action", "Respell pitches")
             ),
    UiAction("reset-groupings",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Regroup &rhythms"),
             TranslatableString("action", "Regroup rhythms")
             ),
    UiAction("resequence-rehearsal-marks",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Resequence re&hearsal marks"),
             TranslatableString("action", "Resequence rehearsal marks")
             ),
    UiAction("unroll-repeats",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Unroll repeats"),
             TranslatableString("action", "Unroll repeats")
             ),
    UiAction("copy-lyrics-to-clipboard",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Copy &lyrics to clipboard"),
             TranslatableString("action", "Copy lyrics")
             ),
    UiAction("del-empty-measures",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Remove empty trailing meas&ures"),
             TranslatableString("action", "Remove empty trailing measures")
             ),
    UiAction("parts",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Parts"),
             TranslatableString("action", "Manage parts"),
             IconCode::Code::PAGE
             ),
    UiAction("view-mode-page",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Page view"),
             TranslatableString("action", "Display page view"),
             IconCode::Code::PAGE_VIEW
             ),
    UiAction("view-mode-float",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Floating"),
             TranslatableString("action", "Floating"),
             IconCode::Code::PAGE_VIEW // TODO, Icon needed?
             ),
    UiAction("view-mode-continuous",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Continuous view (horizontal)"),
             TranslatableString("action", "Display continuous view (horizontal)"),
             IconCode::Code::CONTINUOUS_VIEW
             ),
    UiAction("view-mode-single",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Continuous view (vertical)"),
             TranslatableString("action", "Display continuous view (vertical)"),
             IconCode::Code::CONTINUOUS_VIEW_VERTICAL
             ),
    UiAction("find",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Find / Go to"),
             TranslatableString("action", "Find / Go to")
             ),
    UiAction("staff-properties",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Staff/Part properties…"),
             TranslatableString("action", "Staff/Part properties…")
             ),
    UiAction("staff-text-properties",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Staff text properties…"),
             TranslatableString("action", "Staff text properties…")
             ),
    UiAction("system-text-properties",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "System text properties…"),
             TranslatableString("action", "System text properties…")
             ),
    UiAction("measure-properties",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Measure properties…"),
             TranslatableString("action", "Measure properties…")
             ),
    UiAction("measures-per-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Measures per s&ystem…"),
             TranslatableString("action", "Measures per system…")
             ),
    UiAction("undo",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Undo"),
             TranslatableString("action", "Undo"),
             IconCode::Code::UNDO
             ),
    UiAction("redo",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Redo"),
             TranslatableString("action", "Redo"),
             IconCode::Code::REDO
             ),
    UiAction("voice-x12",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice &1-2"),
             TranslatableString("action", "Exchange voice 1-2")
             ),
    UiAction("voice-x13",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice 1-3"),
             TranslatableString("action", "Exchange voice 1-3")
             ),
    UiAction("voice-x14",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice 1-&4"),
             TranslatableString("action", "Exchange voice 1-4")
             ),
    UiAction("voice-x23",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice &2-3"),
             TranslatableString("action", "Exchange voice 2-3")
             ),
    UiAction("voice-x24",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice 2-4"),
             TranslatableString("action", "Exchange voice 2-4")
             ),
    UiAction("voice-x34",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Exchange voice &3-4"),
             TranslatableString("action", "Exchange voice 3-4")
             ),
    UiAction("system-break",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Add/remove system break"),
             TranslatableString("action", "Add/remove system break")
             ),
    UiAction("page-break",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add/remove page break"),
             TranslatableString("action", "Add/remove page break")
             ),
    UiAction("apply-system-lock",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Add/remove system lock"),
             TranslatableString("action", "Add/remove system lock")
             ),
    UiAction("move-measure-to-prev-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move measure to previous system"),
             TranslatableString("action", "Move measure to previous system"),
             IconCode::Code::ARROW_UP
             ),
    UiAction("move-measure-to-next-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Move measure to next system"),
             TranslatableString("action", "Move measure to next system"),
             IconCode::Code::ARROW_DOWN
             ),
    UiAction("make-into-system",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Create system from selection"),
             TranslatableString("action", "Create system from selection")
             ),
    UiAction("section-break",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add/remove section break"),
             TranslatableString("action", "Add/remove section break")
             ),
    UiAction("split-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Split measure before selected note/rest"),
             TranslatableString("action", "Split measure before selected note/rest")
             ),
    UiAction("join-measures",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Join selected measures"),
             TranslatableString("action", "Join selected measures")
             ),
    UiAction("insert-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert one measure before selection"),
             TranslatableString("action", "Insert one measure before selection"),
             IconCode::Code::INSERT_ONE_MEASURE
             ),
    UiAction("insert-measures",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert measures before selection…"),
             TranslatableString("action", "Insert measures before selection…")
             ),
    UiAction("insert-measures-after-selection",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert measures after selection…"),
             TranslatableString("action", "Insert measures after selection…")
             ),
    UiAction("insert-measures-at-start-of-score",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert measures at start of score…"),
             TranslatableString("action", "Insert measures at start of score…")
             ),
    UiAction("append-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert one measure at end of score"),
             TranslatableString("action", "Insert one measure at end of score")
             ),
    UiAction("append-measures",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert measures at end of score…"),
             TranslatableString("action", "Insert measures at end of score…")
             ),
    UiAction("insert-hbox",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert &horizontal frame"),
             TranslatableString("action", "Insert horizontal frame"),
             IconCode::Code::HORIZONTAL_FRAME
             ),
    UiAction("insert-vbox",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert &vertical frame"),
             TranslatableString("action", "Insert vertical frame"),
             IconCode::Code::VERTICAL_FRAME
             ),
    UiAction("insert-textframe",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert &text frame"),
             TranslatableString("action", "Insert text frame"),
             IconCode::Code::TEXT_FRAME
             ),
    UiAction("insert-fretframe",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert &fretboard diagram legend"),
             TranslatableString("action", "Insert &fretboard diagram legend"),
             IconCode::Code::FRET_FRAME
             ),
    UiAction("append-hbox",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Append h&orizontal frame"),
             TranslatableString("action", "Append horizontal frame")
             ),
    UiAction("append-vbox",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Append v&ertical frame"),
             TranslatableString("action", "Append vertical frame")
             ),
    UiAction("append-textframe",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Append te&xt frame"),
             TranslatableString("action", "Append text frame")
             ),
    UiAction("acciaccatura",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Acciaccatura"),
             TranslatableString("action", "Add grace note: acciaccatura"),
             IconCode::Code::ACCIACCATURA
             ),
    UiAction("appoggiatura",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Appoggiatura"),
             TranslatableString("action", "Add grace note: appoggiatura"),
             IconCode::Code::APPOGGIATURA
             ),
    UiAction("grace4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: quarter"),
             TranslatableString("action", "Add grace note: quarter"),
             IconCode::Code::GRACE4
             ),
    UiAction("grace16",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: 16th"),
             TranslatableString("action", "Add grace note: 16th"),
             IconCode::Code::GRACE16
             ),
    UiAction("grace32",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: 32nd"),
             TranslatableString("action", "Add grace note: 32nd"),
             IconCode::Code::GRACE32
             ),
    UiAction("grace8after",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: 8th after"),
             TranslatableString("action", "Add grace note: eighth after"),
             IconCode::Code::GRACE8_AFTER
             ),
    UiAction("grace16after",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: 16th after"),
             TranslatableString("action", "Add grace note: 16th after"),
             IconCode::Code::GRACE16_AFTER
             ),
    UiAction("grace32after",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace: 32nd after"),
             TranslatableString("action", "Add grace note: 32nd after"),
             IconCode::Code::GRACE32_AFTER
             ),
    UiAction("beam-auto",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Auto beam"),
             TranslatableString("action", "Auto beam"),
             IconCode::Code::AUTO_TEXT
             ),
    UiAction("beam-none",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "No beam"),
             TranslatableString("action", "No beam"),
             IconCode::Code::BEAM_NONE
             ),
    UiAction("beam-break-left",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Break beam left"),
             TranslatableString("action", "Break beam left"),
             IconCode::Code::BEAM_BREAK_LEFT
             ),
    UiAction("beam-break-inner-8th",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Break inner beams (8th)"),
             TranslatableString("action", "Break inner beams (eighth)"),
             IconCode::Code::BEAM_BREAK_INNER_8TH
             ),
    UiAction("beam-break-inner-16th",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Break inner beams (16th)"),
             TranslatableString("action", "Break inner beams (16th)"),
             IconCode::Code::BEAM_BREAK_INNER_16TH
             ),
    UiAction("beam-join",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Join beams"),
             TranslatableString("action", "Join beams"),
             IconCode::Code::BEAM_JOIN
             ),
    UiAction("beam-feathered-decelerate",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Feathered beam, decelerate"),
             TranslatableString("action", "Add feathered beam: decelerate"),
             IconCode::Code::BEAM_FEATHERED_DECELERATE
             ),
    UiAction("beam-feathered-accelerate",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Feathered beam, accelerate"),
             TranslatableString("action", "Add feathered beam: accelerate"),
             IconCode::Code::BEAM_FEATHERED_ACCELERATE
             ),
    UiAction("add-brackets",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add brackets to accidental"),
             TranslatableString("action", "Add brackets to accidental"),
             IconCode::Code::BRACKET_PARENTHESES_SQUARE
             ),
    UiAction("add-braces",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add braces to element"),
             TranslatableString("action", "Add braces to element"),
             IconCode::Code::BRACE
             ),
    UiAction("add-parentheses",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add parentheses to element"),
             TranslatableString("action", "Add parentheses to element"),
             IconCode::Code::BRACKET_PARENTHESES
             ),
    UiAction("interval1",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Unison"),
             TranslatableString("action", "Enter interval: unison")
             ),
    UiAction("interval2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Se&cond above"),
             TranslatableString("action", "Enter interval: second above")
             ),
    UiAction("interval3",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Thir&d above"),
             TranslatableString("action", "Enter interval: third above")
             ),
    UiAction("interval4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Fou&rth above"),
             TranslatableString("action", "Enter interval: fourth above")
             ),
    UiAction("interval5",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Fift&h above"),
             TranslatableString("action", "Enter interval: fifth above")
             ),
    UiAction("interval6",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Si&xth above"),
             TranslatableString("action", "Enter interval: sixth above")
             ),
    UiAction("interval7",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Seve&nth above"),
             TranslatableString("action", "Enter interval: seventh above")
             ),
    UiAction("interval8",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Octave &above"),
             TranslatableString("action", "Enter interval: octave above")
             ),
    UiAction("interval9",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Ninth abov&e"),
             TranslatableString("action", "Enter interval: ninth above")
             ),
    UiAction("interval-2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Second below"),
             TranslatableString("action", "Enter interval: second below")
             ),
    UiAction("interval-3",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Third below"),
             TranslatableString("action", "Enter interval: third below")
             ),
    UiAction("interval-4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "F&ourth below"),
             TranslatableString("action", "Enter interval: fourth below")
             ),
    UiAction("interval-5",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Fifth below"),
             TranslatableString("action", "Enter interval: fifth below")
             ),
    UiAction("interval-6",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "S&ixth below"),
             TranslatableString("action", "Enter interval: sixth below")
             ),
    UiAction("interval-7",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Se&venth below"),
             TranslatableString("action", "Enter interval: seventh below")
             ),
    UiAction("interval-8",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Octave &below"),
             TranslatableString("action", "Enter interval: octave below")
             ),
    UiAction("interval-9",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Ninth belo&w"),
             TranslatableString("action", "Enter interval: ninth below")
             ),
    UiAction("note-c",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteC,
             Enter_note_X.arg(noteC)
             ),
    UiAction("note-d",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteD,
             Enter_note_X.arg(noteD)
             ),
    UiAction("note-e",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteE,
             Enter_note_X.arg(noteE)
             ),
    UiAction("note-f",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteF,
             Enter_note_X.arg(noteF)
             ),
    UiAction("note-g",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteG,
             Enter_note_X.arg(noteG)
             ),
    UiAction("note-a",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteA,
             Enter_note_X.arg(noteA)
             ),
    UiAction("note-b",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             noteB,
             Enter_note_X.arg(noteB)
             ),
    UiAction("chord-c",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteC),
             Add_note_X_to_chord.arg(noteC)
             ),
    UiAction("chord-d",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteD),
             Add_note_X_to_chord.arg(noteD)
             ),
    UiAction("chord-e",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteE),
             Add_note_X_to_chord.arg(noteE)
             ),
    UiAction("chord-f",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteF),
             Add_note_X_to_chord.arg(noteF)
             ),
    UiAction("chord-g",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteG),
             Add_note_X_to_chord.arg(noteG)
             ),
    UiAction("chord-a",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteA),
             Add_note_X_to_chord.arg(noteA)
             ),
    UiAction("chord-b",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Add_X_to_chord.arg(noteB),
             Add_note_X_to_chord.arg(noteB)
             ),
    UiAction("insert-c",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteC)
             ),
    UiAction("insert-d",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteD)
             ),
    UiAction("insert-e",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteE)
             ),
    UiAction("insert-f",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteF)
             ),
    UiAction("insert-g",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteG)
             ),
    UiAction("insert-a",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteA)
             ),
    UiAction("insert-b",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             Insert_X.arg(noteB)
             ),
    UiAction("rest",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Rest"),
             TranslatableString("action", "Enter rest"),
             IconCode::Code::REST
             ),
    UiAction("rest-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             X_TAB.arg(TranslatableString("action", "Rest")),
             X_TAB.arg(TranslatableString("action", "Enter rest"))
             ),
    UiAction("fret-0",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(0),
             enter_TAB_fret_X.arg(0)
             ),
    UiAction("fret-1",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(1),
             enter_TAB_fret_X.arg(1)
             ),
    UiAction("fret-2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(2),
             enter_TAB_fret_X.arg(2)
             ),
    UiAction("fret-3",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(3),
             enter_TAB_fret_X.arg(3)
             ),
    UiAction("fret-4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(4),
             enter_TAB_fret_X.arg(4)
             ),
    UiAction("fret-5",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(5),
             enter_TAB_fret_X.arg(5)
             ),
    UiAction("fret-6",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(6),
             enter_TAB_fret_X.arg(6)
             ),
    UiAction("fret-7",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(7),
             enter_TAB_fret_X.arg(7)
             ),
    UiAction("fret-8",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(8),
             enter_TAB_fret_X.arg(8)
             ),
    UiAction("fret-9",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(9),
             enter_TAB_fret_X.arg(9)
             ),
    UiAction("fret-10",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(10),
             enter_TAB_fret_X.arg(10)
             ),
    UiAction("fret-11",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(11),
             enter_TAB_fret_X.arg(11)
             ),
    UiAction("fret-12",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(12),
             enter_TAB_fret_X.arg(12)
             ),
    UiAction("fret-13",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(13),
             enter_TAB_fret_X.arg(13)
             ),
    UiAction("fret-14",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             fret_X_TAB.arg(14),
             enter_TAB_fret_X.arg(14)
             ),
    UiAction("add-8va",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Ottava 8va &alta"),
             TranslatableString("action", "Add ottava 8va alta")
             ),
    UiAction("add-8vb",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Ottava 8va &bassa"),
             TranslatableString("action", "Add ottava 8va bassa")
             ),
    UiAction("add-dynamic",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Dynamic"),
             TranslatableString("action", "Add dynamic")
             ),
    UiAction("add-hairpin",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Crescendo"),
             TranslatableString("action", "Add hairpin: crescendo")
             ),
    UiAction("add-hairpin-reverse",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Decrescendo"),
             TranslatableString("action", "Add hairpin: decrescendo")
             ),
    UiAction("add-noteline",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Note-anchored line"),
             TranslatableString("action", "Add note-anchored line"),
             IconCode::Code::NOTE_ANCHORED_LINE
             ),
    UiAction("chord-tie",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add tied note to chord")
             ),
    UiAction("add-image",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Image")
             ),
    UiAction("title-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Title"),
             TranslatableString("action", "Add text: title")
             ),
    UiAction("subtitle-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Subtitle"),
             TranslatableString("action", "Add text: subtitle")
             ),
    UiAction("composer-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Composer"),
             TranslatableString("action", "Add text: composer")
             ),
    UiAction("poet-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Lyricist"),
             TranslatableString("action", "Add text: lyricist")
             ),
    UiAction("part-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Part name"),
             TranslatableString("action", "Add text: part name")
             ),
    UiAction("frame-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Text"),
             TranslatableString("action", "Add frame text")
             ),
    UiAction("system-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Syst&em text"),
             TranslatableString("action", "Add text: system text")
             ),
    UiAction("staff-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "St&aff text"),
             TranslatableString("action", "Add text: staff text")
             ),
    UiAction("expression-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "E&xpression text"),
             TranslatableString("action", "Add text: expression text")
             ),
    UiAction("rehearsalmark-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Rehearsal mark"),
             TranslatableString("action", "Add text: rehearsal mark")
             ),
    UiAction("instrument-change-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Instrument change"),
             TranslatableString("action", "Add text: instrument change")
             ),
    UiAction("fingering-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Fingering"),
             TranslatableString("action", "Add text: fingering")
             ),
    UiAction("sticking-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Stic&king"),
             TranslatableString("action", "Add text: sticking")
             ),
    UiAction("chord-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "C&hord symbol"),
             TranslatableString("action", "Add text: chord symbol")
             ),
    UiAction("roman-numeral-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "R&oman numeral analysis"),
             TranslatableString("action", "Add text: Roman numeral analysis")
             ),
    UiAction("nashville-number-text",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Nashville number"),
             TranslatableString("action", "Add text: Nashville number")
             ),
    UiAction("lyrics",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "L&yrics"),
             TranslatableString("action", "Add text: lyrics")
             ),
    UiAction("figured-bass",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Figured &bass"),
             TranslatableString("action", "Add text: figured bass")
             ),
    UiAction("tempo",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Tempo &marking"),
             TranslatableString("action", "Add text: tempo marking")
             ),
    UiAction("duplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Duplet"),
             TranslatableString("action", "Enter tuplet: duplet")
             ),
    UiAction("triplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Triplet"),
             TranslatableString("action", "Enter tuplet: triplet")
             ),
    UiAction("quadruplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Quadruplet"),
             TranslatableString("action", "Enter tuplet: quadruplet")
             ),
    UiAction("quintuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Q&uintuplet"),
             TranslatableString("action", "Enter tuplet: quintuplet")
             ),
    UiAction("sextuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Se&xtuplet"),
             TranslatableString("action", "Enter tuplet: sextuplet")
             ),
    UiAction("septuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Se&ptuplet"),
             TranslatableString("action", "Enter tuplet: septuplet")
             ),
    UiAction("octuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Octuplet"),
             TranslatableString("action", "Enter tuplet: octuplet")
             ),
    UiAction("nonuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "&Nonuplet"),
             TranslatableString("action", "Enter tuplet: nonuplet")
             ),
    UiAction("tuplet-dialog",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Othe&r…"),
             TranslatableString("action", "Enter tuplet: other…")
             ),
    UiAction("stretch-",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Decrease layout stretch"),
             TranslatableString("action", "Decrease layout stretch")
             ),
    UiAction("stretch+",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Increase layout stretch"),
             TranslatableString("action", "Increase layout stretch")
             ),
    UiAction("reset-stretch",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "&Reset layout stretch"),
             TranslatableString("action", "Reset layout stretch")
             ),
    UiAction("reset-text-style-overrides",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Reset &text style overrides"),
             TranslatableString("action", "Reset all text style overrides to default")
             ),
    UiAction("reset-beammode",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Reset &beams"),
             TranslatableString("action", "Reset beams to default grouping")
             ),
    UiAction("reset",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Reset s&hapes and positions"),
             TranslatableString("action", "Reset shapes and positions")
             ),
    UiAction("reset-to-default-layout",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Reset entire score to &default layout"),
             TranslatableString("action", "Reset entire score to default layout")
             ),
    UiAction("zoomin",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom in"),
             TranslatableString("action", "Zoom in"),
             IconCode::Code::ZOOM_IN
             ),
    UiAction("zoomout",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom out"),
             TranslatableString("action", "Zoom out"),
             IconCode::Code::ZOOM_OUT
             ),
    UiAction("zoom100",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom to 100%"),
             TranslatableString("action", "Zoom to 100%")
             ),
    UiAction("zoom-page-width",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom to page width"),
             TranslatableString("action", "Zoom to page width")
             ),
    UiAction("zoom-whole-page",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom to whole page"),
             TranslatableString("action", "Zoom to whole page")
             ),
    UiAction("zoom-two-pages",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Zoom to two pages"),
             TranslatableString("action", "Zoom to two pages")
             ),
    UiAction("get-location",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Accessibility: Get location"),
             TranslatableString("action", "Accessibility: get location")
             ),
    UiAction("edit-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Edit element"),
             TranslatableString("action", "Edit element")
             ),
    UiAction("select-prev-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to beginning of measure"),
             TranslatableString("action", "Select to beginning of measure")
             ),
    UiAction("select-next-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to end of measure"),
             TranslatableString("action", "Select to end of measure")
             ),
    UiAction("select-begin-line",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to beginning of line"),
             TranslatableString("action", "Select to beginning of line")
             ),
    UiAction("select-end-line",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to end of line"),
             TranslatableString("action", "Select to end of line")
             ),
    UiAction("select-begin-score",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to beginning of score"),
             TranslatableString("action", "Select to beginning of score")
             ),
    UiAction("select-end-score",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Select to end of score"),
             TranslatableString("action", "Select to end of score")
             ),
    UiAction("select-staff-above",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add staff above to selection"),
             TranslatableString("action", "Add to selection: staff above")
             ),
    UiAction("select-staff-below",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add staff below to selection"),
             TranslatableString("action", "Add to selection: staff below")
             ),
    UiAction("scr-prev",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Screen: Previous"),
             TranslatableString("action", "Jump to previous screen")
             ),
    UiAction("scr-next",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Screen: Next"),
             TranslatableString("action", "Jump to next screen")
             ),
    UiAction("page-prev",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Page: Previous"),
             TranslatableString("action", "Jump to previous page")
             ),
    UiAction("page-next",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Page: Next"),
             TranslatableString("action", "Jump to next page")
             ),
    UiAction("page-top",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Page: Top of first"),
             TranslatableString("action", "Jump to top of first page")
             ),
    UiAction("page-end",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Page: Bottom of last"),
             TranslatableString("action", "Jump to bottom of last page")
             ),
    UiAction("help",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Context sensitive help")
             ),
    UiAction("repeat-sel",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Repeat selection"),
             TranslatableString("action", "Repeat selection")
             ),
    UiAction("toggle-score-lock",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Lock/unlock all systems"),
             TranslatableString("action", "Lock/unlock all systems")
             ),
    UiAction("toggle-system-lock",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Lock/unlock selected system(s)"),
             TranslatableString("action", "Lock/unlock selected system(s)"),
             IconCode::Code::SYSTEM_LOCK
             ),
    UiAction("enh-both",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Change enharmonic spelling (both modes)"),
             TranslatableString("action", "Change enharmonic spelling (concert and written pitch)"),
             IconCode::Code::NONE
             ),
    UiAction("enh-current",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Change enharmonic spelling (current mode)"),
             TranslatableString("action", "Change enharmonic spelling (current mode only)"),
             IconCode::Code::NONE
             ),
    UiAction("flip",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Flip direction"),
             TranslatableString("action", "Flip direction"),
             IconCode::Code::NOTE_FLIP
             ),
    UiAction("flip-horizontally",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Flip horizontally"),
             TranslatableString("action", "Flip horizontally")),
    UiAction(TOGGLE_CONCERT_PITCH_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Concert pitch"),
             TranslatableString("action", "Toggle concert pitch"),
             IconCode::Code::TUNING_FORK,
             Checkable::Yes
             ),
    UiAction("next-word",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Next word"),
             TranslatableString("action", "Go to next word")
             ),
    UiAction("next-text-element",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Next text element"),
             TranslatableString("action", "Go to next text element")
             ),
    UiAction("prev-text-element",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Previous text element"),
             TranslatableString("action", "Go to previous text element")
             ),
    UiAction("next-beat-TEXT",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Next beat (Chord symbol)"),
             TranslatableString("action", "Advance cursor: next beat (chord symbols)")
             ),
    UiAction("prev-beat-TEXT",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Previous beat (Chord symbol)"),
             TranslatableString("action", "Advance cursor: previous beat (chord symbols)")
             ),
    UiAction("advance-longa",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance longa (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: longa (figured bass/chord symbols)")
             ),
    UiAction("advance-breve",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance breve (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: breve (figured bass/chord symbols)")
             ),
    UiAction("advance-1",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance whole note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: whole note (figured bass/chord symbols)")
             ),
    UiAction("advance-2",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance half note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: half note (figured bass/chord symbols)")
             ),
    UiAction("advance-4",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance quarter note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: quarter note (figured bass/chord symbols)")
             ),
    UiAction("advance-8",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance eighth note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: eighth note (figured bass/chord symbols)")
             ),
    UiAction("advance-16",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance 16th note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: 16th note (figured bass/chord symbols)")
             ),
    UiAction("advance-32",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance 32nd note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: 32nd note (figured bass/chord symbols)")
             ),
    UiAction("advance-64",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Advance 64th note (F.B./Chord symbol)"),
             TranslatableString("action", "Advance cursor: 64th note (figured bass/chord symbols)")
             ),
    UiAction("next-lyric-verse",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Next lyric verse"),
             TranslatableString("action", "Move text/go to next lyric verse")
             ),
    UiAction("prev-lyric-verse",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Previous lyric verse"),
             TranslatableString("action", "Move text/go to previous lyric verse")
             ),
    UiAction("next-syllable",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Next syllable"),
             TranslatableString("action", "Lyrics: enter hyphen")
             ),
    UiAction("add-melisma",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_TEXT_EDITING,
             TranslatableString("action", "Add extension line"),
             TranslatableString("action", "Lyrics: enter extension line")
             ),
    UiAction("add-lyric-verse",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add lyrics verse"),
             TranslatableString("action", "Add lyrics verse")
             ),
    UiAction("text-b",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Bold face"),
             TranslatableString("action", "Format text: bold"),
             Checkable::Yes
             ),
    UiAction("text-i",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Italic"),
             TranslatableString("action", "Format text: italic"),
             Checkable::Yes
             ),
    UiAction("text-u",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Underline"),
             TranslatableString("action", "Format text: underline"),
             Checkable::Yes
             ),
    UiAction("text-s",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Strikethrough"),
             TranslatableString("action", "Format text: strikethrough"),
             Checkable::Yes
             ),
    UiAction("text-sub",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Subscript"),
             TranslatableString("action", "Format text: subscript"),
             Checkable::Yes
             ),
    UiAction("text-sup",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Superscript"),
             TranslatableString("action", "Format text: superscript"),
             Checkable::Yes
             ),
    UiAction("pitch-up-diatonic",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Diatonic pitch up"),
             TranslatableString("action", "Move pitch up diatonically")
             ),
    UiAction("pitch-down-diatonic",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Diatonic pitch down"),
             TranslatableString("action", "Move pitch down diatonically")
             ),
    UiAction("top-staff",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Go to top staff"),
             TranslatableString("action", "Go to top staff")
             ),
    UiAction("empty-trailing-measure",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Go to first empty trailing measure"),
             TranslatableString("action", "Go to first empty trailing measure")
             ),
    UiAction("mirror-note",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Mirror notehead"),
             TranslatableString("action", "Mirror notehead")
             ),
    UiAction("add-turn",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle turn"),
             TranslatableString("action", "Add ornament: turn")
             ),
    UiAction("add-turn-inverted",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle inverted turn"),
             TranslatableString("action", "Add ornament: inverted turn")
             ),
    UiAction("add-turn-slash",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle turn with slash"),
             TranslatableString("action", "Add ornament: turn with slash")
             ),
    UiAction("add-trill",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle trill"),
             TranslatableString("action", "Add ornament: trill")
             ),
    UiAction("add-short-trill",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle short trill"),
             TranslatableString("action", "Add ornament: short trill")
             ),
    UiAction("add-mordent",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle mordent"),
             TranslatableString("action", "Add ornament: mordent")
             ),
    UiAction("add-tremblement",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle tremblement"),
             TranslatableString("action", "Add ornament: tremblement")
             ),
    UiAction("add-prall-mordent",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle prall mordent"),
             TranslatableString("action", "Add ornament: prall mordent")
             ),
    UiAction("add-shake",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle shake"),
             TranslatableString("action", "Add ornament: shake")
             ),
    UiAction("add-shake-muffat",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle shake (Muffat)"),
             TranslatableString("action", "Add ornament: shake (Muffat)")
             ),
    UiAction("add-tremblement-couperin",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle tremblement appuyé (Couperin)"),
             TranslatableString("action", "Add ornament: tremblement appuyé (Couperin)")
             ),
    UiAction("add-up-bow",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle up bow"),
             TranslatableString("action", "Add bowing: up bow")
             ),
    UiAction("add-down-bow",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle down bow"),
             TranslatableString("action", "Add bowing: down bow")
             ),
    UiAction("reset-style",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Reset style"),
             TranslatableString("action", "Reset all style values to default")
             ),
    UiAction("clef-violin",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add treble clef"),
             TranslatableString("action", "Add clef: treble")
             ),
    UiAction("clef-bass",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add bass clef"),
             TranslatableString("action", "Add clef: bass")
             ),
    UiAction("sharp2-post",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add double-sharp"),
             TranslatableString("action", "Add accidental: double-sharp")
             ),
    UiAction("sharp-post",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add sharp"),
             TranslatableString("action", "Add accidental: sharp")
             ),
    UiAction("nat-post",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add natural"),
             TranslatableString("action", "Add accidental: natural")
             ),
    UiAction("flat-post",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add flat"),
             TranslatableString("action", "Add accidental: flat")
             ),
    UiAction("flat2-post",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Add double-flat"),
             TranslatableString("action", "Add accidental: double-flat")
             ),
    UiAction("transpose-up",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Transpose up"),
             TranslatableString("action", "Transpose up a semitone")
             ),
    UiAction("transpose-down",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Transpose down"),
             TranslatableString("action", "Transpose down a semitone")
             ),
    UiAction("pitch-up-diatonic-alterations",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Diatonic pitch up (keep degree alterations)"),
             TranslatableString("action", "Move pitch up diatonically (keep degree alterations)")
             ),
    UiAction("pitch-down-diatonic-alterations",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Diatonic pitch down (keep degree alterations)"),
             TranslatableString("action", "Move pitch down diatonically (keep degree alterations)")
             ),
    UiAction("full-measure-rest",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Full measure rest"),
             TranslatableString("action", "Insert full measure rest")
             ),
    UiAction("toggle-mmrest",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle multimeasure rests"),
             TranslatableString("action", "Toggle multimeasure rests")
             ),
    UiAction("toggle-hide-empty",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle empty staves"),
             TranslatableString("action", "Show/hide empty staves")
             ),
    UiAction("set-visible",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Set visible"),
             TranslatableString("action", "Make selected element(s) visible")
             ),
    UiAction("unset-visible",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Set invisible"),
             TranslatableString("action", "Make selected element(s) invisible")
             ),
    UiAction("toggle-autoplace",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle automatic placement for selected elements"),
             TranslatableString("action", "Toggle automatic placement for selected elements")
             ),
    UiAction("autoplace-enabled",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle automatic placement for entire score"),
             TranslatableString("action", "Toggle automatic placement for entire score")
             ),
    UiAction("string-above",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "String above (TAB)"),
             TranslatableString("action", "Go to string above (TAB)")
             ),
    UiAction("string-below",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "String below (TAB)"),
             TranslatableString("action", "Go to string below (TAB)")
             ),
    UiAction(NOTE_INPUT_ACTION_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Note input"),
             TranslatableString("action", "Toggle note input mode"),
             IconCode::Code::EDIT,
             Checkable::Yes
             ),
    UiAction("toggle-insert-mode",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert/overwrite"),
             TranslatableString("action", "Toggle note input mode: insert/overwrite")
             ),
    UiAction("note-input-by-note-name",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Input by note name"),
             TranslatableString("action", "Toggle note input mode: input by note name"),
             IconCode::Code::EDIT
             ),
    UiAction("note-input-by-duration",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Input by duration"),
             TranslatableString("action", "Toggle note input mode: input by duration"),
             IconCode::Code::DURATION_CURSOR
             ),
    UiAction("note-input-rhythm",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Rhythm only (not pitch)"),
             TranslatableString("action", "Toggle note input mode: rhythm only (not pitch)"),
             IconCode::Code::RHYTHM_ONLY
             ),
    UiAction("note-input-repitch",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Re-pitch existing notes"),
             TranslatableString("action", "Toggle note input mode: re-pitch existing notes"),
             IconCode::Code::RE_PITCH
             ),
    UiAction("note-input-realtime-auto",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Real-time (metronome)"),
             TranslatableString("action", "Toggle note input mode: real-time (metronome)"),
             IconCode::Code::METRONOME
             ),
    UiAction("note-input-realtime-manual",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Real-time (foot pedal)"),
             TranslatableString("action", "Toggle note input mode: real-time (foot pedal)"),
             IconCode::Code::FOOT_PEDAL
             ),
    UiAction("note-input-timewise",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Insert"),
             TranslatableString("action", "Toggle note input mode: insert (increases measure duration)"),
             IconCode::Code::NOTE_PLUS
             ),
    UiAction("realtime-advance",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Real-time advance"),
             TranslatableString("action", "Real-time advance"),
             IconCode::Code::METRONOME
             ),
    UiAction("note-longa",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Longa"),
             TranslatableString("action", "Set duration: longa"),
             IconCode::Code::LONGO
             ),
    UiAction("note-breve",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Double whole note"),
             TranslatableString("action", "Set duration: double whole note"),
             IconCode::Code::NOTE_WHOLE_DOUBLE
             ),
    UiAction("pad-note-1",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Whole note"),
             TranslatableString("action", "Set duration: whole note"),
             IconCode::Code::NOTE_WHOLE
             ),
    UiAction("pad-note-2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Half note"),
             TranslatableString("action", "Set duration: half note"),
             IconCode::Code::NOTE_HALF
             ),
    UiAction("pad-note-4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Quarter note"),
             TranslatableString("action", "Set duration: quarter note"),
             IconCode::Code::NOTE_QUARTER
             ),
    UiAction("pad-note-8",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "Eighth note"),
             TranslatableString("action", "Set duration: eighth note"),
             IconCode::Code::NOTE_8TH
             ),
    UiAction("pad-note-16",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "16th note"),
             TranslatableString("action", "Set duration: 16th note"),
             IconCode::Code::NOTE_16TH
             ),
    UiAction("pad-note-32",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "32nd note"),
             TranslatableString("action", "Set duration: 32nd note"),
             IconCode::Code::NOTE_32ND
             ),
    UiAction("pad-note-64",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "64th note"),
             TranslatableString("action", "Set duration: 64th note"),
             IconCode::Code::NOTE_64TH
             ),
    UiAction("pad-note-128",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "128th note"),
             TranslatableString("action", "Set duration: 128th note"),
             IconCode::Code::NOTE_128TH
             ),
    UiAction("pad-note-256",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "256th note"),
             TranslatableString("action", "Set duration: 256th note"),
             IconCode::Code::NOTE_256TH
             ),
    UiAction("pad-note-512",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "512th note"),
             TranslatableString("action", "Set duration: 512th note"),
             IconCode::Code::NOTE_512TH
             ),
    UiAction("pad-note-1024",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_NOT_NOTE_INPUT_STAFF_TAB,
             TranslatableString("action", "1024th note"),
             TranslatableString("action", "Set duration: 1024th note"),
             IconCode::Code::NOTE_1024TH
             ),
    UiAction("pad-note-1-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "Whole note")),
             X_TAB.arg(TranslatableString("action", "Set duration: whole note")),
             IconCode::Code::NOTE_WHOLE
             ),
    UiAction("pad-note-2-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "Half note")),
             X_TAB.arg(TranslatableString("action", "Set duration: half note")),
             IconCode::Code::NOTE_HALF
             ),
    UiAction("pad-note-4-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "Quarter note")),
             X_TAB.arg(TranslatableString("action", "Set duration: quarter note")),
             IconCode::Code::NOTE_QUARTER
             ),
    UiAction("pad-note-8-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "Eighth note")),
             X_TAB.arg(TranslatableString("action", "Set duration: eighth note")),
             IconCode::Code::NOTE_8TH
             ),
    UiAction("pad-note-16-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "16th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 16th note")),
             IconCode::Code::NOTE_16TH
             ),
    UiAction("pad-note-32-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "32nd note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 32nd note")),
             IconCode::Code::NOTE_32ND
             ),
    UiAction("pad-note-64-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "64th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 64th note")),
             IconCode::Code::NOTE_64TH
             ),
    UiAction("pad-note-128-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "128th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 128th note")),
             IconCode::Code::NOTE_128TH
             ),
    UiAction("pad-note-256-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "256th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 256th note")),
             IconCode::Code::NOTE_256TH
             ),
    UiAction("pad-note-512-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "512th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 512th note")),
             IconCode::Code::NOTE_512TH
             ),
    UiAction("pad-note-1024-TAB",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             X_TAB.arg(TranslatableString("action", "1024th note")),
             X_TAB.arg(TranslatableString("action", "Set duration: 1024th note")),
             IconCode::Code::NOTE_1024TH
             ),
    UiAction("pad-dot",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Augmentation dot"),
             TranslatableString("action", "Toggle duration dot"),
             IconCode::Code::NOTE_DOTTED
             ),
    UiAction("pad-dot2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Double augmentation dot"),
             TranslatableString("action", "Toggle duration dot: double"),
             IconCode::Code::NOTE_DOTTED_2
             ),
    UiAction("pad-dot3",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Triple augmentation dot"),
             TranslatableString("action", "Toggle duration dot: triple"),
             IconCode::Code::NOTE_DOTTED_3
             ),
    UiAction("pad-dot4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Quadruple augmentation dot"),
             TranslatableString("action", "Toggle duration dot: quadruple"),
             IconCode::Code::NOTE_DOTTED_4
             ),
    UiAction("pad-rest",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Rest"),
             TranslatableString("action", "Toggle rest"),
             IconCode::Code::REST
             ),
    UiAction("next-segment-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Accessibility: Next segment element"),
             TranslatableString("action", "Select next in-staff element")
             ),
    UiAction("prev-segment-element",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Accessibility: Previous segment element"),
             TranslatableString("action", "Select previous in-staff element")
             ),
    UiAction("flat",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle flat"),
             TranslatableString("action", "Toggle accidental: flat"),
             IconCode::Code::FLAT
             ),
    UiAction("flat2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle double-flat"),
             TranslatableString("action", "Toggle accidental: double-flat"),
             IconCode::Code::FLAT_DOUBLE
             ),
    UiAction("nat",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle natural"),
             TranslatableString("action", "Toggle accidental: natural"),
             IconCode::Code::NATURAL
             ),
    UiAction("sharp",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle sharp"),
             TranslatableString("action", "Toggle accidental: sharp"),
             IconCode::Code::SHARP
             ),
    UiAction("sharp2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Toggle double-sharp"),
             TranslatableString("action", "Toggle accidental: double-sharp"),
             IconCode::Code::SHARP_DOUBLE
             ),
    UiAction("tie",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Tie"),
             TranslatableString("action", "Add tied note"),
             IconCode::Code::NOTE_TIE
             ),
    UiAction("lv",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Laissez vibrer"),
             TranslatableString("action", "Add laissez vibrer"),
             IconCode::Code::NOTE_LV
             ),
    UiAction("add-slur",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Slur"),
             TranslatableString("action", "Add slur"),
             IconCode::Code::NOTE_SLUR
             ),
    UiAction("add-marcato",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Marcato"),
             TranslatableString("action", "Add articulation: marcato"),
             IconCode::Code::MARCATO
             ),
    UiAction("add-sforzato",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Accent"),
             TranslatableString("action", "Add articulation: accent"),
             IconCode::Code::ACCENT
             ),
    UiAction("add-tenuto",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Tenuto"),
             TranslatableString("action", "Add articulation: tenuto"),
             IconCode::Code::TENUTO
             ),
    UiAction("add-staccato",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Staccato"),
             TranslatableString("action", "Add articulation: staccato"),
             IconCode::Code::STACCATO
             ),
    UiAction("cross-staff-beaming",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Cross-staff beaming"),
             IconCode::Code::CROSS_STAFF_BEAMING
             ),
    UiAction("tuplet",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Tuplet"),
             IconCode::Code::NOTE_TUPLET
             ),
    UiAction("voice-1",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Voice 1"),
             TranslatableString("action", "Use voice 1"),
             IconCode::Code::VOICE_1
             ),
    UiAction("voice-2",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Voice 2"),
             TranslatableString("action", "Use voice 2"),
             IconCode::Code::VOICE_2
             ),
    UiAction("voice-3",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Voice 3"),
             TranslatableString("action", "Use voice 3"),
             IconCode::Code::VOICE_3
             ),
    UiAction("voice-4",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Voice 4"),
             TranslatableString("action", "Use voice 4"),
             IconCode::Code::VOICE_4
             ),
    UiAction("voice-assignment-all-in-instrument",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "All voices on instrument"),
             TranslatableString("action", "Use all voices on instrument")
             ),
    UiAction("voice-assignment-all-in-staff",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "All voices on staff"),
             TranslatableString("action", "Use all voices on staff")
             ),
    UiAction("notation-context-menu",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_FOCUSED
             ),
    UiAction("insert-staff-type-change",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Staff type change"),
             TranslatableString("action", "Insert staff type change"),
             IconCode::Code::STAFF_TYPE_CHANGE
             ),
    UiAction("notation-popup-menu",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_FOCUSED
             ),
    UiAction("standard-bend",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Standard bend"),
             TranslatableString("action", "Standard bend"),
             IconCode::Code::GUITAR_BEND_REGULAR
             ),
    UiAction("pre-bend",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Pre-bend"),
             TranslatableString("action", "Pre-bend"),
             IconCode::Code::GUITAR_PRE_BEND
             ),
    UiAction("grace-note-bend",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Grace note bend"),
             TranslatableString("action", "Grace note bend"),
             IconCode::Code::GUITAR_GRACE_NOTE_BEND
             ),
    UiAction("slight-bend",
             mu::context::UiCtxProjectFocused,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Slight bend"),
             TranslatableString("action", "Slight bend"),
             IconCode::Code::GUITAR_SLIGHT_BEND
             ),
};

const UiActionList NotationUiActions::m_scoreConfigActions = {
    UiAction(SHOW_INVISIBLE_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show &invisible"),
             TranslatableString("action", "Show/hide invisible elements"),
             Checkable::Yes
             ),
    UiAction(SHOW_UNPRINTABLE_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show f&ormatting"),
             TranslatableString("action", "Show/hide formatting"),
             Checkable::Yes
             ),
    UiAction(SHOW_FRAMES_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show &frames"),
             TranslatableString("action", "Show/hide frames"),
             Checkable::Yes
             ),
    UiAction(SHOW_PAGEBORDERS_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show page &margins"),
             TranslatableString("action", "Show/hide page margins"),
             Checkable::Yes
             ),
    UiAction(SHOW_SOUND_FLAGS,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show sound flags"), // todo &
             TranslatableString("action", "Show/hide sound flags"),
             Checkable::Yes
             ),
    UiAction(SHOW_IRREGULAR_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Mark i&rregular measures"),
             TranslatableString("action", "Mark irregular measures"),
             Checkable::Yes
             )
};

const UiActionList NotationUiActions::m_engravingDebuggingActions = {
    UiAction("show-element-bounding-rects",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show element bounding rectangles"),
             TranslatableString("action", "Show/hide element bounding rectangles"),
             Checkable::Yes
             ),
    UiAction("color-element-shapes",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Color element shapes"),
             Checkable::Yes
             ),
    UiAction("show-segment-shapes",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show segment shapes"),
             TranslatableString("action", "Show/hide segment shapes"),
             Checkable::Yes
             ),
    UiAction("color-segment-shapes",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Color segment shapes"),
             Checkable::Yes
             ),
    UiAction("show-skylines",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show skylines"),
             TranslatableString("action", "Show/hide skylines"),
             Checkable::Yes
             ),
    UiAction("show-system-bounding-rects",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show system bounding rectangles"),
             TranslatableString("action", "Show/hide system bounding rectangles"),
             Checkable::Yes
             ),
    UiAction("show-element-masks",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show element masks"),
             TranslatableString("action", "Show/hide element masks"),
             Checkable::Yes
             ),
    UiAction("show-corrupted-measures",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Show corrupted measures"),
             TranslatableString("action", "Show/hide corrupted measures"),
             Checkable::Yes
             ),
    UiAction("check-for-score-corruptions",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Check for score corruptions")
             ),
    UiAction("edit-strings",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED
             )
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
        ActionCodeList actions;
        for (const UiAction& action : m_scoreConfigActions) {
            actions.push_back(action.code);
        }
        m_actionCheckedChanged.send(actions);

        if (m_controller->currentNotationInteraction()) {
            m_controller->currentNotationInteraction()->scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
                static const std::unordered_map<ScoreConfigType, std::string> configActions = {
                    { ScoreConfigType::ShowInvisibleElements, SHOW_INVISIBLE_CODE },
                    { ScoreConfigType::ShowUnprintableElements, SHOW_UNPRINTABLE_CODE },
                    { ScoreConfigType::ShowFrames, SHOW_FRAMES_CODE },
                    { ScoreConfigType::ShowPageMargins, SHOW_PAGEBORDERS_CODE },
                    { ScoreConfigType::MarkIrregularMeasures, SHOW_IRREGULAR_CODE },
                    { ScoreConfigType::ShowSoundFlags, SHOW_SOUND_FLAGS },
                };

                m_actionCheckedChanged.send({ configActions.at(configType) });
            });
        }

        m_actionCheckedChanged.send({ TOGGLE_CONCERT_PITCH_CODE });
        m_controller->currentNotationStyleChanged().onNotify(this, [this]() {
            m_actionCheckedChanged.send({ TOGGLE_CONCERT_PITCH_CODE });
        });
    });

    engravingConfiguration()->debuggingOptionsChanged().onNotify(this, [this]() {
        ActionCodeList actions;
        for (const UiAction& action : m_engravingDebuggingActions) {
            actions.push_back(action.code);
        }
        m_actionCheckedChanged.send(actions);
    });
}

const UiActionList& NotationUiActions::actionsList() const
{
    static UiActionList alist;
    if (alist.empty()) {
        alist.insert(alist.end(), m_actions.begin(), m_actions.end());
        alist.insert(alist.end(), m_scoreConfigActions.begin(), m_scoreConfigActions.end());
        alist.insert(alist.end(), m_engravingDebuggingActions.begin(), m_engravingDebuggingActions.end());
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

bool NotationUiActions::isScoreConfigAction(const ActionCode& code) const
{
    for (const UiAction& a : m_scoreConfigActions) {
        if (a.code == code) {
            return true;
        }
    }
    return false;
}

bool NotationUiActions::isScoreConfigChecked(const ActionCode& code, const ScoreConfig& cfg) const
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
    if (SHOW_SOUND_FLAGS == code) {
        return cfg.isShowSoundFlags;
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

    if (act.code == TOGGLE_CONCERT_PITCH_CODE) {
        auto style = m_controller->currentNotationStyle();
        if (style) {
            return style->styleValue(StyleId::concertPitch).toBool();
        }
    }

    if (isScoreConfigAction(act.code)) {
        auto interaction = m_controller->currentNotationInteraction();
        if (interaction) {
            return isScoreConfigChecked(act.code, interaction->scoreConfig());
        }
    }

    auto engravingDebuggingActionsSearch = NotationActionController::engravingDebuggingActions.find(act.code);
    if (engravingDebuggingActionsSearch != NotationActionController::engravingDebuggingActions.cend()) {
        return engravingConfiguration()->debuggingOptions().*(engravingDebuggingActionsSearch->second);
    }

    return false;
}

muse::async::Channel<ActionCodeList> NotationUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<ActionCodeList> NotationUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

DurationType NotationUiActions::actionDurationType(const ActionCode& actionCode)
{
    static const QMap<ActionCode, DurationType> durations = {
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
    static const QMap<ActionCode, AccidentalType> accidentals = {
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
    static const QMap<ActionCode, int> dots = {
        { "pad-dot", 1 },
        { "pad-dot2", 2 },
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
    static const QMap<ActionCode, int> voices {
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
    static const QMap<ActionCode, SymbolId> articulations {
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

const muse::ui::ToolConfig& NotationUiActions::defaultNoteInputBarConfig()
{
    static ToolConfig config;
    if (!config.isValid()) {
        config.items = {
            { "note-input-by-note-name", true },
            { "note-input-by-duration", true },
            { "note-input-rhythm", false },
            { "note-input-repitch", false },
            { "note-input-realtime-auto", false },
            { "note-input-realtime-manual", false },
            { "note-input-timewise", false },
            { "", true },
            { "pad-note-1024", false },
            { "pad-note-512", false },
            { "pad-note-256", false },
            { "pad-note-128", false },
            { "pad-note-64", true },
            { "pad-note-32", true },
            { "pad-note-16", true },
            { "pad-note-8", true },
            { "pad-note-4", true },
            { "pad-note-2", true },
            { "pad-note-1", true },
            { "note-breve", false },
            { "note-longa", false },
            { "", true },
            { "pad-dot", true },
            { "pad-dot2", false },
            { "pad-dot3", false },
            { "pad-dot4", false },
            { "pad-rest", true },
            { "", true },
            { "flat2", true },
            { "flat", true },
            { "nat", true },
            { "sharp", true },
            { "sharp2", true },
            { "", true },
            { "tie", true },
            { "add-slur", true },
            { "lv", false },
            { "", true },
            { "add-marcato", true },
            { "add-sforzato", true },
            { "add-tenuto", true },
            { "add-staccato", true },
            { "", true },
            { "cross-staff-beaming", false },
            { "tuplet", true },
            { "flip", true },
            { "", true },
            { "voice-1", true },
            { "voice-2", true },
            { "voice-3", false },
            { "voice-4", false }
        };
    }
    return config;
}
