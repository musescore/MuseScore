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

#include "score.h"
#include "drumset.h"
#include "sym.h"
#include "rest.h"
#include "mscore.h"
#include "accidental.h"
#include "articulation.h"
#include "draw/pen.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
    : Element(s), m_noteheadSymbol(SymId::noSym)
{
    m_lineIndex = 1000;
    m_duration = TDuration(TDuration::DurationType::V_INVALID);
    m_isRest = false;
}

bool ShadowNote::isValid() const
{
    return m_noteheadSymbol != SymId::noSym;
}

void ShadowNote::setState(SymId noteSymbol, TDuration duration, bool rest, qreal segmentSkylineTopY, qreal segmentSkylineBottomY)
{
    m_noteheadSymbol = noteSymbol;
    m_duration = duration;
    m_isRest = rest;
    m_segmentSkylineTopY = segmentSkylineTopY;
    m_segmentSkylineBottomY = segmentSkylineBottomY;
}

SymId ShadowNote::getNoteFlag() const
{
    SymId flag = SymId::noSym;
    if (m_isRest) {
        return flag;
    }
    TDuration::DurationType type = m_duration.type();
    switch (type) {
    case TDuration::DurationType::V_LONG:
        flag = SymId::lastSym;
        break;
    case TDuration::DurationType::V_BREVE:
        flag = SymId::noSym;
        break;
    case TDuration::DurationType::V_WHOLE:
        flag = SymId::noSym;
        break;
    case TDuration::DurationType::V_HALF:
        flag = SymId::lastSym;
        break;
    case TDuration::DurationType::V_QUARTER:
        flag = SymId::lastSym;
        break;
    case TDuration::DurationType::V_EIGHTH:
        flag = computeUp() ? SymId::flag8thUp : SymId::flag8thDown;
        break;
    case TDuration::DurationType::V_16TH:
        flag = computeUp() ? SymId::flag16thUp : SymId::flag16thDown;
        break;
    case TDuration::DurationType::V_32ND:
        flag = computeUp() ? SymId::flag32ndUp : SymId::flag32ndDown;
        break;
    case TDuration::DurationType::V_64TH:
        flag = computeUp() ? SymId::flag64thUp : SymId::flag64thDown;
        break;
    case TDuration::DurationType::V_128TH:
        flag = computeUp() ? SymId::flag128thUp : SymId::flag128thDown;
        break;
    case TDuration::DurationType::V_256TH:
        flag = computeUp() ? SymId::flag256thUp : SymId::flag256thDown;
        break;
    case TDuration::DurationType::V_512TH:
        flag = computeUp() ? SymId::flag512thUp : SymId::flag512thDown;
        break;
    case TDuration::DurationType::V_1024TH:
        flag = computeUp() ? SymId::flag1024thUp : SymId::flag1024thDown;
        break;
    default:
        flag = SymId::noSym;
    }
    return flag;
}

//---------------------------------------------------------
//   computeUp
//---------------------------------------------------------

