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

static void drawInputStringMarks(const StaffType& staffType, muse::draw::Painter* p, int string, const Color& color, const RectF& rect);

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
        drawInputStringMarks(*staffType, painter, state.string(), fillColor, cursorRect);
    }
}

/// In TAB's, draws the marks within the input 'blue cursor' required to
/// identify the current target input string.
///
/// Implements the specific of historic TAB styles for instruments with more
/// strings than TAB lines. For strings normally represented by TAB lines, no
/// mark is required. For strings not represented by TAB lines (e.g. bass
/// strings in lutes and similar), either a sequence of slashes OR some ledger
/// line-like lines OR the ordinal of the string are used, according to the
/// TAB style (French or Italian) and the string position.
///
/// Note: assumes the string parameter is within legal bounds, i.e.:
/// 0 <= string <= [instrument strings] - 1
///
/// @param p       the Painter to draw into
/// @param string  the instrument physical string for which to draw the mark (0 = top string)
/// @param rect    the rect note input cursor
///
/// Moved from StaffType class.
static void drawInputStringMarks(const StaffType& staffType, muse::draw::Painter* p, int string, const Color& color, const RectF& rect)
{
    static constexpr double LEDGER_LINE_THICKNESS = 0.15; // in sp
    static constexpr double LEDGER_LINE_LEFTX = 0.25; // in % of cursor rectangle width
    static constexpr double LEDGER_LINE_RIGHTX = 0.75; // in % of cursor rectangle width

    const double spatium = staffType.spatium();
    const double lineDist = staffType.lineDistance().toMM(spatium);

    bool hasFret = false;
    const String text = staffType.tabBassStringPrefix(string, &hasFret);

    const double lw = LEDGER_LINE_THICKNESS * spatium; // use a fixed width
    const muse::draw::Pen pen(color, lw);
    p->setPen(pen);

    // draw conventional 'ledger lines', if required
    const int numOfLedgerLines = staffType.numOfTabLedgerLines(string);
    const double x1 = rect.x() + rect.width() * LEDGER_LINE_LEFTX;
    const double x2 = rect.x() + rect.width() * LEDGER_LINE_RIGHTX;

    // cursor rect is 1 line dist. high, and it is:
    // centred on the line for "frets on strings"    => lower top ledger line 1/2 line dist.
    // sitting on the line for "frets above strings" => lower top ledger line 1 full line dist
    double y = rect.top() + lineDist * (staffType.onLines() ? 0.5 : 1.0);
    for (int i = 0; i < numOfLedgerLines; i++) {
        p->drawLine(LineF(x1, y, x2, y));
        y += lineDist / numOfLedgerLines;     // insert other lines between top line and tab body
    }

    // draw the text, if any
    if (!text.isEmpty()) {
        p->setFont(staffType.fretFont());
        p->drawText(PointF(rect.left(), rect.top() + lineDist), text);
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
