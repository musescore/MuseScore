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

//    int ticks = controller()->widthInTicks();
//    int noteHeight = controller()->noteHeight();
//    double xZoom = controller()->xZoom();

    //Ms::Measure* lm = _staff->score()->lastMeasure();
    //Ms::Fraction widthInTicks = lm->tick() + lm->ticks();
    //int ticks = widthInTicks.ticks();  //Scaled by Mscore::division * 4 (which is 480 * 4)
    //, pulses per quarter note, ticks per beat


    //notation::INotationPtr notation = globalContext()->currentNotation();
    //if (!notation) {
    //    return;
    //}

    //Ms::Score* score = notation->elements()->msScore();
    //std::vector<Ms::Element*> selectedElements = notation->interaction()->selection()->elements();
    //m_selectedStaves.clear();
    //m_activeStaff = -1;
    //for (Element* e: selectedElements)
    //{
    //    int idx = e->staffIdx();
    //    qDebug() << "ele idx " << idx;
    //    m_activeStaff = idx;
    //    if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end())
    //    {
    //        m_selectedStaves.push_back(idx);

    //    }
    //}

    Ms::Fraction beats = controller()->widthInBeats();
    double beat = beats.numerator() / (double)beats.denominator();
    setImplicitSize((int)(beat * m_wholeNoteWidth), m_noteHeight * 128);

//    Score* score = controller()->score();

//    Measure* lm = score->lastMeasure();
//    _ticks = (lm->tick() + lm->ticks()).ticks();
//    scene()->setSceneRect(0.0, 0.0, double((_ticks + MAP_OFFSET * 2) * _xZoom), _noteHeight * 128);

    update();
}

void PianorollView::paint(QPainter* p)
{    
//    if (m_icon.isNull()) {
    p->fillRect(0, 0, width(), height(), m_color);
//        return;
//    }

    p->setPen(Qt::blue);
    p->drawEllipse(0, 0, width(), height());
    //p->fill(0, 0, width(), height(), m_color);

//    const QIcon::Mode mode = m_selected ? QIcon::Selected : QIcon::Active;
//    const QIcon::State state = m_active ? QIcon::On : QIcon::Off;
//    m_icon.paint(p, QRect(0, 0, width(), height()), Qt::AlignCenter, mode, state);

    int value = controller()->getNotes();
    int j = 9;
}
