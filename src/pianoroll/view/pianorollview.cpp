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
#include "libmscore/pitchspelling.h"

#include <QPainter>
#include <QGuiApplication>

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

static const qreal MIN_DRAG_DIST_SQ = 9;

//--------------------

PianorollView::PianorollView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void PianorollView::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    controller()->pitchHighlightChanged().onNotify(this, [this](){
        update();
    });
}

void PianorollView::onNotationChanged()
{
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

    buildNoteData();
    updateBoundingSize();
}

void PianorollView::onCurrentNotationChanged()
{
    buildNoteData();
    updateBoundingSize();
    //update();

}

void PianorollView::onSelectionChanged()
{
    buildNoteData();
    update();
}

void PianorollView::buildNoteData()
{
    for (auto block : m_noteList)
        delete block;
    m_noteList.clear();
    m_activeStaff = -1;
    m_selectedStaves.clear();

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }


    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
    std::vector<Ms::Element*> selectedElements = notation->interaction()->selection()->elements();
    for (Ms::Element* e: selectedElements)
    {
        int idx = e->staffIdx();
        m_activeStaff = idx;
        if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end())
        {
            m_selectedStaves.push_back(idx);
        }
    }

    if (m_activeStaff == -1)
        m_activeStaff = 0;

    for (int staffIndex : m_selectedStaves)
    {
        Ms::Staff* staff = score->staff(staffIndex);

        for (Ms::Segment* s = staff->score()->firstSegment(Ms::SegmentType::ChordRest); s; s = s->next1(Ms::SegmentType::ChordRest))
        {
            for (int voice = 0; voice < VOICES; ++voice)
            {
                int track = voice + staffIndex * VOICES;
                Ms::Element* e = s->element(track);
                if (e && e->isChord())
                    addChord(toChord(e), voice, staffIndex);
            }
        }
    }
}

void PianorollView::addChord(Ms::Chord* chrd, int voice, int staffIdx)
{
    for (Ms::Chord* c : chrd->graceNotes())
        addChord(c, voice, staffIdx);

    for (Ms::Note* note : chrd->notes())
    {
        if (note->tieBack())
              continue;

        m_noteList.push_back(new NoteBlock { note, voice, staffIdx });
    }
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

void PianorollView::setCenterX(double value)
{
    if (value == m_centerX)
        return;
    m_centerX = value;

    update();
    emit centerXChanged();
}

void PianorollView::setCenterY(double value)
{
    if (value == m_centerY)
        return;
    m_centerY = value;

    update();
    emit centerYChanged();
}

void PianorollView::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth)
        return;
    m_displayObjectWidth = value;
    update();

    emit displayObjectWidthChanged();
}

void PianorollView::setDisplayObjectHeight(double value)
{
    if (value == m_displayObjectHeight)
        return;
    m_displayObjectHeight = value;
    update();

    emit displayObjectHeightChanged();
}

void PianorollView::setTool(PianorollTool value)
{
    if (value == m_tool)
        return;
    m_tool = value;
    emit toolChanged();
}


void PianorollView::updateBoundingSize()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Ms::Score* score = notation->elements()->msScore();
    //    Ms::Fraction beats = controller()->widthInBeats();
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction wholeNotesFrac = lm->tick() + lm->ticks();
    double wholeNotes = wholeNotesFrac.numerator() / (double)wholeNotesFrac.denominator();
    setImplicitSize((int)(wholeNotes * m_wholeNoteWidth), m_noteHeight * NUM_PITCHES);

    setDisplayObjectWidth(wholeNotes * m_wholeNoteWidth);
    setDisplayObjectHeight(m_noteHeight * NUM_PITCHES);

    update();
}

int PianorollView::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollView::pixelXToWholeNote(int pixX) const
{

    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}

int PianorollView::pitchToPixelY(double pitch) const
{
    return (128 - pitch) * m_noteHeight - m_centerY * m_displayObjectHeight + height() / 2;
}

double PianorollView::pixelYToPitch(int pixY) const
{
    return 128 - ((pixY + m_centerY * m_displayObjectHeight - height() / 2) / m_noteHeight);
}

