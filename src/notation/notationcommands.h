/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#pragma once

#include "rcommand/typedcommand.h"
#include "notationcommandsenums.h" // IWYU pragma: export

namespace mu::notation {
using muse::MnemonicString;
using muse::TranslatableString;
using muse::rcommand::Command;
using muse::rcommand::DataType;
using muse::rcommand::FieldsScheme;
using muse::rcommand::Field;
using muse::rcommand::Presence;
using muse::rcommand::Availability;
using muse::rcommand::Annotation;
using muse::rcommand::Decoration;
using muse::rcommand::Checkable;

struct CancelCommand {
    cprop_decl id = Command("command://notation/cancel");
    cprop_decl title = MnemonicString("action", "Cancel");
    cprop_decl description = TranslatableString("action", "Cancel the current notation operation");
    cprop_decl availabilities = Availability::All;
};

struct UndoCommand {
    cprop_decl id = Command("command://notation/undo");
    cprop_decl title = MnemonicString("action", "Undo");
    cprop_decl description = TranslatableString("action", "Undo the last notation operation");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::UNDO);
    cprop_decl availabilities = Availability::All;
};

struct RedoCommand {
    cprop_decl id = Command("command://notation/redo");
    cprop_decl title = MnemonicString("action", "Redo");
    cprop_decl description = TranslatableString("action", "Redo the last notation operation");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::REDO);
    cprop_decl availabilities = Availability::All;
};

struct CopyCommand {
    cprop_decl id = Command("command://notation/copy");
    cprop_decl title = MnemonicString("action", "&Copy");
    cprop_decl description = TranslatableString("action", "Copy the current notation element");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::COPY);
    cprop_decl availabilities = Availability::All;
    cprop_decl annotations = Annotation::Idempotent;
};

struct CopyPasteSwapCommand {
    cprop_decl id = Command("command://notation/copy-paste-swap");
    cprop_decl title = MnemonicString("action", "&Swap with clipboard");
    cprop_decl description = TranslatableString("action", "Copy/paste: swap with clipboard");
    cprop_decl availabilities = Availability::All;
};

struct CopyLyricsCommand {
    cprop_decl id = Command("command://notation/copy-lyrics");
    cprop_decl title = MnemonicString("action", "Copy &lyrics to clipboard");
    cprop_decl description = TranslatableString("action", "Copy lyrics");
    cprop_decl availabilities = Availability::All;
};

struct CutCommand {
    cprop_decl id = Command("command://notation/cut");
    cprop_decl title = MnemonicString("action", "Cu&t");
    cprop_decl description = TranslatableString("action", "Cut the current notation element");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::CUT);
    cprop_decl availabilities = Availability::All;
};

struct PasteCommand {
    cprop_decl id = Command("command://notation/paste");
    cprop_decl title = MnemonicString("action", "Paste");
    cprop_decl description = TranslatableString("action", "Paste the current notation element");
    cprop_decl availabilities = Availability::API;

    PastingType type = PastingType::Default;

    cprop_decl input = FieldsScheme {
        Field{ "type", &PasteCommand::type, "Pasting type", Presence::Optional },
    };
};

// aliases for paste command

struct PasteDefaultCommand {
    cprop_decl id = Command("command://notation/paste/default");
    cprop_decl title = MnemonicString("action", "Past&e");
    cprop_decl description = TranslatableString("action", "Paste the current notation element");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::PASTE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = PasteCommand(PastingType::Default);
};

struct PasteHalfCommand {
    cprop_decl id = Command("command://notation/paste/half");
    cprop_decl title = MnemonicString("action", "Paste &half duration");
    cprop_decl description = TranslatableString("action", "Paste half duration");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = PasteCommand(PastingType::Half);
};

struct PasteDoubleCommand {
    cprop_decl id = Command("command://notation/paste/double");
    cprop_decl title = MnemonicString("action", "Paste &double duration");
    cprop_decl description = TranslatableString("action", "Paste double duration");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = PasteCommand(PastingType::Double);
};

struct PasteSpecialCommand {
    cprop_decl id = Command("command://notation/paste/special");
    cprop_decl title = MnemonicString("action", "Paste special");
    cprop_decl description = TranslatableString("action", "Paste special");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = PasteCommand(PastingType::Special);
};

struct DeleteCommand {
    cprop_decl id = Command("command://notation/delete");
    cprop_decl title = MnemonicString("action", "De&lete");
    cprop_decl description = TranslatableString("action", "Delete the current notation element");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::DELETE_TANK);
    cprop_decl availabilities = Availability::All;
};

// select commands
struct SelectCommand {
    cprop_decl id = Command("command://notation/select");
    cprop_decl title = MnemonicString("action", "Select");
    cprop_decl description = TranslatableString("action", "Select items by target");
    cprop_decl availabilities = Availability::API;
    cprop_decl annotations = Annotation::None;

    SelectionTarget target = SelectionTarget::FirstItem;
    PlayMode playMode = PlayMode::NoPlay;

    cprop_decl input = FieldsScheme {
        Field{ "target", &SelectCommand::target, "Selection target" },
        Field{ "play-mode", &SelectCommand::playMode, "Play mode", Presence::Optional },
    };
};

// aliases for select command
struct SelectFirstItemCommand {
    cprop_decl id = Command("command://notation/select/first-item");
    cprop_decl title = MnemonicString("action", "First element");
    cprop_decl description = TranslatableString("action", "Go to first element in score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::FirstItem, PlayMode::PlayChord);
};

struct SelectLastItemCommand {
    cprop_decl id = Command("command://notation/select/last-item");
    cprop_decl title = MnemonicString("action", "Last element");
    cprop_decl description = TranslatableString("action", "Go to last element in score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::LastItem, PlayMode::PlayChord);
};

struct SelectNextItemCommand {
    cprop_decl id = Command("command://notation/select/next-item");
    cprop_decl title = MnemonicString("action", "Next element");
    cprop_decl description = TranslatableString("action", "Select next element in score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NextItem, PlayMode::PlayNote);
};

struct SelectPrevItemCommand {
    cprop_decl id = Command("command://notation/select/prev-item");
    cprop_decl title = MnemonicString("action", "Previous element");
    cprop_decl description = TranslatableString("action", "Select previous element in score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::PrevItem, PlayMode::PlayNote);
};

struct SelectNextSegmentCommand {
    cprop_decl id = Command("command://notation/select/next-segment");
    cprop_decl title = MnemonicString("action", "Next segment element");
    cprop_decl description = TranslatableString("action", "Select next segment element");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NextSegment, PlayMode::PlayNote);
};

struct SelectPrevSegmentCommand {
    cprop_decl id = Command("command://notation/select/prev-segment");
    cprop_decl title = MnemonicString("action", "Previous segment element");
    cprop_decl description = TranslatableString("action", "Select previous segment element");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::PrevSegment, PlayMode::PlayNote);
};

struct SelectNextTrackCommand {
    cprop_decl id = Command("command://notation/select/next-track");
    cprop_decl title = MnemonicString("action", "Next staff or voice");
    cprop_decl description = TranslatableString("action", "Go to next staff or voice");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NextTrack, PlayMode::PlayChord);
};

struct SelectPrevTrackCommand {
    cprop_decl id = Command("command://notation/select/prev-track");
    cprop_decl title = MnemonicString("action", "Previous staff or voice");
    cprop_decl description = TranslatableString("action", "Go to previous staff or voice");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::PrevTrack, PlayMode::PlayChord);
};

struct SelectNextFrameCommand {
    cprop_decl id = Command("command://notation/select/next-frame");
    cprop_decl title = MnemonicString("action", "Next frame");
    cprop_decl description = TranslatableString("action", "Go to next frame");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NextFrame);
};

struct SelectPrevFrameCommand {
    cprop_decl id = Command("command://notation/select/prev-frame");
    cprop_decl title = MnemonicString("action", "Previous frame");
    cprop_decl description = TranslatableString("action", "Go to previous frame");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::PrevFrame);
};

struct SelectNextSystemCommand {
    cprop_decl id = Command("command://notation/select/next-system");
    cprop_decl title = MnemonicString("action", "Next system");
    cprop_decl description = TranslatableString("action", "Go to next system");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NextSystem);
};

struct SelectPrevSystemCommand {
    cprop_decl id = Command("command://notation/select/prev-system");
    cprop_decl title = MnemonicString("action", "Previous system");
    cprop_decl description = TranslatableString("action", "Go to previous system");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::PrevSystem);
};

struct SelectUpNoteInChordCommand {
    cprop_decl id = Command("command://notation/select/up-note-in-chord");
    cprop_decl title = MnemonicString("action", "Up note in chord");
    cprop_decl description = TranslatableString("action", "Select note/rest above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::UpNoteInChord);
};

struct SelectDownNoteInChordCommand {
    cprop_decl id = Command("command://notation/select/down-note-in-chord");
    cprop_decl title = MnemonicString("action", "Down note in chord");
    cprop_decl description = TranslatableString("action", "Select note/rest below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::DownNoteInChord);
};

struct SelectTopNoteInChordCommand {
    cprop_decl id = Command("command://notation/select/top-note-in-chord");
    cprop_decl title = MnemonicString("action", "Top note in chord");
    cprop_decl description = TranslatableString("action", "Select top note in chord");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::TopNoteInChord);
};

struct SelectBottomNoteInChordCommand {
    cprop_decl id = Command("command://notation/select/bottom-note-in-chord");
    cprop_decl title = MnemonicString("action", "Bottom note in chord");
    cprop_decl description = TranslatableString("action", "Select bottom note in chord");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::BottomNoteInChord);
};

struct SelectTopStaffCommand {
    cprop_decl id = Command("command://notation/select/top-staff");
    cprop_decl title = MnemonicString("action", "Go to top staff");
    cprop_decl description = TranslatableString("action", "Go to top staff");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::TopStaff, PlayMode::PlayChord);
};

struct SelectEmptyTrailingMeasureCommand {
    cprop_decl id = Command("command://notation/select/empty-trailing-measure");
    cprop_decl title = MnemonicString("action", "Go to first empty trailing measure");
    cprop_decl description = TranslatableString("action", "Go to first empty trailing measure");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::EmptyTrailingMeasure);
};

struct SelectSimilarCommand {
    cprop_decl id = Command("command://notation/select/similar");
    cprop_decl title = MnemonicString("action", "Similar");
    cprop_decl description = TranslatableString("action", "Select similar elements");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::Similar);
};

struct SelectSimilarInStaffCommand {
    cprop_decl id = Command("command://notation/select/similar-in-staff");
    cprop_decl title = MnemonicString("action", "Similar on this staff");
    cprop_decl description = TranslatableString("action", "Select similar elements on the same staff");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::SimilarInStaff);
};

struct SelectSimilarInRangeCommand {
    cprop_decl id = Command("command://notation/select/similar-in-range");
    cprop_decl title = MnemonicString("action", "Similar in this range");
    cprop_decl description = TranslatableString("action", "Select similar elements in the selected range");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::SimilarInRange);
};

struct SelectNotesInChordCommand {
    cprop_decl id = Command("command://notation/select/notes-in-chord");
    cprop_decl title = MnemonicString("action", "Notes in chord");
    cprop_decl description = TranslatableString("action", "Select notes in chord");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::NotesInChord);
};

struct SelectAllCommand {
    cprop_decl id = Command("command://notation/select/all");
    cprop_decl title = MnemonicString("action", "Select &all");
    cprop_decl description = TranslatableString("action", "Select all");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::All);
};

struct SelectSectionCommand {
    cprop_decl id = Command("command://notation/select/section");
    cprop_decl title = MnemonicString("action", "Select sectio&n");
    cprop_decl description = TranslatableString("action", "Select section");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SelectCommand(SelectionTarget::Section);
};

struct AddToSelectionCommand {
    cprop_decl id = Command("command://notation/add-to-selection");
    cprop_decl title = MnemonicString("action", "Add to selection");
    cprop_decl description = TranslatableString("action", "Add items to selection by target");
    cprop_decl availabilities = Availability::API;
    cprop_decl annotations = Annotation::None;

    SelectionTarget target = SelectionTarget::BeginSystem;

    cprop_decl input = FieldsScheme {
        Field{ "target", &AddToSelectionCommand::target, "Selection target" },
    };
};

// aliases for add to selection command
struct AddToSelectionNextChordCommand {
    cprop_decl id = Command("command://notation/add-to-selection/next-chord");
    cprop_decl title = MnemonicString("action", "Add next chord to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: next note/rest");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::NextChord);
};

struct AddToSelectionPrevChordCommand {
    cprop_decl id = Command("command://notation/add-to-selection/prev-chord");
    cprop_decl title = MnemonicString("action", "Add previous chord to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: previous note/rest");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::PrevChord);
};

struct AddToSelectionNextMeasureCommand {
    cprop_decl id = Command("command://notation/add-to-selection/next-measure");
    cprop_decl title = MnemonicString("action", "Add next measure to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: next measure");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::NextMeasure);
};

struct AddToSelectionPrevMeasureCommand {
    cprop_decl id = Command("command://notation/add-to-selection/prev-measure");
    cprop_decl title = MnemonicString("action", "Add previous measure to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: previous measure");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::PrevMeasure);
};

struct AddToSelectionAboveStaffCommand {
    cprop_decl id = Command("command://notation/add-to-selection/above-staff");
    cprop_decl title = MnemonicString("action", "Add staff above to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: staff above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::AboveStaff);
};

struct AddToSelectionBelowStaffCommand {
    cprop_decl id = Command("command://notation/add-to-selection/below-staff");
    cprop_decl title = MnemonicString("action", "Add staff below to selection");
    cprop_decl description = TranslatableString("action", "Add to selection: staff below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::BelowStaff);
};

struct AddToSelectionBeginSystemCommand {
    cprop_decl id = Command("command://notation/add-to-selection/begin-system");
    cprop_decl title = MnemonicString("action", "Select to beginning of system");
    cprop_decl description = TranslatableString("action", "Add to selection: beginning of system");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::BeginSystem);
};

struct AddToSelectionEndSystemCommand {
    cprop_decl id = Command("command://notation/add-to-selection/end-system");
    cprop_decl title = MnemonicString("action", "Select to end of system");
    cprop_decl description = TranslatableString("action", "Add to selection: end of system");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::EndSystem);
};

