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
using namespace mu::uicomponents;

static ActionCode NOTE_INPUT_ACTION_CODE("note-input");

void AddMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        if (!notationNoteInput()) {
            return;
        }

        notationNoteInput()->stateChanged().onNotify(this, [this]() {
            m_actionsReceiveAvailableChanged.send({ NOTE_INPUT_ACTION_CODE });
        });
    });
}

bool AddMenuController::contains(const ActionCode& actionCode) const
{
    ActionCodeList actions = this->actions();
    return std::find(actions.begin(), actions.end(), actionCode) != actions.end();
}

ActionState AddMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);
    state.checkable = actionCheckable(actionCode);
    state.checked = actionChecked(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> AddMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

IMasterNotationPtr AddMenuController::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPtr AddMenuController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr AddMenuController::notationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationNoteInputPtr AddMenuController::notationNoteInput() const
{
    return notationInteraction() ? notationInteraction()->noteInput() : nullptr;
}

ActionCodeList AddMenuController::actions() const
{
    static ActionCodeList actions = {
        NOTE_INPUT_ACTION_CODE,
        "note-c",
        "note-d",
        "note-e",
        "note-f",
        "note-g",
        "note-a",
        "note-b",
        "chord-c",
        "chord-d",
        "chord-e",
        "chord-f",
        "chord-g",
        "chord-a",
        "chord-b",
        "interval1",
        "interval2",
        "interval3",
        "interval4",
        "interval5",
        "interval6",
        "interval7",
        "interval8",
        "interval9",
        "interval-2",
        "interval-3",
        "interval-4",
        "interval-5",
        "interval-6",
        "interval-7",
        "interval-8",
        "interval-9",
        "duplet",
        "triplet",
        "quadruplet",
        "quintuplet",
        "sextuplet",
        "septuplet",
        "octuplet",
        "nonuplet",
        "tuplet-dialog",
        "insert-measure",
        "insert-measures",
        "append-measure",
        "append-measures",
        "insert-hbox",
        "insert-vbox",
        "insert-textframe",
        "append-hbox",
        "append-vbox",
        "append-textframe",
        "title-text",
        "subtitle-text",
        "composer-text",
        "poet-text",
        "part-text",
        "system-text",
        "staff-text",
        "expression-text",
        "rehearsalmark-text",
        "instrument-change-text",
        "fingering-text",
        "sticking-text",
        "chord-text",
        "roman-numeral-text",
        "nashville-number-text",
        "lyrics",
        "figured-bass",
        "tempo",
        "add-slur",
        "add-hairpin",
        "add-hairpin-reverse",
        "add-8va",
        "add-8vb",
        "add-noteline"
    };

    return actions;
}

bool AddMenuController::actionEnabled(const ActionCode& actionCode) const
{
    return controller()->actionAvailable(actionCode);
}

bool AddMenuController::actionCheckable(const ActionCode& actionCode) const
{
    if (NOTE_INPUT_ACTION_CODE == actionCode) {
        return true;
    }

    return false;
}

bool AddMenuController::actionChecked(const ActionCode& actionCode) const
{
    if (NOTE_INPUT_ACTION_CODE == actionCode) {
        return notationNoteInput() ? notationNoteInput()->isNoteInputMode() : false;
    }

    return false;
}
