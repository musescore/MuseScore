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


QPixmap* PianorollRuler::markIcon[3];

static const char* rmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "........##########",
      "........#########.",
      "........########..",
      "........#######...",
      "........######....",
      "........#####.....",
      "........####......",
      "........###.......",
      "........##........",
      "........##........",
      "........##........"};

static const char* lmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##########........",
      ".#########........",
      "..########........",
      "...#######........",
      "....######........",
      ".....#####........",
      "......####........",
      ".......###........",
      "........##........",
      "........##........",
      "........##........"};

static const char* cmark_xpm[]={
      "18 18 2 1",
      "# c #ff0000",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##################",
      ".################.",
      "..##############..",
      "...############...",
      "....##########....",
      ".....########.....",
      "......######......",
      ".......####.......",
      "........##........",
      "........##........",
      "........##........"};


PianorollRuler::PianorollRuler(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    m_font2.setPixelSize(14);
//    m_font2.setBold(true);
    m_font1.setPixelSize(10);

}


void PianorollRuler::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

void PianorollRuler::onNotationChanged()
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

void PianorollRuler::onCurrentNotationChanged()
{
    updateBoundingSize();
}

void PianorollRuler::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth)
        return;
    m_wholeNoteWidth = value;
    updateBoundingSize();

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
    updateBoundingSize();

    emit displayObjectWidthChanged();
}

void PianorollRuler::updateBoundingSize()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* score = notation->elements()->msScore();
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction wholeNotesFrac = lm->tick() + lm->ticks();
    double wholeNotes = wholeNotesFrac.numerator() / (double)wholeNotesFrac.denominator();

    setDisplayObjectWidth(wholeNotes * m_wholeNoteWidth);

    update();
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

    Ms::Score* score = notation->elements()->msScore();
    Ms::Staff* staff = score->staff(0);

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);

    //Draw lines
    const int minGap = 60;
    int lastDrawPos = -1;
    int measureIndex = 0;

    for (Ms::MeasureBase* m = score->first(); m; m = m->next())
    {
        measureIndex++;
        Ms::Fraction start = m->tick();  //fraction representing number of whole notes since start of score.  Expressed in terms of the note getting the beat in this bar

        int pos = wholeNoteToPixelX(start);

        if (lastDrawPos == -1 || pos - lastDrawPos >= minGap)
        {
            lastDrawPos = pos;

            p->setPen(penLineMajor);
            p->drawLine(pos, 0, pos, height());

            p->setFont(m_font2);
            p->setPen(m_colorText);
            QString text;
            text.setNum(measureIndex);
            int pixelSize = m_font2.pixelSize();
            p->drawText(pos + 4, pixelSize + 2, text);

        }


    }

}
