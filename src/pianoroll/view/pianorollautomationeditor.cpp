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

#include "pianorollautomationeditor.h"

#include <QPainter>

#include "audio/iplayer.h"

using namespace mu::pianoroll;


PianorollAutomationEditor::PianorollAutomationEditor(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{

}

void PianorollAutomationEditor::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playback()->player()->playbackPositionMsecs().onReceive(this, [this](audio::TrackSequenceId currentTrackSequence, const audio::msecs_t newPosMsecs) {
        int tick = score()->utime2utick(newPosMsecs / 1000.);
        setPlaybackPosition(Ms::Fraction::fromTicks(tick));
    });
}

void PianorollAutomationEditor::onNotationChanged()
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

void PianorollAutomationEditor::onCurrentNotationChanged()
{
    updateBoundingSize();
}

Ms::Score* PianorollAutomationEditor::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation)
    {
        return nullptr;
    }

    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
    return score;
}

void PianorollAutomationEditor::setPlaybackPosition(Ms::Fraction value)
{
    if (value == m_playbackPosition)
        return;
    m_playbackPosition = value;
    update();

    emit playbackPositionChanged();
}

void PianorollAutomationEditor::setAutomationAttribute(AutomationAttribute value)
{
    if (value == m_automationAttribute)
        return;
    m_automationAttribute = value;
    update();

    emit automationAttributeChanged();
}


void PianorollAutomationEditor::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth)
        return;
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollAutomationEditor::setCenterX(double value)
{
    if (value == m_centerX)
        return;
    m_centerX = value;
    update();

    emit centerXChanged();
}

void PianorollAutomationEditor::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth)
        return;
    m_displayObjectWidth = value;
    updateBoundingSize();

    emit displayObjectWidthChanged();
}

void PianorollAutomationEditor::setTuplet(int value)
{
    if (value == m_tuplet)
        return;
    m_tuplet = value;
    update();

    emit tupletChanged();
}

void PianorollAutomationEditor::setSubdivision(int value)
{
    if (value == m_subdivision)
        return;
    m_subdivision = value;
    update();

    emit subdivisionChanged();
}
void PianorollAutomationEditor::updateBoundingSize()
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

int PianorollAutomationEditor::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollAutomationEditor::pixelXToWholeNote(int pixX) const
{
    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}


void PianorollAutomationEditor::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* score = notation->elements()->msScore();
    Ms::Staff* staff = score->staff(0);
    Ms::Part* part = staff->part();

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction end = lm->tick() + lm->ticks();

//    qreal y1 = pitchToPixelY(0);
//    qreal y2 = pitchToPixelY(128);
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
    for (Ms::MeasureBase* m = score->first(); m; m = m->next())
    {
        Ms::Fraction start = m->tick();  //fraction representing number of whole notes since start of score.  Expressed in terms of the note getting the beat in this bar
        Ms::Fraction len = m->ticks();  //Beats in bar / note length with the beat

        p->setPen(penLineMajor);
        int x = wholeNoteToPixelX(start);
        p->drawLine(x, 0, x, height());

        //Beats
        int beatWidth = m_wholeNoteWidth * len.numerator() / len.denominator();
        if (beatWidth < minBeatGap)
            continue;

        for (int i = 1; i < len.numerator(); ++i)
        {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
            int x = wholeNoteToPixelX(beat);
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
                int x = wholeNoteToPixelX(subbeat);
                p->setPen(penLineSub);
                p->drawLine(x, 0, x, height());
            }
        }

    }


}