struct AddToSelectionBeginScoreCommand {
    cprop_decl id = Command("command://notation/add-to-selection/begin-score");
    cprop_decl title = MnemonicString("action", "Select to beginning of score");
    cprop_decl description = TranslatableString("action", "Add to selection: beginning of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::BeginScore);
};

struct AddToSelectionEndScoreCommand {
    cprop_decl id = Command("command://notation/add-to-selection/end-score");
    cprop_decl title = MnemonicString("action", "Select to end of score");
    cprop_decl description = TranslatableString("action", "Add to selection: end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddToSelectionCommand(SelectionTarget::EndScore);
};

struct OpenSelectionOptionsCommand {
    cprop_decl id = Command("command://notation/open-selection-options");
    cprop_decl title = MnemonicString("action", "Selection options");
    cprop_decl description = TranslatableString("action", "Open selection options");
    cprop_decl availabilities = Availability::UI;
};

struct ShowLocationCommand {
    cprop_decl id = Command("command://notation/show-location");
    cprop_decl title = MnemonicString("action", "Show location");
    cprop_decl description = TranslatableString("action", "Show current location in score");
    cprop_decl availabilities = Availability::UI;
};

// note input commands
struct ToggleNoteInputCommand {
    cprop_decl id = Command("command://notation/toggle-note-input");
    cprop_decl title = MnemonicString("action", "Toggle note input");
    cprop_decl description = TranslatableString("action", "Toggle note input");
    cprop_decl availabilities = Availability::API;

    NoteInputMethod method = NoteInputMethod::DEFAULT;

    cprop_decl input = FieldsScheme {
        Field{ "method", &ToggleNoteInputCommand::method, "Note input method" },
    };
};

// aliases for toggle note input command
struct ToggleNoteInputDefaultCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/default");
    cprop_decl title = MnemonicString("action", "Note input");
    cprop_decl description = TranslatableString("action", "Toggle note input mode");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::EDIT, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::DEFAULT);
};

struct ToggleNoteInputByNameCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/by-note-name");
    cprop_decl title = MnemonicString("action", "Note input by note name");
    cprop_decl description = TranslatableString("action", "Toggle note input mode by note name");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::EDIT, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::BY_NOTE_NAME);
};

struct ToggleNoteInputByDurationCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/by-duration");
    cprop_decl title = MnemonicString("action", "Note input by duration");
    cprop_decl description = TranslatableString("action", "Toggle note input mode by duration");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::DURATION_CURSOR, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::BY_DURATION);
};

struct ToggleNoteInputRhythmCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/rhythm");
    cprop_decl title = MnemonicString("action", "Rhythm only (not pitch)");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: rhythm only (not pitch)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::RHYTHM_ONLY, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::RHYTHM);
};

struct ToggleNoteInputRepitchCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/repitch");
    cprop_decl title = MnemonicString("action", "Re-pitch existing notes");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: re-pitch existing notes");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::RE_PITCH, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::REPITCH);
};

struct ToggleNoteInputRealtimeAutoCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/realtime-auto");
    cprop_decl title = MnemonicString("action", "Real-time (metronome)");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: real-time (metronome)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::METRONOME, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::REALTIME_AUTO);
};

struct ToggleNoteInputRealtimeManualCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/realtime-manual");
    cprop_decl title = MnemonicString("action", "Real-time (foot pedal)");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: real-time (foot pedal)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::FOOT_PEDAL, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::REALTIME_MANUAL);
};

struct ToggleNoteInputTimewiseCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/timewise");
    cprop_decl title = MnemonicString("action", "Insert");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: insert (increases measure duration)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_PLUS, Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleNoteInputCommand(NoteInputMethod::TIMEWISE);
};

struct ToggleInsertModeCommand {
    cprop_decl id = Command("command://notation/toggle-note-input/toggle-insert-mode");
    cprop_decl title = MnemonicString("action", "Insert/overwrite");
    cprop_decl description = TranslatableString("action", "Toggle note input mode: insert/overwrite");
    cprop_decl availabilities = Availability::UI;
};

struct RealtimeAdvanceCommand {
    cprop_decl id = Command("command://notation/realtime-advance");
    cprop_decl title = MnemonicString("action", "Real-time advance");
    cprop_decl description = TranslatableString("action", "Real-time advance");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::METRONOME);
    cprop_decl availabilities = Availability::All;
};

// note input set duration commands
struct SetDurationCommand {
    cprop_decl id = Command("command://notation/set-duration");
    cprop_decl title = MnemonicString("action", "Set duration");
    cprop_decl description = TranslatableString("action", "Set duration");
    cprop_decl availabilities = Availability::API;

    DurationType duration = DurationType::V_QUARTER;

    cprop_decl input = FieldsScheme {
        Field{ "duration", &SetDurationCommand::duration, "Duration" },
    };
};

// aliases for set duration commands
struct SetDurationLongaCommand {
    cprop_decl id = Command("command://notation/set-duration/longa");
    cprop_decl title = MnemonicString("action", "Longa");
    cprop_decl description = TranslatableString("action", "Set duration: longa");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::LONGO);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_LONG);
};

struct SetDurationBreveCommand {
    cprop_decl id = Command("command://notation/set-duration/breve");
    cprop_decl title = MnemonicString("action", "Breve");
    cprop_decl description = TranslatableString("action", "Set duration: breve");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_WHOLE_DOUBLE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_BREVE);
};

struct SetDurationWholeCommand {
    cprop_decl id = Command("command://notation/set-duration/whole");
    cprop_decl title = MnemonicString("action", "Whole note");
    cprop_decl description = TranslatableString("action", "Set duration: whole note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_WHOLE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_WHOLE);
};

struct SetDurationHalfCommand {
    cprop_decl id = Command("command://notation/set-duration/half");
    cprop_decl title = MnemonicString("action", "Half note");
    cprop_decl description = TranslatableString("action", "Set duration: half note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_HALF);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_HALF);
};

struct SetDurationQuarterCommand {
    cprop_decl id = Command("command://notation/set-duration/quarter");
    cprop_decl title = MnemonicString("action", "Quarter note");
    cprop_decl description = TranslatableString("action", "Set duration: quarter note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_QUARTER);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_QUARTER);
};

struct SetDurationEighthCommand {
    cprop_decl id = Command("command://notation/set-duration/eighth");
    cprop_decl title = MnemonicString("action", "Eighth note");
    cprop_decl description = TranslatableString("action", "Set duration: eighth note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_8TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_EIGHTH);
};

struct SetDuration16thCommand {
    cprop_decl id = Command("command://notation/set-duration/16th");
    cprop_decl title = MnemonicString("action", "16th note");
    cprop_decl description = TranslatableString("action", "Set duration: 16th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_16TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_16TH);
};

struct SetDuration32ndCommand {
    cprop_decl id = Command("command://notation/set-duration/32nd");
    cprop_decl title = MnemonicString("action", "32nd note");
    cprop_decl description = TranslatableString("action", "Set duration: 32nd note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_32ND);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_32ND);
};

struct SetDuration64thCommand {
    cprop_decl id = Command("command://notation/set-duration/64th");
    cprop_decl title = MnemonicString("action", "64th note");
    cprop_decl description = TranslatableString("action", "Set duration: 64th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_64TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_64TH);
};

struct SetDuration128thCommand {
    cprop_decl id = Command("command://notation/set-duration/128th");
    cprop_decl title = MnemonicString("action", "128th note");
    cprop_decl description = TranslatableString("action", "Set duration: 128th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_128TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_128TH);
};

struct SetDuration256thCommand {
    cprop_decl id = Command("command://notation/set-duration/256th");
    cprop_decl title = MnemonicString("action", "256th note");
    cprop_decl description = TranslatableString("action", "Set duration: 256th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_256TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_256TH);
};

struct SetDuration512thCommand {
    cprop_decl id = Command("command://notation/set-duration/512th");
    cprop_decl title = MnemonicString("action", "512th note");
    cprop_decl description = TranslatableString("action", "Set duration: 512th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_512TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_512TH);
};

struct SetDuration1024thCommand {
    cprop_decl id = Command("command://notation/set-duration/1024th");
    cprop_decl title = MnemonicString("action", "1024th note");
    cprop_decl description = TranslatableString("action", "Set duration: 1024th note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_1024TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetDurationCommand(DurationType::V_1024TH);
};

// increase/decrease duration commands
struct ChangeDurationCommand {
    cprop_decl id = Command("command://notation/change-duration");
    cprop_decl title = MnemonicString("action", "Change duration");
    cprop_decl description = TranslatableString("action", "Change duration");
    cprop_decl availabilities = Availability::API;

    ChangeDurationMode mode = ChangeDurationMode::Double;

    cprop_decl input = FieldsScheme {
        Field{ "mode", &ChangeDurationCommand::mode, "Change duration mode" },
    };
};

// aliases for change duration command
struct ChangeDurationDoubleCommand {
    cprop_decl id = Command("command://notation/change-duration/double");
    cprop_decl title = MnemonicString("action", "Double duration");
    cprop_decl description = TranslatableString("action", "Double selected duration");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ChangeDurationCommand(ChangeDurationMode::Double);
};

struct ChangeDurationHalveCommand {
    cprop_decl id = Command("command://notation/change-duration/halve");
    cprop_decl title = MnemonicString("action", "Halve duration");
    cprop_decl description = TranslatableString("action", "Halve selected duration");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ChangeDurationCommand(ChangeDurationMode::Halve);
};

struct ChangeDurationDoubleDottedCommand {
    cprop_decl id = Command("command://notation/change-duration/double-dotted");
    cprop_decl title = MnemonicString("action", "Double selected duration (dotted)");
    cprop_decl description = TranslatableString("action", "Double selected duration (includes dotted values)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ChangeDurationCommand(ChangeDurationMode::DoubleDotted);
};

struct ChangeDurationHalveDottedCommand {
    cprop_decl id = Command("command://notation/change-duration/halve-dotted");
    cprop_decl title = MnemonicString("action", "Halve selected duration (dotted)");
    cprop_decl description = TranslatableString("action", "Halve selected duration (includes dotted values)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ChangeDurationCommand(ChangeDurationMode::HalveDotted);
};

struct ToggleDotsCommand {
    cprop_decl id = Command("command://notation/toggle-dots");
    cprop_decl title = MnemonicString("action", "Toggle dots");
    cprop_decl description = TranslatableString("action", "Toggle dots");
    cprop_decl availabilities = Availability::API;

    int dots = 1;

    cprop_decl input = FieldsScheme {
        Field{ "dots", &ToggleDotsCommand::dots, "Number of dots", 1, (int)engraving::MAX_DOTS },
    };
};

// aliases for toggle dots command
struct ToggleDotCommand {
    cprop_decl id = Command("command://notation/toggle-dots/1");
    cprop_decl title = MnemonicString("action", "Augmentation dot");
    cprop_decl description = TranslatableString("action", "Toggle duration dot");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_DOTTED);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleDotsCommand(1);
};

struct ToggleDot2Command {
    cprop_decl id = Command("command://notation/toggle-dots/2");
    cprop_decl title = MnemonicString("action", "Double augmentation dot");
    cprop_decl description = TranslatableString("action", "Toggle duration dot: double");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_DOTTED_2);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleDotsCommand(2);
};

struct ToggleDot3Command {
    cprop_decl id = Command("command://notation/toggle-dots/3");
    cprop_decl title = MnemonicString("action", "Triple augmentation dot");
    cprop_decl description = TranslatableString("action", "Toggle duration dot: triple");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_DOTTED_3);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleDotsCommand(3);
};

struct ToggleDot4Command {
    cprop_decl id = Command("command://notation/toggle-dots/4");
    cprop_decl title = MnemonicString("action", "Quadruple augmentation dot");
    cprop_decl description = TranslatableString("action", "Toggle duration dot: quadruple");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_DOTTED_4);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleDotsCommand(4);
};

struct ToggleRestCommand {
    cprop_decl id = Command("command://notation/toggle-rest");
    cprop_decl title = MnemonicString("action", "Rest");
    cprop_decl description = TranslatableString("action", "Toggle rest");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::REST);
    cprop_decl availabilities = Availability::All;
};

struct RequestNoteInputStateCommand {
    cprop_decl id = Command("command://notation/request-noteinput-state");
    cprop_decl title = MnemonicString("action", "Get note input state");
    cprop_decl description = TranslatableString("action", "Get note input state");
    cprop_decl availabilities = muse::rcommand::Availability::API;
    cprop_decl annotations = muse::rcommand::Annotation::ReadOnly;

    struct Output {
        bool enabled = false;
        NoteInputMethod method = NoteInputMethod::BY_NOTE_NAME;
    };

    using O = RequestNoteInputStateCommand::Output;
    cprop_decl output = std::tuple {
        Field{ "enabled", &O::enabled, "Flag indicating whether the note input mode is enabled" },
        Field{ "method", &O::method, "The method used to input the note" },
    };
};

// add note command
struct AddNoteCommand {
    cprop_decl id = muse::rcommand::Command("command://notation/add-note");
    cprop_decl title = MnemonicString("action", "Add note");
    cprop_decl description = TranslatableString("action", "Add note");
    cprop_decl availabilities = Availability::API;

    NoteName note = NoteName::C;
    NoteAddingMode mode = NoteAddingMode::NextChord;

    cprop_decl input = std::tuple {
        Field{ "note", &AddNoteCommand::note, "Note name" },
        Field{ "mode", &AddNoteCommand::mode, "Adding mode", Presence::Optional },
    };
};

//! NOTE Shared templates for note commands, declared once to avoid translation duplication.
//! They are created on first use, so they cannot be used before initialization
//! (the static initialization order across translation units is undefined).
namespace prv {
using TS = TranslatableString;
//: Note
inline const TS& noteC() { static const TS s = TranslatableString("action", "C"); return s; }
//: Note
inline const TS& noteD() { static const TS s = TranslatableString("action", "D"); return s; }
//: Note
inline const TS& noteE() { static const TS s = TranslatableString("action", "E"); return s; }
//: Note
inline const TS& noteF() { static const TS s = TranslatableString("action", "F"); return s; }
//: Note
inline const TS& noteG() { static const TS s = TranslatableString("action", "G"); return s; }
//: Note
inline const TS& noteA() { static const TS s = TranslatableString("action", "A"); return s; }
//: Note
inline const TS& noteB() { static const TS s = TranslatableString("action", "B"); return s; }
//: Enter note %1
inline const TS& Enter_note_X() { static const TS s = TranslatableString("action", "Enter note %1"); return s; }
//: Add %1 to chord
inline const TS& Add_X_to_chord() { static const TS s = TranslatableString("action", "Add %1 to chord"); return s; }
//: Add note %1 to chord
inline const TS& Add_note_X_to_chord() { static const TS s = TranslatableString("action", "Add note %1 to chord"); return s; }
//: Insert %1
inline const TS& Insert_X() { static const TS s = TranslatableString("action", "Insert %1"); return s; }
//: Fret %1 (TAB)
inline const TS& fret_X_TAB() { static const TS s = TranslatableString("action", "Fret %1 (TAB)"); return s; }
//: Enter TAB: fret %1
inline const TS& enter_TAB_fret_X() { static const TS s = TranslatableString("action", "Enter TAB: fret %1"); return s; }
} // namespace prv

// aliases for add note command
struct AddNoteC_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/c");
    cprop_decl title = MnemonicString(prv::noteC());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteC());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::C, NoteAddingMode::NextChord);
};

struct AddNoteD_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/d");
    cprop_decl title = MnemonicString(prv::noteD());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteD());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::D, NoteAddingMode::NextChord);
};

struct AddNoteE_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/e");
    cprop_decl title = MnemonicString(prv::noteE());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteE());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::E, NoteAddingMode::NextChord);
};

