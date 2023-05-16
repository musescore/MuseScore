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

#include "stretchedbend.h"

#include "draw/fontmetrics.h"

#include "layout/tlayout.h"

#include "chord.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "tie.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   textFlags
//---------------------------------------------------------

static int textFlags = draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip;

//---------------------------------------------------------
//   forward declarations of static functions
//---------------------------------------------------------

static void drawText(mu::draw::Painter* painter, const PointF& pos, const String& text);
static RectF textBoundingRect(const mu::draw::FontMetrics& fm, const PointF& pos, const String& text);
static PainterPath bendCurveFromPoints(const PointF& p1, const PointF& p2);
static int bendTone(int notePitch);

//---------------------------------------------------------
//   static values
//---------------------------------------------------------

static constexpr double s_bendHeightMultiplier = .2; /// how much height differs for bend pitches

//---------------------------------------------------------
//   StretchedBend
//---------------------------------------------------------

StretchedBend::StretchedBend(Note* parent)
    : Bend(parent, ElementType::STRETCHED_BEND)
{
}

//---------------------------------------------------------
//   fillDrawPoints
//---------------------------------------------------------

void StretchedBend::fillDrawPoints()
{
    if (m_points.size() < 2) {
        return;
    }

    m_drawPoints.clear();

    if (Tie* tie = toNote(parent())->tieBack()) {
        Note* backTied = tie->startNote();
        if (StretchedBend* lastBend = (backTied ? backTied->bend() : nullptr)) {
            auto lastBendPoints = lastBend->m_drawPoints;
            if (!lastBendPoints.empty() && lastBendPoints.back() == m_points[0].pitch) {
                m_skipFirstPoint = true;
            }
        }
    }

    for (size_t i = 0; i < m_points.size(); i++) {
        m_drawPoints.push_back(m_points[i].pitch);
    }
}

//---------------------------------------------------------
//   fillSegments
//---------------------------------------------------------

void StretchedBend::fillSegments()
{
    m_bendSegments.clear();

    size_t n = m_drawPoints.size();
    if (n < 2) {
        return;
    }

    bool isPrevBendUp = false;
    PointF upBendDefaultSrc = PointF(m_noteWidth + m_spatium * .8, 0);
    PointF downBendDefaultSrc = PointF(m_noteWidth * .5, -m_noteHeight * .5 - m_spatium * .2);

    PointF src = m_drawPoints[0] == 0 ? upBendDefaultSrc : downBendDefaultSrc;
    PointF dest(0, 0);

    int lastPointPitch = m_drawPoints.back();
    m_releasedToInitial = (0 == lastPointPitch);

    double baseBendHeight = m_spatium * 1.5;
    BendSegmentType prevLineType = BendSegmentType::NO_TYPE;

    for (size_t pt = 0; pt < n - 1; pt++) {
        int pitch = m_drawPoints[pt];
        int nextPitch = m_drawPoints[pt + 1];

        BendSegmentType type = BendSegmentType::NO_TYPE;
        int tone = bendTone(nextPitch);

        /// PRE-BEND (+BEND, +RELEASE)
        if (pt == 0 && pitch != 0) {
            int prebendTone = bendTone(pitch);
            double minY = std::min(-m_notePos.y(), src.y());
            dest = PointF(src.x(), minY - bendHeight(prebendTone) - baseBendHeight);
            if (!m_skipFirstPoint) {
                m_bendSegments.push_back({ src, dest, BendSegmentType::LINE_UP, prebendTone });
            }

            src.ry() = dest.y();
        }

        /// PRE-BEND - - -
        if (pitch == nextPitch) {
            if (pt == 0) {
                type = BendSegmentType::LINE_STROKED;
            }
        } else {
            bool bendUp = pitch < nextPitch;
            if (bendUp && isPrevBendUp && pt > 0) {
                // prevent double bendUp rendering
                int prevBendPitch = m_drawPoints[pt - 1];
                // remove prev segment if there are two bendup in a row
                if (!m_bendSegments.empty()) {
                    m_bendSegments.pop_back();
                    if (prevBendPitch > 0 && prevBendPitch < pitch && !m_bendSegments.empty()) {
                        m_bendSegments.back().tone = bendTone(0);
                    }
                }
                // We need to reset the src position in case we remove or change prev bend
                src = upBendDefaultSrc;
            }

            isPrevBendUp = bendUp;

            if (bendUp) {
                double minY = std::min(-m_notePos.y(), src.y());
                dest.ry() = minY - bendHeight(tone) - baseBendHeight;
                type = BendSegmentType::CURVE_UP;
            } else {
                if (m_releasedToInitial) {
                    dest.ry() = 0;
                } else {
                    dest.ry() = src.y() + baseBendHeight;
                }

                type = BendSegmentType::CURVE_DOWN;
            }
        }
        // prevent double curve down rendering
        if (prevLineType == BendSegmentType::CURVE_DOWN && type == BendSegmentType::CURVE_DOWN) {
            continue;
        }

        if (type != BendSegmentType::NO_TYPE) {
            m_bendSegments.push_back({ src, dest, type, tone });
        }

        src = dest;
        prevLineType = type;
    }
}

