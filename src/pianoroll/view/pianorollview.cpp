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

#include "pianorollview.h"

#include "libmscore/element.h"
#include "libmscore/measure.h"
#include "libmscore/fraction.h"
#include "libmscore/pos.h"

#include <QPainter>

using namespace mu::pianoroll;


//PianoItem::PianoItem(Note* n, PianorollView* pianoView)
//   : _note(n), _pianoView(pianoView)
//      {
//      }

//--------------------

PianorollView::PianorollView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    int j = 9;
}

void PianorollView::onNotationChanged()
{
    //updateBoundingSize();
    //update();
    auto notation = globalContext()->currentNotation();
    if (notation)
    {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            onSelectionChanged();
        });

        notation->notationChanged().onNotify(this, [this]() {
            onCurrentNotationChanged();
        });
    }
}

void PianorollView::onCurrentNotationChanged()
{
    updateBoundingSize();
    //update();

}

void PianorollView::onSelectionChanged()
{
    int j = 9;
}

void PianorollView::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth)
        return;
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollView::setNoteHeight(int value)
{
    if (value == m_noteHeight)
        return;
    m_noteHeight = value;
    updateBoundingSize();

    emit noteHeightChanged();
}

void PianorollView::setTuplet(int value)
{
    if (value == m_tuplet)
        return;
    m_tuplet = value;

    emit tupletChanged();
}

void PianorollView::setSubdivision(int value)
{
    if (value == m_subdivision)
        return;
    m_subdivision = value;

    emit subdivisionChanged();
}

void PianorollView::setTool(PianorollTool value)
{
    if (value == m_tool)
        return;
    m_tool = value;
    emit toolChanged();
}


void PianorollView::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });


//    globalContext()->currentNotationChanged().onNotify(this, [this]() {
//        onCurrentNotationChanged();
//    });

//    controller()->noteLayoutChanged().onNotify(this, [this]() {
//        onNotationChanged();
//    });

    //auto notation = globalContext()->currentNotation();
    //if (!notation)
    //{
    //    return;
    //}

    //notation->undoStack()->stackChanged().onNotify(this, [this]() {
    //    onNotationChanged();
    //});

//    notation->notationChanged().onNotify(this, [this]() {
//        onNotationChanged();
    //});
}

void PianorollView::updateBoundingSize()
{
    //    setImplicitSize((int)(32 * m_wholeNoteWidth), m_noteHeight * 128);


    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* score = notation->elements()->msScore();
    //    Ms::Fraction beats = controller()->widthInBeats();
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction beats = lm->tick() + lm->ticks();
    double beat = beats.numerator() / (double)beats.denominator();
    setImplicitSize((int)(beat * m_wholeNoteWidth), m_noteHeight * NUM_PITCHES);

//    Ms::Score* score = notation->elements()->msScore();
//    Ms::Measure* lm = score->lastMeasure();
//    Ms::Fraction beats = lm->tick() + lm->ticks();

//    int width = tickToPixelX(beats.ticks());
//    setImplicitSize(width, m_noteHeight * NUM_PITCHES);

    update();
}

int PianorollView::tickToPixelX(int tick)
{
    return static_cast<int>(tick * m_wholeNoteWidth);
}

int PianorollView::pixelXToTick(int pixX)
{
    return static_cast<int>(pixX / m_wholeNoteWidth);
}