struct AddNoteF_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/f");
    cprop_decl title = MnemonicString(prv::noteF());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteF());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::F, NoteAddingMode::NextChord);
};

struct AddNoteG_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/g");
    cprop_decl title = MnemonicString(prv::noteG());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteG());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::G, NoteAddingMode::NextChord);
};

struct AddNoteA_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/a");
    cprop_decl title = MnemonicString(prv::noteA());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteA());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::A, NoteAddingMode::NextChord);
};

struct AddNoteB_ToNextChordCommand {
    cprop_decl id = Command("command://notation/add-note/next-chord/b");
    cprop_decl title = MnemonicString(prv::noteB());
    cprop_decl description = prv::Enter_note_X().arg(prv::noteB());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::B, NoteAddingMode::NextChord);
};

struct AddNoteC_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/c");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteC()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteC());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::C, NoteAddingMode::CurrentChord);
};

struct AddNoteD_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/d");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteD()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteD());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::D, NoteAddingMode::CurrentChord);
};

struct AddNoteE_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/e");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteE()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteE());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::E, NoteAddingMode::CurrentChord);
};

struct AddNoteF_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/f");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteF()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteF());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::F, NoteAddingMode::CurrentChord);
};

struct AddNoteG_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/g");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteG()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteG());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::G, NoteAddingMode::CurrentChord);
};

struct AddNoteA_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/a");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteA()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteA());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::A, NoteAddingMode::CurrentChord);
};

struct AddNoteB_ToCurrentChordCommand {
    cprop_decl id = Command("command://notation/add-note/current-chord/b");
    cprop_decl title = MnemonicString(prv::Add_X_to_chord().arg(prv::noteB()));
    cprop_decl description = prv::Add_note_X_to_chord().arg(prv::noteB());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::B, NoteAddingMode::CurrentChord);
};

struct AddNoteC_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/c");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteC()));
    cprop_decl description = prv::Insert_X().arg(prv::noteC());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::C, NoteAddingMode::InsertChord);
};

struct AddNoteD_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/d");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteD()));
    cprop_decl description = prv::Insert_X().arg(prv::noteD());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::D, NoteAddingMode::InsertChord);
};

struct AddNoteE_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/e");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteE()));
    cprop_decl description = prv::Insert_X().arg(prv::noteE());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::E, NoteAddingMode::InsertChord);
};

struct AddNoteF_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/f");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteF()));
    cprop_decl description = prv::Insert_X().arg(prv::noteF());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::F, NoteAddingMode::InsertChord);
};

struct AddNoteG_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/g");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteG()));
    cprop_decl description = prv::Insert_X().arg(prv::noteG());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::G, NoteAddingMode::InsertChord);
};

struct AddNoteA_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/a");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteA()));
    cprop_decl description = prv::Insert_X().arg(prv::noteA());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::A, NoteAddingMode::InsertChord);
};

struct AddNoteB_ToInsertChordCommand {
    cprop_decl id = Command("command://notation/add-note/insert-chord/b");
    cprop_decl title = MnemonicString(prv::Insert_X().arg(prv::noteB()));
    cprop_decl description = prv::Insert_X().arg(prv::noteB());
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddNoteCommand(NoteName::B, NoteAddingMode::InsertChord);
};

// accidental commands
struct ToggleAccidentalCommand {
    cprop_decl id = Command("command://notation/toggle-accidental");
    cprop_decl title = MnemonicString("action", "Toggle accidental");
    cprop_decl description = TranslatableString("action", "Toggle accidental");
    cprop_decl availabilities = Availability::API;

    AccidentalType type = AccidentalType::NATURAL;
    PlayMode playMode = PlayMode::PlayNote;
    bool affectNoteInput = false;

    cprop_decl input = FieldsScheme {
        Field{ "type", &ToggleAccidentalCommand::type, "Accidental type" },
        Field{ "play-mode", &ToggleAccidentalCommand::playMode, "Play mode", Presence::Optional },
        Field{ "affect-note-input", &ToggleAccidentalCommand::affectNoteInput, "Affect note input", Presence::Optional },
    };
};

// aliases for toggle accidental command
struct ToggleFlat2Command {
    cprop_decl id = Command("command://notation/toggle-flat2");
    cprop_decl title = MnemonicString("action", "Toggle double-flat");
    cprop_decl description = TranslatableString("action", "Toggle accidental: double-flat");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::FLAT_DOUBLE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleAccidentalCommand(AccidentalType::FLAT2, PlayMode::NoPlay, true);
};

struct ToggleFlatCommand {
    cprop_decl id = Command("command://notation/toggle-flat");
    cprop_decl title = MnemonicString("action", "Toggle flat");
    cprop_decl description = TranslatableString("action", "Toggle accidental: flat");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::FLAT);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleAccidentalCommand(AccidentalType::FLAT, PlayMode::NoPlay, true);
};

struct ToggleNaturalCommand {
    cprop_decl id = Command("command://notation/toggle-natural");
    cprop_decl title = MnemonicString("action", "Toggle natural");
    cprop_decl description = TranslatableString("action", "Toggle accidental: natural");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NATURAL);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleAccidentalCommand(AccidentalType::NATURAL, PlayMode::NoPlay, true);
};

struct ToggleSharpCommand {
    cprop_decl id = Command("command://notation/toggle-sharp");
    cprop_decl title = MnemonicString("action", "Toggle sharp");
    cprop_decl description = TranslatableString("action", "Toggle accidental: sharp");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::SHARP);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleAccidentalCommand(AccidentalType::SHARP, PlayMode::NoPlay, true);
};

struct ToggleSharp2Command {
    cprop_decl id = Command("command://notation/toggle-sharp2");
    cprop_decl title = MnemonicString("action", "Toggle double-sharp");
    cprop_decl description = TranslatableString("action", "Toggle accidental: double-sharp");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::SHARP_DOUBLE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleAccidentalCommand(AccidentalType::SHARP2, PlayMode::NoPlay, true);
};

// voice commands
struct UseVoiceCommand {
    cprop_decl id = Command("command://notation/use-voice");
    cprop_decl title = MnemonicString("action", "Voice");
    cprop_decl description = TranslatableString("action", "Use voice");
    cprop_decl availabilities = Availability::API;
    cprop_decl annotations = Annotation::Idempotent;

    int voice = 0;   // voice number (1-4)!!, not index (0-3)

    cprop_decl input = FieldsScheme {
        Field{ "voice", &UseVoiceCommand::voice, "Voice", 1, (int)engraving::MAX_VOICES },
    };
};

// aliases for use voice command

struct UseVoice1Command {
    cprop_decl id = Command("command://notation/use-voice/1");
    cprop_decl title = MnemonicString("action", "Voice 1");
    cprop_decl description = TranslatableString("action", "Use voice 1");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::VOICE_1);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = UseVoiceCommand(1);
};

struct UseVoice2Command {
    cprop_decl id = Command("command://notation/use-voice/2");
    cprop_decl title = MnemonicString("action", "Voice 2");
    cprop_decl description = TranslatableString("action", "Use voice 2");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::VOICE_2);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = UseVoiceCommand(2);
};

struct UseVoice3Command {
    cprop_decl id = Command("command://notation/use-voice/3");
    cprop_decl title = MnemonicString("action", "Voice 3");
    cprop_decl description = TranslatableString("action", "Use voice 3");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::VOICE_3);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = UseVoiceCommand(3);
};

struct UseVoice4Command {
    cprop_decl id = Command("command://notation/use-voice/4");
    cprop_decl title = MnemonicString("action", "Voice 4");
    cprop_decl description = TranslatableString("action", "Use voice 4");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::VOICE_4);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = UseVoiceCommand(4);
};

struct SwapVoicesCommand {
    cprop_decl id = Command("command://notation/swap-voices");
    cprop_decl title = MnemonicString("action", "Swap voices");
    cprop_decl description = TranslatableString("action", "Swap two voices");
    cprop_decl availabilities = Availability::API;
    cprop_decl annotations = Annotation::None;

    int voiceA = 0; // voice number (1-4)!!, not index (0-3)
    int voiceB = 0; // voice number (1-4)!!, not index (0-3)

    cprop_decl input = FieldsScheme {
        Field{ "voice-a", &SwapVoicesCommand::voiceA, "First of the two voices to swap", 1, (int)engraving::MAX_VOICES },
        Field{ "voice-b", &SwapVoicesCommand::voiceB, "Second of the two voices to swap", 1, (int)engraving::MAX_VOICES },
    };
};

// aliases for swap voices command

struct SwapVoice12Command {
    cprop_decl id = Command("command://notation/swap-voices/1-2");
    cprop_decl title = MnemonicString("action", "Exchange voice &1-2");
    cprop_decl description = TranslatableString("action", "Exchange voice 1-2");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(1, 2);
};

struct SwapVoice13Command {
    cprop_decl id = Command("command://notation/swap-voices/1-3");
    cprop_decl title = MnemonicString("action", "Exchange voice 1-3");
    cprop_decl description = TranslatableString("action", "Exchange voice 1-3");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(1, 3);
};

struct SwapVoice14Command {
    cprop_decl id = Command("command://notation/swap-voices/1-4");
    cprop_decl title = MnemonicString("action", "Exchange voice 1-&4");
    cprop_decl description = TranslatableString("action", "Exchange voice 1-4");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(1, 4);
};

struct SwapVoice23Command {
    cprop_decl id = Command("command://notation/swap-voices/2-3");
    cprop_decl title = MnemonicString("action", "Exchange voice &2-3");
    cprop_decl description = TranslatableString("action", "Exchange voice 2-3");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(2, 3);
};

struct SwapVoice24Command {
    cprop_decl id = Command("command://notation/swap-voices/2-4");
    cprop_decl title = MnemonicString("action", "Exchange voice 2-4");
    cprop_decl description = TranslatableString("action", "Exchange voice 2-4");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(2, 4);
};

struct SwapVoice34Command {
    cprop_decl id = Command("command://notation/swap-voices/3-4");
    cprop_decl title = MnemonicString("action", "Exchange voice &3-4");
    cprop_decl description = TranslatableString("action", "Exchange voice 3-4");
    cprop_decl availabilities = Availability::UI;
    cprop_decl annotations = Annotation::None;
    cprop_decl redirect = SwapVoicesCommand(3, 4);
};

// tuplet commands
struct AddTupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet");
    cprop_decl title = MnemonicString("action", "Tuplet");
    cprop_decl description = TranslatableString("action", "Add tuplet");
    cprop_decl availabilities = Availability::API;

    std::string ratio;   // Fraction as string (e.g. "3/2", "4/3", "5/4", "6/5", "7/6", "8/7", "9/8")
    TupletNumberType numberType = TupletNumberType::DEFAULT;
    TupletBracketType bracketType = TupletBracketType::DEFAULT;
    bool autoBaseLen = true;
    TupletSelectedDuration selectedDurationAs = TupletSelectedDuration::DEFAULT;

    cprop_decl input = FieldsScheme {
        Field{ "ratio", &AddTupletCommand::ratio, "Tuplet ratio (e.g. 3/2, 4/3, 5/4, 6/5, 7/6, 8/7, 9/8)" },
        Field{ "number-type", &AddTupletCommand::numberType, "Tuplet number type", Presence::Optional },
        Field{ "bracket-type", &AddTupletCommand::bracketType, "Tuplet bracket type", Presence::Optional },
        Field{ "auto-baselen", &AddTupletCommand::autoBaseLen, "Auto base length", Presence::Optional },
        Field{ "selected-duration-as", &AddTupletCommand::selectedDurationAs, "Use selected duration as subdivision or tuplet-length",
               Presence::Optional },
    };

    AddTupletCommand() = default;
    AddTupletCommand(int tupletCount)
        : ratio(std::to_string(tupletCount) + "/2") {}
};

// aliases for add tuplet command
struct AddDupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/duplet");
    cprop_decl title = MnemonicString("action", "Duplet");
    cprop_decl description = TranslatableString("action", "Add duplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(2);
};

struct AddTripletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/triplet");
    cprop_decl title = MnemonicString("action", "Triplet");
    cprop_decl description = TranslatableString("action", "Add triplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(3);
};

struct AddQuadrupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/quadruplet");
    cprop_decl title = MnemonicString("action", "Quadruplet");
    cprop_decl description = TranslatableString("action", "Add quadruplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(4);
};

struct AddQuintupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/quintuplet");
    cprop_decl title = MnemonicString("action", "Quintuplet");
    cprop_decl description = TranslatableString("action", "Add quintuplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(5);
};

struct AddSextupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/sextuplet");
    cprop_decl title = MnemonicString("action", "Sextuplet");
    cprop_decl description = TranslatableString("action", "Add sextuplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(6);
};

struct AddSeptupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/septuplet");
    cprop_decl title = MnemonicString("action", "Septuplet");
    cprop_decl description = TranslatableString("action", "Add septuplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(7);
};

struct AddOctupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/octuplet");
    cprop_decl title = MnemonicString("action", "Octuplet");
    cprop_decl description = TranslatableString("action", "Add octuplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(8);
};

struct AddNonupletCommand {
    cprop_decl id = Command("command://notation/add-tuplet/nonuplet");
    cprop_decl title = MnemonicString("action", "Nonuplet");
    cprop_decl description = TranslatableString("action", "Add nonuplet");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTupletCommand(9);
};

