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
using namespace muse::draw;

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

void ShadowNote::setState(SymId noteSymbol, TDuration duration, bool rest, double segmentSkylineTopY, double segmentSkylineBottomY,
                          AccidentalType accidentalType, const std::set<SymId>& articulationIds)
{
    m_noteheadSymbol = noteSymbol;
    m_duration = duration;
    m_isRest = rest;
    m_segmentSkylineTopY = segmentSkylineTopY;
    m_segmentSkylineBottomY = segmentSkylineBottomY;
    m_accidentalType = accidentalType;
    m_articulationIds = articulationIds;
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

AccidentalType ShadowNote::accidentalType() const
{
    return m_accidentalType;
}

const std::set<SymId>& ShadowNote::articulationIds() const
{
    return m_articulationIds;
}

double ShadowNote::segmentSkylineBottomY() const
{
    return m_segmentSkylineBottomY;
}

double ShadowNote::segmentSkylineTopY() const
{
    return m_segmentSkylineTopY;
}

bool ShadowNote::ledgerLinesVisible() const
{
    return !m_isRest && m_lineIndex < 100 && m_lineIndex > -100;
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

void ShadowNote::drawArticulations(Painter* painter) const
{
    double noteheadWidth = symWidth(m_noteheadSymbol);
    double ms = spatium();
    double x1 = noteheadWidth * .5 - (ms * mag());
    double x2 = x1 + 2 * ms * mag();
    ms *= .5;
    double y1 = -ms * (m_lineIndex) + m_segmentSkylineTopY;
    double y2 = -ms * (m_lineIndex) + m_segmentSkylineBottomY;

    RectF boundRect = RectF(PointF(x1, y1), PointF(x2, y2));

    for (const SymId& artic: m_articulationIds) {
        bool isMarcato = Articulation::symId2ArticulationName(artic).contains(u"marcato");

        if (isMarcato) {
            drawMarcato(painter, artic, boundRect);
        } else {
            drawArticulation(painter, artic, boundRect);
        }
    }
}

void ShadowNote::drawMarcato(Painter* painter, const SymId& artic, RectF& boundRect) const
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

void ShadowNote::drawArticulation(Painter* painter, const SymId& artic, RectF& boundRect) const
{
    PointF coord;
    double spacing = spatium();
    SymId articSym = artic;

    bool up = !computeUp();
    if (articSym == SymId::articLaissezVibrerAbove || articSym == SymId::articLaissezVibrerBelow) {
        articSym = up ? SymId::articLaissezVibrerAbove : SymId::articLaissezVibrerBelow;
        const int upDir = up ? -1 : 1;
        const double noteHeight = symHeight(m_noteheadSymbol);

        double center = 0.5 * symWidth(m_noteheadSymbol);
        double visualInset = 0.1 * spacing;

        coord.rx() = center + visualInset - symBbox(articSym).left();
        coord.ry() = (noteHeight / 2 + 0.2 * spacing) * upDir;
        boundRect.setWidth(boundRect.width() + symWidth(articSym));
    } else {
        if (up) {
            double topY = boundRect.y();
            if (topY > 0) {
                topY = 0;
            }
            coord.ry() = topY - symHeight(articSym);
            boundRect.setTop(topY - symHeight(articSym) - spacing);
        } else {
            double bottomY = boundRect.bottomLeft().y();
            if (bottomY < 0) {
                bottomY = symHeight(m_noteheadSymbol);
            }
            coord.ry() = bottomY + symHeight(articSym);
            boundRect.setHeight(bottomY + symHeight(articSym) + spacing);
        }
    }

    drawSymbol(articSym, painter, coord);
}
}
