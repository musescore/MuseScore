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

#include "shadownote.h"

#include "draw/types/pen.h"

#include "accidental.h"
#include "articulation.h"
#include "hook.h"
#include "measure.h"
#include "rest.h"
#include "score.h"
#include "stafftype.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
    : EngravingItem(ElementType::SHADOW_NOTE, s), m_noteheadSymbol(SymId::noSym)
{
    m_lineIndex = 1000;
    m_duration = TDuration(DurationType::V_INVALID);
    m_isRest = false;
}

bool ShadowNote::isValid() const
{
    return m_noteheadSymbol != SymId::noSym;
}

void ShadowNote::setState(SymId noteSymbol, TDuration duration, bool rest, double segmentSkylineTopY, double segmentSkylineBottomY)
{
    m_noteheadSymbol = noteSymbol;
    m_duration = duration;
    m_isRest = rest;
    m_segmentSkylineTopY = segmentSkylineTopY;
    m_segmentSkylineBottomY = segmentSkylineBottomY;
}

bool ShadowNote::hasStem() const
{
    return !m_isRest && m_duration.hasStem();
}

bool ShadowNote::hasFlag() const
{
    return hasStem() && m_duration.hooks() > 0;
}

SymId ShadowNote::flagSym() const
{
    if (!hasStem()) {
        return SymId::noSym;
    }

    bool straight = style().styleB(Sid::useStraightNoteFlags);
    int hooks = computeUp() ? m_duration.hooks() : -m_duration.hooks();
    return Hook::symIdForHookIndex(hooks, straight);
}

//---------------------------------------------------------
//   computeUp
//---------------------------------------------------------

bool ShadowNote::computeUp() const
{
    const StaffType* staffType = this->staffType();
    bool tabStaff = staffType && staffType->isTabStaff();

    if (tabStaff && (staffType->stemless() || !staffType->stemThrough())) {
        // if TAB staff with no stems or stem beside staff
        return true;
    }

    if (score()->tick2measure(m_tick)->hasVoices(staffIdx()) || voice() != 0) {
        return !(voice() & 1);
    } else {
        return m_lineIndex > (staffType ? staffType->middleLine() : 4);
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(mu::draw::Painter*) const
{
    UNREACHABLE;
}

void ShadowNote::drawArticulations(mu::draw::Painter* painter) const
{
    double noteheadWidth = symWidth(m_noteheadSymbol);
    double ms = spatium();
    double x1 = noteheadWidth * .5 - (ms * mag());
    double x2 = x1 + 2 * ms * mag();
    ms *= .5;
    double y1 = -ms * (m_lineIndex) + m_segmentSkylineTopY;
    double y2 = -ms * (m_lineIndex) + m_segmentSkylineBottomY;

    RectF boundRect = RectF(PointF(x1, y1), PointF(x2, y2));

    for (const SymId& artic: score()->inputState().articulationIds()) {
        bool isMarcato = Articulation::symId2ArticulationName(artic).contains(u"marcato");

        if (isMarcato) {
            drawMarcato(painter, artic, boundRect);
        } else {
            drawArticulation(painter, artic, boundRect);
        }
    }
}

void ShadowNote::drawMarcato(mu::draw::Painter* painter, const SymId& artic, RectF& boundRect) const
{
    PointF coord;
    double spacing = spatium();

    double topY = boundRect.y();
    if (topY > 0) {
        topY = 0;
    }
    coord.ry() = topY - symHeight(artic);

    boundRect.setTop(boundRect.y() - symHeight(artic) - spacing);
    drawSymbol(artic, painter, coord);
}

void ShadowNote::drawArticulation(mu::draw::Painter* painter, const SymId& artic, RectF& boundRect) const
{
    PointF coord;
    double spacing = spatium();

    bool up = !computeUp();
    if (up) {
        double topY = boundRect.y();
        if (topY > 0) {
            topY = 0;
        }
        coord.ry() = topY - symHeight(artic);
        boundRect.setTop(topY - symHeight(artic) - spacing);
    } else {
        double bottomY = boundRect.bottomLeft().y();
        if (bottomY < 0) {
            bottomY = symHeight(m_noteheadSymbol);
        }
        coord.ry() = bottomY + symHeight(artic);
        boundRect.setHeight(bottomY + symHeight(artic) + spacing);
    }

    drawSymbol(artic, painter, coord);
}
}