void PianorollView::paint(QPainter* p)
{    
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    //Find staff to draw from
    Ms::Score* score = notation->elements()->msScore();
    //std::vector<Ms::Element*> selectedElements = notation->interaction()->selection()->elements();
    //std::vector<int> selectedStaves;
    //int activeStaff = -1;
    //for (Ms::Element* e: selectedElements)
    //{
    //    int idx = e->staffIdx();
    //    qDebug() << "ele idx " << idx;
    //    activeStaff = idx;
    //    if (std::find(selectedStaves.begin(), selectedStaves.end(), idx) == selectedStaves.end())
    //    {
    //        selectedStaves.push_back(idx);
    //    }
    //}

    //if (activeStaff == -1)
    //    activeStaff = 0;
//        return;
    Ms::Staff* staff = score->staff(m_activeStaff);
    Ms::Part* part = staff->part();



    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    Ms::Measure* lm = score->lastMeasure();
    Ms::Fraction end = lm->tick() + lm->ticks();

    qreal y1 = pitchToPixelY(0);
    qreal y2 = pitchToPixelY(128);
    qreal x1 = wholeNoteToPixelX(0);
    qreal x2 = wholeNoteToPixelX(end);


    //-----------------------------------
    //Draw horizontal grid lines
    Ms::Interval transp = part->instrument()->transpose();

    //MIDI notes span [0, 127] and map to pitches with MIDI pitch 0 being the note C-1

    //Colored stripes
    for (int pitch = 0; pitch < NUM_PITCHES; ++pitch)
    {
//        int y = (127 - pitch) * m_noteHeight;
        double y = pitchToPixelY(pitch + 1);

        int degree = (pitch - transp.chromatic + 60) % 12;
        const BarPattern& pat = barPatterns[m_barPattern];


        if (controller()->isPitchHighlight(pitch))
        {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyHighlight);
        }
        else if (!pat.isWhiteKey[degree])
        {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyBlack);
        }
        else
        {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyWhite);
        }
    }

    //Bounding lines
    for (int pitch = 0; pitch <= NUM_PITCHES; ++pitch)
    {
//        int y = (128 - pitch) * m_noteHeight;
        double y = pitchToPixelY(pitch);
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
//        int x = m_wholeNoteWidth * start.numerator() / start.denominator();
        int x = wholeNoteToPixelX(start);
        p->drawLine(x, 0, x, height());

        //Beats
        int beatWidth = m_wholeNoteWidth * len.numerator() / len.denominator();
        if (beatWidth < minBeatGap)
            continue;

        for (int i = 1; i < len.numerator(); ++i)
        {
            Ms::Fraction beat = start + Ms::Fraction(i, len.denominator());
//            int x = m_wholeNoteWidth * beat.numerator() / beat.denominator();
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
//                int x = m_wholeNoteWidth * subbeat.numerator() / subbeat.denominator();
                int x = wholeNoteToPixelX(subbeat);
                p->setPen(penLineSub);
                p->drawLine(x, 0, x, height());
            }
        }

    }

    {
        p->setPen(penLineMajor);
//        int x = m_wholeNoteWidth * end.numerator() / end.denominator();
        int x = wholeNoteToPixelX(end);
        p->drawLine(x, 0, x, height());
    }

    //-----------------
    //Notes

    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    for (NoteBlock* block: m_noteList)
    {
        drawNoteBlock(p, block);
    }



//    //Draw locators
//    for (int i = 0; i < 3; ++i) {
//          if (_locator[i].valid())
//                {
//                p->setPen(QPen(i == 0 ? Qt::red : Qt::blue, 2));
//                qreal x = tickToPixelX(_locator[i].time(TType::TICKS));
//                p->drawLine(x, y1, x, y2);
//                }
//          }

    //Draw drag selection box
    if (m_dragStarted && m_dragStyle == DragStyle::SELECTION_RECT && m_tool == PianorollTool::SELECT)
    {
        int minX = qMin(m_mouseDownPos.x(), m_lastMousePos.x());
        int minY = qMin(m_mouseDownPos.y(), m_lastMousePos.y());
        int maxX = qMax(m_mouseDownPos.x(), m_lastMousePos.x());
        int maxY = qMax(m_mouseDownPos.y(), m_lastMousePos.y());
        QRectF rect(minX, minY, maxX - minX + 1, maxY - minY + 1);


        const QColor fillColor = QColor(
            m_colorSelectionBox.red(), m_colorSelectionBox.green(), m_colorSelectionBox.blue(),
            128);

        p->setPen(QPen(m_colorSelectionBox, 2));
        p->setBrush(QBrush(fillColor, Qt::SolidPattern));
        p->drawRect(rect);
    }

}


