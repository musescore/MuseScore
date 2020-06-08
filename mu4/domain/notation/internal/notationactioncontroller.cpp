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
    dispatcher()->reg("domain/notation/note-input", this, &NotationActionController::toggleNoteInput);
    dispatcher()->reg("domain/notation/pad-note-4", [this]() { padNote(Pad::NOTE4); });
    dispatcher()->reg("domain/notation/pad-note-8", [this]() { padNote(Pad::NOTE8); });
    dispatcher()->reg("domain/notation/pad-note-16", [this]() { padNote(Pad::NOTE16); });
    dispatcher()->reg("domain/notation/put-note", this, &NotationActionController::putNote);
}

std::shared_ptr<INotation> NotationActionController::currentNotation() const
{
    return globalContext()->currentNotation();
}

void NotationActionController::toggleNoteInput()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    if (notation->inputState()->isNoteEnterMode()) {
        notation->endNoteEntry();
    } else {
        notation->startNoteEntry();
    }
}

void NotationActionController::padNote(const Pad& pad)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->padNote(pad);
}

void NotationActionController::putNote(const actions::ActionData& data)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    IF_ASSERT_FAILED(data.count() > 2) {
        return;
    }

    QPoint pos = data.arg<QPoint>(0);
    bool replace = data.arg<bool>(1);
    bool insert = data.arg<bool>(2);

    notation->putNote(pos, replace, insert);
}
