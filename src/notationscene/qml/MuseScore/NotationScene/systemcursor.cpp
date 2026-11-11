/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "systemcursor.h"

#include "draw/painter.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/system.h"

#include "notation/inotation.h"
#include "notation/inotationelements.h" // IWYU pragma: keep

using namespace mu::engraving;
using namespace mu::notation;
using namespace muse;

static double systemBottomY(const Score* score, const System* system)
{
    double bottomY = 0.0;

    for (size_t i = 0; i < score->nstaves(); ++i) {
        const SysStaff* ss = system->staff(i);

        if (!ss->show() || !score->staff(i)->show()) {
            continue;
        }

        bottomY = ss->bbox().bottom();
    }

    return bottomY;
}

static RectF calculateRect(double x, const Score* score, const System* system, double bottomY)
{
    const double spatium = score->style().spatium();

    return RectF {
        x - spatium,
        system->staffCanvasYpage(0) - 3.0 * spatium,
        0.4 * spatium,
        bottomY + 6.0 * spatium
    };
}

static double interpolateXByTicks(int targetTick, int startTick, int endTick, double segmentStartX, double segmentEndX)
{
    const int durationTicks = endTick - startTick;

    if (durationTicks > 0) {
        return segmentStartX + (segmentEndX - segmentStartX) * double(targetTick - startTick) / double(durationTicks);
    } else {
        return segmentStartX;
    }
}

void SystemCursor::paint(draw::Painter* painter)
{
    if (!isVisible()) {
        return;
    }

    painter->fillRect(m_rect, color());
}

void SystemCursor::setNotation(INotationPtr notation)
{
    if (m_notation == notation) {
        return;
    }

    if (m_notation) {
        m_notation->elements()->msScore()->changesChannel().disconnect(this);
        m_notation->viewModeChanged().disconnect(this);
    }

    if (notation) {
        notation->elements()->msScore()->changesChannel().onReceive(this, [this](const ScoreChanges&) {
            m_cache.clear();
        });

        notation->viewModeChanged().onNotify(this, [this]() {
            m_cache.clear();
        });
    }

    m_notation = notation;
    m_cache.clear();

    // The old tick and rect belong to the old score. With no score there's nothing to point at, so drop them.
    // (With a new score, the owner moves the cursor to a tick in it.)
    if (!m_notation) {
        m_tick = 0;
        m_rect = RectF();
    }
}

void SystemCursor::move(midi::tick_t newTick)
{
    m_tick = newTick;
    m_rect = resolveCursorRectByTick(newTick);
}

bool SystemCursor::refresh()
{
    return refresh(m_tick);
}

bool SystemCursor::refresh(midi::tick_t newTick)
{
    m_cache.clear();

    const RectF oldRect = m_rect;
    move(newTick);

    return m_rect != oldRect;
}

RectF SystemCursor::resolveCursorRectByTick(int newTick) const
{
    const Score* score = m_notation ? m_notation->elements()->msScore() : nullptr;

    if (!score) {
        return RectF();
    }

    const Fraction fracTick = Fraction::fromTicks(newTick);

    if (m_cache.segment && m_cache.segmentStartTick <= fracTick && fracTick < m_cache.segmentEndTick) {
        const double x = interpolateXByTicks(
            newTick,
            m_cache.segmentStartTick.ticks(),
            m_cache.segmentEndTick.ticks(),
            m_cache.segmentStartX,
            m_cache.segmentEndX
            );

        return calculateRect(x, score, m_cache.system, m_cache.systemBottomY);
    }

    const Measure* measure = nullptr;
    const System* system = nullptr;

    if (m_cache.measure && m_cache.measure->tick() <= fracTick && fracTick < m_cache.measure->endTick()) {
        measure = m_cache.measure;
        system = m_cache.system;
    } else {
        measure = score->tick2measureMM(fracTick);

        if (!measure) {
            m_cache.clear();
            return RectF();
        }

        system = measure->system();

        if (!system || !system->page() || system->staves().empty()) {
            m_cache.clear();
            return RectF();
        }
    }

    // Segments normally own [startTick, endTick), since a measure's end tick is the next measure's start tick.
    // The score's final tick has no next measure, so there the last segment owns its end tick too (playback
    // stops at that tick when it reaches the end of the score).
    const bool isScoreEnd = (fracTick == measure->endTick());

    for (const Segment* currentSegment = measure->first(SegmentType::ChordRest); currentSegment;) {
        const Fraction segmentStartTick = currentSegment->tick();
        const double segmentStartX = currentSegment->canvasPos().x();

        const Segment* nextSegment = currentSegment->next(SegmentType::ChordRest);

        while (nextSegment && !nextSegment->visible()) {
            nextSegment = nextSegment->next(SegmentType::ChordRest);
        }

        Fraction segmentEndTick;
        double segmentEndX = 0.0;

        if (nextSegment) {
            segmentEndTick = nextSegment->tick();
            segmentEndX = nextSegment->canvasPos().x();
        } else {
            segmentEndTick = measure->endTick();

            const Segment* endBar = measure->findSegment(SegmentType::EndBarLine, segmentEndTick);

            segmentEndX = endBar ? endBar->canvasPos().x() : measure->canvasPos().x() + measure->width();
        }

        const bool isPastSegment = isScoreEnd
                                   ? (nextSegment != nullptr)
                                   : (segmentEndTick <= fracTick);

        if (fracTick < segmentStartTick || isPastSegment) {
            currentSegment = nextSegment;
            continue;
        }

        if (m_cache.system != system) {
            m_cache.system = system;
            m_cache.systemBottomY = systemBottomY(score, system);
        }

        m_cache.measure = measure;
        m_cache.segment = currentSegment;
        m_cache.segmentStartTick = segmentStartTick;
        m_cache.segmentEndTick = segmentEndTick;
        m_cache.segmentStartX = segmentStartX;
        m_cache.segmentEndX = segmentEndX;

        const double x = interpolateXByTicks(
            newTick,
            segmentStartTick.ticks(),
            segmentEndTick.ticks(),
            segmentStartX,
            segmentEndX
            );

        return calculateRect(x, score, system, m_cache.systemBottomY);
    }

    m_cache.clear();
    return RectF();
}

bool SystemCursor::isVisible() const
{
    return isRenderable()
           && color().alpha() > 0;
}
