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

#include "pianorollruler.h"

#include <QPainter>

using namespace mu::pianoroll;


PianorollRuler::PianorollRuler(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{

}

void PianorollRuler::onNotationChanged()
{
    //updateBoundingSize();
    //update();
}

void PianorollRuler::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth)
        return;
    m_wholeNoteWidth = value;
    //updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollRuler::setCenterX(double value)
{
    if (value == m_centerX)
        return;
    m_centerX = value;

    update();
    emit centerXChanged();
}

void PianorollRuler::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth)
        return;
    m_displayObjectWidth = value;
    update();

    emit displayObjectWidthChanged();
}

void PianorollRuler::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

int PianorollRuler::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollRuler::pixelXToWholeNote(int pixX) const
{

    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}

void PianorollRuler::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }
}
