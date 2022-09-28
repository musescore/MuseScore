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

#include "pianorollkeyboard.h"

#include <QPainter>
#include <QGuiApplication>

#include "libmscore/drumset.h"

#include "pianorollview.h"

using namespace mu::pianoroll;
using namespace mu::engraving;
using namespace mu::midi;

struct KeyShape
{
    bool white;
    double marginBottom;
    double marginTop;
};

static const KeyShape keyShapes[] {
    { true, 0, 1.5 },
    { false, 0, 1 },
    { true, -.5, 1.5 },
    { false, 0, 1 },
    { true, -.5, 1 },

    { true, 0, 1.5 },
    { false, 0, 1 },
    { true, -.5, 1.5 },
    { false, 0, 1 },
    { true, -.5, 1.5 },
    { false, 0, 1 },
    { true, -.5, 1 },
};

static const QString pitchNames[]{
    "C",
    "C♯/D♭",
    "D",
    "D♯/E♭",
    "E",
    "F",
    "F♯/G♭",
    "G",
    "G♯/A♭",
    "A",
    "A♯/B♭",
    "B",
};

PianorollKeyboard::PianorollKeyboard(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void PianorollKeyboard::load()
{
    onNotationChanged();
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

void PianorollKeyboard::onNotationChanged()
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

    updateBoundingSize();
}

void PianorollKeyboard::onSelectionChanged()
{
    update();
}

void PianorollKeyboard::onCurrentNotationChanged()
{
    updateBoundingSize();
}

void PianorollKeyboard::setNoteHeight(double value)
{
    if (value == m_noteHeight) {
        return;
    }
    m_noteHeight = value;
    updateBoundingSize();

    emit noteHeightChanged();
}

void PianorollKeyboard::setCenterY(double value)
{
    if (value == m_centerY) {
        return;
    }
    m_centerY = value;
    update();

    emit centerYChanged();
}

void PianorollKeyboard::setDisplayObjectHeight(double value)
{
    if (value == m_displayObjectHeight) {
        return;
    }
    m_displayObjectHeight = value;
    updateBoundingSize();

    emit displayObjectHeightChanged();
}

void PianorollKeyboard::updateBoundingSize()
{
    setDisplayObjectHeight(m_noteHeight * PianorollView::NUM_PITCHES);

    update();
}

int PianorollKeyboard::pitchToPixelY(double pitch) const
{
    return (128 - pitch) * m_noteHeight - m_centerY * m_displayObjectHeight + height() / 2;
}

double PianorollKeyboard::pixelYToPitch(int pixY) const
{
    return 128 - ((pixY + m_centerY * m_displayObjectHeight - height() / 2) / m_noteHeight);
}

void PianorollKeyboard::setTooltipText(QString value)
{
    if (m_tooltipText == value) {
        return;
    }
    
    m_tooltipText = value;
    emit m_tooltipText;
}

Staff* PianorollKeyboard::getActiveStaff()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Score* score = notation->elements()->msScore();
    std::vector<EngravingItem*> selectedElements = notation->interaction()->selection()->elements();
    std::vector<int> selectedStaves;
    int activeStaff = -1;
    for (EngravingItem* e : selectedElements) {
        int idx = e->staffIdx();
        qDebug() << "ele idx " << idx;
        activeStaff = idx;
        if (std::find(selectedStaves.begin(), selectedStaves.end(), idx) == selectedStaves.end()) {
            selectedStaves.push_back(idx);
        }
    }

    if (activeStaff == -1) {
        activeStaff = 0;
    }
    Staff* staff = score->staff(activeStaff);
    return staff;
}

