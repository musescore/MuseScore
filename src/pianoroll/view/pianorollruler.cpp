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

#include "pianorollruler.h"

#include <QPainter>

#include "audio/iplayer.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

QPixmap* PianorollRuler::markIcon[3];

static const char* rmark_xpm[]={
    "18 18 2 1",
    "# c #0000ff",
    ". c None",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "........##########",
    "........#########.",
    "........########..",
    "........#######...",
    "........######....",
    "........#####.....",
    "........####......",
    "........###.......",
    "........##........",
    "........##........",
    "........##........" };

static const char* lmark_xpm[]={
    "18 18 2 1",
    "# c #0000ff",
    ". c None",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "##########........",
    ".#########........",
    "..########........",
    "...#######........",
    "....######........",
    ".....#####........",
    "......####........",
    ".......###........",
    "........##........",
    "........##........",
    "........##........" };

static const char* cmark_xpm[]={
    "18 18 2 1",
    "# c #ff0000",
    ". c None",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "..................",
    "##################",
    ".################.",
    "..##############..",
    "...############...",
    "....##########....",
    ".....########.....",
    "......######......",
    ".......####.......",
    "........##........",
    "........##........",
    "........##........" };

PianorollRuler::PianorollRuler(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    m_font2.setPixelSize(14);
    m_font1.setPixelSize(10);

    if (markIcon[0] == 0) {
        markIcon[0] = new QPixmap(cmark_xpm);
        markIcon[1] = new QPixmap(lmark_xpm);
        markIcon[2] = new QPixmap(rmark_xpm);
    }
}

void PianorollRuler::load()
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

void PianorollRuler::onNotationChanged()
{
    auto notation = globalContext()->currentNotation();
    if (notation) {
        notation->notationChanged().onNotify(this, [this]() {
            onCurrentNotationChanged();
        });
    }

    updateBoundingSize();
}

void PianorollRuler::onCurrentNotationChanged()
{
    updateBoundingSize();
}

Score* PianorollRuler::score()
{
    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return nullptr;
    }

    //Find staff to draw from
    Score* score = notation->elements()->msScore();
    return score;
}

void PianorollRuler::setPlaybackPosition(Fraction value)
{
    if (value == m_playbackPosition) {
        return;
    }
    m_playbackPosition = value;
    update();

    emit playbackPositionChanged();
}

void PianorollRuler::setWholeNoteWidth(double value)
{
    if (value == m_wholeNoteWidth) {
        return;
    }
    m_wholeNoteWidth = value;
    updateBoundingSize();

    emit wholeNoteWidthChanged();
}

void PianorollRuler::setCenterX(double value)
{
    if (value == m_centerX) {
        return;
    }
    m_centerX = value;
    update();

    emit centerXChanged();
}

void PianorollRuler::setDisplayObjectWidth(double value)
{
    if (value == m_displayObjectWidth) {
        return;
    }
    m_displayObjectWidth = value;
    updateBoundingSize();

    emit displayObjectWidthChanged();
}

void PianorollRuler::updateBoundingSize()
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

int PianorollRuler::wholeNoteToPixelX(double tick) const
{
    return tick * m_wholeNoteWidth - m_centerX * m_displayObjectWidth + width() / 2;
}

double PianorollRuler::pixelXToWholeNote(int pixX) const
{
    return (pixX + m_centerX * m_displayObjectWidth - width() / 2) / m_wholeNoteWidth;
}

void PianorollRuler::mousePressEvent(QMouseEvent* event)
{
    double wholeNote = pixelXToWholeNote(event->pos().x());
    Fraction frac(wholeNote * 1000, 1000);
    int ticks = frac.ticks();

    Score* curScore = score();
    qreal time = curScore->utick2utime(ticks);

    playback()->player()->seek(0, time);

//    update();
}

void PianorollRuler::mouseMoveEvent(QMouseEvent* event)
{
}

void PianorollRuler::paint(QPainter* p)
{
    p->fillRect(0, 0, width(), height(), m_colorBackground);

    notation::INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    Score* score = notation->elements()->msScore();
    Staff* staff = score->staff(0);

    const QPen penLineMajor = QPen(m_colorGridLine, 2.0, Qt::SolidLine);
    const QPen penLineMinor = QPen(m_colorGridLine, 1.0, Qt::SolidLine);

    //Draw lines
    const int minGap = 50;
    int lastDrawPos = -1;

    for (MeasureBase* m = score->first(); m; m = m->next()) {
        int posX = floor(wholeNoteToPixelX(m->tick().toDouble()));

        if (m->no() != 0 && posX - lastDrawPos < minGap) {
            continue;
        }

        lastDrawPos = posX;

        p->setPen(penLineMajor);
        p->drawLine(posX, 0, posX, height());

        p->setFont(m_font2);
        p->setPen(m_colorText);
        QString text;
        text.setNum(m->no() + 1);
        int pixelSize = m_font2.pixelSize();
        p->drawText(posX + 4, pixelSize + 2, text);
    }

    //Draw playhead
    {
        p->setPen(m_colorPlaybackLine);
        int xp = wholeNoteToPixelX(m_playbackPosition);
        QPixmap* pm = markIcon[0];
        p->drawPixmap(xp - pm->width() / 2, height() - pm->height(), *pm);
    }
}