struct OpenTupletConfigureCommand {
    cprop_decl id = Command("command://notation/open-tuplet-configure");
    cprop_decl title = MnemonicString("action", "Othe&r…");
    cprop_decl description = TranslatableString("action", "Show tuplet configure");
    cprop_decl availabilities = Availability::UI;
};

struct AddBoxesCommand {
    cprop_decl id = Command("command://notation/add-boxes");
    cprop_decl title = MnemonicString("action", "Add boxes");
    cprop_decl description = TranslatableString("action", "Add boxes");
    cprop_decl availabilities = Availability::API;

    BoxType boxType = BoxType::Unknown;
    AddBoxesTarget target = AddBoxesTarget::BeforeSelection;
    int count = 1;
    bool needInputCount = false;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddBoxesCommand::boxType, "Box type" },
        Field{ "target", &AddBoxesCommand::target, "Target" },
        Field{ "count", &AddBoxesCommand::count, "Count", Presence::Optional },
        Field{ "need-input-count", &AddBoxesCommand::needInputCount, "Need input count (interactive mode)", Presence::Optional },
    };

    AddBoxesCommand() = default;
    AddBoxesCommand(BoxType boxType, AddBoxesTarget target, int count = 1, bool needInputCount = false)
        : boxType(boxType), target(target), count(count), needInputCount(needInputCount) {}
};

// aliases for add boxes command

struct InsertHBoxCommand {
    cprop_decl id = Command("command://notation/insert-hbox");
    cprop_decl title = MnemonicString("action", "Insert &horizontal frame");
    cprop_decl description = TranslatableString("action", "Insert horizontal frame");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Horizontal, AddBoxesTarget::BeforeSelection);
};

struct InsertVBoxCommand {
    cprop_decl id = Command("command://notation/insert-vbox");
    cprop_decl title = MnemonicString("action", "Insert &vertical frame");
    cprop_decl description = TranslatableString("action", "Insert vertical frame");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Vertical, AddBoxesTarget::BeforeSelection);
};

struct InsertTextFrameCommand {
    cprop_decl id = Command("command://notation/insert-textframe");
    cprop_decl title = MnemonicString("action", "Insert &text frame");
    cprop_decl description = TranslatableString("action", "Insert text frame");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Text, AddBoxesTarget::BeforeSelection);
};

struct InsertFretFrameCommand {
    cprop_decl id = Command("command://notation/insert-fretframe");
    cprop_decl title = MnemonicString("action", "Insert &fretboard diagram legend");
    cprop_decl description = TranslatableString("action", "Insert fretboard diagram legend");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Fret, AddBoxesTarget::BeforeSelection);
};

struct AppendHBoxCommand {
    cprop_decl id = Command("command://notation/append-hbox");
    cprop_decl title = MnemonicString("action", "&Horizontal frame");
    cprop_decl description = TranslatableString("action", "Insert horizontal frame at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Horizontal, AddBoxesTarget::AtEndOfScore);
};

struct AppendVBoxCommand {
    cprop_decl id = Command("command://notation/append-vbox");
    cprop_decl title = MnemonicString("action", "&Vertical frame");
    cprop_decl description = TranslatableString("action", "Insert vertical frame at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Vertical, AddBoxesTarget::AtEndOfScore);
};

struct AppendTextFrameCommand {
    cprop_decl id = Command("command://notation/append-textframe");
    cprop_decl title = MnemonicString("action", "&Text frame");
    cprop_decl description = TranslatableString("action", "Insert text frame at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Text, AddBoxesTarget::AtEndOfScore);
};

struct AppendFretFrameCommand {
    cprop_decl id = Command("command://notation/append-fretframe");
    cprop_decl title = MnemonicString("action", "&Fretboard diagram legend");
    cprop_decl description = TranslatableString("action", "Insert fretboard diagram legend at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Fret, AddBoxesTarget::AtEndOfScore);
};

struct InsertMeasureCommand {
    cprop_decl id = Command("command://notation/insert-measure");
    cprop_decl title = MnemonicString("action", "&Insert one measure before selection");
    cprop_decl description = TranslatableString("action", "Insert one measure before selection");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::INSERT_ONE_MEASURE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::BeforeSelection);
};

struct AppendMeasureCommand {
    cprop_decl id = Command("command://notation/append-measure");
    cprop_decl title = MnemonicString("action", "Insert &one measure at end of score");
    cprop_decl description = TranslatableString("action", "Insert one measure at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::AtEndOfScore);
};

struct InsertMeasuresAfterSelectionCommand {
    cprop_decl id = Command("command://notation/insert-measures-after-selection");
    cprop_decl title = MnemonicString("action", "Insert &after selection…");
    cprop_decl description = TranslatableString("action", "Insert measures after selection");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::AfterSelection, -1, true);
};

struct InsertMeasuresAtStartOfScoreCommand {
    cprop_decl id = Command("command://notation/insert-measures-at-start-of-score");
    cprop_decl title = MnemonicString("action", "Insert at &start of score…");
    cprop_decl description = TranslatableString("action", "Insert measures at start of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::AtStartOfScore, -1, true);
};

struct InsertMeasuresCommand {
    cprop_decl id = Command("command://notation/insert-measures");
    cprop_decl title = MnemonicString("action", "Insert &before selection…");
    cprop_decl description = TranslatableString("action", "Insert measures before selection");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::BeforeSelection, -1, true);
};

struct AppendMeasuresCommand {
    cprop_decl id = Command("command://notation/append-measures");
    cprop_decl title = MnemonicString("action", "Insert at &end of score…");
    cprop_decl description = TranslatableString("action", "Insert measures at end of score");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddBoxesCommand(BoxType::Measure, AddBoxesTarget::AtEndOfScore, -1, true);
};

// add grace notes commands
struct AddGraceNoteCommand {
    cprop_decl id = Command("command://notation/add-grace-note");
    cprop_decl title = MnemonicString("action", "Add grace note");
    cprop_decl description = TranslatableString("action", "Add grace note");
    cprop_decl availabilities = Availability::API;

    GraceNoteType type = GraceNoteType::NORMAL;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddGraceNoteCommand::type, "Grace note type" },
    };
};

// aliases for add grace notes command

struct AddAcciaccaturaCommand {
    cprop_decl id = Command("command://notation/add-grace-note/acciaccatura");
    cprop_decl title = MnemonicString("action", "Acciaccatura");
    cprop_decl description = TranslatableString("action", "Add grace note: acciaccatura");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::ACCIACCATURA);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::ACCIACCATURA);
};

struct AddAppoggiaturaCommand {
    cprop_decl id = Command("command://notation/add-grace-note/appoggiatura");
    cprop_decl title = MnemonicString("action", "Appoggiatura");
    cprop_decl description = TranslatableString("action", "Add grace note: appoggiatura");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::APPOGGIATURA);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::APPOGGIATURA);
};

struct AddGrace4Command {
    cprop_decl id = Command("command://notation/add-grace-note/grace4");
    cprop_decl title = MnemonicString("action", "Grace: quarter");
    cprop_decl description = TranslatableString("action", "Add grace note: quarter");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE4);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE4);
};

struct AddGrace16Command {
    cprop_decl id = Command("command://notation/add-grace-note/grace16");
    cprop_decl title = MnemonicString("action", "Grace: 16th");
    cprop_decl description = TranslatableString("action", "Add grace note: 16th");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE16);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE16);
};

struct AddGrace32Command {
    cprop_decl id = Command("command://notation/add-grace-note/grace32");
    cprop_decl title = MnemonicString("action", "Grace: 32nd");
    cprop_decl description = TranslatableString("action", "Add grace note: 32nd");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE32);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE32);
};

struct AddGrace8AfterCommand {
    cprop_decl id = Command("command://notation/add-grace-note/grace8-after");
    cprop_decl title = MnemonicString("action", "Grace: 8th after");
    cprop_decl description = TranslatableString("action", "Add grace note: eighth after");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE8_AFTER);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE8_AFTER);
};

struct AddGrace16AfterCommand {
    cprop_decl id = Command("command://notation/add-grace-note/grace16-after");
    cprop_decl title = MnemonicString("action", "Grace: 16th after");
    cprop_decl description = TranslatableString("action", "Add grace note: 16th after");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE16_AFTER);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE16_AFTER);
};

struct AddGrace32AfterCommand {
    cprop_decl id = Command("command://notation/add-grace-note/grace32-after");
    cprop_decl title = MnemonicString("action", "Grace: 32nd after");
    cprop_decl description = TranslatableString("action", "Add grace note: 32nd after");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::GRACE32_AFTER);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddGraceNoteCommand(GraceNoteType::GRACE32_AFTER);
};

struct AddIntervalCommand {
    cprop_decl id = Command("command://notation/add-interval");
    cprop_decl title = MnemonicString("action", "Add interval");
    cprop_decl description = TranslatableString("action", "Add interval");
    cprop_decl availabilities = Availability::API;

    int interval = 0;

    cprop_decl input = FieldsScheme {
        Field{ "interval", &AddIntervalCommand::interval, "Interval from -10 to 10, excluding 0 and -1", -10, 10 },
    };
};

// interval commands

struct AddInterval1Command {
    cprop_decl id = Command("command://notation/add-interval/1");
    cprop_decl title = MnemonicString("action", "&Unison");
    cprop_decl description = TranslatableString("action", "Enter interval: unison");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(1);
};

struct AddInterval2Command {
    cprop_decl id = Command("command://notation/add-interval/2");
    cprop_decl title = MnemonicString("action", "Se&cond above");
    cprop_decl description = TranslatableString("action", "Enter interval: second above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(2);
};

struct AddInterval3Command {
    cprop_decl id = Command("command://notation/add-interval/3");
    cprop_decl title = MnemonicString("action", "Thir&d above");
    cprop_decl description = TranslatableString("action", "Enter interval: third above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(3);
};

struct AddInterval4Command {
    cprop_decl id = Command("command://notation/add-interval/4");
    cprop_decl title = MnemonicString("action", "Fou&rth above");
    cprop_decl description = TranslatableString("action", "Enter interval: fourth above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(4);
};

struct AddInterval5Command {
    cprop_decl id = Command("command://notation/add-interval/5");
    cprop_decl title = MnemonicString("action", "Fift&h above");
    cprop_decl description = TranslatableString("action", "Enter interval: fifth above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(5);
};

struct AddInterval6Command {
    cprop_decl id = Command("command://notation/add-interval/6");
    cprop_decl title = MnemonicString("action", "Si&xth above");
    cprop_decl description = TranslatableString("action", "Enter interval: sixth above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(6);
};

struct AddInterval7Command {
    cprop_decl id = Command("command://notation/add-interval/7");
    cprop_decl title = MnemonicString("action", "Seve&nth above");
    cprop_decl description = TranslatableString("action", "Enter interval: seventh above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(7);
};

struct AddInterval8Command {
    cprop_decl id = Command("command://notation/add-interval/8");
    cprop_decl title = MnemonicString("action", "Octave &above");
    cprop_decl description = TranslatableString("action", "Enter interval: octave above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(8);
};

struct AddInterval9Command {
    cprop_decl id = Command("command://notation/add-interval/9");
    cprop_decl title = MnemonicString("action", "Ninth abov&e");
    cprop_decl description = TranslatableString("action", "Enter interval: ninth above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(9);
};

struct AddInterval10Command {
    cprop_decl id = Command("command://notation/add-interval/10");
    cprop_decl title = MnemonicString("action", "Tenth above");
    cprop_decl description = TranslatableString("action", "Enter interval: tenth above");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(10);
};

struct AddIntervalMinus2Command {
    cprop_decl id = Command("command://notation/add-interval/-2");
    cprop_decl title = MnemonicString("action", "&Second below");
    cprop_decl description = TranslatableString("action", "Enter interval: second below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-2);
};

struct AddIntervalMinus3Command {
    cprop_decl id = Command("command://notation/add-interval/-3");
    cprop_decl title = MnemonicString("action", "&Third below");
    cprop_decl description = TranslatableString("action", "Enter interval: third below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-3);
};

struct AddIntervalMinus4Command {
    cprop_decl id = Command("command://notation/add-interval/-4");
    cprop_decl title = MnemonicString("action", "F&ourth below");
    cprop_decl description = TranslatableString("action", "Enter interval: fourth below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-4);
};

struct AddIntervalMinus5Command {
    cprop_decl id = Command("command://notation/add-interval/-5");
    cprop_decl title = MnemonicString("action", "&Fifth below");
    cprop_decl description = TranslatableString("action", "Enter interval: fifth below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-5);
};

struct AddIntervalMinus6Command {
    cprop_decl id = Command("command://notation/add-interval/-6");
    cprop_decl title = MnemonicString("action", "S&ixth below");
    cprop_decl description = TranslatableString("action", "Enter interval: sixth below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-6);
};

struct AddIntervalMinus7Command {
    cprop_decl id = Command("command://notation/add-interval/-7");
    cprop_decl title = MnemonicString("action", "Se&venth below");
    cprop_decl description = TranslatableString("action", "Enter interval: seventh below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-7);
};

struct AddIntervalMinus8Command {
    cprop_decl id = Command("command://notation/add-interval/-8");
    cprop_decl title = MnemonicString("action", "Octave &below");
    cprop_decl description = TranslatableString("action", "Enter interval: octave below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-8);
};

struct AddIntervalMinus9Command {
    cprop_decl id = Command("command://notation/add-interval/-9");
    cprop_decl title = MnemonicString("action", "Ninth belo&w");
    cprop_decl description = TranslatableString("action", "Enter interval: ninth below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-9);
};

struct AddIntervalMinus10Command {
    cprop_decl id = Command("command://notation/add-interval/-10");
    cprop_decl title = MnemonicString("action", "Tenth below");
    cprop_decl description = TranslatableString("action", "Enter interval: tenth below");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddIntervalCommand(-10);
};

struct AddTextCommand {
    cprop_decl id = Command("command://notation/add-text");
    cprop_decl title = MnemonicString("action", "Add text");
    cprop_decl description = TranslatableString("action", "Add text");
    cprop_decl availabilities = Availability::API;

