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

#include "pianorollautomationnote.h"

#include <QPainter>

#include "audio/iplayer.h"
#include "automation/automationvelocity.h"
#include "automation/automationvelocityabs.h"
#include "automation/automationposition.h"
#include "automation/automationduration.h"
#include "automation/automationdurationmult.h"
#include "ipianorollautomationmodel.h"

#include "pianorollview.h"

using namespace mu::pianoroll;

std::vector<IPianorollAutomationModel*> PianorollAutomationNote::m_automationModels = {
    new AutomationVelocity(),
    new AutomationVelocityAbs(),
    new AutomationPosition(),
    new AutomationDuration(),
    new AutomationDurationMult(),
};

PianorollAutomationNote::PianorollAutomationNote(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

IPianorollAutomationModel* PianorollAutomationNote::lookupModel(AutomationType type)
{
    for (auto model: m_automationModels) {
        if (model->type() == type) {
            return model;
        }
    }
    return nullptr;
}

void PianorollAutomationNote::load()
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

void PianorollAutomationNote::onNotationChanged()
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

void PianorollAutomationNote::onCurrentNotationChanged()
{
    buildNoteData();
    updateBoundingSize();
}

void PianorollAutomationNote::onSelectionChanged()
{
    buildNoteData();
    update();
}

void PianorollAutomationNote::buildNoteData()
{
    for (auto n: m_noteLevels) {
        delete n;
    }
    m_noteLevels.clear();

    IPianorollAutomationModel* model = lookupModel(m_automationType);
    if (!model) {
        return;
    }

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* curScore = score();

    m_selectedStaves.clear();
    std::vector<Ms::EngravingItem*> selectedElements = notation->interaction()->selection()->elements();
    for (Ms::EngravingItem* e: selectedElements) {
        int idx = e->staffIdx();
        m_activeStaff = idx;
        if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end()) {
            m_selectedStaves.push_back(idx);
        }
    }

    if (m_activeStaff == -1) {
        m_activeStaff = 0;
    }

    for (int staffIndex : m_selectedStaves) {
        Ms::Staff* staff = curScore->staff(staffIndex);

        for (Ms::Segment* s = staff->score()->firstSegment(Ms::SegmentType::ChordRest); s; s = s->next1(Ms::SegmentType::ChordRest)) {
            for (int voice = 0; voice < PianorollView::VOICES; ++voice) {
                int track = voice + staffIndex * PianorollView::VOICES;
                Ms::EngravingItem* e = s->element(track);
                if (e && e->isChord()) {
                    addChord(toChord(e), voice, staffIndex);
                }
            }
        }
    }
}

void PianorollAutomationNote::addChord(Ms::Chord* chrd, int voice, int staffIdx)
{
    for (Ms::Chord* c : chrd->graceNotes()) {
        addChord(c, voice, staffIdx);
    }

    for (Ms::Note* note : chrd->notes()) {
        if (note->tieBack()) {
            continue;
        }

        m_noteLevels.push_back(new NoteEventBlock { note, voice, staffIdx });
    }
}

Ms::Score* PianorollAutomationNote::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
    return score;
}

Ms::Staff* PianorollAutomationNote::activeStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();

    Ms::Staff* staff = score->staff(m_activeStaff);
    return staff;
}

void PianorollAutomationNote::setPlaybackPosition(Ms::Fraction value)
{
    if (value == m_playbackPosition) {
        return;
    }
    m_playbackPosition = value;
    update();

    emit playbackPositionChanged();
}

void PianorollAutomationNote::setAutomationType(AutomationType value)
{
    if (value == m_automationType) {
        return;
    }
    m_automationType = value;
    update();

    emit automationTypeChanged();
}

void PianorollAutomationNote::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth) {
        return;
    }
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollAutomationNote::setCenterX(double value)
{
    if (value == m_centerX) {
        return;
    }
    m_centerX = value;
    update();

    emit centerXChanged();
}

void PianorollAutomationNote::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth) {
        return;
    }
    m_displayObjectWidth = value;
    updateBoundingSize();

    emit displayObjectWidthChanged();
}

void PianorollAutomationNote::setTuplet(int value)
{
    if (value == m_tuplet) {
        return;
    }
    m_tuplet = value;
    update();

    emit tupletChanged();
}

void PianorollAutomationNote::setSubdivision(int value)
{
    if (value == m_subdivision) {
        return;
    }
    m_subdivision = value;
    update();

    emit subdivisionChanged();
}

void PianorollAutomationNote::updateBoundingSize()
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

