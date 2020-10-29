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
#include "notationactioncontroller.h"

#include <QPoint>

#include "log.h"

using namespace mu::notation;
using namespace mu::actions;

void NotationActionController::init()
{
    dispatcher()->reg(this, "note-input", this, &NotationActionController::toggleNoteInput);
    dispatcher()->reg(this, "pad-note-4", [this]() { padNote(Pad::NOTE4); });
    dispatcher()->reg(this, "pad-note-8", [this]() { padNote(Pad::NOTE8); });
    dispatcher()->reg(this, "pad-note-16", [this]() { padNote(Pad::NOTE16); });
    dispatcher()->reg(this, "put-note", this, &NotationActionController::putNote);

    //! NOTE For historical reasons, the name of the action does not match what needs to be done.
    dispatcher()->reg(this, "next-element", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "prev-element", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "next-chord", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "prev-chord", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "next-measure", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "prev-measure", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "next-track", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "prev-track", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "pitch-up", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "pitch-down", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "pitch-up-octave", [this](const ActionName& action) { moveAction(action); });
    dispatcher()->reg(this, "pitch-down-octave", [this](const ActionName& action) { moveAction(action); });

    dispatcher()->reg(this, "delete", this, &NotationActionController::deleteSelection);

    dispatcher()->reg(this, "edit-style", this, &NotationActionController::openPageStyle);
}

bool NotationActionController::canReceiveAction(const actions::ActionName&) const
{
    return true;
}

std::shared_ptr<INotation> NotationActionController::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteractionPtr NotationActionController::currentNotationInteraction() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->interaction();
}

void NotationActionController::toggleNoteInput()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    if (interaction->inputState()->isNoteEnterMode()) {
        interaction->endNoteEntry();
    } else {
        interaction->startNoteEntry();
    }
}

void NotationActionController::padNote(const Pad& pad)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->padNote(pad);
}

void NotationActionController::putNote(const actions::ActionData& data)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    IF_ASSERT_FAILED(data.count() > 2) {
        return;
    }

    QPoint pos = data.arg<QPoint>(0);
    bool replace = data.arg<bool>(1);
    bool insert = data.arg<bool>(2);

    interaction->putNote(pos, replace, insert);
}

void NotationActionController::moveAction(const actions::ActionName& action)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Element* el = interaction->selection()->element();
    if (!el) {
        LOGW() << "no selection element";
        return;
    }

    if (el->isLyrics()) {
        NOT_IMPLEMENTED;
    } else if (el->isTextBase()) {
        moveText(interaction, action);
    } else {
        if ("pitch-up" == action) {
            if (el->isRest()) {
                NOT_IMPLEMENTED << action << ", el->isRest";
            } else {
                interaction->movePitch(MoveDirection::Up, PitchMode::CHROMATIC);
            }
        } else if ("pitch-down" == action) {
            if (el->isRest()) {
                NOT_IMPLEMENTED << action << ", el->isRest";
            } else {
                interaction->movePitch(MoveDirection::Down, PitchMode::CHROMATIC);
            }
        } else if ("pitch-up-octave" == action) {
            interaction->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
        } else if ("pitch-down-octave" == action) {
            interaction->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
        } else if ("next-element" == action) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Element);
        } else if ("prev-element" == action) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Element);
        } else if ("next-chord" == action) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
        } else if ("prev-chord" == action) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
        } else if ("next-measure" == action) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
        } else if ("prev-measure" == action) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
        } else if ("next-track" == action) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Track);
        } else if ("prev-track" == action) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Track);
        } else {
            NOT_SUPPORTED << action;
        }
    }
}

void NotationActionController::moveText(INotationInteractionPtr interaction, const actions::ActionName& action)
{
    MoveDirection direction = MoveDirection::Undefined;
    bool quickly = false;

    if ("next-chord" == action) {
        direction = MoveDirection::Right;
    } else if ("next-measure" == action) {
        direction = MoveDirection::Right;
        quickly = true;
    } else if ("prev-chord" == action) {
        direction = MoveDirection::Left;
    } else if ("prev-measure" == action) {
        direction = MoveDirection::Left;
        quickly = true;
    } else if ("pitch-up" == action) {
        direction = MoveDirection::Up;
    } else if ("pitch-down" == action) {
        direction = MoveDirection::Down;
    } else if ("pitch-up-octave" == action) {
        direction = MoveDirection::Up;
        quickly = true;
    } else if ("pitch-down-octave" == action) {
        direction = MoveDirection::Down;
        quickly = true;
    } else {
        NOT_SUPPORTED << action;
        return;
    }

    interaction->moveText(direction, quickly);
}

void NotationActionController::deleteSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->deleteSelection();
}

void NotationActionController::openPageStyle()
{
    interactive()->open("musescore://notation/style");
}