    TextStyleType type = TextStyleType::DEFAULT;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddTextCommand::type, "Text style type" },
    };
};

// aliases for add text command

struct AddTitleTextCommand {
    cprop_decl id = Command("command://notation/add-text/title");
    cprop_decl title = MnemonicString("action", "&Title");
    cprop_decl description = TranslatableString("action", "Add text: title");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::TITLE);
};

struct AddSubtitleTextCommand {
    cprop_decl id = Command("command://notation/add-text/subtitle");
    cprop_decl title = MnemonicString("action", "&Subtitle");
    cprop_decl description = TranslatableString("action", "Add text: subtitle");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::SUBTITLE);
};

struct AddComposerTextCommand {
    cprop_decl id = Command("command://notation/add-text/composer");
    cprop_decl title = MnemonicString("action", "&Composer");
    cprop_decl description = TranslatableString("action", "Add text: composer");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::COMPOSER);
};

struct AddLyricistTextCommand {
    cprop_decl id = Command("command://notation/add-text/lyricist");
    cprop_decl title = MnemonicString("action", "&Lyricist");
    cprop_decl description = TranslatableString("action", "Add text: lyricist");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::LYRICIST);
};

struct AddPartTextCommand {
    cprop_decl id = Command("command://notation/add-text/part-name");
    cprop_decl title = MnemonicString("action", "&Part name");
    cprop_decl description = TranslatableString("action", "Add text: part name");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::INSTRUMENT_EXCERPT);
};

struct AddFrameTextCommand {
    cprop_decl id = Command("command://notation/add-text/frame");
    cprop_decl title = MnemonicString("action", "Text");
    cprop_decl description = TranslatableString("action", "Add frame text");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::FRAME);
};

struct AddSystemTextCommand {
    cprop_decl id = Command("command://notation/add-text/system");
    cprop_decl title = MnemonicString("action", "Syst&em text");
    cprop_decl description = TranslatableString("action", "Add text: system text");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::SYSTEM);
};

struct AddStaffTextCommand {
    cprop_decl id = Command("command://notation/add-text/staff");
    cprop_decl title = MnemonicString("action", "St&aff text");
    cprop_decl description = TranslatableString("action", "Add text: staff text");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::STAFF);
};

struct AddExpressionTextCommand {
    cprop_decl id = Command("command://notation/add-text/expression");
    cprop_decl title = MnemonicString("action", "E&xpression text");
    cprop_decl description = TranslatableString("action", "Add text: expression text");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::EXPRESSION);
};

struct AddRehearsalMarkTextCommand {
    cprop_decl id = Command("command://notation/add-rehearsalmark");
    cprop_decl title = MnemonicString("action", "&Rehearsal mark");
    cprop_decl description = TranslatableString("action", "Add text: rehearsal mark");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::REHEARSAL_MARK);
};

struct AddInstrumentChangeTextCommand {
    cprop_decl id = Command("command://notation/add-instrument-change");
    cprop_decl title = MnemonicString("action", "&Instrument change");
    cprop_decl description = TranslatableString("action", "Add text: instrument change");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::INSTRUMENT_CHANGE);
};

struct AddFingeringTextCommand {
    cprop_decl id = Command("command://notation/add-fingering");
    cprop_decl title = MnemonicString("action", "&Fingering");
    cprop_decl description = TranslatableString("action", "Add text: fingering");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::FINGERING);
};

struct AddStickingTextCommand {
    cprop_decl id = Command("command://notation/add-sticking");
    cprop_decl title = MnemonicString("action", "Stic&king");
    cprop_decl description = TranslatableString("action", "Add text: sticking");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::STICKING);
};

struct AddChordTextCommand {
    cprop_decl id = Command("command://notation/add-chord-symbol");
    cprop_decl title = MnemonicString("action", "C&hord symbol");
    cprop_decl description = TranslatableString("action", "Add text: chord symbol");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::HARMONY_A);
};

struct AddRomanNumeralTextCommand {
    cprop_decl id = Command("command://notation/add-text/roman-numeral");
    cprop_decl title = MnemonicString("action", "R&oman numeral analysis");
    cprop_decl description = TranslatableString("action", "Add text: Roman numeral analysis");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::HARMONY_ROMAN);
};

struct AddNashvilleNumberTextCommand {
    cprop_decl id = Command("command://notation/add-text/nashville-number");
    cprop_decl title = MnemonicString("action", "&Nashville number");
    cprop_decl description = TranslatableString("action", "Add text: Nashville number");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::HARMONY_NASHVILLE);
};

struct AddLyricsCommand {
    cprop_decl id = Command("command://notation/add-lyrics");
    cprop_decl title = MnemonicString("action", "L&yrics");
    cprop_decl description = TranslatableString("action", "Add text: lyrics");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::LYRICS_ODD);
};

struct AddTempoCommand {
    cprop_decl id = Command("command://notation/add-tempo");
    cprop_decl title = MnemonicString("action", "Tempo &marking");
    cprop_decl description = TranslatableString("action", "Add text: tempo marking");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddTextCommand(TextStyleType::TEMPO);
};

struct SetBeamCommand {
    cprop_decl id = Command("command://notation/set-beam");
    cprop_decl title = MnemonicString("action", "Set beam");
    cprop_decl description = TranslatableString("action", "Set beam by mode");
    cprop_decl availabilities = Availability::API;

    BeamMode mode = BeamMode::AUTO;

    cprop_decl input = FieldsScheme {
        Field{ "mode", &SetBeamCommand::mode, "Beam mode" },
    };
};

// aliases for add beam command

struct SetBeamAutoCommand {
    cprop_decl id = Command("command://notation/set-beam/auto");
    cprop_decl title = MnemonicString("action", "Auto beam");
    cprop_decl description = TranslatableString("action", "Add beam: auto");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::AUTO_TEXT);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::AUTO);
};

struct SetBeamNoneCommand {
    cprop_decl id = Command("command://notation/set-beam/none");
    cprop_decl title = MnemonicString("action", "No beam");
    cprop_decl description = TranslatableString("action", "Add beam: none");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::BEAM_NONE);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::NONE);
};

struct SetBeamBeginCommand {
    cprop_decl id = Command("command://notation/set-beam/begin");
    cprop_decl title = MnemonicString("action", "Break beam left");
    cprop_decl description = TranslatableString("action", "Break beam left");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::BEAM_BREAK_LEFT);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::BEGIN);
};

struct SetBeamBegin16Command {
    cprop_decl id = Command("command://notation/set-beam/begin16");
    cprop_decl title = MnemonicString("action", "Break inner beams (8th)");
    cprop_decl description = TranslatableString("action", "Break inner beams (eighth)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::BEAM_BREAK_INNER_8TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::BEGIN16);
};

struct SetBeamBegin32Command {
    cprop_decl id = Command("command://notation/set-beam/begin32");
    cprop_decl title = MnemonicString("action", "Break inner beams (16th)");
    cprop_decl description = TranslatableString("action", "Break inner beams (16th)");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::BEAM_BREAK_INNER_16TH);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::BEGIN32);
};

struct SetBeamMidCommand {
    cprop_decl id = Command("command://notation/set-beam/mid");
    cprop_decl title = MnemonicString("action", "Join beams");
    cprop_decl description = TranslatableString("action", "Join beams");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::BEAM_JOIN);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = SetBeamCommand(BeamMode::MID);
};

struct SetBeamSelectedRangeCommand {
    cprop_decl id = Command("command://notation/set-beam/selected-range");
    cprop_decl title = MnemonicString("action", "Beam selected range");
    cprop_decl description = TranslatableString("action", "Beam selected range");
    cprop_decl availabilities = Availability::All;
};

struct AddOrnamentCommand {
    cprop_decl id = Command("command://notation/add-ornament");
    cprop_decl title = MnemonicString("action", "Add ornament");
    cprop_decl description = TranslatableString("action", "Add ornament");
    cprop_decl availabilities = Availability::API;

    OrnamentType type = OrnamentType::UNDEFINED;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddOrnamentCommand::type, "Ornament type" },
    };
};

// add ornament commands

struct AddOrnamentTurnCommand {
    cprop_decl id = Command("command://notation/add-ornament/turn");
    cprop_decl title = MnemonicString("action", "Toggle turn");
    cprop_decl description = TranslatableString("action", "Add ornament: turn");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TURN);
};

struct AddOrnamentTurnInvertedCommand {
    cprop_decl id = Command("command://notation/add-ornament/turn-inverted");
    cprop_decl title = MnemonicString("action", "Toggle inverted turn");
    cprop_decl description = TranslatableString("action", "Add ornament: inverted turn");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TURN_INVERTED);
};

struct AddOrnamentTurnSlashCommand {
    cprop_decl id = Command("command://notation/add-ornament/turn-slash");
    cprop_decl title = MnemonicString("action", "Toggle turn with slash");
    cprop_decl description = TranslatableString("action", "Add ornament: turn with slash");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TURN_SLASH);
};

struct AddOrnamentTurnUpCommand {
    cprop_decl id = Command("command://notation/add-ornament/turn-up");
    cprop_decl title = MnemonicString("action", "Toggle turn up");
    cprop_decl description = TranslatableString("action", "Add ornament: turn up");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TURN_UP);
};

struct AddOrnamentTurnInvertedUpCommand {
    cprop_decl id = Command("command://notation/add-ornament/turn-inverted-up");
    cprop_decl title = MnemonicString("action", "Toggle vertical inverted turn");
    cprop_decl description = TranslatableString("action", "Add ornament: vertical inverted turn");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TURN_INVERTED_UP);
};

struct AddOrnamentTrillCommand {
    cprop_decl id = Command("command://notation/add-ornament/trill");
    cprop_decl title = MnemonicString("action", "Toggle trill");
    cprop_decl description = TranslatableString("action", "Add ornament: trill");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TRILL);
};

struct AddOrnamentShortTrillCommand {
    cprop_decl id = Command("command://notation/add-ornament/short-trill");
    cprop_decl title = MnemonicString("action", "Toggle short trill");
    cprop_decl description = TranslatableString("action", "Add ornament: short trill");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::SHORT_TRILL);
};

struct AddOrnamentMordentCommand {
    cprop_decl id = Command("command://notation/add-ornament/mordent");
    cprop_decl title = MnemonicString("action", "Toggle mordent");
    cprop_decl description = TranslatableString("action", "Add ornament: mordent");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::MORDENT);
};

struct AddOrnamentHaydnCommand {
    cprop_decl id = Command("command://notation/add-ornament/haydn");
    cprop_decl title = MnemonicString("action", "Toggle Haydn ornament");
    cprop_decl description = TranslatableString("action", "Add ornament: Haydn ornament");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::HAYDN);
};

struct AddOrnamentTremblementCommand {
    cprop_decl id = Command("command://notation/add-ornament/tremblement");
    cprop_decl title = MnemonicString("action", "Toggle tremblement");
    cprop_decl description = TranslatableString("action", "Add ornament: tremblement");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TREMBLEMENT);
};

struct AddOrnamentPrallMordentCommand {
    cprop_decl id = Command("command://notation/add-ornament/prall-mordent");
    cprop_decl title = MnemonicString("action", "Toggle prall mordent");
    cprop_decl description = TranslatableString("action", "Add ornament: prall mordent");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::PRALL_MORDENT);
};

struct AddOrnamentShakeCommand {
    cprop_decl id = Command("command://notation/add-ornament/shake");
    cprop_decl title = MnemonicString("action", "Toggle shake");
    cprop_decl description = TranslatableString("action", "Add ornament: shake");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::SHAKE);
};

struct AddOrnamentShakeMuffatCommand {
    cprop_decl id = Command("command://notation/add-ornament/shake-muffat");
    cprop_decl title = MnemonicString("action", "Toggle shake (Muffat)");
    cprop_decl description = TranslatableString("action", "Add ornament: shake (Muffat)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::SHAKE_MUFFAT);
};

struct AddOrnamentTremblementCouperinCommand {
    cprop_decl id = Command("command://notation/add-ornament/tremblement-couperin");
    cprop_decl title = MnemonicString("action", "Toggle tremblement appuyé (Couperin)");
    cprop_decl description = TranslatableString("action", "Add ornament: tremblement appuyé (Couperin)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOrnamentCommand(OrnamentType::TREMBLEMENT_COUPERIN);
};

struct FlipCommand {
    cprop_decl id = Command("command://notation/flip");
    cprop_decl title = MnemonicString("action", "Flip direction");
    cprop_decl description = TranslatableString("action", "Flip direction");
    cprop_decl decoration = muse::rcommand::Decoration(muse::ui::IconCode::Code::NOTE_FLIP);
};

struct AddFretCommand {
    cprop_decl id = Command("command://notation/add-fret");
    cprop_decl title = MnemonicString("action", "Add fret");
    cprop_decl description = TranslatableString("action", "Add fret");
    cprop_decl availabilities = Availability::API;

    int num = 0;

    cprop_decl input = FieldsScheme {
        Field{ "num", &AddFretCommand::num, "Number", 0, 14 },
    };
};

// aliases for add fret command

struct AddFret0Command {
    cprop_decl id = Command("command://notation/add-fret/0");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(0));
    cprop_decl description = prv::enter_TAB_fret_X().arg(0);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(0);
};

struct AddFret1Command {
    cprop_decl id = Command("command://notation/add-fret/1");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(1));
    cprop_decl description = prv::enter_TAB_fret_X().arg(1);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(1);
};

struct AddFret2Command {
    cprop_decl id = Command("command://notation/add-fret/2");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(2));
    cprop_decl description = prv::enter_TAB_fret_X().arg(2);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(2);
};

struct AddFret3Command {
    cprop_decl id = Command("command://notation/add-fret/3");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(3));
    cprop_decl description = prv::enter_TAB_fret_X().arg(3);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(3);
};

struct AddFret4Command {
    cprop_decl id = Command("command://notation/add-fret/4");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(4));
    cprop_decl description = prv::enter_TAB_fret_X().arg(4);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(4);
};

struct AddFret5Command {
    cprop_decl id = Command("command://notation/add-fret/5");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(5));
    cprop_decl description = prv::enter_TAB_fret_X().arg(5);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(5);
};

struct AddFret6Command {
    cprop_decl id = Command("command://notation/add-fret/6");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(6));
    cprop_decl description = prv::enter_TAB_fret_X().arg(6);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(6);
};

struct AddFret7Command {
    cprop_decl id = Command("command://notation/add-fret/7");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(7));
    cprop_decl description = prv::enter_TAB_fret_X().arg(7);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(7);
};

struct AddFret8Command {
    cprop_decl id = Command("command://notation/add-fret/8");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(8));
    cprop_decl description = prv::enter_TAB_fret_X().arg(8);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(8);
};

struct AddFret9Command {
    cprop_decl id = Command("command://notation/add-fret/9");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(9));
    cprop_decl description = prv::enter_TAB_fret_X().arg(9);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(9);
};

