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
#include <qquickitem.h>

#include "audio/iplayer.h"
#include "libmscore/property.h"
#include "libmscore/staff.h"
#include "libmscore/animationtrack.h"
#include "libmscore/animationkey.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

struct KeyBlock
{
    AnimationKey* key;
};

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
        setPlaybackPosition(Fraction::fromTicks(tick));
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
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Score* curScore = score();

    m_selectedStaves.clear();
    std::vector<EngravingItem*> selectedElements = notation->interaction()->selection()->elements();
    for (EngravingItem* e: selectedElements) {
        int idx = e->staffIdx();
        m_activeStaff = idx;
        if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end()) {
            m_selectedStaves.push_back(idx);
        }
    }

    if (m_activeStaff == -1) {
        m_activeStaff = 0;
    }

//    staff = activeStaff();
//    if (!staff)
//        return;

//    Ms::Pid id = Ms::Pid::END;
//    if (!m_propertyName.isEmpty())
//        id = Ms::propertyId(m_propertyName);

//    if (id == Ms::Pid::END)
//        return;

//    Ms::AnimationTrack* track = staff->getAnimationTrack(m_propertyName);
//    if (track)
//    {
//        for (auto key: track->keys())
//        {
//            m_keyBlocks.push_back(new KeyBlock{key});
//        }
//    }
}

Score* PianorollAutomationCurves::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Score* score = notation->elements()->msScore();
    return score;
}

Staff* PianorollAutomationCurves::activeStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Score* score = notation->elements()->msScore();

    Staff* staff = score->staff(m_activeStaff);
    return staff;
}

void PianorollAutomationCurves::setPlaybackPosition(Fraction value)
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

    Score* score = notation->elements()->msScore();
    Measure* lm = score->lastMeasure();
    Fraction wholeNotesFrac = lm->tick() + lm->ticks();
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

AnimationTrack* PianorollAutomationCurves::getAnimationTrack()
{
    Staff* staff = activeStaff();
    if (!staff) {
        return nullptr;
    }

    Pid id = Pid::END;
    if (!m_propertyName.isEmpty()) {
        id = propertyId(m_propertyName.toStdString());
    }

    if (id == Pid::END) {
        return nullptr;
    }

    AnimationTrack* track = staff->getAnimationTrack(m_propertyName.toStdString());
    return track;
}

AnimationKey* PianorollAutomationCurves::pickKey(QPointF point)
{
    Staff* staff = activeStaff();
    if (!staff) {
        return nullptr;
    }

    Pid id = Pid::END;
    if (!m_propertyName.isEmpty()) {
        id = propertyId(m_propertyName.toStdString());
    }

    double pMax = propertyMaxValue(id).toDouble();
    double pMin = propertyMinValue(id).toDouble();

    AnimationTrack* track = getAnimationTrack();
    if (!track) {
        return nullptr;
    }

    for (AnimationKey* key: track->keys()) {
        int x = wholeNoteToPixelX(key->tick());
        int y = valueToPixY(key->value(), pMin, pMax);

        int dx = x - point.x();
        int dy = y - point.y();
        if (dx * dx + dy * dy < m_vertexRadius * m_vertexRadius) {
            return key;
        }
    }
    return nullptr;
}

void PianorollAutomationCurves::mouseDoubleClickEvent(QMouseEvent* event)
{
    Score* curScore = score();
    Staff* staff = activeStaff();

    if (!staff) {
        return;
    }

    Pid id = Pid::END;
    if (!m_propertyName.isEmpty()) {
        id = propertyId(m_propertyName.toStdString());
    }

    if (id == Pid::END) {
        return;
    }
    double pMax = propertyMaxValue(id).toDouble();
    double pMin = propertyMinValue(id).toDouble();

    //Visible area we are rendering to
    Measure* lm = curScore->lastMeasure();
    Fraction end = lm->tick() + lm->ticks();

    double endTicks = end.numerator() / (double)end.denominator();

    double val = pixYToValue(m_lastMousePos.y(), pMin, pMax);
    double tick = pixelXToWholeNote(m_lastMousePos.x());
    val = qMin(qMax(val, pMin), pMax);
    tick = qMin(qMax(tick, 0.0), endTicks);

    Fraction tickFrac(floor(tick * Constants::division), Constants::division);

    AnimationTrack* track = staff->getAnimationTrack(m_propertyName.toStdString());
    if (!track) {
        track = staff->createAnimationTrack(m_propertyName.toStdString());
    }
    track->addKey(tickFrac, val);

    update();
}

void PianorollAutomationCurves::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        m_mouseDown = true;
        m_mouseDownPos = e->pos();
        m_lastMousePos = m_mouseDownPos;
        m_draggedKey = pickKey(m_mouseDownPos);
    }
}

void PianorollAutomationCurves::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_dragging) {
        finishDrag();
    }

    m_mouseDown = false;
    m_dragging = false;
    m_draggedKey = nullptr;
}

