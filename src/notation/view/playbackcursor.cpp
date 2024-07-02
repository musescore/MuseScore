/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/engravingobject.h"
#include "dom/tie.h"
#include "engraving/dom/system.h"
#include <algorithm>

using namespace mu::notation;
using namespace muse;

void PlaybackCursor::paint(muse::draw::Painter* painter)
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

static bool noteContainsTick(const Note* note, muse::midi::tick_t tick)
{
    double fraction = ((double)tick - (double)note->tick().ticks()) * 1000.0 / note->playTicks();
    for (const mu::engraving::NoteEvent& ev : note->playEvents()) {
        if (ev.ontime() <= fraction && fraction < ev.offtime()) {
            return true;
        }
    }
    return false;
}

RemarkingResult PlaybackCursor::move(muse::midi::tick_t tick)
{
    auto r = resolveCursorRectByTick(tick);
    m_rect = r.first;
    auto rr = RectF();
    m_markedNotes.erase(std::remove_if(m_markedNotes.begin(), m_markedNotes.end(), [tick, &rr](Note* note) {
        if (!noteContainsTick(note, tick)) {
            note->setMark(false);
            rr.unite(note->canvasBoundingRect());
            return true;
        }
        return false;
    }), m_markedNotes.end());
    const mu::engraving::Segment* s = r.second;
    if (m_visible && s) {
        std::vector<mu::engraving::EngravingItem*> selection;
        for (EngravingItem* e : s->elist()) {
            if (e && e->isChord()) {
                for (Note* note : engraving::toChord(e)->notes()) {
                    note = note->firstTiedNote();
                    if (std::any_of(m_markedNotes.begin(), m_markedNotes.end(), [note](Note* n) { return n == note; })) {
                        continue;
                    }
                    if (noteContainsTick(note, tick)) {
                        while (true) {
                            note->setMark(true);
                            m_markedNotes.push_back(note);
                            rr.unite(note->canvasBoundingRect());
                            engraving::Tie* tie = note->tieFor();
                            if (!tie) {
                                break;
                            }
                            note = tie->endNote();
                        }
                    }
                }
            }
        }
    }
    return RemarkingResult{ rr };
}

//! NOTE Copied from ScoreView::moveCursor(const Fraction& tick)
std::pair<muse::RectF, const mu::engraving::Segment*> PlaybackCursor::resolveCursorRectByTick(muse::midi::tick_t _tick) const
{
    if (!m_notation) {
        return std::pair(RectF(), nullptr);
    }

    const mu::engraving::Score* score = m_notation->elements()->msScore();

    Fraction tick = Fraction::fromTicks(_tick);

    Measure* measure = score->tick2measureMM(tick);
    if (!measure) {
        return std::pair(RectF(), nullptr);
    }

    mu::engraving::System* system = measure->system();
    if (!system) {
        return std::pair(RectF(), nullptr);
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
        return std::pair(RectF(), nullptr);
    }

    double y = system->staffYpage(0) + system->page()->pos().y();
    double _spatium = score->style().spatium();

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

    return std::pair(RectF(x, y, w, h), s);
}

bool PlaybackCursor::visible() const
{
    return m_visible;
}

void PlaybackCursor::setVisible(bool arg)
{
    m_visible = arg;
    if (!arg) {
        if (unmarkNotes()) {
            m_notation->notationChanged();
        }
    }
}

const muse::RectF& PlaybackCursor::rect() const
{
    return m_rect;
}

QColor PlaybackCursor::color() const
{
    return configuration()->playbackCursorColor();
}

bool PlaybackCursor::unmarkNotes()
{
    for (Note* note : m_markedNotes) {
        note->setMark(false);
    }
    bool unmarked = !m_markedNotes.empty();
    m_markedNotes.clear();
    return unmarked;
}