QRect PianorollView::boundingRect(Ms::Note* note, Ms::NoteEvent* evt)
{
    Ms::Chord* chord = note->chord();

    Ms::Fraction baseLen = chord->ticks();
    Ms::Fraction tieLen = note->playTicksFraction() - baseLen;
    int pitch = note->pitch() + (evt ? evt->pitch() : 0);
    Ms::Fraction len = (evt ? baseLen * evt->len() / 1000 : baseLen) + tieLen;

    Ms::Fraction start = note->chord()->tick();
    if (evt)
        start += evt->ontime() * baseLen / 1000;


//    int x0 = m_wholeNoteWidth * start.numerator() / start.denominator();
    int x0 = wholeNoteToPixelX(start);
//    int y0 = (127 - pitch) * m_noteHeight;
    int y0 = pitchToPixelY(pitch + 1);
    int width = m_wholeNoteWidth * len.numerator() / len.denominator();

    QRect rect;
    rect.setRect(x0, y0, width, m_noteHeight);
    return rect;
}

void PianorollView::drawNoteBlock(QPainter *p, NoteBlock *block)
{
    Ms::Note* note = block->note;
    if (note->tieBack())
        return;


    QColor noteColor;
    switch (block->voice)
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

    if (block->staffIdx != m_activeStaff)
        noteColor = noteColor.lighter(150);

    p->setBrush(noteColor);
    p->setPen(QPen(noteColor.darker(250)));

    for (Ms::NoteEvent& e : note->playEvents())
    {
        QRect bounds = boundingRect(note, &e);
        p->drawRect(bounds);

        //Pitch name
        if (bounds.width() >= 20 && bounds.height() >= 12)
        {
            QRectF textRect(bounds.x() + 2, bounds.y(), bounds.width() - 6, bounds.height() + 1);
            QRectF textHiliteRect(bounds.x() + 3, bounds.y() + 1, bounds.width() - 6, bounds.height());

            QFont f("FreeSans", 8);
            p->setFont(f);

            //Note name
            QString name = note->tpcUserName();
            p->setPen(QPen(noteColor.lighter(130)));
            p->drawText(textHiliteRect,
                Qt::AlignLeft | Qt::AlignTop, name);

            p->setPen(QPen(noteColor.darker(180)));
            p->drawText(textRect,
                Qt::AlignLeft | Qt::AlignTop, name);
        }
    }

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
            noteColor = noteColor.lighter(150);

        p->setBrush(noteColor);
        p->setPen(QPen(noteColor.darker(250)));

        for (Ms::NoteEvent& e : note->playEvents())
        {
            QRect bounds = boundingRect(note, &e);
            p->drawRect(bounds);

            //Pitch name
            if (bounds.width() >= 20 && bounds.height() >= 12)
            {
                QRectF textRect(bounds.x() + 2, bounds.y(), bounds.width() - 6, bounds.height() + 1);
                QRectF textHiliteRect(bounds.x() + 3, bounds.y() + 1, bounds.width() - 6, bounds.height());

                QFont f("FreeSans", 8);
                p->setFont(f);

                //Note name
                QString name = note->tpcUserName();
                p->setPen(QPen(noteColor.lighter(130)));
                p->drawText(textHiliteRect,
                    Qt::AlignLeft | Qt::AlignTop, name);

                p->setPen(QPen(noteColor.darker(180)));
                p->drawText(textRect,
                    Qt::AlignLeft | Qt::AlignTop, name);
            }
        }
    }
}


void PianorollView::keyReleaseEvent(QKeyEvent* event)
{
    if (m_dragStyle == DragStyle::NOTES || m_dragStyle == DragStyle::SELECTION_RECT)
    {
        if (event->key() == Qt::Key_Escape)
        {
            //Cancel drag
            m_dragStyle = DragStyle::CANCELLED;
            m_dragNoteCache = "";
            m_dragStarted = false;
//            scene()->update();
            update();
        }
    }
}

void PianorollView::mousePressEvent(QMouseEvent* event)
{
    bool rightBn = event->button() == Qt::RightButton;
    if (!rightBn)
    {
        m_mouseDown = true;
//        m_mouseDownPos = mapToScene(event->pos());
        m_mouseDownPos = event->pos();
        m_lastMousePos = m_mouseDownPos;
//        scene()->update();
        update();
    }
}

