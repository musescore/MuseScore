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
#include "scorecallbacks.h"

#include "libmscore/engravingitem.h"
#include "libmscore/lyrics.h"
#include "libmscore/system.h"

#include "inotationinteraction.h"
#include "igetscore.h"

#include "log.h"

using namespace mu::notation;

void ScoreCallbacks::dataChanged(const RectF&)
{
    NOT_IMPLEMENTED;
}

void ScoreCallbacks::updateAll()
{
    NOT_IMPLEMENTED;
}

void ScoreCallbacks::drawBackground(draw::Painter*, const mu::RectF&) const
{
    NOT_IMPLEMENTED;
}

const mu::Rect ScoreCallbacks::geometry() const
{
    NOT_IMPLEMENTED;
    return mu::Rect();
}

qreal ScoreCallbacks::selectionProximity() const
{
    return m_selectionProximity;
}

void ScoreCallbacks::setSelectionProximity(qreal proximity)
{
    m_selectionProximity = proximity;
}

void ScoreCallbacks::setDropTarget(const Ms::EngravingItem* dropTarget)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->setDropTarget(dropTarget, false);
}

void ScoreCallbacks::changeEditElement(Ms::EngravingItem* newElement)
{
    IF_ASSERT_FAILED(m_interaction) {
        return;
    }

    m_interaction->changeEditElement(newElement);
}

//! NOTE: Copied from ScoreView::adjustCanvasPosition
void ScoreCallbacks::adjustCanvasPosition(const Ms::EngravingItem* el, bool playBack, int staffIndex)
{
    IF_ASSERT_FAILED(m_interaction && m_getScore) {
        return;
    }

    if (!el) {
        return;
    }

    if (!configuration()->isAutomaticallyPanEnabled()) {
        return;
    }

    const Ms::MeasureBase* m = nullptr;

    if (el->type() == ElementType::NOTE) {
        m = static_cast<const Note*>(el)->chord()->measure();
    } else if (el->type() == ElementType::REST) {
        m = static_cast<const Rest*>(el)->measure();
    } else if (el->type() == ElementType::CHORD) {
        m = static_cast<const Chord*>(el)->measure();
    } else if (el->type() == ElementType::SEGMENT) {
        m = static_cast<const Ms::Segment*>(el)->measure();
    } else if (el->type() == ElementType::LYRICS) {
        m = static_cast<const Ms::Lyrics*>(el)->measure();
    } else if ((el->type() == ElementType::HARMONY || el->type() == ElementType::FIGURED_BASS)
               && el->parent()->type() == ElementType::SEGMENT) {
        m = static_cast<const Ms::Segment*>(el->parent())->measure();
    } else if (el->type() == ElementType::HARMONY && el->parent()->type() == ElementType::FRET_DIAGRAM
               && el->parent()->parent()->type() == ElementType::SEGMENT) {
        m = static_cast<const Ms::Segment*>(el->parent()->parent())->measure();
    } else if (el->isMeasureBase()) {
        m = static_cast<const Ms::MeasureBase*>(el);
    } else if (el->isSpannerSegment()) {
        EngravingItem* se = static_cast<const Ms::SpannerSegment*>(el)->spanner()->startElement();
        m = static_cast<Measure*>(se->findMeasure());
    } else if (el->isSpanner()) {
        EngravingItem* se = static_cast<const Ms::Spanner*>(el)->startElement();
        m = static_cast<Measure*>(se->findMeasure());
    } else {
        // attempt to find measure
        Ms::EngravingObject* e = el->parent();
        while (e && !e->isMeasureBase()) {
            e = e->parent();
        }
        if (e) {
            m = toMeasureBase(e);
        } else {
            return;
        }
    }
    if (!m) {
        return;
    }

    int staffIdx = el->staffIdx();
    Ms::System* sys = m->system();
    if (!sys) {
        return;
    }

    RectF mRect(m->canvasBoundingRect());
    RectF sysRect;
    if (staffIdx == -1) {
        sysRect = sys->canvasBoundingRect();
    } else {
        sysRect = sys->staff(staffIdx)->bbox();
    }

    // only try to track measure if not during playback
    if (!playBack) {
        sysRect = mRect;
    }

    double _spatium    = m_getScore->score()->spatium();
    const qreal border = _spatium * 3;
    RectF showRect;
    if (staffIndex == -1) {
        showRect = RectF(mRect.x(), sysRect.y(), mRect.width(), sysRect.height())
                   .adjusted(-border, -border, border, border);
    } else {
        // find a box for the individual stave in a system
        RectF stave = RectF(sys->canvasBoundingRect().left(),
                            sys->staffCanvasYpage(staffIndex),
                            sys->width(),
                            sys->staff(staffIndex)->bbox().height());
        showRect = mRect.intersected(stave).adjusted(-border, -border, border, border);
    }

    INotationInteraction::ShowItemRequest request;
    request.item = el;
    request.showRect = showRect;

    m_interaction->showItemRequested().send(request);
}

const Ms::EngravingItem* ScoreCallbacks::dropTarget() const
{
    IF_ASSERT_FAILED(m_interaction) {
        return nullptr;
    }

    return m_interaction->dropTarget();
}

void ScoreCallbacks::setNotationInteraction(INotationInteraction* interaction)
{
    m_interaction = interaction;
}

void ScoreCallbacks::setGetScore(const IGetScore* getScore)
{
    m_getScore = getScore;
}
