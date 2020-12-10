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

#include <QPainter>

using namespace mu::notation;

void NoteInputCursor::paint(QPainter* painter)
{
    if (!isNoteInputMode()) {
        return;
    }

    QRectF cursorRect = rect();
    QColor fillColor = color();
    fillColor.setAlpha(50);
    painter->fillRect(cursorRect, fillColor);

    QRectF leftLine = QRectF(cursorRect.topLeft().x(), cursorRect.topLeft().y(), 3, cursorRect.height());
    QColor lineColor = color();
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

QColor NoteInputCursor::color() const
{
    auto noteInput = currentNoteInput();
    if (!noteInput) {
        return QColor();
    }

    return noteInput->cursorColor();
}