void PianorollView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_dragStyle == DragStyle::CANCELLED) {
        m_dragStyle = DragStyle::NONE;
        m_mouseDown = false;
//        scene()->update();
        update();
        return;
    }

    int modifiers = QGuiApplication::keyboardModifiers();
    bool bnShift = modifiers & Qt::ShiftModifier;
    bool bnCtrl = modifiers & Qt::ControlModifier;

    bool rightBn = event->button() == Qt::RightButton;

    if (rightBn)
        //Right clicks have been handled as popup menu
        return;


    NoteSelectType selType = bnShift ? (bnCtrl ? NoteSelectType::SUBTRACT : NoteSelectType::XOR)
            : (bnCtrl ? NoteSelectType::ADD : NoteSelectType::REPLACE);

    if (m_dragStarted)
    {
        if (m_dragStyle == DragStyle::SELECTION_RECT)
        {
            //Update selection
            qreal minX = qMin(m_mouseDownPos.x(), m_lastMousePos.x());
            qreal minY = qMin(m_mouseDownPos.y(), m_lastMousePos.y());
            qreal maxX = qMax(m_mouseDownPos.x(), m_lastMousePos.x());
            qreal maxY = qMax(m_mouseDownPos.y(), m_lastMousePos.y());

            double startTick = pixelXToWholeNote((int)minX);
            double endTick = pixelXToWholeNote((int)maxX);
            int lowPitch = (int)floor(128 - maxY / noteHeight());
            int highPitch = (int)ceil(128 - minY / noteHeight());

            selectNotes(startTick, endTick, lowPitch, highPitch, selType);
        }
        else if (m_dragStyle == DragStyle::NOTES)
        {
            if (m_tool == PianorollTool::SELECT)
            {
                finishNoteGroupDrag();

                //Keep last note drag event, if any
                if (m_inProgressUndoEvent)
                    m_inProgressUndoEvent = false;
            }
        }

        m_dragStarted = false;
    }
    else {
          //This was just a click, not a drag
        switch (m_tool) {
//          case SELECT:
//                handleSelectionClick();
//                break;
//          case CHANGE_LENGTH:
//                changeChordLength(_mouseDownPos);
//                break;
//          case ERASE:
//                eraseNote(_mouseDownPos);
//                break;
//          case INSERT_NOTE:
//                insertNote(modifiers);
//                break;
//          case APPEND_NOTE:
//                appendNoteToChord(_mouseDownPos);
//                break;
//          case CUT_CHORD:
//                cutChord(_mouseDownPos);
//                break;
//          case TIE:
//                toggleTie(_mouseDownPos);
//                break;
        default:
            break;
        }
    }


    m_dragStyle = DragStyle::NONE;
    m_mouseDown = false;
    //scene()->update();
    update();
}


void PianorollView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragStyle == DragStyle::CANCELLED)
        return;

//    m_lastMousePos = mapToScene(event->pos());
    m_lastMousePos = event->pos();

    if (m_mouseDown && !m_dragStarted)
    {
        qreal dx = m_lastMousePos.x() - m_mouseDownPos.x();
        qreal dy = m_lastMousePos.y() - m_mouseDownPos.y();

        if (dx * dx + dy * dy >= MIN_DRAG_DIST_SQ)
        {
            //Start dragging
            m_dragStarted = true;

            //Check for move note
            double tick = pixelXToWholeNote(m_mouseDownPos.x());
            int mouseDownPitch = pixelYToPitch(m_mouseDownPos.y());

            NoteBlock* pi = pickNote(m_mouseDownPos.x(), m_mouseDownPos.y());
            if (pi && m_tool == PianorollTool::SELECT) {
                  if (!pi->note->selected())
                  {
                      selectNotes(tick, tick, mouseDownPitch, mouseDownPitch, NoteSelectType::REPLACE);
                  }

                  m_dragStyle = DragStyle::NOTES;
                  m_dragStartPitch = mouseDownPitch;
                  m_dragNoteCache = serializeSelectedNotes();
              }
              else if (!pi && m_tool == PianorollTool::SELECT)
                  m_dragStyle = DragStyle::SELECTION_RECT;
              else
                  m_dragStyle = DragStyle::NONE;
        }
    }

    if (m_dragStarted)
    {
        switch (m_tool) {
        case PianorollTool::SELECT:
        case PianorollTool::EDIT:
        case PianorollTool::CUT:
//                scene()->update();
            update();
            break;
        case PianorollTool::ERASE:
//            eraseNote(m_lastMousePos);
            break;
        default:
            break;
        }
    }


    //Update mouse tracker