bool ShadowNote::computeUp() const
{
    Staff* st = staff();
    const StaffType* tab = st ? st->staffTypeForElement(this) : 0;
    bool tabStaff = tab && tab->isTabStaff();

    if (tabStaff && (tab->stemless() || !tab->stemThrough())) {
        // if TAB staff with no stems or stem beside staff
        return true;
    }

    if (score()->tick2measure(m_tick)->hasVoices(staffIdx()) || voice() != 0) {
        return !(voice() & 1);
    } else {
        return m_lineIndex > (st ? st->middleLine(tick()) : 4);
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (!visible() || !isValid()) {
        return;
    }

    PointF ap(pagePos());
    painter->translate(ap);
    qreal lw = score()->styleP(Sid::stemWidth);
    Pen pen(MScore::selectColor[voice()].lighter(SHADOW_NOTE_LIGHT), lw, PenStyle::SolidLine, PenCapStyle::FlatCap);
    painter->setPen(pen);

    // Draw the accidental
    SymId acc = Accidental::subtype2symbol(score()->inputState().accidentalType());
    if (acc != SymId::noSym) {
        PointF posAcc;
        posAcc.rx() -= symWidth(acc) + score()->styleP(Sid::accidentalNoteDistance) * mag();
        drawSymbol(acc, painter, posAcc);
    }

    // Draw the notehead
    drawSymbol(m_noteheadSymbol, painter);

    // Draw the dots
    qreal sp = spatium();
    qreal sp2 = sp / 2;
    qreal noteheadWidth = symWidth(m_noteheadSymbol);

    PointF posDot;
    if (m_duration.dots() > 0) {
        qreal d  = score()->styleP(Sid::dotNoteDistance) * mag();
        qreal dd = score()->styleP(Sid::dotDotDistance) * mag();
        posDot.rx() += (noteheadWidth + d);
        if (!m_isRest) {
            posDot.ry() -= (m_lineIndex % 2 == 0 ? sp2 : 0);
        } else {
            posDot.ry() += Rest::getDotline(m_duration.type()) * sp2 * mag();
        }
        for (int i = 0; i < m_duration.dots(); i++) {
            posDot.rx() += dd * i;
            drawSymbol(SymId::augmentationDot, painter, posDot, 1);
            posDot.rx() -= dd * i;
        }
    }

    // Draw stem and flag
    SymId flag = getNoteFlag();
    if (flag != SymId::noSym) {
        bool up = computeUp();
        qreal x = up ? (noteheadWidth - (lw / 2)) : lw / 2;
        qreal y1 = symSmuflAnchor(m_noteheadSymbol, up ? SmuflAnchorId::stemUpSE : SmuflAnchorId::stemDownNW).y();
        qreal y2 = (up ? -3.5 : 3.5) * sp * mag();

        if (flag != SymId::lastSym) { // If there is a flag
            if (up && m_duration.dots() && !(m_lineIndex & 1)) {
                y2 -= sp2 * mag(); // Lengthen stem to avoid collision of flag with dots
            }
            drawSymbol(flag, painter, PointF(x - (lw / 2), y2), 1);
            y2 += symSmuflAnchor(flag, up ? SmuflAnchorId::stemUpNW : SmuflAnchorId::stemDownSW).y();
        }
        painter->drawLine(LineF(x, y1, x, y2));
    }

    // Draw ledger lines if needed
    if (!m_isRest && m_lineIndex < 100 && m_lineIndex > -100) {
        qreal extraLen = score()->styleP(Sid::ledgerLineLength) * mag();
        qreal x1 = -extraLen;
        qreal x2 = noteheadWidth + extraLen;

        lw = score()->styleP(Sid::ledgerLineWidth);
        pen.setWidthF(lw);
        painter->setPen(pen);

        for (int i = -2; i >= m_lineIndex; i -= 2) {
            qreal y = sp2 * mag() * (i - m_lineIndex);
            painter->drawLine(LineF(x1, y, x2, y));
        }
        int l = staff()->lines(tick()) * 2; // first ledger line below staff
        for (int i = l; i <= m_lineIndex; i += 2) {
            qreal y = sp2 * mag() * (i - m_lineIndex);
            painter->drawLine(LineF(x1, y, x2, y));
        }
    }

    drawArticulations(painter);

    painter->translate(-ap);
}

void ShadowNote::drawArticulations(mu::draw::Painter* painter) const
{
    qreal noteheadWidth = symWidth(m_noteheadSymbol);
    qreal ms = spatium();
    qreal x1 = noteheadWidth * .5 - (ms * mag());
    qreal x2 = x1 + 2 * ms * mag();
    ms *= .5;
    qreal y1 = -ms * (m_lineIndex) + m_segmentSkylineTopY;
    qreal y2 = -ms * (m_lineIndex) + m_segmentSkylineBottomY;

    RectF boundRect = RectF(PointF(x1, y1), PointF(x2, y2));

    for (const SymId& articulation: score()->inputState().articulationIds()) {
        bool isMarcato = QString(Articulation::symId2ArticulationName(articulation)).contains("marcato");

        if (isMarcato) {
            drawMarcato(painter, articulation, boundRect);
        } else {
            drawArticulation(painter, articulation, boundRect);
        }
    }
}

void ShadowNote::drawMarcato(mu::draw::Painter* painter, const SymId& articulation, RectF& boundRect) const
{
    PointF coord;
    qreal spacing = spatium();

    qreal topY = boundRect.y();
    if (topY > 0) {
        topY = 0;
    }
    coord.ry() = topY - symHeight(articulation);

    boundRect.setY(boundRect.y() - symHeight(articulation) - spacing);
    drawSymbol(articulation, painter, coord);
}

void ShadowNote::drawArticulation(mu::draw::Painter* painter, const SymId& articulation, RectF& boundRect) const
{
    PointF coord;
    qreal spacing = spatium();

    bool up = !computeUp();
    if (up) {
        qreal topY = boundRect.y();
        if (topY > 0) {
            topY = 0;
        }
        coord.ry() = topY - symHeight(articulation);
        boundRect.setY(topY - symHeight(articulation) - spacing);
    } else {
        qreal bottomY = boundRect.bottomLeft().y();
        if (bottomY < 0) {
            bottomY = symHeight(m_noteheadSymbol);
        }
        coord.ry() = bottomY + symHeight(articulation);
        boundRect.setHeight(bottomY + symHeight(articulation) + spacing);
    }

    drawSymbol(articulation, painter, coord);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void ShadowNote::layout()
{
    if (!isValid()) {
        setbbox(RectF());
        return;
    }
    qreal _spatium = spatium();
    RectF newBbox;
    RectF noteheadBbox = symBbox(m_noteheadSymbol);
    SymId flag = getNoteFlag();

    // TODO: Take into account accidentals and articulations?

    // Layout dots
    qreal dotWidth = 0;
    if (m_duration.dots() > 0) {
        qreal noteheadWidth = symWidth(m_noteheadSymbol);
        qreal d  = score()->styleP(Sid::dotNoteDistance) * mag();
        qreal dd = score()->styleP(Sid::dotDotDistance) * mag();
        dotWidth += (noteheadWidth + d);
        for (int i = 0; i < m_duration.dots(); i++) {
            dotWidth += dd * i;
        }
    }
    newBbox.setRect(noteheadBbox.x(), noteheadBbox.y(), noteheadBbox.width() + dotWidth, noteheadBbox.height());

    // Layout stem and flag
    if (flag != SymId::noSym) {
        bool up = computeUp();
        qreal x = noteheadBbox.x();
        qreal w = noteheadBbox.width();

        qreal stemWidth = score()->styleP(Sid::stemWidth);
        qreal stemLenght = (up ? -3.5 : 3.5) * _spatium;
        qreal stemAnchor = symSmuflAnchor(m_noteheadSymbol, up ? SmuflAnchorId::stemUpSE : SmuflAnchorId::stemDownNW).y();
        if (up && flag != SymId::lastSym && m_duration.dots() && !(m_lineIndex & 1)) {
            stemLenght -= 0.5 * spatium(); // Lengthen stem to avoid collision of flag with dots
        }
        newBbox |= RectF(up ? x + w - stemWidth : x,
                         stemAnchor,
                         stemWidth,
                         stemLenght - stemAnchor);

        if (flag != SymId::lastSym) { // If there is a flag
            RectF flagBbox = symBbox(flag);
            newBbox |= RectF(up ? x + w - stemWidth : x,
                             stemAnchor + stemLenght + flagBbox.y(),
                             flagBbox.width(),
                             flagBbox.height());
        }
    }

    // Layout ledger lines if needed
    if (!m_isRest && m_lineIndex < 100 && m_lineIndex > -100) {
        qreal extraLen = score()->styleP(Sid::ledgerLineLength) * mag();
        qreal x = noteheadBbox.x() - extraLen;
        qreal w = noteheadBbox.width() + 2 * extraLen;

        qreal lw = score()->styleP(Sid::ledgerLineWidth);

        InputState ps = score()->inputState();
        RectF r(x, -lw * .5, w, lw);
        for (int i = -2; i >= m_lineIndex; i -= 2) {
            newBbox |= r.translated(PointF(0, _spatium * .5 * (i - m_lineIndex)));
        }
        int l = staff()->lines(tick()) * 2; // first ledger line below staff
        for (int i = l; i <= m_lineIndex; i += 2) {
            newBbox |= r.translated(PointF(0, _spatium * .5 * (i - m_lineIndex)));
        }
    }
    setbbox(newBbox);
}
}
