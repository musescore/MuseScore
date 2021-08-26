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
#include "libmscore/noteevent.h"
#include "libmscore/note.h"

#include <QPainter>

using namespace mu::pianoroll;



const BarPattern PianorollView::barPatterns[] = {
      {QT_TRANSLATE_NOOP("BarPattern", "C major / A minor"),   {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "D♭ major / B♭ minor"), {1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D major / B minor"),   {0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "E♭ major / C minor"),  {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "E major / C♯ minor"),  {0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "F major / D minor"),   {1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "G♭ major / E♭ minor"), {0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "G major / E minor"),   {1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "A♭ major / F minor"),  {1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "A major / F♯ minor"),  {0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "B♭ major / G minor"),  {1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "B major / G♯ minor"),  {0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "C Diminished"),  {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D♭ Diminished"), {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D Diminished"),  {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "C Half/Whole"),  {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D♭ Half/Whole"), {0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "D Half/Whole"),  {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "C Whole tone"),  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D♭ Whole tone"), {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1}},
      {QT_TRANSLATE_NOOP("BarPattern", "C Augmented"),   {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D♭ Augmented"),  {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "D Augmented"),   {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0}},
      {QT_TRANSLATE_NOOP("BarPattern", "E♭ Augmented"),  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1}},
      {"",              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

//--------------------

PianorollView::PianorollView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    memset(m_pitchHighlight, 0, 128);
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
    update();
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
    std::vector<Ms::Element*> selectedElements = notation->interaction()->selection()->elements();
    std::vector<int> selectedStaves;
    int activeStaff = -1;
    for (Ms::Element* e: selectedElements)
    {
        int idx = e->staffIdx();
        qDebug() << "ele idx " << idx;
        activeStaff = idx;
        if (std::find(selectedStaves.begin(), selectedStaves.end(), idx) == selectedStaves.end())
        {
            selectedStaves.push_back(idx);
        }
    }

    if (activeStaff == -1)
        activeStaff = 0;
//        return;
    Ms::Staff* staff = score->staff(activeStaff);
    Ms::Part* part = staff->part();



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

    //-----------------------------------
    //Draw horizontal grid lines
    Ms::Interval transp = part->instrument()->transpose();

    //MIDI notes span [0, 127] and map to pitches with MIDI pitch 0 being the note C-1

    //Colored stripes
    for (int pitch = 0; pitch < NUM_PITCHES; ++pitch)
    {
        int y = (127 - pitch) * m_noteHeight;

        int degree = (pitch - transp.chromatic + 60) % 12;
        const BarPattern& pat = barPatterns[m_barPattern];

        if (m_pitchHighlight[pitch])
        {
            p->fillRect(0, y, width(), m_noteHeight, m_colorKeyHighlight);
        }
        else if (!pat.isWhiteKey[degree])
        {
            p->fillRect(0, y, width(), m_noteHeight, m_colorKeyBlack);
        }
    }

    //Bounding lines
    for (int pitch = 0; pitch <= NUM_PITCHES; ++pitch)
    {
        int y = (128 - pitch) * m_noteHeight;
        int degree = (pitch - transp.chromatic + 60) % 12;
        p->setPen(degree == 0 ? penLineMajor : penLineMinor);
        p->drawLine(QLineF(0, y, width(), y));
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

    //-----------------
    //Notes
    for (int staffIndex : selectedStaves)
    {
        Ms::Staff* staff = score->staff(staffIndex);
//        activeStaff

        for (Ms::Segment* s = staff->score()->firstSegment(Ms::SegmentType::ChordRest); s; s = s->next1(Ms::SegmentType::ChordRest))
        {
            for (int voice = 0; voice < VOICES; ++voice)
            {
                int track = voice + staffIndex * VOICES;
                Ms::Element* e = s->element(track);
                if (e && e->isChord())
                    drawChord(p, toChord(e), voice, staffIndex == activeStaff);
            }
        }

    }

//    Ms::SegmentType st = ;


    int value = controller()->getNotes();
}

QRect PianorollView::boundingRect(Ms::Note* note, Ms::NoteEvent* evt)
{
    Ms::Chord* chord = note->chord();
//    int ticks = chord->ticks().ticks();
//    int tieLen = note->playTicks() - ticks;
//    int pitch = note->pitch() + (evt ? evt->pitch() : 0);
//    int len = (evt ? ticks * evt->len() / 1000 : ticks) + tieLen;

//    int x1 = _note->chord()->tick().ticks()
//        + (evt ? evt->ontime() * ticks / 1000 : 0);
//    qreal y1 = pitch;

//    QRect rect;
//    rect.setRect(x1, y1, len, 1);
//    return rect;

    Ms::Fraction baseLen = chord->ticks();
    Ms::Fraction tieLen = note->playTicksFraction() - baseLen;
    int pitch = note->pitch() + (evt ? evt->pitch() : 0);
    Ms::Fraction len = (evt ? baseLen * evt->len() / 1000 : baseLen) + tieLen;

    Ms::Fraction start = note->chord()->tick();
    if (evt)
        start += evt->ontime() * baseLen / 1000;


    int x0 = m_wholeNoteWidth * start.numerator() / start.denominator();
    int y0 = (127 - pitch) * m_noteHeight;
    int width = m_wholeNoteWidth * len.numerator() / len.denominator();

    QRect rect;
    rect.setRect(x0, y0, width, m_noteHeight);
    return rect;
}

void PianorollView::drawChord(QPainter* p, Ms::Chord* chrd, int voice, bool active)
{
    for (Ms::Chord* c : chrd->graceNotes())
        drawChord(p, c, voice, active);

    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    for (Ms::Note* note : chrd->notes())
    {
        if (note->tieBack())
            continue;

//            _noteList.append(new PianoItem(note, this));
//        Ms::Fraction start = chord->tick();
//        Ms::Fraction len = chord->ticks();
//        Ms::Fraction end = start + len;
//        int pitch = note->pitch();

        QColor noteColor;
        switch (voice)
        {
        case 0:
            noteColor = m_colorNoteVoice1;
            break;
        case 1:
            noteColor = m_colorNoteVoice2;
            break;
        case 2:
            noteColor = m_colorNoteVoice3;
            break;
        case 3:
            noteColor = m_colorNoteVoice4;
            break;
        }

        if (note->selected())
            noteColor = m_colorNoteSel;
        if (!active)
            noteColor = m_colorNoteGhost;

        p->setBrush(noteColor);
        p->setPen(QPen(noteColor.darker(250)));

//        QRectF bounds = boundingRectPixels(evt);

//        int x0 = m_wholeNoteWidth * start.numerator() / start.denominator();
//        int x1 = m_wholeNoteWidth * end.numerator() / end.denominator();
//        int width = m_wholeNoteWidth * len.numerator() / len.denominator();

        for (Ms::NoteEvent& e : note->playEvents())
        {
            QRect bounds = boundingRect(note, &e);
            p->drawRect(bounds);

//              paintNoteBlock(painter, &e);
        }

//        QRect bounds(x0, pitch * m_noteHeight, len, m_noteHeight);
//        p->drawRect(bounds);

    }
}
