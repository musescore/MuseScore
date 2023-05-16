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

    bool straight = score()->styleB(Sid::useStraightNoteFlags);
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

void ShadowNote::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    if (!visible() || !isValid()) {
        return;
    }

    PointF ap(pagePos());
    painter->translate(ap);
    double lw = score()->styleMM(Sid::stemWidth) * mag();
    Pen pen(engravingConfiguration()->highlightSelectionColor(voice()), lw, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    bool up = computeUp();

    // Draw the accidental
    SymId acc = Accidental::subtype2symbol(score()->inputState().accidentalType());
    if (acc != SymId::noSym) {
        PointF posAcc;
        posAcc.rx() -= symWidth(acc) + score()->styleMM(Sid::accidentalNoteDistance) * mag();
        drawSymbol(acc, painter, posAcc);
    }

    // Draw the notehead
    drawSymbol(m_noteheadSymbol, painter);

    // Draw the dots
    double sp = spatium();
    double sp2 = sp / 2;
    double noteheadWidth = symWidth(m_noteheadSymbol);

    PointF posDot;
    if (m_duration.dots() > 0) {
        double d  = score()->styleMM(Sid::dotNoteDistance) * mag();
        double dd = score()->styleMM(Sid::dotDotDistance) * mag();
        posDot.rx() += (noteheadWidth + d);

        if (m_isRest) {
            posDot.ry() += Rest::getDotline(m_duration.type()) * sp2;
        } else {
            posDot.ry() -= (m_lineIndex % 2 == 0 ? sp2 : 0);
        }

        if (hasFlag() && up) {
            posDot.rx() = std::max(posDot.rx(), noteheadWidth + symBbox(flagSym()).right());
        }

        for (int i = 0; i < m_duration.dots(); i++) {
            posDot.rx() += dd * i;
            drawSymbol(SymId::augmentationDot, painter, posDot);
            posDot.rx() -= dd * i;
        }
    }

    // Draw stem and flag
    if (hasStem()) {
        double x = up ? (noteheadWidth - (lw / 2)) : lw / 2;
        double y1 = symSmuflAnchor(m_noteheadSymbol, up ? SmuflAnchorId::stemUpSE : SmuflAnchorId::stemDownNW).y();
        double y2 = (up ? -3.5 : 3.5) * sp;

        if (hasFlag()) {
            SymId flag = flagSym();
            drawSymbol(flag, painter, PointF(x - (lw / 2), y2));
            y2 += symSmuflAnchor(flag, up ? SmuflAnchorId::stemUpNW : SmuflAnchorId::stemDownSW).y();
        }
        painter->drawLine(LineF(x, y1, x, y2));
    }

    // Draw ledger lines if needed
    if (!m_isRest && m_lineIndex < 100 && m_lineIndex > -100) {
        double extraLen = score()->styleS(Sid::ledgerLineLength).val() * sp;
        double x1 = -extraLen;
        double x2 = noteheadWidth + extraLen;
        double step = sp2 * staffType()->lineDistance().val();

        lw = score()->styleMM(Sid::ledgerLineWidth) * mag();
        pen.setWidthF(lw);
        painter->setPen(pen);

        for (int i = -2; i >= m_lineIndex; i -= 2) {
            double y = step * (i - m_lineIndex);
            painter->drawLine(LineF(x1, y, x2, y));
        }
        int l = staffType()->lines() * 2; // first ledger line below staff
        for (int i = l; i <= m_lineIndex; i += 2) {
            double y = step * (i - m_lineIndex);
            painter->drawLine(LineF(x1, y, x2, y));
        }
    }

    drawArticulations(painter);

    painter->translate(-ap);
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
