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
                          bool beyondScore,
                          AccidentalType accidentalType, const std::set<SymId>& articulationIds)
{
    m_noteheadSymbol = noteSymbol;
    m_duration = duration;
    m_isRest = rest;
    m_segmentSkylineTopY = segmentSkylineTopY;
    m_segmentSkylineBottomY = segmentSkylineBottomY;
    m_beyondScore = beyondScore;
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

// draw articulations, laying them out in a fashion similar to
// ChordLayout::layoutArticulations
void ShadowNote::drawArticulations(Painter* painter) const
{
    // Approximate bounding box of note, including stem, relative to notehead
    double noteheadWidth = symWidth(m_noteheadSymbol);
    double noteheadHeight = symHeight(m_noteheadSymbol);
    double ms = spatium();
    double x1 = noteheadWidth * .5 - (ms * mag());
    double x2 = x1 + 2 * ms * mag();
    bool up = !computeUp();
    double y01 = -noteheadHeight * .5;
    double y02 = -y01;
    if (hasStem()) {
        // Stem set at 3.5 spatium in SingleDraw::draw(const ShadowNote* item ...
        if (up) {
            y02 = ms * 3.5;
        } else {
            y01 = -ms * 3.5;
        }
    }
    RectF boundRect = RectF(PointF(x1, y01), PointF(x2, y02));

    // Place articulations kept close to the note first, near the notehead
    // Articulations close to the note are defined in Articulation::layoutCloseToNote
    bool wasStaccato = false; // keep track for special accent-staccato kerning
    SymId prevArticulation = mu::engraving::SymId::noSym;
    for (const SymId& artic: m_articulationIds) {
        String articName = Articulation::symId2ArticulationName(artic);
        if (articName == u"staccato") {
            wasStaccato = true;
            drawCloseArticulation(painter, artic, boundRect, up, prevArticulation);
            prevArticulation = artic;
        } else {
            if (articName == u"tenuto") {
                drawCloseArticulation(painter, artic, boundRect, up, prevArticulation);
                prevArticulation = artic;
            }
            wasStaccato = false;
        }
    }

    // Adjust bounding box to include the whole of the staff segment,
    // then place articulations not closeToNote
    ms *= .5;
    // double y1 = -ms * (m_lineIndex) + m_segmentSkylineTopY;
    // double y2 = -ms * (m_lineIndex) + m_segmentSkylineBottomY;
    double y1 = -m_lineIndex * ms;
    double y2 = (this->staffType()->bottomLine() - m_lineIndex) * ms;
    if (boundRect.y() > y1) {
        boundRect.setTop(y1);
        if (up) {
            wasStaccato = false;
        }
    }
    if (boundRect.bottom() < y2) {
        boundRect.setBottom(y2);
        if (!up) {
            wasStaccato = false;
        }
    }

    for (const SymId& artic: m_articulationIds) {
        String articName = Articulation::symId2ArticulationName(artic);
        if (articName == u"staccato" || articName == u"tenuto") {
            continue;
        } else {
            if (articName.contains(u"marcato")) {
                // Marcato always above staff
                drawFarArticulation(painter, artic, boundRect, true, false);
            } else if (articName == u"accent") {
                // Accents get special kerning if adjacent to staccato
                drawFarArticulation(painter, artic, boundRect, up, wasStaccato);
            } else {
                // Other articulations (e.g., laissez-vibrer)
                drawFarArticulation(painter, artic, boundRect,  up, false);
            }
            wasStaccato = false;
        }
    }
}

void ShadowNote::drawCloseArticulation(muse::draw::Painter* painter, const SymId& artic, RectF& boundRect, bool up,
                                       const SymId& prevArticulation) const
{
    PointF coord;
    double spacing = spatium();
    SymId articSym = artic;
    double articHeight = symHeight(articSym);

    // Center the articulation on the note-head
    coord.rx() = boundRect.x() + (boundRect.width() - symWidth(artic)) / 2;

    // Place articulation above or below boundRect and adjust boundRect accordingly
    double minDist = 0.4 * spacing;
    if (up) {
        // If operating inside the staff, center articulation between lines
        if (m_lineIndex > 1) {
            coord.ry() = (articHeight - spacing * (3 - (m_lineIndex & 1))) / 2;
            if (prevArticulation != mu::engraving::SymId::noSym) {
                if (m_lineIndex > 2) {
                    coord.ry() -= spacing;
                } else {
                    coord.ry() -= minDist + symHeight(prevArticulation);
                }
            }
        } else {
            double topY = boundRect.y();
            coord.ry() = topY - minDist;
        }
        boundRect.setTop(coord.ry() - articHeight);
    } else {
        if (m_lineIndex < this->staffType()->bottomLine() - 1) {
            coord.ry() = (articHeight + spacing * (3 - (m_lineIndex & 1))) / 2;
            if (prevArticulation != mu::engraving::SymId::noSym) {
                if (m_lineIndex < this->staffType()->bottomLine() - 2) {
                    coord.ry() += spacing;
                } else {
                    coord.ry() += minDist + symHeight(prevArticulation);
                }
            }
        } else {
            double bottomY = boundRect.bottom();
            coord.ry() = bottomY + minDist + articHeight;
        }
        boundRect.setBottom(coord.ry());
    }

    drawSymbol(articSym, painter, coord);
}

void ShadowNote::drawFarArticulation(muse::draw::Painter* painter, const SymId& artic, RectF& boundRect, bool up,
                                     bool accentStaccatoKern) const
{
    PointF coord;
    double spacing = spatium();
    SymId articSym = artic;
    double articHeight = symHeight(articSym);

    // If a LaissezVibrer, place beside the note and widen the bounding box
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
        // If on the stem side of a note (e.g., marcato on a low note), move artic over for stem,
        // else, center it on the notehead;
        coord.rx() = boundRect.x() + (boundRect.width() - symWidth(artic)) / 2;
        if (up == computeUp() && hasStem()) {
            coord.rx() += 0.25 * symWidth(m_noteheadSymbol);
        }

        // Place articulation above or below boundRect and adjust boundRect accordingly
        if (up) {
            double topY = boundRect.y();
            coord.ry() = topY - (accentStaccatoKern ? 0.2 : 0.4) * spacing;
            boundRect.setTop(coord.ry() - articHeight);
        } else {
            double bottomY = boundRect.bottom();
            coord.ry() = bottomY + (accentStaccatoKern ? 0.2 : 0.4) * spacing + articHeight;
            boundRect.setBottom(coord.ry());
        }
    }

    drawSymbol(articSym, painter, coord);
}
}