struct AddFret10Command {
    cprop_decl id = Command("command://notation/add-fret/10");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(10));
    cprop_decl description = prv::enter_TAB_fret_X().arg(10);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(10);
};

struct AddFret11Command {
    cprop_decl id = Command("command://notation/add-fret/11");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(11));
    cprop_decl description = prv::enter_TAB_fret_X().arg(11);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(11);
};

struct AddFret12Command {
    cprop_decl id = Command("command://notation/add-fret/12");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(12));
    cprop_decl description = prv::enter_TAB_fret_X().arg(12);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(12);
};

struct AddFret13Command {
    cprop_decl id = Command("command://notation/add-fret/13");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(13));
    cprop_decl description = prv::enter_TAB_fret_X().arg(13);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(13);
};

struct AddFret14Command {
    cprop_decl id = Command("command://notation/add-fret/14");
    cprop_decl title = MnemonicString(prv::fret_X_TAB().arg(14));
    cprop_decl description = prv::enter_TAB_fret_X().arg(14);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddFretCommand(14);
};

// text navigation commands

struct EditTextAdvanceCommand {
    cprop_decl id = Command("command://notation/edittext-advance");
    cprop_decl title = MnemonicString("action", "Advance cursor");
    cprop_decl description = TranslatableString("action", "Advance cursor in chord symbols / figured bass");
    cprop_decl availabilities = Availability::API;

    DurationType duration = DurationType::V_QUARTER;

    cprop_decl input = FieldsScheme {
        Field{ "duration", &EditTextAdvanceCommand::duration, "Duration to advance by" },
    };
};

// aliases for edittext advance command

struct EditTextAdvanceLongaCommand {
    cprop_decl id = Command("command://notation/edittext-advance/longa");
    cprop_decl title = MnemonicString("action", "Advance cursor: longa (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: longa (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_LONG);
};

struct EditTextAdvanceBreveCommand {
    cprop_decl id = Command("command://notation/edittext-advance/breve");
    cprop_decl title = MnemonicString("action", "Advance cursor: breve (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: breve (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_BREVE);
};

struct EditTextAdvance1Command {
    cprop_decl id = Command("command://notation/edittext-advance/1");
    cprop_decl title = MnemonicString("action", "Advance cursor: whole note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: whole note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_WHOLE);
};

struct EditTextAdvance2Command {
    cprop_decl id = Command("command://notation/edittext-advance/2");
    cprop_decl title = MnemonicString("action", "Advance cursor: half note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: half note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_HALF);
};

struct EditTextAdvance4Command {
    cprop_decl id = Command("command://notation/edittext-advance/4");
    cprop_decl title = MnemonicString("action", "Advance cursor: quarter note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: quarter note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_QUARTER);
};

struct EditTextAdvance8Command {
    cprop_decl id = Command("command://notation/edittext-advance/8");
    cprop_decl title = MnemonicString("action", "Advance cursor: eighth note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: eighth note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_EIGHTH);
};

struct EditTextAdvance16Command {
    cprop_decl id = Command("command://notation/edittext-advance/16");
    cprop_decl title = MnemonicString("action", "Advance cursor: 16th note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: 16th note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_16TH);
};

struct EditTextAdvance32Command {
    cprop_decl id = Command("command://notation/edittext-advance/32");
    cprop_decl title = MnemonicString("action", "Advance cursor: 32nd note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: 32nd note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_32ND);
};

struct EditTextAdvance64Command {
    cprop_decl id = Command("command://notation/edittext-advance/64");
    cprop_decl title = MnemonicString("action", "Advance cursor: 64th note (chord symbols/figured bass)");
    cprop_decl description = TranslatableString("action", "Advance cursor: 64th note (chord symbols/figured bass)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextAdvanceCommand(DurationType::V_64TH);
};

struct EditTextFontStyleCommand {
    cprop_decl id = Command("command://notation/edittext/fontstyle");
    cprop_decl title = MnemonicString("action", "Toggle font style");
    cprop_decl description = TranslatableString("action", "Toggle font style of the edited text");
    cprop_decl availabilities = Availability::API;

    FontStyle style = FontStyle::Bold;

    cprop_decl input = FieldsScheme {
        Field{ "style", &EditTextFontStyleCommand::style, "Font style" },
    };
};

// aliases for edittext toggle font style command

struct EditTextBoldCommand {
    cprop_decl id = Command("command://notation/edittext/fontstyle/bold");
    cprop_decl title = MnemonicString("action", "Toggle bold");
    cprop_decl description = TranslatableString("action", "Toggle bold");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextFontStyleCommand(FontStyle::Bold);
};

struct EditTextItalicCommand {
    cprop_decl id = Command("command://notation/edittext/fontstyle/italic");
    cprop_decl title = MnemonicString("action", "Toggle italic");
    cprop_decl description = TranslatableString("action", "Toggle italic");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextFontStyleCommand(FontStyle::Italic);
};

struct EditTextUnderlineCommand {
    cprop_decl id = Command("command://notation/edittext/fontstyle/underline");
    cprop_decl title = MnemonicString("action", "Toggle underline");
    cprop_decl description = TranslatableString("action", "Toggle underline");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextFontStyleCommand(FontStyle::Underline);
};

struct EditTextStrikeCommand {
    cprop_decl id = Command("command://notation/edittext/fontstyle/strike");
    cprop_decl title = MnemonicString("action", "Toggle strike");
    cprop_decl description = TranslatableString("action", "Toggle strike");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextFontStyleCommand(FontStyle::Strike);
};

struct EditTextValignCommand {
    cprop_decl id = Command("command://notation/edittext/valign");
    cprop_decl title = MnemonicString("action", "Toggle vertical alignment");
    cprop_decl description = TranslatableString("action", "Toggle subscript / superscript of the edited text");
    cprop_decl availabilities = Availability::API;

    VerticalAlignment align = VerticalAlignment::AlignSubScript;

    cprop_decl input = FieldsScheme {
        Field{ "align", &EditTextValignCommand::align, "Vertical alignment" },
    };
};

// aliases for edittext valign command

struct EditTextSubScriptCommand {
    cprop_decl id = Command("command://notation/edittext/valign/subscript");
    cprop_decl title = MnemonicString("action", "Toggle subscript");
    cprop_decl description = TranslatableString("action", "Toggle subscript");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextValignCommand(VerticalAlignment::AlignSubScript);
};

struct EditTextSuperScriptCommand {
    cprop_decl id = Command("command://notation/edittext/valign/superscript");
    cprop_decl title = MnemonicString("action", "Toggle superscript");
    cprop_decl description = TranslatableString("action", "Toggle superscript");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = EditTextValignCommand(VerticalAlignment::AlignSuperScript);
};

struct ToggleTieCommand {
    cprop_decl id = Command("command://notation/toggle-tie");
    cprop_decl title = MnemonicString("action", "Tie");
    cprop_decl description = TranslatableString("action", "Add tied note");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_TIE);
    cprop_decl availabilities = Availability::All;
};

struct AddSlurCommand {
    cprop_decl id = Command("command://notation/add-slur");
    cprop_decl title = MnemonicString("action", "Slur");
    cprop_decl description = TranslatableString("action", "Add slur");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_SLUR);
    cprop_decl availabilities = Availability::All;
};

struct ToggleLaissezVibrerCommand {
    cprop_decl id = Command("command://notation/toggle-laissez-vibrer");
    cprop_decl title = MnemonicString("action", "Laissez vibrer");
    cprop_decl description = TranslatableString("action", "Add laissez vibrer");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::NOTE_LV);
    cprop_decl availabilities = Availability::All;
};
}

namespace mu::notation {
// text navigation commands

struct TextNavigateCommand {
    cprop_decl id = Command("command://notation/text-navigate");
    cprop_decl title = MnemonicString("action", "Navigate to text");
    cprop_decl description = TranslatableString("action", "Navigate to the next or previous text element");
    cprop_decl availabilities = Availability::API;

    TextMoveDirection direction = TextMoveDirection::Next;
    TextNavigationMode mode = TextNavigationMode::Element;

    cprop_decl input = FieldsScheme {
        Field{ "direction", &TextNavigateCommand::direction, "Navigation direction" },
        Field{ "mode", &TextNavigateCommand::mode, "Navigation mode", Presence::Optional },
    };
};

// aliases for navigate to text command

struct TextNavigateNextWordCommand {
    cprop_decl id = Command("command://notation/text-navigate/next-word");
    cprop_decl title = MnemonicString("action", "Edit next word");
    cprop_decl description = TranslatableString("action", "Go to edit next notation word");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = TextNavigateCommand(TextMoveDirection::Next, TextNavigationMode::Word);
};

struct TextNavigateNextElementCommand {
    cprop_decl id = Command("command://notation/text-navigate/next-element");
    cprop_decl title = MnemonicString("action", "Edit next text element");
    cprop_decl description = TranslatableString("action", "Go to edit next notation text element");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = TextNavigateCommand(TextMoveDirection::Next, TextNavigationMode::Element);
};

struct TextNavigatePrevElementCommand {
    cprop_decl id = Command("command://notation/text-navigate/prev-element");
    cprop_decl title = MnemonicString("action", "Edit previous text element");
    cprop_decl description = TranslatableString("action", "Go to edit previous notation text element");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = TextNavigateCommand(TextMoveDirection::Prev, TextNavigationMode::Element);
};

struct TextNavigateNextBeatCommand {
    cprop_decl id = Command("command://notation/text-navigate/next-beat");
    cprop_decl title = MnemonicString("action", "Advance cursor: next beat (chord symbols)");
    cprop_decl description = TranslatableString("action", "Advance cursor: next beat (chord symbols)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = TextNavigateCommand(TextMoveDirection::Next, TextNavigationMode::Beat);
};

struct TextNavigatePrevBeatCommand {
    cprop_decl id = Command("command://notation/text-navigate/prev-beat");
    cprop_decl title = MnemonicString("action", "Advance cursor: previous beat (chord symbols)");
    cprop_decl description = TranslatableString("action", "Advance cursor: previous beat (chord symbols)");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = TextNavigateCommand(TextMoveDirection::Prev, TextNavigationMode::Beat);
};

// lyrics navigation commands

struct LyricsNavigateCommand {
    cprop_decl id = Command("command://notation/lyrics-navigate");
    cprop_decl title = MnemonicString("action", "Navigate lyrics");
    cprop_decl description = TranslatableString("action", "Navigate between lyric verses or syllables");
    cprop_decl availabilities = Availability::API;

    LyricsNavigateAction action = LyricsNavigateAction::NextVerse;

    cprop_decl input = FieldsScheme {
        Field{ "action", &LyricsNavigateCommand::action, "Lyrics navigation action" },
    };
};

struct LyricsNavigateNextVerseCommand {
    cprop_decl id = Command("command://notation/lyrics-navigate/next-verse");
    cprop_decl title = MnemonicString("action", "Next lyric verse");
    cprop_decl description = TranslatableString("action", "Move text/go to next lyric verse");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = LyricsNavigateCommand(LyricsNavigateAction::NextVerse);
};

struct LyricsNavigatePrevVerseCommand {
    cprop_decl id = Command("command://notation/lyrics-navigate/prev-verse");
    cprop_decl title = MnemonicString("action", "Previous lyric verse");
    cprop_decl description = TranslatableString("action", "Move text/go to previous lyric verse");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = LyricsNavigateCommand(LyricsNavigateAction::PrevVerse);
};

struct LyricsNavigateNextSyllableCommand {
    cprop_decl id = Command("command://notation/lyrics-navigate/next-syllable");
    cprop_decl title = MnemonicString("action", "Next syllable");
    cprop_decl description = TranslatableString("action", "Lyrics: enter hyphen");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = LyricsNavigateCommand(LyricsNavigateAction::NextSyllable);
};

// lyrics editing commands

struct EditLyricsAddMelismaCommand {
    cprop_decl id = Command("command://notation/editlyrics-add-melisma");
    cprop_decl title = MnemonicString("action", "Add extension line");
    cprop_decl description = TranslatableString("action", "Lyrics: enter extension line");
    cprop_decl availabilities = Availability::All;
};

struct EditLyricsAddVerseCommand {
    cprop_decl id = Command("command://notation/editlyrics-add-verse");
    cprop_decl title = MnemonicString("action", "Add lyrics verse");
    cprop_decl description = TranslatableString("action", "Add lyrics verse");
    cprop_decl availabilities = Availability::All;
};

struct ExtendToNextNoteCommand {
    cprop_decl id = Command("command://notation/extend-to-next-note");
    cprop_decl title = MnemonicString("action", "Extend to next note");
    cprop_decl description = TranslatableString("action", "Extend to next note");
    cprop_decl availabilities = Availability::All;
};

struct ToggleArticulationCommand {
    cprop_decl id = Command("command://notation/toggle-articulation");
    cprop_decl title = MnemonicString("action", "Toggle articulation");
    cprop_decl description = TranslatableString("action", "Toggle articulation");
    cprop_decl availabilities = Availability::API;

    ArticulationType type = ArticulationType::UNDEFINED;

    cprop_decl input = FieldsScheme {
        Field{ "type", &ToggleArticulationCommand::type, "Articulation type" },
    };
};

struct ToggleMarcatoCommand {
    cprop_decl id = Command("command://notation/toggle-articulation/marcato");
    cprop_decl title = MnemonicString("action", "Marcato");
    cprop_decl description = TranslatableString("action", "Add articulation: marcato");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::MARCATO);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleArticulationCommand(ArticulationType::MARCATO);
};

struct ToggleSforzatoCommand {
    cprop_decl id = Command("command://notation/toggle-articulation/sforzato");
    cprop_decl title = MnemonicString("action", "Accent");
    cprop_decl description = TranslatableString("action", "Add articulation: accent");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::ACCENT);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleArticulationCommand(ArticulationType::SFORZATO);
};

struct ToggleTenutoCommand {
    cprop_decl id = Command("command://notation/toggle-articulation/tenuto");
    cprop_decl title = MnemonicString("action", "Tenuto");
    cprop_decl description = TranslatableString("action", "Add articulation: tenuto");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::TENUTO);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleArticulationCommand(ArticulationType::TENUTO);
};

struct ToggleStaccatoCommand {
    cprop_decl id = Command("command://notation/toggle-articulation/staccato");
    cprop_decl title = MnemonicString("action", "Staccato");
    cprop_decl description = TranslatableString("action", "Add articulation: staccato");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::STACCATO);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleArticulationCommand(ArticulationType::STACCATO);
};

// voice commands

inline static const muse::rcommand::Command VOICE_ASSIGNMENT_ALL_IN_INSTR_COMMAND("command://notation/voice-assignment-all-in-instrument");
inline static const muse::rcommand::Command VOICE_ASSIGNMENT_ALL_IN_STAFF_COMMAND("command://notation/voice-assignment-all-in-staff");

//inline static const muse::rcommand::Command FLIP_COMMAND("command://notation/flip");
inline static const muse::rcommand::Command FLIP_HORIZONTALLY_COMMAND("command://notation/flip-horizontally");

//inline static const muse::rcommand::Command ADD_NOTE_COMMAND("command://notation/add-note"); // with params
inline static const muse::rcommand::Command ADD_DRUM_NOTE_COMMAND("command://notation/add-drum-note"); // with params

inline static const muse::rcommand::Command ENTER_REST_COMMAND("command://notation/enter-rest");

// add commands

inline static const muse::rcommand::Command ADD_FRETBOARD_DIAGRAM_COMMAND("command://notation/add-fretboard-diagram");

struct AddOttavaCommand {
    cprop_decl id = Command("command://notation/add-ottava");
    cprop_decl title = MnemonicString("action", "Add ottava");
    cprop_decl description = TranslatableString("action", "Add ottava");
    cprop_decl availabilities = Availability::API;

