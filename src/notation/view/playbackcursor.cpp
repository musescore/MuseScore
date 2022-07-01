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
#include "playbackcursor.h"

#include "engraving/libmscore/system.h"
#include "engraving/libmscore/scorefont.h"
#include "engraving/libmscore/tie.h"

using namespace mu::notation;

PlaybackCursor::~PlaybackCursor()
{
    resetPlayedNotes();
}

void PlaybackCursor::paint(mu::draw::Painter* painter)
{
    if (!m_visible) {
        return;
    }

    painter->fillRect(m_rect, color());
}

void PlaybackCursor::setNotation(INotationPtr notation)
{
    m_notation = notation;
}

void PlaybackCursor::move(midi::tick_t tick)
{
    updateCursorByTick(tick);
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
void PlaybackCursor::updateCursorByTick(midi::tick_t _tick)
{
    if (!m_notation) {
        return;
    }

    const mu::engraving::Score* score = m_notation->elements()->msScore();

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return;
    }

    mu::engraving::System* system = measure->system();
    if (!system) {
        return;
    }

    qreal x = 0.0;
    mu::engraving::Segment* s = nullptr;
    for (s = measure->first(mu::engraving::SegmentType::ChordRest); s;) {
        Fraction t1 = s->tick();
        int x1 = s->canvasPos().x();
        qreal x2 = 0.0;
        Fraction t2;

        mu::engraving::Segment* ns = s->next(mu::engraving::SegmentType::ChordRest);
        while (ns && !ns->visible()) {
            ns = ns->next(mu::engraving::SegmentType::ChordRest);
        }

        if (ns) {
            t2 = ns->tick();
            x2 = ns->canvasPos().x();
        } else {
            t2 = measure->endTick();
            // measure->width is not good enough because of courtesy keysig, timesig
            mu::engraving::Segment* seg = measure->findSegment(mu::engraving::SegmentType::EndBarLine, measure->tick() + measure->ticks());
            if (seg) {
                x2 = seg->canvasPos().x();
            } else {
                x2 = measure->canvasPos().x() + measure->width(); // safety, should not happen
            }
        }

        if (tick >= t1 && tick < t2) {
            Fraction dt = t2 - t1;
            qreal dx = x2 - x1;
            x = x1 + dx * (tick - t1).ticks() / dt.ticks();
            break;
        }
        s = ns;
    }

    if (!s) {
        return;
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score->spatium();

    double w  = 8;
    double h  = 6 * _spatium;
    //
    // set cursor height for whole system
    //
    double y2 = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        mu::engraving::SysStaff* ss = system->staff(i);
        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }
        y2 = ss->bbox().bottom();
    }

    h += y2;
    x -= _spatium;
    y -= 3 * _spatium;

    m_rect = RectF(x, y, w, h);

    resetPlayedNotes();

    if (!visible()) {
        return;
    }

    for (EngravingItem* element : s->elist()) {
        if (!element) {
            continue;
        }

        if (!element->isChord()) {
            continue;
        }

        const Chord* chord = toChord(element);
        for (Note* note1 : chord->notes()) {
            while (note1) {
                for (EngravingObject* eO : note1->linkList()) {
                    if (!eO->isNote()) {
                        continue;
                    }

                    Note* currentNote = toNote(eO);
                    m_playingNotes.push_back(currentNote);
                }

                note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
            }
        }
    }

    colorPlayingNotes();
}

void PlaybackCursor::colorPlayingNotes()
{
    for (Note* note: m_playingNotes) {
        note->setMark(true);
    }
}

void PlaybackCursor::resetPlayedNotes()
{
    for (Note* note: m_playingNotes) {
        note->setMark(false);
    }

    m_playingNotes.clear();
}

bool PlaybackCursor::visible() const
{
    return m_visible;
}

void PlaybackCursor::setVisible(bool arg)
{
    m_visible = arg;

    if (!m_visible) {
        resetPlayedNotes();
    }
}

const mu::RectF& PlaybackCursor::rect() const
{
    return m_rect;
}

QColor PlaybackCursor::color() const
{
    return configuration()->playbackCursorColor();
}