//---------------------------------------------------------
//   stretchSegments
//---------------------------------------------------------

void StretchedBend::stretchSegments()
{
    if (m_bendSegments.empty()) {
        return;
    }
    size_t segsSize = m_bendSegments.size();

    double bendStart = m_bendSegments[0].src.x();
    double bendEnd = m_stretchedMode ? nextSegmentX() : m_spatium * 6;

    if (bendStart > bendEnd) {
        m_bendSegments.clear();
        return;
    }

    m_bendSegments[segsSize - 1].dest.rx() = bendEnd;

    double step = (bendEnd - bendStart) / segsSize;

    for (auto i = segsSize - 1; i > 0; --i) {
        auto& lastSeg = m_bendSegments[i];
        auto& prevSeg = m_bendSegments[i - 1];
        if (lastSeg.type != BendSegmentType::LINE_UP && prevSeg.type != BendSegmentType::LINE_UP
            && lastSeg.type != BendSegmentType::LINE_STROKED) {
            bendEnd -= step;
            lastSeg.src.rx() = bendEnd;
            prevSeg.dest.rx() = bendEnd;
        }
    }
}

//---------------------------------------------------------
//   layoutStretched
//---------------------------------------------------------

void StretchedBend::layoutStretched()
{
    LayoutContext ctx(score());
    v0::TLayout::layoutStretched(this, ctx);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StretchedBend::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;

    setupPainter(painter);
    layoutDraw(false, painter);
}

//---------------------------------------------------------
//   layoutDraw
//---------------------------------------------------------

void StretchedBend::layoutDraw(const bool layoutMode, mu::draw::Painter* painter) const
{
    if (!layoutMode && !painter) {
        return;
    }

    // For chained bends we need to draw text at least once
    bool isTextDrawn = false;

    for (const BendSegment& bendSegment : m_bendSegments) {
        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = String::fromUtf8(label[bendSegment.tone]);

        switch (bendSegment.type) {
        case BendSegmentType::LINE_UP:
        {
            if (layoutMode) {
                m_boundingRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
                m_boundingRect.unite(m_arrowUp.translated(dest).boundingRect());

                mu::draw::FontMetrics fm(font(m_spatium));
                m_boundingRect.unite(textBoundingRect(fm, dest, text));
                /// TODO: remove after fixing bRect
                m_boundingRect.setHeight(m_boundingRect.height() + m_spatium);
                m_boundingRect.setY(m_boundingRect.y() - m_spatium);
            } else {
                painter->drawLine(LineF(src, dest));
                painter->setBrush(curColor());
                painter->drawPolygon(m_arrowUp.translated(dest));
                /// TODO: remove substraction after fixing bRect
                drawText(painter, dest - PointF(0, m_spatium * 0.5), text);
            }

            break;
        }

        case BendSegmentType::CURVE_UP:
        case BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == BendSegmentType::CURVE_UP);
            double endY = dest.y() + m_bendArrowWidth * (bendUp ? 1 : -1);

            PainterPath path = bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? m_arrowUp : m_arrowDown);

            if (layoutMode) {
                m_boundingRect.unite(path.boundingRect());
                m_boundingRect.unite(arrowPath.translated(dest).boundingRect());
            } else {
                painter->setBrush(BrushStyle::NoBrush);
                painter->drawPath(path);
                painter->setBrush(curColor());
                painter->drawPolygon(arrowPath.translated(dest));
            }

            if (!isTextDrawn || (bendUp && !m_releasedToInitial)) {
                if (layoutMode) {
                    mu::draw::FontMetrics fm(font(m_spatium));
                    m_boundingRect.unite(textBoundingRect(fm, dest - PointF(m_spatium, 0), text));
                    /// TODO: remove after fixing bRect
                    m_boundingRect.setHeight(m_boundingRect.height() + m_spatium);
                    m_boundingRect.setY(m_boundingRect.y() - m_spatium);
                } else {
                    // We need to draw text only for bendup elems
                    if (bendUp) {
                        double textLabelOffset = (!bendUp && !m_releasedToInitial ? m_spatium : 0);
                        PointF textPoint = dest + PointF(textLabelOffset, -textLabelOffset);
                        /// TODO: remove substraction after fixing bRect
                        drawText(painter, textPoint - PointF(0, m_spatium * 0.5), text);
                    }
                }

                isTextDrawn = true;
            }

            break;
        }

        case BendSegmentType::LINE_STROKED:
        {
            if (layoutMode) {
                m_boundingRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
            } else {
                PainterPath path;
                path.moveTo(src + PointF(m_bendArrowWidth, 0));
                path.lineTo(dest);
                Pen p(painter->pen());
                p.setStyle(PenStyle::DashLine);
                painter->strokePath(path, p);
            }

            break;
        }

        default:
            break;
        }
    }
}