    OttavaType type = OttavaType::OTTAVA_8VA;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddOttavaCommand::type, "Ottava type" },
    };
};

struct AddOttava8vaCommand {
    cprop_decl id = Command("command://notation/add-ottava/8va");
    cprop_decl title = MnemonicString("action", "Ottava 8va &alta");
    cprop_decl description = TranslatableString("action", "Add ottava 8va alta");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOttavaCommand(OttavaType::OTTAVA_8VA);
};

struct AddOttava8vbCommand {
    cprop_decl id = Command("command://notation/add-ottava/8vb");
    cprop_decl title = MnemonicString("action", "Ottava 8va &bassa");
    cprop_decl description = TranslatableString("action", "Add ottava 8va bassa");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddOttavaCommand(OttavaType::OTTAVA_8VB);
};

inline static const muse::rcommand::Command ADD_DYNAMIC_COMMAND("command://notation/add-dynamic");
inline static const muse::rcommand::Command ADD_HAIRPIN_COMMAND("command://notation/add-hairpin");
inline static const muse::rcommand::Command ADD_HAIRPIN_REVERSE_COMMAND("command://notation/add-hairpin-reverse");
inline static const muse::rcommand::Command INCREASE_DYNAMIC_COMMAND("command://notation/increase-dynamic");
inline static const muse::rcommand::Command DECREASE_DYNAMIC_COMMAND("command://notation/decrease-dynamic");
inline static const muse::rcommand::Command ADD_NOTELINE_COMMAND("command://notation/add-noteline");

inline static const muse::rcommand::Command ADD_IMAGE_COMMAND("command://notation/add-image");

inline static const muse::rcommand::Command ADD_UP_BOW_COMMAND("command://notation/add-up-bow");
inline static const muse::rcommand::Command ADD_DOWN_BOW_COMMAND("command://notation/add-down-bow");

// add text commands

inline static const muse::rcommand::Command ADD_FIGURED_BASS_COMMAND("command://notation/add-figured-bass");

// add brackets commands
inline static const muse::rcommand::Command ADD_BRACKETS_COMMAND("command://notation/add-brackets");
inline static const muse::rcommand::Command ADD_PARENTHESES_COMMAND("command://notation/add-parentheses");
inline static const muse::rcommand::Command ADD_BRACES_COMMAND("command://notation/add-braces");

// clef commands

struct AddClefCommand {
    cprop_decl id = Command("command://notation/add-clef");
    cprop_decl title = MnemonicString("action", "Add clef");
    cprop_decl description = TranslatableString("action", "Insert a clef at the current position");
    cprop_decl availabilities = Availability::API;

    ClefType type = ClefType::G;

    cprop_decl input = FieldsScheme {
        Field{ "type", &AddClefCommand::type, "Clef type" },
    };
};

// aliases for add clef command

struct AddClefViolinCommand {
    cprop_decl id = Command("command://notation/add-clef/violin");
    cprop_decl title = MnemonicString("action", "Add treble clef");
    cprop_decl description = TranslatableString("action", "Add clef: treble");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddClefCommand(ClefType::G);
};

struct AddClefBassCommand {
    cprop_decl id = Command("command://notation/add-clef/bass");
    cprop_decl title = MnemonicString("action", "Add bass clef");
    cprop_decl description = TranslatableString("action", "Add clef: bass");
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = AddClefCommand(ClefType::F);
};

inline static const muse::rcommand::Command ADD_FULL_MEASURE_REST_COMMAND("command://notation/add-full-measure-rest");

inline static const muse::rcommand::Command EDIT_ELEMENT_COMMAND("command://notation/edit-element");

// move commands
inline static const muse::rcommand::Command MOVE_RIGHT_COMMAND("command://notation/move-right");
inline static const muse::rcommand::Command MOVE_LEFT_COMMAND("command://notation/move-left");
inline static const muse::rcommand::Command MOVE_RIGHT_QUICKLY_COMMAND("command://notation/move-right-quickly");
inline static const muse::rcommand::Command MOVE_LEFT_QUICKLY_COMMAND("command://notation/move-left-quickly");
inline static const muse::rcommand::Command MOVE_UP_COMMAND("command://notation/move-up");
inline static const muse::rcommand::Command MOVE_DOWN_COMMAND("command://notation/move-down");
inline static const muse::rcommand::Command SWAP_LEFT_COMMAND("command://notation/swap-left");
inline static const muse::rcommand::Command SWAP_RIGHT_COMMAND("command://notation/swap-right");

inline static const muse::rcommand::Command PITCH_UP_COMMAND("command://notation/pitch-up");
inline static const muse::rcommand::Command PITCH_DOWN_COMMAND("command://notation/pitch-down");
inline static const muse::rcommand::Command PITCH_UP_OCTAVE_COMMAND("command://notation/pitch-up-octave");
inline static const muse::rcommand::Command PITCH_DOWN_OCTAVE_COMMAND("command://notation/pitch-down-octave");
inline static const muse::rcommand::Command PITCH_UP_DIATONIC_COMMAND("command://notation/pitch-up-diatonic");
inline static const muse::rcommand::Command PITCH_DOWN_DIATONIC_COMMAND("command://notation/pitch-down-diatonic");
inline static const muse::rcommand::Command PITCH_UP_DIATONIC_ALTERATIONS_COMMAND("command://notation/pitch-up-diatonic-alterations");
inline static const muse::rcommand::Command PITCH_DOWN_DIATONIC_ALTERATIONS_COMMAND("command://notation/pitch-down-diatonic-alterations");

// properties commands
inline static const muse::rcommand::Command TOGGLE_VISIBLE_COMMAND("command://notation/toggle-visible");

// snap commands

struct ToggleSnapToCommand {
    cprop_decl id = Command("command://notation/toggle-snap-to");
    cprop_decl title = MnemonicString("action", "Toggle snap to item");
    cprop_decl description = TranslatableString("action", "Toggle snapping of the selected hairpin or tempo change to the adjacent item");
    cprop_decl availabilities = Availability::API;

    SnapDirection direction = SnapDirection::Next;

    cprop_decl input = FieldsScheme {
        Field{ "direction", &ToggleSnapToCommand::direction, "Direction of the adjacent item" },
    };
};

// aliases for toggle snap to command

struct ToggleSnapToPrevCommand {
    cprop_decl id = Command("command://notation/toggle-snap-to/prev");
    cprop_decl title = MnemonicString("action", "Snap to &previous");
    cprop_decl description = TranslatableString("action", "Snap to previous");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleSnapToCommand(SnapDirection::Prev);
};

struct ToggleSnapToNextCommand {
    cprop_decl id = Command("command://notation/toggle-snap-to/next");
    cprop_decl title = MnemonicString("action", "Snap to &next");
    cprop_decl description = TranslatableString("action", "Snap to next");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleSnapToCommand(SnapDirection::Next);
};

// layout commands

// layout commands: measure
inline static const muse::rcommand::Command MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND("command://notation/move-measure-to-prev-system");
inline static const muse::rcommand::Command MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND("command://notation/move-measure-to-next-system");
inline static const muse::rcommand::Command SPLIT_MEASURE_COMMAND("command://notation/split-measure");
inline static const muse::rcommand::Command JOIN_MEASURES_COMMAND("command://notation/join-measures");

// layout commands: section
inline static const muse::rcommand::Command TOGGLE_SECTION_BREAK_COMMAND("command://notation/toggle-section-break");

// layout commands: system
inline static const muse::rcommand::Command TOGGLE_SYSTEM_BREAK_COMMAND("command://notation/toggle-system-break");
inline static const muse::rcommand::Command APPLY_SYSTEM_LOCK_COMMAND("command://notation/apply-system-lock");
inline static const muse::rcommand::Command TOGGLE_SYSTEM_LOCK_COMMAND("command://notation/toggle-system-lock");
inline static const muse::rcommand::Command MAKE_INTO_SYSTEM_COMMAND("command://notation/make-into-system");
inline static const muse::rcommand::Command MOVE_SYSTEM_TO_PREV_PAGE_COMMAND("command://notation/move-system-to-prev-page");
inline static const muse::rcommand::Command MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND("command://notation/move-system-to-next-page");

// layout commands: page
inline static const muse::rcommand::Command TOGGLE_PAGE_BREAK_COMMAND("command://notation/toggle-page-break");
inline static const muse::rcommand::Command APPLY_PAGE_LOCK_COMMAND("command://notation/apply-page-lock");
inline static const muse::rcommand::Command TOGGLE_PAGE_LOCK_COMMAND("command://notation/toggle-page-lock");
inline static const muse::rcommand::Command MAKE_INTO_PAGE_COMMAND("command://notation/make-into-page");

// layout commands: score
inline static const muse::rcommand::Command TOGGLE_SCORE_LOCK_COMMAND("command://notation/toggle-score-lock");

// layout commands: stretch
inline static const muse::rcommand::Command STRETCH_DECREASE_COMMAND("command://notation/stretch-decrease");
inline static const muse::rcommand::Command STRETCH_INCREASE_COMMAND("command://notation/stretch-increase");
inline static const muse::rcommand::Command STRETCH_RESET_COMMAND("command://notation/stretch-reset");

// open properties

struct OpenPageSettingsCommand {
    cprop_decl id = Command("command://notation/open-page-settings");
    cprop_decl title = MnemonicString("action", "&Page settings…");
    cprop_decl description = TranslatableString("action", "Page settings");
    cprop_decl availabilities = Availability::All;
};

struct OpenEditStringsCommand {
    cprop_decl id = Command("command://notation/open-edit-strings");
    cprop_decl title = MnemonicString("action", "Edit strings…");
    cprop_decl description = TranslatableString("action", "Edit strings");
    cprop_decl availabilities = Availability::All;
};

struct OpenBreaksCommand {
    cprop_decl id = Command("command://notation/open-breaks");
    cprop_decl title = MnemonicString("action", "Measures per s&ystem…");
    cprop_decl description = TranslatableString("action", "Measures per system");
    cprop_decl availabilities = Availability::All;
};

struct OpenStaffPropertiesCommand {
    cprop_decl id = Command("command://notation/open-staff-properties");
    cprop_decl title = MnemonicString("action", "Instrument / Staff properties…");
    cprop_decl description = TranslatableString("action", "Instrument / Staff properties");
    cprop_decl availabilities = Availability::All;
};

struct OpenStaffTextPropertiesCommand {
    cprop_decl id = Command("command://notation/open-staff-text-properties");
    cprop_decl title = MnemonicString("action", "Staff text properties…");
    cprop_decl description = TranslatableString("action", "Staff text properties");
    cprop_decl availabilities = Availability::All;
};

struct OpenSystemTextPropertiesCommand {
    cprop_decl id = Command("command://notation/open-system-text-properties");
    cprop_decl title = MnemonicString("action", "System text properties…");
    cprop_decl description = TranslatableString("action", "System text properties");
    cprop_decl availabilities = Availability::All;
};

struct OpenMeasurePropertiesCommand {
    cprop_decl id = Command("command://notation/open-measure-properties");
    cprop_decl title = MnemonicString("action", "Measure properties…");
    cprop_decl description = TranslatableString("action", "Measure properties");
    cprop_decl availabilities = Availability::All;
};

struct OpenTransposeCommand {
    cprop_decl id = Command("command://notation/open-transpose");
    cprop_decl title = MnemonicString("action", "&Transpose…");
    cprop_decl description = TranslatableString("action", "Transpose");
    cprop_decl availabilities = Availability::All;
};

struct OpenPartsCommand {
    cprop_decl id = Command("command://notation/open-parts");
    cprop_decl title = MnemonicString("action", "&Parts…");
    cprop_decl description = TranslatableString("action", "Parts");
    cprop_decl decoration = Decoration(muse::ui::IconCode::Code::PAGE);
    cprop_decl availabilities = Availability::All;
};

struct OpenEditGridSizeCommand {
    cprop_decl id = Command("command://notation/open-editgridsize");
    cprop_decl title = MnemonicString("action", "&Grid size…");
    cprop_decl description = TranslatableString("action", "Grid size");
    cprop_decl availabilities = Availability::All;
};

struct OpenRealizeChordSymbolsCommand {
    cprop_decl id = Command("command://notation/open-realizechordsymbols");
    cprop_decl title = MnemonicString("action", "Realize &chord symbols");
    cprop_decl description = TranslatableString("action", "Realize chord symbols");
    cprop_decl availabilities = Availability::All;
};

// style commands

struct LoadStyleCommand {
    cprop_decl id = Command("command://notation/load-style");
    cprop_decl title = MnemonicString("action", "&Load style…");
    cprop_decl description = TranslatableString("action", "Load style");
    cprop_decl availabilities = Availability::All;
};

struct SaveStyleCommand {
    cprop_decl id = Command("command://notation/save-style");
    cprop_decl title = MnemonicString("action", "S&ave style…");
    cprop_decl description = TranslatableString("action", "Save style");
    cprop_decl availabilities = Availability::All;
};

inline static const muse::rcommand::Command OPEN_EDIT_STYLE_COMMAND("command://notation/open-edit-style");
inline static const muse::rcommand::Command TOGGLE_CONCERT_PITCH_COMMAND("command://notation/toggle-concert-pitch");

// reset commands
inline static const muse::rcommand::Command RESET_TEXT_STYLE_OVERRIDES_COMMAND("command://notation/reset-text-style-overrides");
inline static const muse::rcommand::Command RESET_BEAMS_COMMAND("command://notation/reset-beams");
inline static const muse::rcommand::Command RESET_SHAPES_AND_POSITIONS_COMMAND("command://notation/reset-shapes-and-positions");
inline static const muse::rcommand::Command RESET_TO_DEFAULT_LAYOUT_COMMAND("command://notation/reset-to-default-layout");

// appearance commands
inline static const muse::rcommand::Command FREEZE_CURRENT_PLACEMENT_COMMAND("command://notation/freeze-current-placement");

// show commands

struct ToggleScoreConfigCommand {
    cprop_decl id = Command("command://notation/show");
    cprop_decl title = MnemonicString("action", "Show");
    cprop_decl description = TranslatableString("action", "Toggle score display option");
    cprop_decl availabilities = Availability::API;

