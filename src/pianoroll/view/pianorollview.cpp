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

#include "libmscore/measure.h"
#include "types/fraction.h"
#include "libmscore/pos.h"
#include "libmscore/noteevent.h"
#include "libmscore/note.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/undo.h"
#include "libmscore/tuplet.h"
#include "notation/inotationplayback.h"
#include "notation/inotationinteraction.h"
#include "audio/iplayer.h"
#include "audio/audiotypes.h"

#include <QPainter>
#include <QCursor>
#include <QGuiApplication>
#include <QXmlStreamWriter>

using namespace mu::pianoroll;
using namespace mu::engraving;

static const qreal MIN_DRAG_DIST_SQ = 9;

//--------------------

PianorollView::PianorollView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    //setMouseTracking(true);
    setAcceptHoverEvents(true);
}

void PianorollView::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    controller()->pitchHighlightChanged().onNotify(this, [this]() {
        update();
    });

    playback()->player()->playbackPositionMsecs().onReceive(this,
                                                            [this](audio::TrackSequenceId,
                                                                   const audio::msecs_t newPosMsecs) {
        int tick = score()->utime2utick(newPosMsecs / 1000.0);
        setPlaybackPosition(Fraction::fromTicks(tick));
    });
}

Score* PianorollView::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Score* score = notation->elements()->msScore();
    return score;
}

void PianorollView::onNotationChanged()
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
    for (auto block : m_noteList) {
        delete block;
    }
    m_noteList.clear();
    m_activeStaff = -1;
    m_selectedStaves.clear();

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    //Find staff to draw from
    Score* score = notation->elements()->msScore();
    std::vector<EngravingItem*> selectedElements = notation->interaction()->selection()->elements();
    for (EngravingItem* e: selectedElements) {
        int idx = (int)e->staffIdx();
        m_activeStaff = idx;
        if (std::find(m_selectedStaves.begin(), m_selectedStaves.end(), idx) == m_selectedStaves.end()) {
            m_selectedStaves.push_back(idx);
        }
    }

    if (m_activeStaff == -1) {
        m_activeStaff = 0;
    }

    for (int staffIndex : m_selectedStaves) {
        Staff* staff = score->staff(staffIndex);

        for (Segment* s = staff->score()->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                int track = voice + staffIndex * VOICES;
                EngravingItem* e = s->element(track);
                if (e && e->isChord()) {
                    addChord(toChord(e), voice, staffIndex);
                }
            }
        }
    }
}

void PianorollView::addChord(Chord* chrd, int voice, int staffIdx)
{
    for (Chord* c : chrd->graceNotes()) {
        addChord(c, voice, staffIdx);
    }

    for (Note* note : chrd->notes()) {
        if (note->tieBack()) {
            continue;
        }

        m_noteList.push_back(new NoteBlock { note, voice, staffIdx });
    }
}

void PianorollView::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth) {
        return;
    }
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollView::setNoteHeight(double value)
{
    if (value == m_noteHeight) {
        return;
    }
    m_noteHeight = value;
    updateBoundingSize();

    emit noteHeightChanged();
}

void PianorollView::setTuplet(int value)
{
    if (value == m_tuplet) {
        return;
    }
    m_tuplet = value;
    update();

    emit tupletChanged();
}

void PianorollView::setSubdivision(int value)
{
    if (value == m_subdivision) {
        return;
    }
    m_subdivision = value;
    update();

    emit subdivisionChanged();
}

void PianorollView::setStripePattern(int value)
{
    if (value == m_stripePattern) {
        return;
    }
    m_stripePattern = value;
    update();

    emit stripePatternChanged();
}

void PianorollView::setCenterX(double value)
{
    if (value == m_centerX) {
        return;
    }
    m_centerX = value;

    update();
    emit centerXChanged();
}

void PianorollView::setCenterY(double value)
{
    if (value == m_centerY) {
        return;
    }
    m_centerY = value;

    update();
    emit centerYChanged();
}

void PianorollView::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth) {
        return;
    }
    m_displayObjectWidth = value;
    update();

    emit displayObjectWidthChanged();
}

void PianorollView::setDisplayObjectHeight(double value)
{
    if (value == m_displayObjectHeight) {
        return;
    }
    m_displayObjectHeight = value;
    update();

    emit displayObjectHeightChanged();
}

void PianorollView::setPlaybackPosition(Fraction value)
{
    if (value == m_playbackPosition) {
        return;
    }
    m_playbackPosition = value;
    update();

    emit playbackPositionChanged();
}

void PianorollView::setTool(PianorollTool value)
{
    if (value == m_tool) {
        return;
    }
    m_tool = value;
    update();
    emit toolChanged();
}

