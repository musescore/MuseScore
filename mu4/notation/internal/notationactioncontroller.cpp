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

    dispatcher()->reg(this, "note-longa", [this]() { padNote(Pad::NOTE00); });
    dispatcher()->reg(this, "note-breve", [this]() { padNote(Pad::NOTE0); });
    dispatcher()->reg(this, "pad-note-1", [this]() { padNote(Pad::NOTE1); });
    dispatcher()->reg(this, "pad-note-2", [this]() { padNote(Pad::NOTE2); });
    dispatcher()->reg(this, "pad-note-4", [this]() { padNote(Pad::NOTE4); });
    dispatcher()->reg(this, "pad-note-8", [this]() { padNote(Pad::NOTE8); });
    dispatcher()->reg(this, "pad-note-16", [this]() { padNote(Pad::NOTE16); });
    dispatcher()->reg(this, "pad-note-32", [this]() { padNote(Pad::NOTE32); });
    dispatcher()->reg(this, "pad-note-64", [this]() { padNote(Pad::NOTE64); });
    dispatcher()->reg(this, "pad-note-128", [this]() { padNote(Pad::NOTE128); });
    dispatcher()->reg(this, "pad-note-256", [this]() { padNote(Pad::NOTE256); });
    dispatcher()->reg(this, "pad-note-512", [this]() { padNote(Pad::NOTE512); });
    dispatcher()->reg(this, "pad-note-1024", [this]() { padNote(Pad::NOTE1024); });
    dispatcher()->reg(this, "pad-dot", [this]() { padNote(Pad::DOT); });
    dispatcher()->reg(this, "pad-dotdot", [this]() { padNote(Pad::DOTDOT); });
    dispatcher()->reg(this, "pad-dot3", [this]() { padNote(Pad::DOT3); });
    dispatcher()->reg(this, "pad-dot4", [this]() { padNote(Pad::DOT4); });
    dispatcher()->reg(this, "pad-rest", [this]() { padNote(Pad::REST); });

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

    dispatcher()->reg(this, "cut", this, &NotationActionController::cutSelection);
    dispatcher()->reg(this, "copy", this, &NotationActionController::copySelection);
    dispatcher()->reg(this, "paste", [this]() { pasteSelection(PastingType::Default); });
    dispatcher()->reg(this, "paste-half", [this]() { pasteSelection(PastingType::Half); });
    dispatcher()->reg(this, "paste-double", [this]() { pasteSelection(PastingType::Double); });
    dispatcher()->reg(this, "paste-special", [this]() { pasteSelection(PastingType::Special); });
    dispatcher()->reg(this, "swap", this, &NotationActionController::swapSelection);
    dispatcher()->reg(this, "delete", this, &NotationActionController::deleteSelection);
    dispatcher()->reg(this, "undo", this, &NotationActionController::undo);
    dispatcher()->reg(this, "redo", this, &NotationActionController::redo);
    dispatcher()->reg(this, "select-all", this, &NotationActionController::selectAll);

    dispatcher()->reg(this, "edit-style", this, &NotationActionController::openPageStyle);
    dispatcher()->reg(this, "staff-properties", this, &NotationActionController::openStaffProperties);
    dispatcher()->reg(this, "add-remove-breaks", this, &NotationActionController::openBreaksDialog);
    dispatcher()->reg(this, "edit-info", this, &NotationActionController::openScoreProperties);
    dispatcher()->reg(this, "transpose", this, &NotationActionController::openTransposeDialog);
    dispatcher()->reg(this, "parts", this, &NotationActionController::openPartsDialog);

    dispatcher()->reg(this, "voice-x12", [this]() { swapVoices(0, 1); });
    dispatcher()->reg(this, "voice-x13", [this]() { swapVoices(0, 2); });
    dispatcher()->reg(this, "voice-x14", [this]() { swapVoices(0, 3); });
    dispatcher()->reg(this, "voice-x23", [this]() { swapVoices(1, 2); });
    dispatcher()->reg(this, "voice-x24", [this]() { swapVoices(1, 3); });
    dispatcher()->reg(this, "voice-x34", [this]() { swapVoices(2, 3); });
}

bool NotationActionController::canReceiveAction(const actions::ActionName&) const
{
    return true;
}

INotationPtr NotationActionController::currentNotation() const
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

void NotationActionController::swapVoices(int voiceIndex1, int voiceIndex2)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->swapVoices(voiceIndex1, voiceIndex2);
}

void NotationActionController::cutSelection()
{
    copySelection();
    deleteSelection();
}

void NotationActionController::copySelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->copySelection();
}

void NotationActionController::pasteSelection(PastingType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Fraction scale = resolvePastingScale(interaction, type);
    interaction->pasteSelection(scale);
}

Fraction NotationActionController::resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const
{
    const Fraction DEFAULT_SCALE(1, 1);

    switch (type) {
    case PastingType::Default: return DEFAULT_SCALE;
    case PastingType::Half: return Fraction(1, 2);
    case PastingType::Double: return Fraction(2, 1);
    case PastingType::Special:
        Fraction scale = DEFAULT_SCALE;
        Fraction duration = interaction->inputState()->duration().fraction();

        if (duration.isValid() && !duration.isZero()) {
            scale = duration * 4;
            scale.reduce();
        }

        return scale;
    }

    return DEFAULT_SCALE;
}

void NotationActionController::deleteSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->deleteSelection();
}

void NotationActionController::swapSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->swapSelection();
}

void NotationActionController::undo()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->undoStack()->undo();
}

void NotationActionController::redo()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->undoStack()->redo();
}

void NotationActionController::selectAll()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->selectAll();
}

void NotationActionController::openPageStyle()
{
    interactive()->open("musescore://notation/style");
}

void NotationActionController::openStaffProperties()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    int staffIdx = interaction->selection()->range()->startStaffIndex();
    interactive()->open("musescore://notation/staffproperties?staffIdx=" + QString::number(staffIdx).toStdString());
}

void NotationActionController::openBreaksDialog()
{
    interactive()->open("musescore://notation/breaks");
}

void NotationActionController::openScoreProperties()
{
    interactive()->open("musescore://notation/properties");
}

void NotationActionController::openTransposeDialog()
{
    interactive()->open("musescore://notation/transpose");
}

void NotationActionController::openPartsDialog()
{
    interactive()->open("musescore://notation/parts");
}