    ScoreConfigType type = ScoreConfigType::ShowInvisibleElements;

    cprop_decl input = FieldsScheme {
        Field{ "type", &ToggleScoreConfigCommand::type, "Score display option" },
    };
};

struct ShowInvisibleCommand {
    cprop_decl id = Command("command://notation/show/invisible");
    cprop_decl title = MnemonicString("action", "Show &invisible");
    cprop_decl description = TranslatableString("action", "Show/hide invisible elements");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::ShowInvisibleElements);
};

struct ShowUnprintableCommand {
    cprop_decl id = Command("command://notation/show/unprintable");
    cprop_decl title = MnemonicString("action", "Show f&ormatting");
    cprop_decl description = TranslatableString("action", "Show/hide formatting");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::ShowUnprintableElements);
};

struct ShowFramesCommand {
    cprop_decl id = Command("command://notation/show/frames");
    cprop_decl title = MnemonicString("action", "Show &frames");
    cprop_decl description = TranslatableString("action", "Show/hide frames");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::ShowFrames);
};

struct ShowPageBordersCommand {
    cprop_decl id = Command("command://notation/show/pageborders");
    cprop_decl title = MnemonicString("action", "Show page &margins");
    cprop_decl description = TranslatableString("action", "Show/hide page margins");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::ShowPageMargins);
};

struct ShowSoundFlagsCommand {
    cprop_decl id = Command("command://notation/show/soundflags");
    cprop_decl title = MnemonicString("action", "Show sound flags"); // todo &
    cprop_decl description = TranslatableString("action", "Show/hide sound flags");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::ShowSoundFlags);
};

struct ShowIrregularCommand {
    cprop_decl id = Command("command://notation/show/irregular");
    cprop_decl title = MnemonicString("action", "Mark i&rregular measures");
    cprop_decl description = TranslatableString("action", "Mark irregular measures");
    cprop_decl decoration = Decoration(Checkable::Yes);
    cprop_decl availabilities = Availability::UI;
    cprop_decl redirect = ToggleScoreConfigCommand(ScoreConfigType::MarkIrregularMeasures);
};

// staff commands
inline static const muse::rcommand::Command STAFF_EXPLODE_COMMAND("command://notation/staff-explode");
inline static const muse::rcommand::Command STAFF_IMPLODE_COMMAND("command://notation/staff-implode");

// remove commands
inline static const muse::rcommand::Command REMOVE_SELECTED_RANGE_COMMAND("command://notation/remove-selected-range");
inline static const muse::rcommand::Command REMOVE_EMPTY_TRAILING_MEASURES_COMMAND("command://notation/remove-empty-trailing-measures");

// slash commands
inline static const muse::rcommand::Command SLASH_FILL_COMMAND("command://notation/slash-fill");
inline static const muse::rcommand::Command SLASH_RHYTHM_COMMAND("command://notation/slash-rhythm");

// spelling commands
inline static const muse::rcommand::Command PITCH_SPELL_COMMAND("command://notation/pitch-spell");
inline static const muse::rcommand::Command PITCH_SPELL_SHARPS_COMMAND("command://notation/pitch-spell-sharps");
inline static const muse::rcommand::Command PITCH_SPELL_FLATS_COMMAND("command://notation/pitch-spell-flats");
inline static const muse::rcommand::Command ENHARMONIC_SPELL_BOTH_COMMAND("command://notation/enharmonic-spell-both");
inline static const muse::rcommand::Command ENHARMONIC_SPELL_CURRENT_COMMAND("command://notation/enharmonic-spell-current");

// screen commands
inline static const muse::rcommand::Command SCREEN_PUT_NOTE_COMMAND("command://notation/screen-put-note"); // with params
inline static const muse::rcommand::Command SCREEN_REMOVE_NOTE_COMMAND("command://notation/screen-remove-note"); // with params
inline static const muse::rcommand::Command SCREEN_EDIT_TEXT_COMMAND("command://notation/screen-edit-text"); // with params
inline static const muse::rcommand::Command SCREEN_EDIT_ELEMENT_COMMAND("command://notation/screen-edit-element"); // with params

// others commands
inline static const muse::rcommand::Command REGROUP_RHYTHMS_COMMAND("command://notation/regroup-rhythms");
inline static const muse::rcommand::Command RESEQUENCE_REHEARSAL_MARKS_COMMAND("command://notation/resequence-rehearsal-marks");
inline static const muse::rcommand::Command UNROLL_REPEATS_COMMAND("command://notation/unroll-repeats");
inline static const muse::rcommand::Command REPEAT_SELECTION_COMMAND("command://notation/repeat-selection");
inline static const muse::rcommand::Command TRANSPOSE_UP_COMMAND("command://notation/transpose-up");
inline static const muse::rcommand::Command TRANSPOSE_DOWN_COMMAND("command://notation/transpose-down");
inline static const muse::rcommand::Command TOGGLE_MMREST_COMMAND("command://notation/toggle-mmrest");
inline static const muse::rcommand::Command TOGGLE_HIDE_EMPTY_COMMAND("command://notation/toggle-hide-empty");
inline static const muse::rcommand::Command MIRROR_NOTEHEAD_COMMAND("command://notation/mirror-notehead");
inline static const muse::rcommand::Command SET_VISIBLE_COMMAND("command://notation/set-visible");
inline static const muse::rcommand::Command UNSET_VISIBLE_COMMAND("command://notation/unset-visible");
inline static const muse::rcommand::Command TOGGLE_AUTOPLACE_COMMAND("command://notation/toggle-autoplace");
inline static const muse::rcommand::Command AUTOPLACE_ENABLED_COMMAND("command://notation/autoplace-enabled");
inline static const muse::rcommand::Command TOGGLE_AUTOMATION_COMMAND("command://notation/toggle-automation");
inline static const muse::rcommand::Command SELECT_AUTOMATION_TYPE_COMMAND("command://notation/select-automation-type"); // with params

// TAB commands
inline static const muse::rcommand::Command SET_DURATION_WHOLE_TAB_COMMAND("command://notation/set-duration-whole-tab");
inline static const muse::rcommand::Command SET_DURATION_HALF_TAB_COMMAND("command://notation/set-duration-half-tab");
inline static const muse::rcommand::Command SET_DURATION_QUARTER_TAB_COMMAND("command://notation/set-duration-quarter-tab");
inline static const muse::rcommand::Command SET_DURATION_EIGHTH_TAB_COMMAND("command://notation/set-duration-eighth-tab");
inline static const muse::rcommand::Command SET_DURATION_16TH_TAB_COMMAND("command://notation/set-duration-16th-tab");
inline static const muse::rcommand::Command SET_DURATION_32ND_TAB_COMMAND("command://notation/set-duration-32nd-tab");
inline static const muse::rcommand::Command SET_DURATION_64TH_TAB_COMMAND("command://notation/set-duration-64th-tab");
inline static const muse::rcommand::Command SET_DURATION_128TH_TAB_COMMAND("command://notation/set-duration-128th-tab");
inline static const muse::rcommand::Command SET_DURATION_256TH_TAB_COMMAND("command://notation/set-duration-256th-tab");
inline static const muse::rcommand::Command SET_DURATION_512TH_TAB_COMMAND("command://notation/set-duration-512th-tab");
inline static const muse::rcommand::Command SET_DURATION_1024TH_TAB_COMMAND("command://notation/set-duration-1024th-tab");

inline static const muse::rcommand::Command ENTER_REST_TAB_COMMAND("command://notation/enter-rest-tab");

inline static const muse::rcommand::Command ADD_STANDARD_BEND_COMMAND("command://notation/add-standard-bend");
inline static const muse::rcommand::Command ADD_PRE_BEND_COMMAND("command://notation/add-pre-bend");
inline static const muse::rcommand::Command ADD_GRACE_NOTE_BEND_COMMAND("command://notation/add-grace-note-bend");
inline static const muse::rcommand::Command ADD_SLIGHT_BEND_COMMAND("command://notation/add-slight-bend");
inline static const muse::rcommand::Command ADD_DIVE_COMMAND("command://notation/add-dive");
inline static const muse::rcommand::Command ADD_PRE_DIVE_COMMAND("command://notation/add-pre-dive");
inline static const muse::rcommand::Command ADD_DIP_COMMAND("command://notation/add-dip");
inline static const muse::rcommand::Command ADD_SCOOP_COMMAND("command://notation/add-scoop");

inline static const muse::rcommand::Command ADD_HAMMER_ON_PULL_OFF_COMMAND("command://notation/add-hammer-on-pull-off");

inline static const muse::rcommand::Command GOTO_STRING_ABOVE_COMMAND("command://notation/goto-string-above");
inline static const muse::rcommand::Command GOTO_STRING_BELOW_COMMAND("command://notation/goto-string-below");

// view commands
inline static const muse::rcommand::Command ZOOM_IN_COMMAND("command://notation/view/zoom-in");
inline static const muse::rcommand::Command ZOOM_OUT_COMMAND("command://notation/view/zoom-out");
inline static const muse::rcommand::Command ZOOM_TO_PAGE_WIDTH_COMMAND("command://notation/view/zoom-to-page-width");
inline static const muse::rcommand::Command ZOOM_TO_WHOLE_PAGE_COMMAND("command://notation/view/zoom-to-whole-page");
inline static const muse::rcommand::Command ZOOM_TO_TWO_PAGES_COMMAND("command://notation/view/zoom-to-two-pages");
inline static const muse::rcommand::Command ZOOM_TO_100_COMMAND("command://notation/view/zoom-to-100");
inline static const muse::rcommand::Command ZOOM_TO_PERCENT_COMMAND("command://notation/view/zoom-to-percent"); // with params

inline static const muse::rcommand::Command VIEW_MODE_PAGE_COMMAND("command://notation/view/mode-page");
inline static const muse::rcommand::Command VIEW_MODE_FLOAT_COMMAND("command://notation/view/mode-float");
inline static const muse::rcommand::Command VIEW_MODE_CONTINUOUS_COMMAND("command://notation/view/mode-continuous");
inline static const muse::rcommand::Command VIEW_MODE_SINGLE_COMMAND("command://notation/view/mode-single");

inline static const muse::rcommand::Command NEXT_SCREEN_COMMAND("command://notation/view/next-screen");
inline static const muse::rcommand::Command PREV_SCREEN_COMMAND("command://notation/view/prev-screen");
inline static const muse::rcommand::Command NEXT_PAGE_COMMAND("command://notation/view/next-page");
inline static const muse::rcommand::Command PREV_PAGE_COMMAND("command://notation/view/prev-page");
inline static const muse::rcommand::Command TOP_OF_FIRST_PAGE_COMMAND("command://notation/view/top-of-first-page");
inline static const muse::rcommand::Command BOTTOM_OF_LAST_PAGE_COMMAND("command://notation/view/bottom-of-last-page");

inline static const muse::rcommand::Command CONTEXT_MENU_OF_SELECTION_COMMAND("command://notation/view/context-menu-of-selection");

inline static const muse::rcommand::Command SHOW_SEARCH_COMMAND("command://notation/view/show-search");

// piano keyboard commands
inline static const muse::rcommand::Command PIANO_KEYBOARD_SET_NUMBER_OF_KEYS_COMMAND("command://notation/piano-keyboard/set-number-of-keys");

// diagnostic commands
inline static const muse::rcommand::Command DIAGNOSTIC_VIEW_REDRAW_COMMAND("command://notation/diagnostic/view-redraw");

inline static const muse::rcommand::Command SHOW_ELEMENT_BOUNDING_RECTS_COMMAND("command://notation/diagnostic/show-element-bounding-rects");
inline static const muse::rcommand::Command COLOR_ELEMENT_SHAPES_COMMAND("command://notation/diagnostic/color-element-shapes");
inline static const muse::rcommand::Command SHOW_SEGMENT_SHAPES_COMMAND("command://notation/diagnostic/show-segment-shapes");
inline static const muse::rcommand::Command COLOR_SEGMENT_SHAPES_COMMAND("command://notation/diagnostic/color-segment-shapes");
inline static const muse::rcommand::Command SHOW_SKYLINES_COMMAND("command://notation/diagnostic/show-skylines");
inline static const muse::rcommand::Command SHOW_SYSTEM_BOUNDING_RECTS_COMMAND("command://notation/diagnostic/show-system-bounding-rects");
inline static const muse::rcommand::Command SHOW_ELEMENT_MASKS_COMMAND("command://notation/diagnostic/show-element-masks");
inline static const muse::rcommand::Command SHOW_LINE_ATTACH_POINTS_COMMAND("command://notation/diagnostic/show-line-attach-points");
inline static const muse::rcommand::Command MARK_EMPTY_STAFF_COMMAND("command://notation/diagnostic/mark-empty-staff");
inline static const muse::rcommand::Command MARK_CORRUPTED_MEASURES_COMMAND("command://notation/diagnostic/mark-corrupted-measures");
inline static const muse::rcommand::Command SHOW_GAP_RESTS_COMMAND("command://notation/diagnostic/show-gap-rests");
inline static const muse::rcommand::Command SHOW_ORIGIN_AND_COMBINED_COMMAND("command://notation/diagnostic/show-origin-and-combined");
inline static const muse::rcommand::Command CHECK_FOR_SCORE_CORRUPTIONS_COMMAND("command://notation/diagnostic/check-for-score-corruptions");

//! NOTE "Read" commands for MCP and maybe API
//inline static const muse::rcommand::Command GET_NOTEINPUT_STATE_COMMAND("command://notation/get-noteinput-state");
}