int PianorollAutomationNote::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollAutomationNote::pixelXToWholeNote(int pixX) const
{
    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}

void PianorollAutomationNote::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        m_mouseDown = true;
        m_mouseDownPos = e->pos();
        m_lastMousePos = m_mouseDownPos;
        m_dragBlock = pickBlock(m_mouseDownPos);
    }
}

void PianorollAutomationNote::mouseReleaseEvent(QMouseEvent* e)
{
    m_mouseDown = false;
    m_dragging = false;
    m_dragBlock = nullptr;
}

void PianorollAutomationNote::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mouseDown) {
        m_lastMousePos = e->pos();

        if (!m_dragging && m_dragBlock) {
            int dx = e->x() - m_mouseDownPos.x();
            int dy = e->y() - m_mouseDownPos.y();
            if (dx * dx + dy * dy > m_pickRadius * m_pickRadius) {
                //Start dragging
                m_dragging = true;
            }
        }

        IPianorollAutomationModel* model = lookupModel(m_automationType);

        if (m_dragging && model) {
            QPointF offset = m_lastMousePos - m_mouseDownPos;

            Ms::Score* curScore = score();
            Ms::Staff* staff = curScore->staff(m_dragBlock->staffIdx);

            double valMax = model->maxValue();
            double valMin = model->minValue();

            int pixMaxY = height() - m_marginY;
            int pixMinY = m_marginY;

            double dragToVal = pixYToValue(m_lastMousePos.y(), valMin, valMax);
            dragToVal = qMin(qMax(dragToVal, valMin), valMax);

            model->setValue(staff, *m_dragBlock, dragToVal);

            update();
        }
    }
}

void PianorollAutomationNote::paint(QPainter* p)
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

    //--------------------
    //Notes
    IPianorollAutomationModel* model = lookupModel(m_automationType);

    if (model) {
        for (int i = 0; i < m_noteLevels.size(); ++i) {
            NoteEventBlock* block = m_noteLevels.at(i);
            Ms::Staff* staff = curScore->staff(block->staffIdx);

            double valMax = model->maxValue();
            double valMin = model->minValue();
            double val = model->value(staff, *block);

            double x0 = wholeNoteToPixelX(block->note->tick());

            int gridSpanY = height() - m_marginY * 2;
            int y = valueToPixY(val, valMin, valMax);

            int x1;
            if (i < m_noteLevels.size() - 1) {
                NoteEventBlock* blockNext = m_noteLevels.at(i + 1);
                x1 = wholeNoteToPixelX(blockNext->note->tick());
            } else {
                x1 = wholeNoteToPixelX(block->note->tick() + block->note->chord()->ticks());
            }

            p->fillRect(x0, y, x1 - x0, gridSpanY - y + m_marginY, m_colorGraphFill);

            p->drawLine(x0, y, x1, y);
            p->drawLine(x0, y, x0, gridSpanY + m_marginY);

            QRectF circleBounds(x0 - m_vertexRadius, y - m_vertexRadius, m_vertexRadius * 2, m_vertexRadius * 2);
            p->setPen(m_colorVertexLine);
            p->setBrush(m_colorVertexFill);
            p->drawEllipse(circleBounds);
        }
    }
}

double PianorollAutomationNote::pixYToValue(double pixY, double valMin, double valMax)
{
    int span = height() - m_marginY * 2;
    double frac = (span + m_marginY - pixY) / (double)span;
    return (valMax - valMin) * frac + valMin;
}

double PianorollAutomationNote::valueToPixY(double value, double valMin, double valMax)
{
    double frac = (value - valMin) / (valMax - valMin);
    int span = height() - m_marginY * 2;
    return m_marginY + span - frac * span;
}

NoteEventBlock* PianorollAutomationNote::pickBlock(QPointF point)
{
    Ms::Score* curScore = score();
    IPianorollAutomationModel* model = lookupModel(m_automationType);

    if (model) {
        for (int i = 0; i < m_noteLevels.size(); ++i) {
            NoteEventBlock* block = m_noteLevels.at(i);
            Ms::Staff* staff = curScore->staff(block->staffIdx);

            double valMax = model->maxValue();
            double valMin = model->minValue();
            double val = model->value(staff, *block);

            double x0 = wholeNoteToPixelX(block->note->tick());
            int y = valueToPixY(val, valMin, valMax);

            QRectF circleBounds(x0 - m_vertexRadius, y - m_vertexRadius, m_vertexRadius * 2, m_vertexRadius * 2);
            if (circleBounds.contains(point)) {
                return block;
            }
        }
    }

    return nullptr;
}