void PianorollView::updateBoundingSize()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Score* score = notation->elements()->msScore();
    //    Ms::Fraction beats = controller()->widthInBeats();
    Measure* lm = score->lastMeasure();
    Fraction wholeNotesFrac = lm->tick() + lm->ticks();
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
    Score* score = notation->elements()->msScore();

    Staff* staff = score->staff(m_activeStaff);
    Part* part = staff->part();

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);
    const QPen penLineSub = QPen(m_colorGridLine, 1.0, Qt::DotLine);

    //Visible area we are rendering to
    Measure* lm = score->lastMeasure();
    Fraction end = lm->tick() + lm->ticks();

    qreal x1 = wholeNoteToPixelX(0);
    qreal x2 = wholeNoteToPixelX(end);

    //-----------------------------------
    //Draw horizontal grid lines
    Interval transp = part->instrument()->transpose();

    //MIDI notes span [0, 127] and map to pitches with MIDI pitch 0 being the note C-1

    //Colored stripes
    for (int pitch = 0; pitch < NUM_PITCHES; ++pitch) {
        double y = pitchToPixelY(pitch + 1);

        int degree = (pitch - transp.chromatic + 60) % 12;
        bool isWhite = (m_stripePattern & (1 << degree)) > 0;

        if (controller()->isPitchHighlight(pitch)) {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyHighlight);
        } else if (!isWhite) {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyBlack);
        } else {
            p->fillRect(x1, y, x2 - x1, m_noteHeight, m_colorKeyWhite);
        }
    }

    //Bounding lines
    for (int pitch = 0; pitch <= NUM_PITCHES; ++pitch) {
        double y = pitchToPixelY(pitch);
        int degree = (pitch - transp.chromatic + 60) % 12;
        p->setPen(degree == 0 ? penLineMajor : penLineMinor);
        p->drawLine(QLineF(0, y, width(), y));
    }

    //-----------------------------------
    //Draw vertial grid lines
    const int minBeatGap = 20;
    for (MeasureBase* m = score->first(); m; m = m->next()) {
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

    {
        p->setPen(penLineMajor);
        int x = wholeNoteToPixelX(end);
        p->drawLine(x, 0, x, height());
    }

    //-----------------
    //Notes

    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    for (NoteBlock* block: m_noteList) {
        drawNoteBlock(p, block);
    }

    if (m_dragStyle == DragStyle::NOTE_POSITION || m_dragStyle == DragStyle::NOTE_LENGTH_END
        || m_dragStyle == DragStyle::NOTE_LENGTH_START || m_dragStyle == DragStyle::DRAW_NOTE
        || m_dragStyle == DragStyle::EVENT_LENGTH || m_dragStyle == DragStyle::EVENT_MOVE
        || m_dragStyle == DragStyle::EVENT_ONTIME) {
        drawDraggedNotes(p);
    }

    //Draw playback position
    {
        p->setPen(QPen(m_colorPlaybackLine, 2));
        int x = wholeNoteToPixelX(m_playbackPosition);
        p->drawLine(x, 0, x, height());
    }

    //Draw drag selection box
    if (m_dragStarted && m_dragStyle == DragStyle::SELECTION_RECT && m_tool == PianorollTool::SELECT) {
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

void PianorollView::drawDraggedNotes(QPainter* painter)
{
    QColor noteColor = m_colorNoteDrag;

    Score* curScore = score();
    Staff* staff = activeStaff();

    if (m_dragStyle == DragStyle::DRAW_NOTE) {
        double startTick = pixelXToWholeNote(m_mouseDownPos.x());
        double endTick = pixelXToWholeNote(m_lastMousePos.x());
        if (startTick > endTick) {
            std::swap(startTick, endTick);
        }

        Fraction startTickFrac = roundToSubdivision(startTick);
        Fraction endTickFrac = roundToSubdivision(endTick, false);

        if (endTickFrac != startTickFrac) {
            double pitch = pixelYToPitch(m_mouseDownPos.y());
            int track = (int)staff->idx() * VOICES + m_editNoteVoice;

            drawDraggedNote(painter, startTickFrac, endTickFrac - startTickFrac, pitch, track, m_colorNoteDrag);
        }
        return;
    }

    if (m_dragStyle == DragStyle::EVENT_LENGTH || m_dragStyle == DragStyle::EVENT_MOVE
        || m_dragStyle == DragStyle::EVENT_ONTIME) {
        Fraction dx = Fraction(m_lastMousePos.x() - m_mouseDownPos.x(), m_wholeNoteWidth).reduced();

        for (int i = 0; i < m_noteList.size(); ++i) {
            NoteBlock* pi = m_noteList[i];
            if (pi->note->selected()) {
                for (NoteEvent& e : pi->note->playEvents()) {
                    Chord* chord = pi->note->chord();
                    Fraction ticks = chord->ticks();
                    Tuplet* tup = chord->tuplet();
                    if (tup) {
                        Fraction frac = tup->ratio();
                        ticks = ticks * frac.inverse();
                    }

                    Fraction start = pi->note->chord()->tick();
                    Fraction len = ticks;
                    Fraction startAdj = start + ticks * e.ontime() / 1000;
                    Fraction lenAdj = ticks * e.len() / 1000;

                    //Calc start, duration of where we dragged to
                    Fraction startNew;
                    Fraction lenNew;
                    switch (m_dragStyle) {
                    case DragStyle::EVENT_ONTIME:
                        startNew = startAdj + dx;
                        lenNew = lenAdj - dx;
                        break;
                    case DragStyle::EVENT_MOVE:
                        startNew = startAdj + dx;
                        lenNew = lenAdj;
                        break;
                    default:
                    case DragStyle::EVENT_LENGTH:
                        startNew = startAdj;
                        lenNew = lenAdj + dx;
                        break;
                    }

                    int pitch = pi->note->pitch();
                    int voice = pi->note->voice();
                    int track = (int)staff->idx() * VOICES + voice;

                    drawDraggedNote(painter, startNew, lenNew, pitch, track, m_colorNoteDrag);
                }
            }
        }

        return;
    }

    Fraction pos(pixelXToWholeNote(m_lastMousePos.x()) * 1000, 1000);
    Measure* m = curScore->tick2measure(pos);

    Fraction timeSig = m->timesig();
    int noteWithBeat = timeSig.denominator();

    //Number of smaller pieces the beat is divided into
    int subbeats = m_tuplet * (1 << m_subdivision);
    int divisions = noteWithBeat * subbeats;

    //Round down to nearest division
    double dragToTick = pixelXToWholeNote(m_lastMousePos.x());
    double startTick = pixelXToWholeNote(m_mouseDownPos.x());
    double offsetTicks = dragToTick - startTick;

    //Adjust offset so that note under cursor is aligned to note divistion
    Fraction pasteTickOffset(0, 1);
    Fraction pasteLengthOffset(0, 1);
    int pitchOffset = 0;

    int dragToPitch = pixelYToPitch(m_lastMousePos.y());
    int startPitch = pixelYToPitch(m_mouseDownPos.y());

    if (m_dragStyle == DragStyle::NOTE_POSITION) {
        double noteStartDraggedTick = m_dragStartTick.numerator() / (double)m_dragStartTick.denominator() + offsetTicks;
        Fraction noteStartDraggedAlignedTick = Fraction(floor(noteStartDraggedTick * divisions), divisions);
        pasteTickOffset = noteStartDraggedAlignedTick - m_dragStartTick;
        pitchOffset = dragToPitch - startPitch;
    } else if (m_dragStyle == DragStyle::NOTE_LENGTH_END) {
        double noteEndDraggedTick = m_dragEndTick.numerator() / (double)m_dragEndTick.denominator() + offsetTicks;
        Fraction noteEndDraggedAlignedTick = Fraction(floor(noteEndDraggedTick * divisions), divisions);
        pasteLengthOffset = noteEndDraggedAlignedTick - m_dragEndTick;
    } else if (m_dragStyle == DragStyle::NOTE_LENGTH_START) {
        double noteStartDraggedTick = m_dragStartTick.numerator() / (double)m_dragStartTick.denominator() + offsetTicks;
        Fraction noteStartDraggedAlignedTick = Fraction(floor(noteStartDraggedTick * divisions), divisions);
        pasteTickOffset = noteStartDraggedAlignedTick - m_dragStartTick;
        pasteLengthOffset = m_dragStartTick - noteStartDraggedAlignedTick;
    }

    //Iterate thorugh note data
    QXmlStreamReader xml(m_dragNoteCache);
    Fraction firstTick;

    while (!xml.atEnd())
    {
        QXmlStreamReader::TokenType tt = xml.readNext();
        if (tt == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == "notes") {
                int num = xml.attributes().value("firstN").toString().toInt();
                int den = xml.attributes().value("firstD").toString().toInt();
                firstTick = Fraction(num, den);
            }
            if (xml.name().toString() == "note") {
                int sn = xml.attributes().value("startN").toString().toInt();
                int sd = xml.attributes().value("startD").toString().toInt();
                Fraction startTick = Fraction(sn, sd);

                int tn = xml.attributes().value("lenN").toString().toInt();
                int td = xml.attributes().value("lenD").toString().toInt();
                Fraction tickLen = Fraction(tn, td);
                tickLen += pasteLengthOffset;
                if (tickLen.numerator() <= 0) {
                    continue;
                }

                int pitch = xml.attributes().value("pitch").toString().toInt();
                int voice = xml.attributes().value("voice").toString().toInt();

                int track = (int)staff->idx() * VOICES + voice;

                drawDraggedNote(painter, startTick + pasteTickOffset, tickLen, pitch + pitchOffset, track, m_colorNoteDrag);
            }
        }
    }
}

void PianorollView::drawDraggedNote(QPainter* painter, Fraction startTick, Fraction frac, int pitch, int track, QColor color)
{
    Q_UNUSED(track);
    painter->setBrush(color);

    painter->setPen(QPen(color.darker(250)));
    int x0 = wholeNoteToPixelX(startTick);
    int x1 = wholeNoteToPixelX(startTick + frac);
    int y0 = pitchToPixelY(pitch);

    QRectF bounds(x0, y0 - m_noteHeight, x1 - x0, m_noteHeight);
    painter->drawRect(bounds);
}

QRect PianorollView::boundingRect(Note* note, bool applyEvents)
{
    for (NoteEvent& e : note->playEvents()) {
        QRect bounds = boundingRect(note, &e, applyEvents);
        return bounds;
    }
    return QRect();
}

QRect PianorollView::boundingRect(Note* note, NoteEvent* evt, bool applyEvents)
{
    Chord* chord = note->chord();
    int pitch = note->pitch() + (evt ? evt->pitch() : 0);

    Fraction ticks = chord->ticks();
    Tuplet* tup = chord->tuplet();
    if (tup) {
        Fraction frac = tup->ratio();
        ticks = ticks * frac.inverse();
    }
    Fraction tieLen = note->playTicksFraction() - ticks;

    Fraction start;
    Fraction len;
    if (evt && applyEvents) {
        start = note->chord()->tick() + ticks * evt->ontime() / 1000;
        len = ticks * evt->len() / 1000 + tieLen;
    } else {
        start = note->chord()->tick();
        len = ticks + tieLen;
    }

    int x0 = wholeNoteToPixelX(start);
    int y0 = pitchToPixelY(pitch + 1);
    int width = m_wholeNoteWidth * len.numerator() / len.denominator();

    QRect rect;
    rect.setRect(x0, y0, width, m_noteHeight);
    return rect;
}

void PianorollView::drawNoteBlock(QPainter* p, NoteBlock* block)
{
    Note* note = block->note;
    if (note->tieBack()) {
        return;
    }

    QColor noteColor;
    if (displayEventAdjustment()) {
        noteColor = m_colorTweaks;
    } else {
        switch (block->voice) {
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
    }

    if (note->selected()) {
        noteColor = m_colorNoteSel;
    }

    if (block->staffIdx != m_activeStaff) {
        noteColor = noteColor.lighter(150);
    }

    p->setBrush(noteColor);
    p->setPen(QPen(noteColor.darker(250)));

    for (NoteEvent& e : note->playEvents()) {
        QRect bounds = boundingRect(note, &e, displayEventAdjustment());
        p->drawRoundedRect(bounds, m_noteRectRoundedRadius, m_noteRectRoundedRadius);

        //Pitch name
        if (bounds.width() >= 20 && bounds.height() >= 12) {
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

void PianorollView::keyReleaseEvent(QKeyEvent* event)
{
    if (m_dragStyle == DragStyle::NOTE_POSITION || m_dragStyle == DragStyle::SELECTION_RECT) {
        if (event->key() == Qt::Key_Escape) {
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
    if (!rightBn) {
        m_mouseDown = true;
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

    if (rightBn) {
        //Right clicks have been handled as popup menu
        return;
    }

    NoteSelectType selType = bnShift ? (bnCtrl ? NoteSelectType::SUBTRACT : NoteSelectType::XOR)
                             : (bnCtrl ? NoteSelectType::ADD : NoteSelectType::REPLACE);

    if (m_dragStarted) {
        if (m_dragStyle == DragStyle::SELECTION_RECT) {
            //Update selection
            qreal minX = qMin(m_mouseDownPos.x(), m_lastMousePos.x());
            qreal minY = qMin(m_mouseDownPos.y(), m_lastMousePos.y());
            qreal maxX = qMax(m_mouseDownPos.x(), m_lastMousePos.x());
            qreal maxY = qMax(m_mouseDownPos.y(), m_lastMousePos.y());

            double startTick = pixelXToWholeNote((int)minX);
            double endTick = pixelXToWholeNote((int)maxX);
            double lowPitch = pixelYToPitch(minY);
            double highPitch = pixelYToPitch(maxY);

            selectNotes(startTick, endTick, lowPitch, highPitch, selType);
        } else if (m_dragStyle == DragStyle::NOTE_POSITION || m_dragStyle == DragStyle::NOTE_LENGTH_START
                   || m_dragStyle == DragStyle::NOTE_LENGTH_END) {
            if (m_tool == PianorollTool::SELECT || m_tool == PianorollTool::ADD) {
                finishNoteGroupDrag();

                //Keep last note drag event, if any
                if (m_inProgressUndoEvent) {
                    m_inProgressUndoEvent = false;
                }
            }
        } else if (m_dragStyle == DragStyle::EVENT_ONTIME || m_dragStyle == DragStyle::EVENT_MOVE
                   || m_dragStyle == DragStyle::EVENT_LENGTH) {
            finishNoteEventAdjustDrag();
        } else if (m_dragStyle == DragStyle::DRAW_NOTE) {
            double startTick = pixelXToWholeNote(m_mouseDownPos.x());
            double endTick = pixelXToWholeNote(m_lastMousePos.x());
            if (startTick > endTick) {
                std::swap(startTick, endTick);
            }

            Fraction startTickFrac = roundToSubdivision(startTick);
            Fraction endTickFrac = roundToSubdivision(endTick, false);

            if (endTickFrac != startTickFrac) {
                double pitch = pixelYToPitch(m_mouseDownPos.y());

                Score* curScore = score();
                Staff* staff = activeStaff();

                int voice = m_editNoteVoice;
                int track = (int)staff->idx() * VOICES + voice;

                Fraction duration = endTickFrac - startTickFrac;

                //Store duration as new length for future single-click note add events
                m_editNoteLength = duration;

                //Do command
                curScore->startCmd();
                addNote(startTickFrac, duration, (int)pitch, track);
                curScore->endCmd();

                buildNoteData();
            }
        }
        m_dragStarted = false;
    } else {
        //This was just a click, not a drag
        switch (m_tool) {
        case PianorollTool::SELECT:
            handleSelectionClick();
            break;
        case PianorollTool::ERASE:
            eraseNote(m_mouseDownPos);
            break;
        case PianorollTool::ADD:
        {
            NoteBlock* pi = pickNote(m_mouseDownPos.x(), m_mouseDownPos.y());
            if (pi) {
                handleSelectionClick();
            } else {
                insertNote(modifiers);
            }
            break;
        }
        case PianorollTool::CUT:
            cutChord(m_mouseDownPos);
            break;
        default:
            break;
        }
    }

    m_dragStyle = DragStyle::NONE;
    m_mouseDown = false;
    //scene()->update();
    update();
}

void PianorollView::hoverMoveEvent(QHoverEvent* event)
{
    if (m_tool == PianorollTool::SELECT || m_tool == PianorollTool::ADD || m_tool == PianorollTool::EVENT_ADJUST) {
        QPointF pos = event->pos();
        NoteBlock* pi = pickNote(pos.x(), pos.y());

        if (pi) {
            QRect bounds = boundingRect(pi->note, displayEventAdjustment());
            if (bounds.contains(pos.x(), pos.y())) {
                if (pos.x() <= bounds.x() + m_dragNoteLengthMargin
                    || pos.x() >= bounds.x() + bounds.width() - m_dragNoteLengthMargin) {
                    setCursor(Qt::SizeHorCursor);
                    return;
                }
            }
        }
    }

    setCursor(Qt::ArrowCursor);
}

void PianorollView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragStyle == DragStyle::CANCELLED) {
        return;
    }

    m_lastMousePos = event->pos();

    if (!m_mouseDown) {
        //Update cursor
    }

    if (m_mouseDown && !m_dragStarted) {
        qreal dx = m_lastMousePos.x() - m_mouseDownPos.x();
        qreal dy = m_lastMousePos.y() - m_mouseDownPos.y();

        if (dx * dx + dy * dy >= MIN_DRAG_DIST_SQ) {
            //Start dragging
            m_dragStarted = true;

            if (event->buttons() & Qt::MiddleButton) {
                m_dragStyle = DragStyle::MOVE_VIEWPORT;
                m_dragViewportStart.setX(m_centerX);
                m_dragViewportStart.setY(m_centerY);
            } else {
                //Check for move note
                double tick = pixelXToWholeNote(m_mouseDownPos.x());
                double mouseDownPitch = pixelYToPitch(m_mouseDownPos.y());

                NoteBlock* pi = pickNote(m_mouseDownPos.x(), m_mouseDownPos.y());
                if (pi && (m_tool == PianorollTool::SELECT || m_tool == PianorollTool::ADD)) {
                    if (!pi->note->selected()) {
                        selectNotes(tick, tick, mouseDownPitch, mouseDownPitch, NoteSelectType::REPLACE);
                    }

                    QRect bounds = boundingRect(pi->note, false);
                    if (m_mouseDownPos.x() <= bounds.x() + m_dragNoteLengthMargin) {
                        m_dragStyle = DragStyle::NOTE_LENGTH_START;
                    } else if (m_mouseDownPos.x() >= bounds.x() + bounds.width() - m_dragNoteLengthMargin) {
                        m_dragStyle = DragStyle::NOTE_LENGTH_END;
                    } else {
                        m_dragStyle = DragStyle::NOTE_POSITION;
                    }

                    m_dragStartPitch = mouseDownPitch;
                    m_dragStartTick = pi->note->tick();
                    m_dragEndTick = m_dragStartTick + pi->note->chord()->ticks();
                    m_dragNoteCache = serializeSelectedNotes();
                } else if (pi && m_tool == PianorollTool::EVENT_ADJUST) {
                    if (!pi->note->selected()) {
                        selectNotes(tick, tick, mouseDownPitch, mouseDownPitch, NoteSelectType::REPLACE);
                    }

                    QRect bounds = boundingRect(pi->note, true);
                    if (m_mouseDownPos.x() <= bounds.x() + m_dragNoteLengthMargin) {
                        m_dragStyle = DragStyle::EVENT_ONTIME;
                    } else if (m_mouseDownPos.x() >= bounds.x() + bounds.width() - m_dragNoteLengthMargin) {
                        m_dragStyle = DragStyle::EVENT_LENGTH;
                    } else {
                        m_dragStyle = DragStyle::EVENT_MOVE;
                    }
                } else if (!pi && m_tool == PianorollTool::SELECT) {
                    m_dragStyle = DragStyle::SELECTION_RECT;
                } else if (!pi && m_tool == PianorollTool::ADD) {
                    m_dragStyle = DragStyle::DRAW_NOTE;
                } else {
                    m_dragStyle = DragStyle::NONE;
                }
            }
        }
    }

    if (m_dragStarted) {
        if (m_dragStyle == DragStyle::MOVE_VIEWPORT) {
            qreal dx = m_lastMousePos.x() - m_mouseDownPos.x();
            qreal dy = m_lastMousePos.y() - m_mouseDownPos.y();

            dx /= width();
            dy /= height();
            setCenterX(m_dragViewportStart.x() + dx);
            setCenterY(m_dragViewportStart.y() + dy);
        } else {
            switch (m_tool) {
            case PianorollTool::SELECT:
            case PianorollTool::ADD:
            case PianorollTool::CUT:
            case PianorollTool::EVENT_ADJUST:
                //                scene()->update();
                update();
                break;
            case PianorollTool::ERASE:
                eraseNote(m_lastMousePos);
                update();
                break;
            default:
                break;
            }
        }
    }
}

void PianorollView::wheelEvent(QWheelEvent* event)
{
    double amount = -(double)event->angleDelta().y() / 400;
    if (event->modifiers() & Qt::ShiftModifier) {
        setCenterX(m_centerX + amount * width() / m_displayObjectWidth);
    } else {
        setCenterY(m_centerY + amount * height() / m_displayObjectHeight);
    }
}

Fraction PianorollView::roundToSubdivision(double wholeNote, bool down)
{
    Score* curScore = score();

    Fraction pos(wholeNote * 1000, 1000);
    Measure* m = curScore->tick2measure(pos);

    Fraction timeSig = m->timesig();
    int noteWithBeat = timeSig.denominator();

    //Number of smaller pieces the beat is divided into
    int subbeats = m_tuplet * (1 << m_subdivision);
    int divisions = noteWithBeat * subbeats;

    //Round down to nearest division
    Fraction roundedTick = Fraction(down ? floor(wholeNote * divisions) : ceil(wholeNote * divisions), divisions);
    return roundedTick;
}

void PianorollView::insertNote(int modifiers)
{
    (void)modifiers;
    double pickTick = pixelXToWholeNote(m_mouseDownPos.x());
    double pickPitch = pixelYToPitch(m_mouseDownPos.y());

    Score* curScore = score();
    Staff* staff = activeStaff();

    Fraction pos(pickTick * 1000, 1000);
    Measure* m = curScore->tick2measure(pos);

    Fraction timeSig = m->timesig();

    Fraction insertPosition = roundToSubdivision(pickTick);

    int voice = m_editNoteVoice;

    int track = (int)staff->idx() * VOICES + voice;
    Fraction noteLen = m_editNoteLength;

    Segment* seg = curScore->tick2segment(insertPosition);
    curScore->expandVoice(seg, track);

    ChordRest* e = curScore->findCR(insertPosition, track);
    if (e) {
        curScore->startCmd();

        addNote(insertPosition, noteLen, pickPitch, track);

        curScore->endCmd();
    }

    buildNoteData();
    update();
}

void PianorollView::cutChord(const QPointF& pos)
{
    Score* curScore = score();
    Staff* staff = activeStaff();

    double pickTick = pixelXToWholeNote(pos.x());
    double pickPitch = pixelYToPitch(pos.y());
    NoteBlock* pn = pickNote(pickTick, pickPitch);

    int voice = pn ? pn->note->voice() : m_editNoteVoice;

    //Find best chord to add to
    int track = (int)staff->idx() * VOICES + voice;

    Fraction insertPosition = roundToSubdivision(pickTick);

    Segment* seg = curScore->tick2segment(insertPosition);
    curScore->expandVoice(seg, track);

    ChordRest* e = curScore->findCR(insertPosition, track);
    if (e && !e->tuplet() && m_tuplet == 1) {
        curScore->startCmd();
        Fraction startTick = e->tick();

        if (insertPosition != startTick) {
            ChordRest* cr0;
            ChordRest* cr1;
            cutChordRest(e, track, insertPosition, cr0, cr1);
        }
        curScore->endCmd();
    }

    buildNoteData();
    update();
}

void PianorollView::eraseNote(const QPointF& pos)
{
    Score* curScore = score();

    NoteBlock* pn = pickNote(pos.x(), pos.y());

    if (pn) {
        curScore->startCmd();
        curScore->deleteItem(pn->note);
        curScore->endCmd();
    }

    buildNoteData();
}

Staff* PianorollView::activeStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    Score* score = notation->elements()->msScore();

    Staff* staff = score->staff(m_activeStaff);
    return staff;
}

bool PianorollView::intersects(NoteBlock* block, int pixX, int pixY)
{
    for (NoteEvent& e : block->note->playEvents()) {
        QRect bounds = boundingRect(block->note, &e, displayEventAdjustment());
        if (bounds.contains(pixX, pixY)) {
            return true;
        }
    }
    return false;
}

bool PianorollView::intersectsPixel(NoteBlock* block, int pixX, int pixY, int width, int height)
{
    QRect hit(pixX, pixY, width, height);

    for (NoteEvent& e : block->note->playEvents()) {
        QRect bounds = boundingRect(block->note, &e, displayEventAdjustment());
        if (width == 0 || height == 0) {
            if (bounds.contains(pixX, pixY)) {
                return true;
            }
        } else if (bounds.intersects(hit)) {
            return true;
        }
    }
    return false;
}

NoteBlock* PianorollView::pickNote(int pixX, int pixY)
{
    for (int i = 0; i < m_noteList.size(); ++i) {
        NoteBlock* block = m_noteList[i];
        if (intersects(block, pixX, pixY)) {
            return block;
        }
    }

    return nullptr;
}

void PianorollView::selectNotes(double startTick, double endTick, double lowPitch, double highPitch, NoteSelectType selType)
{
    int startPixX = wholeNoteToPixelX(startTick);
    int endPixX = wholeNoteToPixelX(endTick);
    int lowPixY = pitchToPixelY(lowPitch);
    int highPixY = pitchToPixelY(highPitch);

    Score* curScore = score();
    //score->masterScore()->cmdState().reset();      // DEBUG: should not be necessary
    curScore->startCmd();

    std::vector<NoteBlock*> oldSel;
    for (int i = 0; i < m_noteList.size(); ++i) {
        NoteBlock* pi = m_noteList[i];
        if (pi->note->selected()) {
            oldSel.push_back(pi);
        }
    }

    Selection& selection = curScore->selection();
    selection.deselectAll();

    for (int i = 0; i < m_noteList.size(); ++i) {
        NoteBlock* pi = m_noteList.at(i);

        bool inBounds = intersectsPixel(pi, startPixX, lowPixY, endPixX - startPixX, highPixY - lowPixY);

        bool sel;
        bool isInList = std::find(oldSel.begin(), oldSel.end(), pi) != oldSel.end();
        switch (selType) {
        default:
        case NoteSelectType::REPLACE:
            sel = inBounds;
            break;
        case NoteSelectType::XOR:
            sel = inBounds != isInList;
            break;
        case NoteSelectType::ADD:
            sel = inBounds || isInList;
            break;
        case NoteSelectType::SUBTRACT:
            sel = !inBounds && isInList;
            break;
        case NoteSelectType::FIRST:
            sel = inBounds && selection.elements().empty();
            break;
        }

        if (sel) {
            selection.add(pi->note);
        }
    }

    //for (Ms::MuseScoreView* view : curScore->getViewer()) {
    //    view->updateAll();
    //}

    //curScore->setUpdateAll();
    //curScore->update();
    curScore->endCmd();

    update();
}

void PianorollView::handleSelectionClick()
{
    int modifiers = QGuiApplication::keyboardModifiers();
    bool bnShift = modifiers & Qt::ShiftModifier;
    bool bnCtrl = modifiers & Qt::ControlModifier;
    NoteSelectType selType = bnShift ? (bnCtrl ? NoteSelectType::SUBTRACT : NoteSelectType::XOR)
                             : (bnCtrl ? NoteSelectType::ADD : NoteSelectType::REPLACE);

//    Score* curScore = score();

    double pickTick = pixelXToWholeNote((int)m_mouseDownPos.x());
    double pickPitch = pixelYToPitch(m_mouseDownPos.y());

    NoteBlock* pi = pickNote(m_mouseDownPos.x(), m_mouseDownPos.y());
    if (pi) {
        m_editNoteLength = pi->note->chord()->ticks();
    }

    if (selType == NoteSelectType::REPLACE) {
        selType = NoteSelectType::FIRST;
    }

//        mscore->play(pn->note());
//        curScore->setPlayNote(false);

    selectNotes(pickTick, pickTick, pickPitch, pickPitch, selType);
}

QString PianorollView::serializeSelectedNotes()
{
    Fraction firstTick;
    bool init = false;
    for (int i = 0; i < m_noteList.size(); ++i) {
        if (m_noteList[i]->note->selected()) {
            Note* note = m_noteList.at(i)->note;
            Fraction startTick = note->chord()->tick();

            if (!init || firstTick > startTick) {
                firstTick = startTick;
                init = true;
            }
        }
    }

    //No valid notes
    if (!init) {
        return QByteArray();
    }

    QString xmlStrn;
    QXmlStreamWriter xml(&xmlStrn);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement("notes");
    xml.writeAttribute("firstN", QString::number(firstTick.numerator()));
    xml.writeAttribute("firstD", QString::number(firstTick.denominator()));

    //bundle notes into XML file & send to clipboard.
    //This is only affects pianoview and is not part of the regular copy/paste process
    for (int i = 0; i < m_noteList.size(); ++i) {
        if (m_noteList[i]->note->selected()) {
            Note* note = m_noteList.at(i)->note;

            Fraction flen = note->playTicksFraction();

            Fraction startTick = note->chord()->tick();
            int pitch = note->pitch();

            int voice = (int)note->voice();

            int veloOff = note->veloOffset();
            VeloType veloType = note->veloType();

            xml.writeStartElement("note");
            xml.writeAttribute("startN", QString::number(startTick.numerator()));
            xml.writeAttribute("startD", QString::number(startTick.denominator()));
            xml.writeAttribute("lenN", QString::number(flen.numerator()));
            xml.writeAttribute("lenD", QString::number(flen.denominator()));
            xml.writeAttribute("pitch", QString::number(pitch));
            xml.writeAttribute("voice", QString::number(voice));
            xml.writeAttribute("veloOff", QString::number(veloOff));
            xml.writeAttribute("veloType", veloType == VeloType::OFFSET_VAL ? "o" : "u");

            for (NoteEvent& evt : note->playEvents()) {
                int ontime = evt.ontime();
                int len = evt.len();

                xml.writeStartElement("evt");
                xml.writeAttribute("ontime", QString::number(ontime));
                xml.writeAttribute("len", QString::number(len));
                xml.writeEndElement();
            }

            xml.writeEndElement();
        }
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return xmlStrn;
}

void PianorollView::finishNoteGroupDrag()
{
    Score* curScore = score();

    Fraction pos(pixelXToWholeNote(m_lastMousePos.x()) * 1000, 1000);
    Measure* m = curScore->tick2measure(pos);

    Fraction timeSig = m->timesig();
    int noteWithBeat = timeSig.denominator();

    //Number of smaller pieces the beat is divided into
    int subbeats = m_tuplet * (1 << m_subdivision);
    int divisions = noteWithBeat * subbeats;

    //Round down to nearest division
    double dragToTick = pixelXToWholeNote(m_lastMousePos.x());
    double startTick = pixelXToWholeNote(m_mouseDownPos.x());
    double offsetTicks = dragToTick - startTick;

    //Adjust offset so that note under cursor is aligned to note divistion
    Fraction pasteTickOffset(0, 1);
    Fraction pasteLengthOffset(0, 1);
    int pitchOffset = 0;

    int dragToPitch = pixelYToPitch(m_lastMousePos.y());
    int startPitch = pixelYToPitch(m_mouseDownPos.y());

    if (m_dragStyle == DragStyle::NOTE_POSITION) {
        double noteStartDraggedTick = m_dragStartTick.numerator() / (double)m_dragStartTick.denominator() + offsetTicks;
        Fraction noteStartDraggedAlignedTick = Fraction(floor(noteStartDraggedTick * divisions), divisions);
        pasteTickOffset = noteStartDraggedAlignedTick - m_dragStartTick;
        pitchOffset = dragToPitch - startPitch;
    } else if (m_dragStyle == DragStyle::NOTE_LENGTH_END) {
        double noteEndDraggedTick = m_dragEndTick.numerator() / (double)m_dragEndTick.denominator() + offsetTicks;
        Fraction noteEndDraggedAlignedTick = Fraction(floor(noteEndDraggedTick * divisions), divisions);
        pasteLengthOffset = noteEndDraggedAlignedTick - m_dragEndTick;
    } else if (m_dragStyle == DragStyle::NOTE_LENGTH_START) {
        double noteStartDraggedTick = m_dragStartTick.numerator() / (double)m_dragStartTick.denominator() + offsetTicks;
        Fraction noteStartDraggedAlignedTick = Fraction(floor(noteStartDraggedTick * divisions), divisions);
        pasteTickOffset = noteStartDraggedAlignedTick - m_dragStartTick;
        pasteLengthOffset = m_dragStartTick - noteStartDraggedAlignedTick;
    }

    //Do command
    curScore->startCmd();

    curScore->cmdDeleteSelection();
    pasteNotes(m_dragNoteCache, pasteTickOffset, pasteLengthOffset, pitchOffset, true);

    curScore->endCmd();

    m_dragNoteCache = QByteArray();

    buildNoteData();
    update();
}

void PianorollView::finishNoteEventAdjustDrag()
{
    Score* curScore = score();
    Fraction dx = Fraction(m_lastMousePos.x() - m_mouseDownPos.x(), m_wholeNoteWidth).reduced();

    for (int i = 0; i < m_noteList.size(); ++i) {
        NoteBlock* pi = m_noteList[i];
        if (pi->note->selected()) {
            for (NoteEvent& e : pi->note->playEvents()) {
                Chord* chord = pi->note->chord();
                Fraction ticks = chord->ticks();
                Tuplet* tup = chord->tuplet();
                if (tup) {
                    Fraction frac = tup->ratio();
                    ticks = ticks * frac.inverse();
                }

                Fraction start = pi->note->chord()->tick();
                Fraction len = ticks;
                Fraction startAdj = start + ticks * e.ontime() / 1000;
                Fraction lenAdj = ticks * e.len() / 1000;

                //Calc start, duration of where we dragged to
                Fraction startNew;
                Fraction lenNew;
                switch (m_dragStyle) {
                case DragStyle::EVENT_ONTIME:
                    startNew = startAdj + dx;
                    lenNew = lenAdj - dx;
                    break;
                case DragStyle::EVENT_MOVE:
                    startNew = startAdj + dx;
                    lenNew = lenAdj;
                    break;
                default:
                case DragStyle::EVENT_LENGTH:
                    startNew = startAdj;
                    lenNew = lenAdj + dx;
                    break;
                }

                int evtOntimeNew = int(((startNew - start).toDouble() / ticks.toDouble()) * 1000);
                int evtLenNew = int((lenNew.toDouble() / ticks.toDouble()) * 1000);
                if (evtLenNew < 1) {
                    evtLenNew = 1;
                }

                NoteEvent ne = e;
                ne.setOntime(evtOntimeNew);
                ne.setLen(evtLenNew);

                curScore->startCmd();
                curScore->undo(new ChangeNoteEvent(pi->note, &e, ne));
                curScore->endCmd();
            }
        }
    }

    update();
}

void PianorollView::pasteNotes(const QString& copiedNotes, Fraction pasteStartTick, Fraction lengthOffset, int pitchOffset,
                               bool xIsOffset)
{
    QXmlStreamReader xml(copiedNotes);
    Fraction firstTick;
    std::vector<Note*> addedNotes;

    Staff* staff = activeStaff();

    while (!xml.atEnd())
    {
        QXmlStreamReader::TokenType tt = xml.readNext();
        if (tt == QXmlStreamReader::StartElement) {
            if (xml.name().toString() == "notes") {
                int num = xml.attributes().value("firstN").toString().toInt();
                int den = xml.attributes().value("firstD").toString().toInt();
                firstTick = Fraction(num, den);
            }
            if (xml.name().toString() == "note") {
                int sn = xml.attributes().value("startN").toString().toInt();
                int sd = xml.attributes().value("startD").toString().toInt();
                Fraction startTick = Fraction(sn, sd);

                int tn = xml.attributes().value("lenN").toString().toInt();
                int td = xml.attributes().value("lenD").toString().toInt();
                Fraction tickLen = Fraction(tn, td);
                tickLen += lengthOffset;
                if (tickLen.numerator() <= 0) {
                    continue;
                }

                int pitch = xml.attributes().value("pitch").toString().toInt();
                int voice = xml.attributes().value("voice").toString().toInt();

                int veloOff = xml.attributes().value("veloOff").toString().toInt();
                QString veloTypeStrn = xml.attributes().value("veloType").toString();
                VeloType veloType = veloTypeStrn == "o" ? VeloType::OFFSET_VAL : VeloType::USER_VAL;

                int track = (int)staff->idx() * VOICES + voice;

                Fraction pos = xIsOffset ? startTick + pasteStartTick : startTick - firstTick + pasteStartTick;

                addedNotes = addNote(pos, tickLen, pitch + pitchOffset, track);
                for (Note* note: qAsConst(addedNotes)) {
                    note->setVeloOffset(veloOff);
                    note->setVeloType(veloType);
                }
            }

            if (xml.name().toString() == "evt") {
                int ontime = xml.attributes().value("ontime").toString().toInt();
                int len = xml.attributes().value("len").toString().toInt();

                NoteEvent ne;
                ne.setOntime(ontime);
                ne.setLen(len);
                for (Note* note: qAsConst(addedNotes)) {
                    NoteEventList& evtList = note->playEvents();
                    if (!evtList.empty()) {
                        NoteEvent* evt = note->noteEvent((int)evtList.size() - 1);
                        staff->score()->undo(new ChangeNoteEvent(note, evt, ne));
                    }
                }
            }
        }
    }
}

std::vector<Note*> PianorollView::addNote(Fraction startTick, Fraction duration, int pitch, int track)
{
    NoteVal nv(pitch);

    Score* curScore = score();

    std::vector<Note*> addedNotes;

    ChordRest* curCr = curScore->findCR(startTick, track);
    if (curCr) {
        ChordRest* cr0 = nullptr;
        ChordRest* cr1 = nullptr;

        if (startTick > curCr->tick()) {
            cutChordRest(curCr, track, startTick, cr0, cr1);    //Cut at the start of existing chord rest
        } else {
            cr1 = curCr;    //We are inserting at start of chordrest
        }
        Fraction cr1End = cr1->tick() + cr1->ticks();
        if (cr1End > startTick + duration) {
            //Cut from middle of enveloping chord
            ChordRest* crMid = nullptr;
            ChordRest* crEnd = nullptr;

            cutChordRest(cr1, track, startTick + duration, crMid, crEnd);
            if (crMid->isChord()) {
                Chord* ch = toChord(crMid);
                addedNotes.push_back(curScore->addNote(ch, nv));
            } else {
                Segment* newSeg = curScore->setNoteRest(crMid->segment(), track, nv, duration);
                if (newSeg) {
                    std::vector<Note*> segNotes = getSegmentNotes(newSeg, track);
                    addedNotes.insert(addedNotes.end(), segNotes.begin(), segNotes.end());
                }
            }
        } else if (cr1End == startTick + duration) {
            if (cr1->isChord()) {
                Chord* ch = toChord(cr1);
                addedNotes.push_back(curScore->addNote(ch, nv));
            } else {
                Segment* newSeg = curScore->setNoteRest(cr1->segment(), track, nv, duration);
                if (newSeg) {
                    std::vector<Note*> segNotes = getSegmentNotes(newSeg, track);
                    addedNotes.insert(addedNotes.end(), segNotes.begin(), segNotes.end());
                }
            }
        } else {
            Segment* newSeg = curScore->setNoteRest(cr1->segment(), track, nv, duration);
            if (newSeg) {
                std::vector<Note*> segNotes = getSegmentNotes(newSeg, track);
                addedNotes.insert(addedNotes.end(), segNotes.begin(), segNotes.end());
            }
        }
    }

    return addedNotes;
}

std::vector<Note*> PianorollView::getSegmentNotes(Segment* seg, int track)
{
    std::vector<Note*> notes;

    ChordRest* cr = seg->cr(track);
    if (cr && cr->isChord()) {
        Chord* chord = toChord(cr);
        append(notes, chord->notes());
    }

    return notes;
}

//---------------------------------------------------------
//   cutChordRest
//   @cr0 Will be set to the first piece of the split chord, or targetCr if no split occurs
//   @cr1 Will be set to the second piece of the split chord, or nullptr if no split occurs
//   @return true if chord was cut
//---------------------------------------------------------

bool PianorollView::cutChordRest(ChordRest* targetCr, int track, Fraction cutTick, ChordRest*& cr0, ChordRest*& cr1)
{
    Fraction startTick = targetCr->segment()->tick();
    Fraction durationTuplet = targetCr->ticks();

    Fraction measureToTuplet(1, 1);
    Fraction tupletToMeasure(1, 1);
    if (targetCr->tuplet()) {
        Fraction ratio = targetCr->tuplet()->ratio();
        measureToTuplet = ratio;
        tupletToMeasure = ratio.inverse();
    }

    Fraction durationMeasure = durationTuplet * tupletToMeasure;

    if (cutTick <= startTick || cutTick >= startTick + durationMeasure) {
        cr0 = targetCr;
        cr1 = nullptr;
        return false;
    }

    //Deselect note being cut
    if (targetCr->isChord()) {
        Chord* ch = toChord(targetCr);
        for (Note* n: ch->notes()) {
            n->setSelected(false);
        }
    } else if (targetCr->isRest()) {
        Rest* r = toRest(targetCr);
        r->setSelected(false);
    }

    //Subdivide at the cut tick
    NoteVal nv(-1);

    Score* curScore = score();
    curScore->setNoteRest(targetCr->segment(), track, nv, (cutTick - targetCr->tick()) * measureToTuplet);
    ChordRest* nextCR = curScore->findCR(cutTick, track);

    Chord* ch0 = 0;

    if (nextCR->isChord()) {
        //Copy chord into initial segment
        Chord* ch1 = toChord(nextCR);

        for (Note* n: ch1->notes()) {
            NoteVal nx = n->noteVal();
            if (!ch0) {
                ChordRest* cr = curScore->findCR(startTick, track);
                curScore->setNoteRest(cr->segment(), track, nx, cr->ticks());
                ch0 = toChord(curScore->findCR(startTick, track));
            } else {
                curScore->addNote(ch0, nx);
            }
        }
        cr0 = ch0;
    } else {
        cr0 = curScore->findCR(startTick, track);
    }

    cr1 = nextCR;
    return true;
}
