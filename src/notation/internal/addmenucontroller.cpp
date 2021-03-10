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
#include "addmenucontroller.h"

using namespace mu::notation;
using namespace mu::actions;

void AddMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool AddMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool AddMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<ActionCodeList> AddMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<ActionCode, AddMenuController::AvailableCallback> AddMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "note-input", std::bind(&AddMenuController::isNoteInputAvailable, this) },
        { "note-c", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::C, NoteAddingMode::NextChord) },
        { "note-d", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::D, NoteAddingMode::NextChord) },
        { "note-e", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::E, NoteAddingMode::NextChord) },
        { "note-f", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::F, NoteAddingMode::NextChord) },
        { "note-g", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::G, NoteAddingMode::NextChord) },
        { "note-a", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::A, NoteAddingMode::NextChord) },
        { "note-b", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::B, NoteAddingMode::NextChord) },
        { "chord-c", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::C, NoteAddingMode::CurrentChord) },
        { "chord-d", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::D, NoteAddingMode::CurrentChord) },
        { "chord-e", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::E, NoteAddingMode::CurrentChord) },
        { "chord-f", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::F, NoteAddingMode::CurrentChord) },
        { "chord-g", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::G, NoteAddingMode::CurrentChord) },
        { "chord-a", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::A, NoteAddingMode::CurrentChord) },
        { "chord-b", std::bind(&AddMenuController::isNoteAvailable, this, NoteName::G, NoteAddingMode::CurrentChord) },
        { "interval1", std::bind(&AddMenuController::isIntervalAvailable, this, 1, IntervalType::Above) },
        { "interval2", std::bind(&AddMenuController::isIntervalAvailable, this, 2, IntervalType::Above) },
        { "interval3", std::bind(&AddMenuController::isIntervalAvailable, this, 3, IntervalType::Above) },
        { "interval4", std::bind(&AddMenuController::isIntervalAvailable, this, 4, IntervalType::Above) },
        { "interval5", std::bind(&AddMenuController::isIntervalAvailable, this, 5, IntervalType::Above) },
        { "interval6", std::bind(&AddMenuController::isIntervalAvailable, this, 6, IntervalType::Above) },
        { "interval7", std::bind(&AddMenuController::isIntervalAvailable, this, 7, IntervalType::Above) },
        { "interval8", std::bind(&AddMenuController::isIntervalAvailable, this, 8, IntervalType::Above) },
        { "interval9", std::bind(&AddMenuController::isIntervalAvailable, this, 9, IntervalType::Above) },
        { "interval-2", std::bind(&AddMenuController::isIntervalAvailable, this, 2, IntervalType::Below) },
        { "interval-3", std::bind(&AddMenuController::isIntervalAvailable, this, 3, IntervalType::Below) },
        { "interval-4", std::bind(&AddMenuController::isIntervalAvailable, this, 4, IntervalType::Below) },
        { "interval-5", std::bind(&AddMenuController::isIntervalAvailable, this, 5, IntervalType::Below) },
        { "interval-6", std::bind(&AddMenuController::isIntervalAvailable, this, 6, IntervalType::Below) },
        { "interval-7", std::bind(&AddMenuController::isIntervalAvailable, this, 7, IntervalType::Below) },
        { "interval-8", std::bind(&AddMenuController::isIntervalAvailable, this, 8, IntervalType::Below) },
        { "interval-9", std::bind(&AddMenuController::isIntervalAvailable, this, 9, IntervalType::Below) },
        { "duplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Duplet) },
        { "triplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Triplet) },
        { "quadruplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Quadruplet) },
        { "quintuplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Quintuplet) },
        { "sextuplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Sextuplet) },
        { "septuplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Septuplet) },
        { "octuplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Octuplet) },
        { "nonuplet", std::bind(&AddMenuController::isTupletAvailable, this, TupletType::Nonuplet) },
        { "tuplet-dialog", std::bind(&AddMenuController::isTupletDialogAvailable, this) },
        { "insert-measure", std::bind(&AddMenuController::isMeasuresAvailable, this, ElementChangeOperation::Insert, 0) },
        { "insert-measures", std::bind(&AddMenuController::isMeasuresAvailable, this, ElementChangeOperation::Insert, 2) },
        { "append-measure", std::bind(&AddMenuController::isMeasuresAvailable, this, ElementChangeOperation::Append, 0) },
        { "append-measures", std::bind(&AddMenuController::isMeasuresAvailable, this, ElementChangeOperation::Append, 2) },
        { "insert-hbox", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Insert, BoxType::Horizontal) },
        { "insert-vbox", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Insert, BoxType::Vertical) },
        { "insert-textframe", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Insert, BoxType::Text) },
        { "append-hbox", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Append, BoxType::Horizontal) },
        { "append-vbox", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Append, BoxType::Vertical) },
        { "append-textframe", std::bind(&AddMenuController::isBoxAvailable, this, ElementChangeOperation::Append, BoxType::Text) },
        { "title-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::TITLE) },
        { "subtitle-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::SUBTITLE) },
        { "composer-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::COMPOSER) },
        { "poet-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::POET) },
        { "part-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::INSTRUMENT_EXCERPT) },
        { "system-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::SYSTEM) },
        { "staff-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::STAFF) },
        { "expression-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::EXPRESSION) },
        { "rehearsalmark-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::REHEARSAL_MARK) },
        { "instrument-change-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::INSTRUMENT_CHANGE) },
        { "fingering-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::FINGERING) },
        { "sticking-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::STICKING) },
        { "chord-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::HARMONY_A) },
        { "roman-numeral-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::HARMONY_ROMAN) },
        { "nashville-number-text", std::bind(&AddMenuController::isTextAvailable, this, TextType::HARMONY_NASHVILLE) },
        { "lyrics", std::bind(&AddMenuController::isTextAvailable, this, TextType::LYRICS_ODD) },
        { "figured-bass", std::bind(&AddMenuController::isFiguredBassAvailable, this) },
        { "tempo", std::bind(&AddMenuController::isTextAvailable, this, TextType::TEMPO) },
        { "add-slur", std::bind(&AddMenuController::isSlurAvailable, this) },
        { "add-hairpin", std::bind(&AddMenuController::isHarpinAvailable, this, HairpinType::CRESC_HAIRPIN) },
        { "add-hairpin-reverse", std::bind(&AddMenuController::isHarpinAvailable, this, HairpinType::DECRESC_HAIRPIN) },
        { "add-8va", std::bind(&AddMenuController::isOttavaAvailable, this, OttavaType::OTTAVA_8VA) },
        { "add-8vb", std::bind(&AddMenuController::isOttavaAvailable, this, OttavaType::OTTAVA_8VB) },
        { "add-noteline", std::bind(&AddMenuController::isNoteLineAvailable, this) }
    };

    return _actions;
}

bool AddMenuController::isNoteInputAvailable() const
{
    return controller()->actionAvailable("note-input");
}

bool AddMenuController::isNoteAvailable(NoteName noteName, NoteAddingMode addingMode) const
{
    std::string mode = noteAddingModeActionCode(addingMode);
    return controller()->actionAvailable(mode + noteNameToString(noteName));
}

bool AddMenuController::isIntervalAvailable(int interval, IntervalType intervalType) const
{
    std::string type = intervalTypeActionCode(intervalType);
    return controller()->actionAvailable(type + std::to_string(interval));
}

bool AddMenuController::isTupletAvailable(TupletType tupletType) const
{
    return controller()->actionAvailable(tupletToString(tupletType));
}

bool AddMenuController::isTupletDialogAvailable() const
{
    return controller()->actionAvailable("tuplet-dialog");
}

bool AddMenuController::isMeasuresAvailable(ElementChangeOperation operation, int count) const
{
    return controller()->actionAvailable(elementChangeOperationToString(operation) + "-" + measuresActionCode(count));
}

bool AddMenuController::isBoxAvailable(ElementChangeOperation operation, BoxType boxType) const
{
    return controller()->actionAvailable(elementChangeOperationToString(operation) + "-" + boxTypeToString(boxType));
}

bool AddMenuController::isTextAvailable(TextType textType) const
{
    return controller()->actionAvailable(textTypeToString(textType));
}

bool AddMenuController::isFiguredBassAvailable() const
{
    return controller()->actionAvailable("figured-bass");
}

bool AddMenuController::isSlurAvailable() const
{
    return controller()->actionAvailable("add-slur");
}

bool AddMenuController::isHarpinAvailable(HairpinType hairpinType) const
{
    return controller()->actionAvailable("add-" + hairpinTypeToString(hairpinType));
}

bool AddMenuController::isOttavaAvailable(OttavaType ottavaType) const
{
    return controller()->actionAvailable("add-" + ottavaTypeToString(ottavaType));
}

bool AddMenuController::isNoteLineAvailable() const
{
    return controller()->actionAvailable("add-noteline");
}

std::string AddMenuController::noteAddingModeActionCode(NoteAddingMode mode) const
{
    switch (mode) {
    case NoteAddingMode::NextChord:
        return "note-";
    case NoteAddingMode::CurrentChord:
        return "chord-";
    case NoteAddingMode::InsertChord:
        return "insert-";
    }

    return std::string();
}

std::string AddMenuController::intervalTypeActionCode(IntervalType type) const
{
    switch (type) {
    case IntervalType::Above:
        return "interval";
    case IntervalType::Below:
        return "interval-";
    }

    return std::string();
}

std::string AddMenuController::measuresActionCode(int count) const
{
    return count > 1 ? "measures" : "measure";
}
