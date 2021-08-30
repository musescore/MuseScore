/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "pianorollkeyboard.h"

#include <QPainter>

#include "pianorollview.h"

using namespace mu::pianoroll;

PianorollKeyboard::PianorollKeyboard(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

void PianorollKeyboard::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

void PianorollKeyboard::onNotationChanged()
{
    auto notation = globalContext()->currentNotation();
    if (notation)
    {
        notation->notationChanged().onNotify(this, [this]() {
            onCurrentNotationChanged();
        });
    }

    updateBoundingSize();
}

void PianorollKeyboard::onCurrentNotationChanged()
{
    updateBoundingSize();
}

void PianorollKeyboard::setNoteHeight(double value)
{
    if (value == m_noteHeight)
        return;
    m_noteHeight = value;
    updateBoundingSize();

    emit noteHeightChanged();
}

void PianorollKeyboard::setCenterY(double value)
{
    if (value == m_centerY)
        return;
    m_centerY = value;
    update();

    emit centerYChanged();
}

void PianorollKeyboard::setDisplayObjectHeight(double value)
{
    if (value == m_displayObjectHeight)
        return;
    m_displayObjectHeight = value;
    updateBoundingSize();

    emit displayObjectHeightChanged();
}

void PianorollKeyboard::updateBoundingSize()
{
//    notation::INotationPtr notation = globalContext()->currentNotation();
//    if (!notation) {
//        return;
//    }

//    Ms::Score* score = notation->elements()->msScore();

    setDisplayObjectHeight(m_noteHeight * PianorollView::NUM_PITCHES);

    update();
}

int PianorollKeyboard::pitchToPixelY(double pitch) const
{
    return (128 - pitch) * m_noteHeight - m_centerY * m_displayObjectHeight + height() / 2;
}

double PianorollKeyboard::pixelYToPitch(int pixY) const
{
    return 128 - ((pixY + m_centerY * m_displayObjectHeight - height() / 2) / m_noteHeight);
}


void PianorollKeyboard::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }
}