void PianorollKeyboard::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    Staff* staff = getActiveStaff();
    Part* part = staff->part();

    //Check for drumset, if any
    Drumset* ds = nullptr;
    Interval transp;
    if (staff) {
        ds = part->instrument()->drumset();
        transp = part->instrument()->transpose();
    }

    const int fontSize = 8;
    QFont font("FreeSans", fontSize);
    p->setFont(font);

    if (ds) {
        Interval transp = part->instrument()->transpose();

        //MIDI notes span [0, 127] and map to pitches with MIDI pitch 0 being the note C-1

        //Colored stripes
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch) {
            double y = pitchToPixelY(pitch + 1);

            int degree = (pitch - transp.chromatic + 60) % 12;
            const BarPattern& pat = PianorollView::barPatterns[0];

            if (!pat.isWhiteKey[degree]) {
                p->fillRect(0, y, width(), m_noteHeight, pitch == m_curKeyPressed ? m_colorDrumHighlight : m_colorDrumBlack);
            } else {
                p->fillRect(0, y, width(), m_noteHeight, pitch == m_curKeyPressed ? m_colorDrumHighlight : m_colorDrumWhite);
            }

            p->setPen(m_colorText);

            QString noteName = ds->name(pitch).toQString();

            QRectF rectText(1, y, width() - 2, m_noteHeight);
            p->drawText(rectText, Qt::AlignBottom | Qt::AlignLeft, noteName);
        }

        //Bounding lines
        p->setPen(m_colorGridLines);
        for (int pitch = 0; pitch <= PianorollView::NUM_PITCHES; ++pitch) {
            double y = pitchToPixelY(pitch);
            p->drawLine(QLineF(0, y, width(), y));
        }
    } else {
        p->setPen(m_colorGridLines);

        //White keys
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch) {
            int degree = (pitch - transp.chromatic + 60) % 12;
            const KeyShape& shape = keyShapes[degree];
            if (shape.white) {
                double y0 = pitchToPixelY(pitch + shape.marginTop);
                double y1 = pitchToPixelY(pitch + shape.marginBottom);

                QRect bounds(0, y0, width(), y1 - y0);
                p->fillRect(bounds, pitch == m_curKeyPressed ? m_colorKeyHighlight : m_colorKeyWhite);
                p->drawRect(bounds);

                if (degree == 0 && font.pointSize() + 2 < bounds.height()) {
                    int octave = (pitch - transp.chromatic) / 12 - 1;
                    QString text = "C" + QString::number(octave);
                    p->drawText(bounds, Qt::AlignBottom | Qt::AlignRight, text);
                }
            }
        }

        //Black keys
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch) {
            int degree = (pitch - transp.chromatic + 60) % 12;
            const KeyShape& shape = keyShapes[degree];
            if (!shape.white) {
                double y0 = pitchToPixelY(pitch + shape.marginTop);
                double y1 = pitchToPixelY(pitch + shape.marginBottom);

                p->fillRect(0, y0, width() * .6, y1 - y0, pitch == m_curKeyPressed ? m_colorKeyHighlight : m_colorKeyBlack);
                p->drawRect(0, y0, width() * .6, y1 - y0);
            }
        }
    }
}

void PianorollKeyboard::sendNoteOn(uint8_t key)
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Event ev;
    ev.setMessageType(Event::MessageType::ChannelVoice10);
    ev.setOpcode(Event::Opcode::NoteOn);
    ev.setNote(key);
    ev.setVelocity(80);

    startNoteInputIfNeed();

    notation->midiInput()->onMidiEventReceived(ev);
}

void PianorollKeyboard::sendNoteOff(uint8_t key)
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Event ev;
    ev.setMessageType(Event::MessageType::ChannelVoice10);
    ev.setOpcode(Event::Opcode::NoteOff);
    ev.setNote(key);

    notation->midiInput()->onMidiEventReceived(ev);
}

void PianorollKeyboard::startNoteInputIfNeed()
{
    auto notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    auto noteInput = notation->interaction()->noteInput();
    if (!noteInput) {
        return;
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    }
}

void PianorollKeyboard::mousePressEvent(QMouseEvent* event)
{
    int pitch = pixelYToPitch(event->pos().y());

    if (pitch < 0 || pitch > 127) {
        pitch = -1;
    }

    m_curKeyPressed = pitch;

    uint modifiers = QGuiApplication::keyboardModifiers();
    bool bnCtrl = modifiers & Qt::ControlModifier;
    if (bnCtrl) {
        //        emit pitchHighlightToggled(pitch);
        bool highlight = controller()->isPitchHighlight(pitch);
        controller()->setPitchHighlight(pitch, !highlight);
    } else {
//        emit keyPressed(pitch);
        sendNoteOn(pitch);
    }

    update();
}

void PianorollKeyboard::mouseReleaseEvent(QMouseEvent*)
{
    if (m_curKeyPressed != -1) {
//        emit keyReleased(m_curKeyPressed);
        sendNoteOff(m_curKeyPressed);
        m_curKeyPressed = -1;
        update();
    }
}

void PianorollKeyboard::mouseMoveEvent(QMouseEvent* event)
{
    int pitch = pixelYToPitch(event->pos().y());

    if (pitch < 0 || pitch > 127) {
        pitch = -1;
    }

    if (pitch != m_curPitch) {
        m_curPitch = pitch;

        //Set tooltip
        int degree = m_curPitch % 12;
        int octave = m_curPitch / 12;

        Staff* staff = getActiveStaff();
        if (staff) {
            Part* part = staff->part();
            Drumset* ds = part->instrument()->drumset();
            if (ds) {
                setTooltipText(ds->name(m_curPitch));
            } else {
                setTooltipText(pitchNames[degree] + QString::number(octave - 1));
            }
        }

        //Send event
//        emit pitchChanged(m_curPitch);

        if ((m_curKeyPressed != -1) && (m_curKeyPressed != pitch)) {
            sendNoteOff(m_curKeyPressed);
//            emit keyReleased(m_curKeyPressed);
            m_curKeyPressed = pitch;
            sendNoteOn(m_curKeyPressed);
//            emit keyPressed(m_curKeyPressed);
        }

        update();
    }
}
