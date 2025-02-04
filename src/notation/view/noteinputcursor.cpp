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
    INotationNoteInputPtr noteInput = currentNoteInput();
    if (!noteInput) {
        return;
    }

    const NoteInputState& state = noteInput->state();
    Color fillColor = configuration()->selectionColor(state.voice());
    RectF cursorRect = noteInput->cursorRect();

    if (!m_isThinLine) {
        Color cursorRectColor = fillColor;
        cursorRectColor.setAlpha(configuration()->cursorOpacity());
        painter->fillRect(cursorRect, cursorRectColor);
    }

    constexpr int leftLineWidth = 3;
    RectF leftLine(cursorRect.topLeft().x(), cursorRect.topLeft().y(), leftLineWidth, cursorRect.height());
    Color lineColor = fillColor;
    painter->fillRect(leftLine, lineColor);

    if (state.staffGroup() == StaffGroup::TAB) {
        const Staff* staff = state.staff();
        const StaffType* staffType = staff ? staff->staffType() : nullptr;

        if (staffType) {
            staffType->drawInputStringMarks(painter, state.string(),
                                            configuration()->selectionColor(state.voice()), cursorRect);
        }
    }
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