//---------------------------------------------------------
//   setupPainter
//---------------------------------------------------------

void StretchedBend::setupPainter(mu::draw::Painter* painter) const
{
    Pen pen(curColor(), lineWidth(), PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Brush(curColor()));

    mu::draw::Font f = font(spatium() * MScore::pixelRatio);
    painter->setFont(f);
}

//---------------------------------------------------------
//   glueNeighbor
//---------------------------------------------------------

void StretchedBend::prepareBends(std::vector<StretchedBend*>& bends)
{
#if 0
    /// glueing extra bends together
    for (StretchedBend* bend : bends) {
        bend->glueNeighbor();
    }

    /// deleting reduntant bends
    auto reduntantIt = std::partition(bends.begin(), bends.end(), [](StretchedBend* bend) { return bend->m_reduntant; });

    for (auto bendIt = bends.begin(); bendIt != reduntantIt; bendIt++) {
        StretchedBend* bendToRemove = *bendIt;
        EngravingObject* parentObj = bendToRemove->parent();
        if (Note* note = dynamic_cast<Note*>(parentObj)) {
            note->remove(bendToRemove);
        }

        delete bendToRemove;
        bendToRemove = nullptr;
    }
#endif
    for (StretchedBend* bend : bends) {
        bend->fillDrawPoints();
    }
}

//---------------------------------------------------------
//   glueNeighbor
//---------------------------------------------------------

void StretchedBend::glueNeighbor()
{
    if (m_reduntant) {
        return;
    }

    std::vector<Note*> ties = toNote(parent())->tiedNotes();
    for (Note* t : ties) {
        assert(!!t);
        if (t->bend() && t != parent()) {
            auto bend = t->bend();

            auto& lastPoints = bend->points();
            for (size_t i = 1; i < lastPoints.size(); ++i) {
                m_points.push_back(lastPoints[i]);
            }

            t->remove(bend);
            bend->m_reduntant = true;
        }
    }
}

//---------------------------------------------------------
//   fillArrows
//---------------------------------------------------------

void StretchedBend::fillArrows()
{
    double aw = 0;
    m_bendArrowWidth = aw = score()->styleMM(Sid::bendArrowWidth);

    m_arrowUp.clear();
    m_arrowDown.clear();

    m_arrowUp << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    m_arrowDown << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);
}

//---------------------------------------------------------
//   drawText
//---------------------------------------------------------

void drawText(mu::draw::Painter* painter, const PointF& pos, const String& text)
{
    painter->drawText(RectF(pos.x(), pos.y(), .0, .0), textFlags, text);
}

//---------------------------------------------------------
//   textBoundingRect
//---------------------------------------------------------

RectF textBoundingRect(const mu::draw::FontMetrics& fm, const PointF& pos, const String& text)
{
    return fm.boundingRect(RectF(pos.x(), pos.y(), 0, 0), textFlags, text);
}

//---------------------------------------------------------
//   bendCurveFromPoints
//---------------------------------------------------------

PainterPath bendCurveFromPoints(const PointF& p1, const PointF& p2)
{
    PainterPath path;

    path.moveTo(p1.x(), p1.y());
    path.cubicTo(p1.x() + (p2.x() - p1.x()) / 2, p1.y(), p2.x(), p1.y() + (p2.y() - p1.y()) / 4, p2.x(), p2.y());

    return path;
}

//---------------------------------------------------------
//   nextSegmentX
//---------------------------------------------------------

double StretchedBend::nextSegmentX() const
{
    Segment* nextSeg = toNote(parent())->chord()->segment()->nextInStaff(
        staffIdx(), SegmentType::ChordRest | SegmentType::BarLine | SegmentType::EndBarLine);
    if (!nextSeg) {
        return 0;
    }

    return nextSeg->pagePos().x() - pagePos().x() - m_spatium;
}

//---------------------------------------------------------
//   bendPitch
//---------------------------------------------------------

int bendTone(int notePitch)
{
    return (notePitch + 12) / 25;
}

//---------------------------------------------------------
//   bendHeight
//---------------------------------------------------------

double StretchedBend::bendHeight(int bendIdx) const
{
    return m_spatium * (bendIdx + 1) * s_bendHeightMultiplier;
}
}