void PianorollAutomationCurves::finishDrag()
{
    Score* curScore = score();
    Staff* staff = activeStaff();

    if (!staff) {
        return;
    }

    Pid id = Pid::END;
    if (!m_propertyName.isEmpty()) {
        id = propertyId(m_propertyName.toStdString());
    }

    if (id == Pid::END) {
        return;
    }
    double pMax = propertyMaxValue(id).toDouble();
    double pMin = propertyMinValue(id).toDouble();

    //Visible area we are rendering to
    Measure* lm = curScore->lastMeasure();
    Fraction end = lm->tick() + lm->ticks();

    double endTicks = end.toDouble();

    double val = pixYToValue(m_lastMousePos.y(), pMin, pMax);
    double tick = pixelXToWholeNote(m_lastMousePos.x());
    val = qMin(qMax(val, pMin), pMax);
    tick = qMin(qMax(tick, 0.0), endTicks);

    Fraction tickFrac(floor(tick * Constants::division), Constants::division);

    AnimationTrack* track = staff->getAnimationTrack(m_propertyName.toStdString());
    track->removeKey(m_draggedKey->tick());
    track->addKey(tickFrac, val);

    update();
}

void PianorollAutomationCurves::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mouseDown) {
        m_lastMousePos = e->pos();

        if (!m_dragging && m_draggedKey) {
            int dx = e->x() - m_mouseDownPos.x();
            int dy = e->y() - m_mouseDownPos.y();
            if (dx * dx + dy * dy > m_pickRadius * m_pickRadius) {
                //Start dragging
                m_dragging = true;
            }
        }

        update();
    }
}

void PianorollAutomationCurves::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Score* curScore = notation->elements()->msScore();
    Staff* staff = curScore->staff(0);
    Part* part = staff->part();

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    Measure* lm = curScore->lastMeasure();
    Fraction end = lm->tick() + lm->ticks();

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
    for (MeasureBase* m = curScore->first(); m; m = m->next()) {
        Fraction start = m->tick();  //fraction representing number of whole notes since start of score.  Expressed in terms of the note getting the beat in this bar
        Fraction len = m->ticks();  //Beats in bar / note length with the beat

        p->setPen(penLineMajor);
        int x = wholeNoteToPixelX(start);
        p->drawLine(x, 0, x, height());

        //Beats
        int beatWidth = m_wholeNoteWidth * len.numerator() / len.denominator();
        if (beatWidth < minBeatGap) {
            continue;
        }

        for (int i = 1; i < len.numerator(); ++i) {
            Fraction beat = start + Fraction(i, len.denominator());
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
            Fraction beat = start + Fraction(i, len.denominator());
            for (int j = 1; j < subbeats; ++j) {
                Fraction subbeat = beat + Fraction(j, len.denominator() * subbeats);
                int x = wholeNoteToPixelX(subbeat);
                p->setPen(penLineSub);
                p->drawLine(x, 0, x, height());
            }
        }
    }

    //------------------
    //Draw note data
    staff = activeStaff();
    if (!staff) {
        return;
    }

    Pid id = Pid::END;
    if (!m_propertyName.isEmpty()) {
        id = propertyId(m_propertyName.toStdString());
    }

    if (id == Pid::END) {
        return;
    }

    double pMax = propertyMaxValue(id).toDouble();
    double pMin = propertyMinValue(id).toDouble();
    double pDef = propertyDefaultValue(id).toDouble();

    const QPen penDataLine = QPen(m_colorDataLine, 1.0, Qt::SolidLine);
    p->setPen(penDataLine);

    double yDefault = valueToPixY(pDef, pMin, pMax);

    AnimationTrack* track = staff->getAnimationTrack(m_propertyName.toStdString());
    if (!track) {
        //Draw line at default level
        p->drawLine(x1, yDefault, x2, yDefault);
        return;
    }

    //Find screen coords of keys in sorted order
    std::map<int, QPoint> points;
    for (AnimationKey* key: track->keys()) {
        int y = valueToPixY(key->value(), pMin, pMax);
        int x = wholeNoteToPixelX(key->tick());

        if (m_dragging && key == m_draggedKey) {
            continue;
        }

        points[key->tick().ticks()] = QPoint(x, y);
    }

    //Special handling for dragged key
    if (m_dragging) {
        double endTicks = end.numerator() / (double)end.denominator();

        double val = pixYToValue(m_lastMousePos.y(), pMin, pMax);
        double tick = pixelXToWholeNote(m_lastMousePos.x());
        val = qMin(qMax(val, pMin), pMax);
        tick = qMin(qMax(tick, 0.0), endTicks);

        int y = valueToPixY(val, pMin, pMax);
        int x = wholeNoteToPixelX(tick);

        int iticks = Fraction::fromFloat(tick).ticks();
        points[iticks] = QPoint(x, y);
    }

    //Draw lines between points
    QPoint prev((int)x1, (int)yDefault);
    for (auto it = points.begin(); it != points.end(); ++it) {
        QPoint cur = (*it).second;
        p->drawLine(prev, cur);
        prev = (*it).second;
    }
    p->drawLine(prev, QPoint(x2, prev.y()));

    //Draw vertices
    for (auto it = points.begin(); it != points.end(); ++it) {
        int x0 = (*it).second.x();
        int y = (*it).second.y();

        QRectF circleBounds(x0 - m_vertexRadius, y - m_vertexRadius, m_vertexRadius * 2, m_vertexRadius * 2);
        p->setPen(m_colorVertexLine);
        p->setBrush(m_colorVertexFill);
        p->drawEllipse(circleBounds);
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