void PianorollView::paint(QPainter* p)
{    
    p->fillRect(0, 0, width(), height(), m_colorKeyWhite);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
//    std::vector<Ms::Element*> selectedElements = notation->interaction()->selection()->elements();
//    std::vector<int> selectedStaves;
//    int activeStaff = -1;
//    for (Ms::Element* e: selectedElements)
//    {
//        int idx = e->staffIdx();
//        qDebug() << "ele idx " << idx;
//        activeStaff = idx;
//        if (std::find(selectedStaves.begin(), selectedStaves.end(), idx) == selectedStaves.end())
//        {
//            selectedStaves.push_back(idx);
//        }
//    }


    //
    const QColor colSelectionBoxFill = QColor(
                      m_colorSelectionBox.red(), m_colorSelectionBox.green(), m_colorSelectionBox.blue(),
                      128);

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    qreal y1 = 0;
    qreal y2 = height();
    qreal x1 = 0;
    qreal x2 = width();

    //Draw horizontal grid lines
    for (int i = 0; i < NUM_PITCHES; ++i)
    {
        if ((i % 4) == 0)
        {
            p->fillRect(0, i * m_noteHeight, width(), m_noteHeight, m_colorKeyBlack);
        }
    }

//    QRect viewportRect = p->viewport();
//    QRect winRect = p->window();
    //QRectF clipRect = p->clipBoundingRect();
    for (int i = 0; i < NUM_PITCHES; ++i)
    {
        int degree = i % 12;
        p->setPen(degree == 0 ? penLineMajor : penLineMinor);
        p->drawLine(QLineF(0, i * m_noteHeight, width(), i * m_noteHeight));
    }

    //-----------------------------------
    //Draw vertial grid lines
    const int minBeatGap = 20;
    for (Ms::MeasureBase* m = score->first(); m; m = m->next())
    {
        Ms::Fraction start = m->tick();  //fraction representing number of whole notes since start of score.  Expressed in terms of the note getting the beat in this bar
        Ms::Fraction len = m->ticks();  //Beats in bar / note length with the beat

        p->setPen(penLineMajor);
        int x = m_wholeNoteWidth * start.numerator() / start.denominator();
        p->drawLine(x, 0, x, height());

        //Beats
        int beatWidth = m_wholeNoteWidth * len.numerator() / len.denominator();
        if (beatWidth < minBeatGap)
            continue;

        for (int i = 1; i < len.numerator(); ++i)
        {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
            int x = m_wholeNoteWidth * beat.numerator() / beat.denominator();
            p->setPen(penLineMinor);
            p->drawLine(x, 0, x, height());

        }


        //Subbeats
        int subbeats = m_tuplet * (1 << m_subdivision);

        int subbeatWidth = m_wholeNoteWidth * len.numerator() / (len.denominator() * subbeats);

        if (subbeatWidth < minBeatGap)
            continue;

        for (int i = 0; i < len.numerator(); ++i)
        {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
            for (int j = 1; j < subbeats; ++j)
            {
                Ms::Fraction subbeat = beat + Ms::Fraction(j, len.denominator() * subbeats);
                int x = m_wholeNoteWidth * subbeat.numerator() / subbeat.denominator();
                p->setPen(penLineSub);
                p->drawLine(x, 0, x, height());
            }
        }

    }

    {
        Ms::Measure* lm = score->lastMeasure();
        Ms::Fraction end = lm->tick() + lm->ticks();
        p->setPen(penLineMajor);
        int x = m_wholeNoteWidth * end.numerator() / end.denominator();
        p->drawLine(x, 0, x, height());
    }




//    const int minBeatGap = 20;
//    Ms::Pos pos1(score->tempomap(), score->sigmap(), qMax(pixelXToTick(x1), 0), Ms::TType::TICKS);
//    Ms::Pos pos2(score->tempomap(), score->sigmap(), qMax(pixelXToTick(x2), 0), Ms::TType::TICKS);

//    int bar1, bar2, beat, tick;
//    pos1.mbt(&bar1, &beat, &tick);
//    pos2.mbt(&bar2, &beat, &tick);


//    for (int bar = bar1; bar <= bar2; ++bar)
//    {
//        Ms::Pos barPos(score->tempomap(), score->sigmap(), bar, 0, 0);

//        //Beat lines
//        int beatsInBar = barPos.timesig().timesig().numerator();
//        int ticksPerBeat = barPos.timesig().timesig().beatTicks();
//        double pixPerBeat = ticksPerBeat * m_beatWidth;
//        int beatSkip = ceil(minBeatGap / pixPerBeat);


//        //Round up to next power of 2
//        beatSkip = (int)pow(2, ceil(log(beatSkip)/log(2)));

//        for (int beat1 = 0; beat1 < beatsInBar; beat1 += beatSkip)
//        {
//          Ms::Pos beatPos(score->tempomap(), score->sigmap(), bar, beat1, 0);
//          double x = tickToPixelX(beatPos.time(Ms::TType::TICKS));
//          p->setPen(penLineMinor);
//          p->drawLine(x, y1, x, y2);

//          int subbeats = m_tuplet * (1 << m_subdiv);

//          for (int sub = 1; sub < subbeats; ++sub)
//          {
//              Ms::Pos subBeatPos(score->tempomap(), score->sigmap(), bar, beat1, sub * Ms::MScore::division / subbeats);
//              x = tickToPixelX(subBeatPos.time(Ms::TType::TICKS));

//              p->setPen(penLineSub);
//              p->drawLine(x, y1, x, y2);
//          }

//        }

//        //Bar line
//        double x = tickToPixelX(barPos.time(Ms::TType::TICKS));
//        p->setPen(x > 0 ? penLineMajor : QPen(Qt::black, 2.0));
//        p->drawLine(x, y1, x, y2);

//    }


    //score->

//    p->setPen(Qt::blue);
//    p->drawEllipse(0, 0, width(), height());

    int value = controller()->getNotes();
}
