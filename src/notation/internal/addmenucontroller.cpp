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

AddMenuController::AddMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > AddMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
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
