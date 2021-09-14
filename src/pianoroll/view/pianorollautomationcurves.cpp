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

#include "pianorollautomationcurves.h"

#include <QPainter>

#include "audio/iplayer.h"

using namespace mu::pianoroll;

PianorollAutomationCurves::PianorollAutomationCurves(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void PianorollAutomationCurves::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playback()->player()->playbackPositionMsecs().onReceive(this,
                                                            [this](audio::TrackSequenceId currentTrackSequence,
                                                                   const audio::msecs_t newPosMsecs) {
        int tick = score()->utime2utick(newPosMsecs / 1000.);
        setPlaybackPosition(Ms::Fraction::fromTicks(tick));
    });
}

void PianorollAutomationCurves::onNotationChanged()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            onSelectionChanged();
        });

        notation->notationChanged().onNotify(this, [this]() {
            onCurrentNotationChanged();
        });
    }

    buildNoteData();
    updateBoundingSize();
}

void PianorollAutomationCurves::onCurrentNotationChanged()
{
    buildNoteData();
    updateBoundingSize();
}

void PianorollAutomationCurves::onSelectionChanged()
{
    buildNoteData();
    update();
}

void PianorollAutomationCurves::buildNoteData()
{
}

Ms::Score* PianorollAutomationCurves::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
    return score;
}

Ms::Staff* PianorollAutomationCurves::activeStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();

    Ms::Staff* staff = score->staff(m_activeStaff);
    return staff;
}

void PianorollAutomationCurves::setPlaybackPosition(Ms::Fraction value)
{
    if (value == m_playbackPosition) {
        return;
    }
    m_playbackPosition = value;
    update();

    emit playbackPositionChanged();
}

void PianorollAutomationCurves::setPropertyName(QString value)
{
    if (value == m_propertyName) {
        return;
    }
    m_propertyName = value;
    update();

    emit propertyNameChanged();
}


void PianorollAutomationCurves::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth) {
        return;
    }
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollAutomationCurves::setCenterX(double value)
{
    if (value == m_centerX) {
        return;
    }
    m_centerX = value;
    update();

    emit centerXChanged();
}

void PianorollAutomationCurves::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth) {
        return;
    }
    m_displayObjectWidth = value;
    updateBoundingSize();

    emit displayObjectWidthChanged();
}

void PianorollAutomationCurves::setTuplet(int value)
{
    if (value == m_tuplet) {
        return;
    }
    m_tuplet = value;
    update();

    emit tupletChanged();
}

void PianorollAutomationCurves::setSubdivision(int value)
{
    if (value == m_subdivision) {
        return;
    }
    m_subdivision = value;
    update();

    emit subdivisionChanged();
}

void PianorollAutomationCurves::updateBoundingSize()
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

int PianorollAutomationCurves::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollAutomationCurves::pixelXToWholeNote(int pixX) const
{
    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}


void PianorollAutomationCurves::mousePressEvent(QMouseEvent* e)
{
//    if (e->button() == Qt::LeftButton) {
//        m_mouseDown = true;
//        m_mouseDownPos = e->pos();
//        m_lastMousePos = m_mouseDownPos;
//        m_dragBlock = pickBlock(m_mouseDownPos);
//    }
}

void PianorollAutomationCurves::mouseReleaseEvent(QMouseEvent* e)
{
//    m_mouseDown = false;
//    m_dragging = false;
//    m_dragBlock = nullptr;
}

void PianorollAutomationCurves::mouseMoveEvent(QMouseEvent* e)
{
//    if (m_mouseDown) {
//        m_lastMousePos = e->pos();

//        if (!m_dragging && m_dragBlock) {
//            int dx = e->x() - m_mouseDownPos.x();
//            int dy = e->y() - m_mouseDownPos.y();
//            if (dx * dx + dy * dy > m_pickRadius * m_pickRadius) {
//                //Start dragging
//                m_dragging = true;
//            }
//        }

//        IPianorollAutomationModel* model = lookupModel(m_automationType);

//        if (m_dragging && model) {
//            QPointF offset = m_lastMousePos - m_mouseDownPos;

//            Ms::Score* curScore = score();
//            Ms::Staff* staff = curScore->staff(m_dragBlock->staffIdx);

//            double valMax = model->maxValue();
//            double valMin = model->minValue();

//            int pixMaxY = height() - m_marginY;
//            int pixMinY = m_marginY;

//            double dragToVal = pixYToValue(m_lastMousePos.y(), valMin, valMax);
//            dragToVal = qMin(qMax(dragToVal, valMin), valMax);

//            model->setValue(staff, *m_dragBlock, dragToVal);

//            update();
//        }
//    }
}

void PianorollAutomationCurves::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* curScore = notation->elements()->msScore();
    Ms::Staff* staff = curScore->staff(0);
    Ms::Part* part = staff->part();

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    Ms::Measure* lm = curScore->lastMeasure();
    Ms::Fraction end = lm->tick() + lm->ticks();

    qreal x1 = wholeNoteToPixelX(0);
    qreal x2 = wholeNoteToPixelX(end);

    p->fillRect(x1, 0, x2 - x1, height(), m_colorGridBackground);

    //-----------------------------------
    //Draw horizontal grid lines
    p->setPen(penLineMinor);
    p->drawLine(QLineF(x1, m_marginY, x2, m_marginY));
    p->drawLine(QLineF(x1, height() - m_marginY, x2, height() - m_marginY));

    //-----------------------------------
    //Draw vertial grid lines
    const int minBeatGap = 20;
    for (Ms::MeasureBase* m = curScore->first(); m; m = m->next()) {
        Ms::Fraction start = m->tick();  //fraction representing number of whole notes since start of score.  Expressed in terms of the note getting the beat in this bar
        Ms::Fraction len = m->ticks();  //Beats in bar / note length with the beat

        p->setPen(penLineMajor);
        int x = wholeNoteToPixelX(start);
        p->drawLine(x, 0, x, height());

        //Beats
        int beatWidth = m_wholeNoteWidth * len.numerator() / len.denominator();
        if (beatWidth < minBeatGap) {
            continue;
        }

        for (int i = 1; i < len.numerator(); ++i) {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
            int x = wholeNoteToPixelX(beat);
            p->setPen(penLineMinor);
            p->drawLine(x, 0, x, height());
        }

        //Subbeats
        int subbeats = m_tuplet * (1 << m_subdivision);

        int subbeatWidth = m_wholeNoteWidth * len.numerator() / (len.denominator() * subbeats);

        if (subbeatWidth < minBeatGap) {
            continue;
        }

        for (int i = 0; i < len.numerator(); ++i) {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
            for (int j = 1; j < subbeats; ++j) {
                Ms::Fraction subbeat = beat + Ms::Fraction(j, len.denominator() * subbeats);
                int x = wholeNoteToPixelX(subbeat);
                p->setPen(penLineSub);
                p->drawLine(x, 0, x, height());
            }
        }
    }
}

double PianorollAutomationCurves::pixYToValue(double pixY, double valMin, double valMax)
{
    int span = height() - m_marginY * 2;
    double frac = (span + m_marginY - pixY) / (double)span;
    return (valMax - valMin) * frac + valMin;
}

double PianorollAutomationCurves::valueToPixY(double value, double valMin, double valMax)
{
    double frac = (value - valMin) / (valMax - valMin);
    int span = height() - m_marginY * 2;
    return m_marginY + span - frac * span;
}