//    QPointF p(mapToScene(event->pos()));
//    int pitch = static_cast<int>((m_noteHeight * 128 - p.y()) / m_noteHeight);
//    emit pitchChanged(pitch);

//    double tick = pixelXToWholeNote(p.x());
//    if (tick < 0)
//    {
//        tick = 0;
//        trackingPos.setTick(tick);
//        trackingPos.setInvalid();
//    }
//    else
//        trackingPos.setTick(tick);

//    emit trackingPosChanged(trackingPos);
}

Ms::Staff* PianorollView::activeStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Ms::Score* score = notation->elements()->msScore();

    Ms::Staff* staff = score->staff(m_activeStaff);
    return staff;
}

NoteBlock* PianorollView::pickNote(int pixX, int pixY)
{
    for (int i = 0; i < m_noteList.size(); ++i)
    {
        NoteBlock* block = m_noteList[i];
        for (Ms::NoteEvent& e : block->note->playEvents())
        {
            QRect bounds = boundingRect(block->note, &e);
            if (bounds.contains(pixX, pixY))
            {
                return block;
            }

        }
    }

    return nullptr;
}


void PianorollView::finishNoteGroupDrag()
{

}

void PianorollView::selectNotes(double startTick, double endTick, int lowPitch, int highPitch, NoteSelectType selType)
{

}


QString PianorollView::serializeSelectedNotes()
      {
    return "";

//      Fraction firstTick;
//      bool init = false;
//      for (int i = 0; i < _noteList.size(); ++i) {
//            if (_noteList[i]->note()->selected()) {
//                  Note* note = _noteList.at(i)->note();
//                  Fraction startTick = note->chord()->tick();

//                  if (!init || firstTick > startTick) {
//                        firstTick = startTick;
//                        init = true;
//                        }
//                  }
//            }

//      //No valid notes
//      if (!init)
//            return QByteArray();

//      QString xmlStrn;
//      QXmlStreamWriter xml(&xmlStrn);
//      xml.setAutoFormatting(true);
//      xml.writeStartDocument();

//      xml.writeStartElement("notes");
//      xml.writeAttribute("firstN", QString::number(firstTick.numerator()));
//      xml.writeAttribute("firstD", QString::number(firstTick.denominator()));

//      //bundle notes into XML file & send to clipboard.
//      //This is only affects pianoview and is not part of the regular copy/paste process
//      for (int i = 0; i < _noteList.size(); ++i) {
//            if (_noteList[i]->note()->selected()) {
//                  Note* note = _noteList[i]->note();

//                  Fraction flen = note->playTicksFraction();

//                  Fraction startTick = note->chord()->tick();
//                  int pitch = note->pitch();

//                  int voice = note->voice();

//                  int veloOff = note->veloOffset();
//                  Note::ValueType veloType = note->veloType();

//                  xml.writeStartElement("note");
//                  xml.writeAttribute("startN", QString::number(startTick.numerator()));
//                  xml.writeAttribute("startD", QString::number(startTick.denominator()));
//                  xml.writeAttribute("lenN", QString::number(flen.numerator()));
//                  xml.writeAttribute("lenD", QString::number(flen.denominator()));
//                  xml.writeAttribute("pitch", QString::number(pitch));
//                  xml.writeAttribute("voice", QString::number(voice));
//                  xml.writeAttribute("veloOff", QString::number(veloOff));
//                  xml.writeAttribute("veloType", veloType == Note::ValueType::OFFSET_VAL ? "o" : "u");

//                  for (NoteEvent& evt : note->playEvents()) {
//                        int ontime = evt.ontime();
//                        int len = evt.len();

//                        xml.writeStartElement("evt");
//                        xml.writeAttribute("ontime", QString::number(ontime));
//                        xml.writeAttribute("len", QString::number(len));
//                        xml.writeEndElement();
//                        }

//                  xml.writeEndElement();
//                  }
//            }

//      xml.writeEndElement();
//      xml.writeEndDocument();

//      return xmlStrn;
      }
