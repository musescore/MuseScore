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
#include "log.h"
#include <QPoint>

using namespace mu::domain::notation;
using namespace mu::actions;

NotationActionController::NotationActionController()
{
    dispatcher()->reg(this, "note-input", this, &NotationActionController::toggleNoteInput);
    dispatcher()->reg(this, "pad-note-4", [this]() { padNote(Pad::NOTE4); });
    dispatcher()->reg(this, "pad-note-8", [this]() { padNote(Pad::NOTE8); });
    dispatcher()->reg(this, "pad-note-16", [this]() { padNote(Pad::NOTE16); });
    dispatcher()->reg(this, "put-note", this, &NotationActionController::putNote);
}

bool NotationActionController::canReceiveAction(const actions::ActionName&) const
{
    return true;
}

std::shared_ptr<INotation> NotationActionController::currentNotation() const
{
    return globalContext()->currentNotation();
}

INotationInteraction* NotationActionController::currentNotationInteraction() const
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
