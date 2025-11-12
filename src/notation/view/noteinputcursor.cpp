/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "noteinputcursor.h"

using namespace mu::notation;
using namespace mu::engraving;

NoteInputCursor::NoteInputCursor(bool isThinLine)
    : m_isThinLine(isThinLine)
{
}

void NoteInputCursor::paint(muse::draw::Painter* painter)
{
    const INotationNoteInputPtr noteInput = currentNoteInput();
    if (!noteInput) {
        return;
    }

    const NoteInputState& state = noteInput->state();
    const Color fillColor = configuration()->selectionColor(state.voice());
    const RectF cursorRect = noteInput->cursorRect();

    if (!m_isThinLine) {
        Color cursorRectColor = fillColor;
        cursorRectColor.setAlpha(configuration()->cursorOpacity());
        painter->fillRect(cursorRect, cursorRectColor);
    }

    if (!state.staff()) {
        return;
    }

    const StaffType* staffType = state.staff()->staffType(state.tick());
    const double spatium = staffType->spatium();
    const int leftLineWidth = 0.15 * spatium;

    const RectF leftLine(cursorRect.topLeft().x(), cursorRect.topLeft().y(), leftLineWidth, cursorRect.height());
    const Color lineColor = fillColor;
    painter->fillRect(leftLine, lineColor);

    if (state.staffGroup() == StaffGroup::TAB) {
        staffType->drawInputStringMarks(painter, state.string(),
                                        configuration()->selectionColor(state.voice()), cursorRect);
    }
}

INotationNoteInputPtr NoteInputCursor::currentNoteInput() const
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->interaction()->noteInput();
}
