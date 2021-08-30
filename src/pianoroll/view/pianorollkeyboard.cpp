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

#include "libmscore/drumset.h"

#include "pianorollview.h"

using namespace mu::pianoroll;

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


PianorollKeyboard::PianorollKeyboard(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
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
    if (notation)
    {
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
    if (value == m_noteHeight)
        return;
    m_noteHeight = value;
    updateBoundingSize();

    emit noteHeightChanged();
}

void PianorollKeyboard::setCenterY(double value)
{
    if (value == m_centerY)
        return;
    m_centerY = value;
    update();

    emit centerYChanged();
}

void PianorollKeyboard::setDisplayObjectHeight(double value)
{
    if (value == m_displayObjectHeight)
        return;
    m_displayObjectHeight = value;
    updateBoundingSize();

    emit displayObjectHeightChanged();
}

void PianorollKeyboard::updateBoundingSize()
{
//    notation::INotationPtr notation = globalContext()->currentNotation();
//    if (!notation) {
//        return;
//    }

//    Ms::Score* score = notation->elements()->msScore();

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


void PianorollKeyboard::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

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

    //Check for drumset, if any
    Ms::Drumset* ds = nullptr;
    Ms::Interval transp;
    if (staff) {
        ds = part->instrument()->drumset();
        transp = part->instrument()->transpose();
    }

    const int fontSize = 8;
    QFont font("FreeSans", fontSize);
    p->setFont(font);

    if (ds)
    {
        Ms::Interval transp = part->instrument()->transpose();

        //MIDI notes span [0, 127] and map to pitches with MIDI pitch 0 being the note C-1

        //Colored stripes
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch)
        {
            double y = pitchToPixelY(pitch + 1);

            int degree = (pitch - transp.chromatic + 60) % 12;
            const BarPattern& pat = PianorollView::barPatterns[0];

            if (!pat.isWhiteKey[degree])
            {
                p->fillRect(0, y, width(), m_noteHeight, m_colorDrumBlack);
            }
            else
            {
                p->fillRect(0, y, width(), m_noteHeight, m_colorDrumWhite);
            }

            p->setPen(m_colorText);

            QString noteName = ds->name(pitch).toUtf8().constData();

            QRectF rectText(1, y, width() - 2, m_noteHeight);
            p->drawText(rectText, Qt::AlignBottom | Qt::AlignLeft, noteName);
        }

        //Bounding lines
        p->setPen(m_colorGridLines);
        for (int pitch = 0; pitch <= PianorollView::NUM_PITCHES; ++pitch)
        {
            double y = pitchToPixelY(pitch);
            p->drawLine(QLineF(0, y, width(), y));
        }

    }
    else
    {
        p->setPen(m_colorGridLines);

        //White keys
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch)
        {
            int degree = (pitch - transp.chromatic + 60) % 12;
            const KeyShape& shape = keyShapes[degree];
            if (shape.white)
            {
                double y0 = pitchToPixelY(pitch + shape.marginTop);
                double y1 = pitchToPixelY(pitch + shape.marginBottom);

                QRect bounds(0, y0, width(), y1 - y0);
                p->fillRect(bounds, m_colorKeyWhite);
//                p->drawLine(0, y0, width(), y0);
                p->drawRect(bounds);

                if (degree == 0 && font.pointSize() + 2 < bounds.height())
                {
                    int octave = (pitch - transp.chromatic) / 12 - 1;
                    QString text = "C" + QString::number(octave);
                    p->drawText(bounds, Qt::AlignBottom | Qt::AlignRight, text);
                }
            }
        }

        //Black keys
        for (int pitch = 0; pitch < PianorollView::NUM_PITCHES; ++pitch)
        {
            int degree = (pitch - transp.chromatic + 60) % 12;
            const KeyShape& shape = keyShapes[degree];
            if (!shape.white)
            {
                double y0 = pitchToPixelY(pitch + shape.marginTop);
                double y1 = pitchToPixelY(pitch + shape.marginBottom);

                p->fillRect(0, y0, width() * .6, y1 - y0, m_colorKeyBlack);
                p->drawRect(0, y0, width() * .6, y1 - y0);
            }

        }

    }

}
