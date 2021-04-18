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
#include "noteinputcursor.h"

using namespace mu::notation;

void NoteInputCursor::paint(mu::draw::Painter* painter)
{
    if (!isNoteInputMode()) {
        return;
    }

    QRectF cursorRect = rect();
    QColor cursorRectColor = cursorColor();
    painter->fillRect(cursorRect, cursorRectColor);

    constexpr int leftLineWidth = 3;
    QRectF leftLine = QRectF(cursorRect.topLeft().x(), cursorRect.topLeft().y(), leftLineWidth, cursorRect.height());
    QColor lineColor = fillColor();
    painter->fillRect(leftLine, lineColor);
}

INotationNoteInputPtr NoteInputCursor::currentNoteInput() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    auto interaction = notation->interaction();
    if (!interaction) {
        return nullptr;
    }

    return interaction->noteInput();
}

bool NoteInputCursor::isNoteInputMode() const
{
    auto noteInput = currentNoteInput();
    if (!noteInput) {
        return false;
    }

    return noteInput->isNoteInputMode();
}

QRectF NoteInputCursor::rect() const
{
    auto noteInput = currentNoteInput();
    if (!noteInput) {
        return QRectF();
    }

    return noteInput->cursorRect();
}

QColor NoteInputCursor::cursorColor() const
{
    QColor color = fillColor();
    color.setAlpha(configuration()->cursorOpacity());
    return color;
}

QColor NoteInputCursor::fillColor() const
{
    auto noteInput = currentNoteInput();
    if (!noteInput) {
        return QColor();
    }

    int voiceIndex = noteInput->state().currentVoiceIndex;
    return configuration()->selectionColor(voiceIndex);
}
