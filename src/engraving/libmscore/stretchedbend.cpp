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

static const char* label[] = {
    "",     "\u00BC",   "\u00BD",   "\u00BE",  /// 0,   1/4, 1/2, 3/4
    "full", "1\u00BC", "1\u00BD", "1\u00BE",   /// 1, 1+1/4...
    "2",    "2\u00BC", "2\u00BD", "2\u00BE",   /// 2, ...
    "3"                                        /// 3
};

static int textFlags = draw::AlignHCenter | draw::AlignBottom | draw::TextDontClip;

static const ElementStyle stretchedBendStyle {
    { Sid::bendFontFace,  Pid::FONT_FACE },
    { Sid::bendFontSize,  Pid::FONT_SIZE },
    { Sid::bendFontStyle, Pid::FONT_STYLE },
    { Sid::bendLineWidth, Pid::LINE_WIDTH },
};

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

static constexpr double BEND_HEIGHT_MULTIPLIER = .2; /// how much height differs for bend pitches

//---------------------------------------------------------
//   StretchedBend
//---------------------------------------------------------

StretchedBend::StretchedBend(Chord* parent)
    : EngravingItem(ElementType::STRETCHED_BEND, parent, ElementFlag::MOVABLE), m_chord(parent)
{
    initElementStyle(&stretchedBendStyle);
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

mu::draw::Font StretchedBend::font(double sp) const
{
    mu::draw::Font f(_fontFace, Font::Type::Unknown);
    f.setBold(_fontStyle & FontStyle::Bold);
    f.setItalic(_fontStyle & FontStyle::Italic);
    f.setUnderline(_fontStyle & FontStyle::Underline);
    f.setStrike(_fontStyle & FontStyle::Strike);
    double m = _fontSize;
    m *= sp / SPATIUM20;

    f.setPointSizeF(m);
    return f;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue StretchedBend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::FONT_FACE:
        return _fontFace;
    case Pid::FONT_SIZE:
        return _fontSize;
    case Pid::FONT_STYLE:
        return int(_fontStyle);
    case Pid::LINE_WIDTH:
        return _lineWidth;
    default:
        return EngravingItem::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StretchedBend::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::FONT_FACE:
        _fontFace = v.value<String>();
        break;
    case Pid::FONT_SIZE:
        _fontSize = v.toReal();
        break;
    case Pid::FONT_STYLE:
        _fontStyle = FontStyle(v.toInt());
        break;
    case Pid::LINE_WIDTH:
        _lineWidth = v.value<Millimetre>();
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   fillDrawPoints
//---------------------------------------------------------

void StretchedBend::fillDrawPoints()
{
    if (m_pitchValues.size() < 2) {
        return;
    }

    m_drawPoints.clear();

    for (size_t i = 0; i < m_pitchValues.size(); i++) {
        m_drawPoints.push_back(m_pitchValues.at(i).pitch);
    }

    auto maxElIt = std::max_element(m_drawPoints.begin(), m_drawPoints.end());
    if (maxElIt != m_drawPoints.end()) {
        m_maxDrawPointRead = *maxElIt;
    }
}

//---------------------------------------------------------
//   fillSegments
//---------------------------------------------------------

void StretchedBend::fillSegments()
{
    IF_ASSERT_FAILED(m_note) {
        LOGE() << "note is not set";
        return;
    }

    m_bendSegments.clear();
    m_highestCoord = 0;

    size_t n = m_drawPoints.size();
    if (n < 2) {
        return;
    }

    double noteWidth = m_note->width();
    double noteHeight = m_note->height();
    PointF notePos = m_note->pos();
    double sp = spatium();
    bool isPrevBendUp = false;
    PointF upBendDefaultSrc = PointF(noteWidth + sp * .8, 0) + notePos;
    PointF downBendDefaultSrc = PointF(noteWidth * .5, -noteHeight * .5 - sp * .2) + notePos;

    PointF src = m_drawPoints.at(0) == 0 ? upBendDefaultSrc : downBendDefaultSrc;
    PointF dest(notePos);

    int lastPointPitch = m_drawPoints.back();
    bool releasedToInitial = (0 == lastPointPitch);
    bool skipFirstPoint = firstPointShouldBeSkipped();

    double baseBendHeight = sp * 1.5;
    int prevTone = 0;
    BendSegmentType prevLineType = BendSegmentType::NO_TYPE;

    for (size_t pt = 0; pt < n - 1; pt++) {
        int pitch = m_drawPoints.at(pt);
        int nextPitch = m_drawPoints.at(pt + 1);

        BendSegmentType type = BendSegmentType::NO_TYPE;
        int tone = bendTone(nextPitch);

        /// PRE-BEND (+BEND, +RELEASE)
        if (pt == 0 && pitch != 0) {
            int prebendTone = bendTone(pitch);
            double minY = std::min(.0, src.y());
            dest = PointF(src.x(), minY - bendHeight(prebendTone) - baseBendHeight);
            if (!skipFirstPoint) {
                bool needsHeightUpdatePrebend = false;
                if (pitch == m_maxDrawPointRead) {
                    m_highestCoord = std::max(m_highestCoord, -dest.y());
                    needsHeightUpdatePrebend = true;
                }

                m_bendSegments.push_back({ src, dest, BendSegmentType::LINE_UP, prebendTone, true, needsHeightUpdatePrebend });
            }

            src.setY(dest.y());
        }

        bool visible = true;
        bool needsHeightUpdate = false;

        /// PRE-BEND - - -
        if (pitch == nextPitch) {
            if (pt == 0) {
                type = BendSegmentType::LINE_STROKED;
                if (pitch == m_maxDrawPointRead) {
                    m_highestCoord = std::max(m_highestCoord, -dest.y());
                    needsHeightUpdate = true;
                }
            }
        } else {
            bool bendUp = pitch < nextPitch;
            if (bendUp && isPrevBendUp && pt > 0) {
                // prevent double bendUp rendering
                int prevBendPitch = m_drawPoints.at(pt - 1);
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
                double minY = std::min(.0, src.y());
                dest.setY(minY - bendHeight(tone) - baseBendHeight);
                type = BendSegmentType::CURVE_UP;
                if (nextPitch == m_maxDrawPointRead) {
                    m_highestCoord = std::max(m_highestCoord, -dest.y());
                    needsHeightUpdate = true;
                }
            } else {
                // skipping extra curves down of bend-release in a chord
                if (pitch == m_maxDrawPointRead) {
                    if (m_note != m_chord->upNote()) {
                        visible = false;
                    }
                }

                if (releasedToInitial) {
                    dest.setY(notePos.y());
                } else {
                    dest.setY(src.y() + bendHeight(prevTone) + baseBendHeight);
                }

                type = BendSegmentType::CURVE_DOWN;
            }
        }
        // prevent double curve down rendering
        if (prevLineType == BendSegmentType::CURVE_DOWN && type == BendSegmentType::CURVE_DOWN) {
            continue;
        }

        if (type != BendSegmentType::NO_TYPE) {
            m_bendSegments.push_back({ src, dest, type, tone, visible, needsHeightUpdate });
        }

        src = dest;
        prevLineType = type;
        prevTone = tone;
    }
}

//---------------------------------------------------------
//   highestCoord
//---------------------------------------------------------

double StretchedBend::highestCoord() const
{
    return m_highestCoord;
}

//---------------------------------------------------------
//   updateHeights
//---------------------------------------------------------

void StretchedBend::updateHeights(double newHighestCoord)
{
    IF_ASSERT_FAILED(!m_bendSegments.empty()) {
        LOGE() << "invalid bend data";
        return;
    }

    // check tied back note
    StretchedBend* lastStretchedBend = backTiedStretchedBend();
    if (lastStretchedBend) {
        const auto& lastBendSegments = lastStretchedBend->m_bendSegments;
        if (!lastBendSegments.empty()) {
            const BendSegment& lastBendEndSegment = lastBendSegments.back();
            BendSegment& thisBendStartSegment = m_bendSegments.at(0);
            thisBendStartSegment.visible = lastBendEndSegment.visible;
            if (thisBendStartSegment.type == BendSegmentType::CURVE_DOWN
                && lastStretchedBend->m_note != lastStretchedBend->m_chord->upNote()) {
                thisBendStartSegment.visible = false;
            }

            if (lastBendEndSegment.needsHeightUpdate) {
                m_bendSegments.at(0).src.setY(lastBendEndSegment.dest.y());
            }
        }
    }

    for (size_t i = 0; i < m_bendSegments.size(); i++) {
        BendSegment& bendSegment = m_bendSegments.at(i);
        if (bendSegment.needsHeightUpdate) {
            double oldHeight = bendSegment.dest.y();
            bendSegment.dest.setY(-newHighestCoord);
            bendSegment.tone = bendTone(m_maxDrawPointUpdated);
            if (i < m_bendSegments.size() - 1) {
                BendSegment& nextBendSegment = m_bendSegments.at(i + 1);
                if (nextBendSegment.src.y() == oldHeight) {
                    nextBendSegment.src.setY(-newHighestCoord);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   fillStretchedSegments
//---------------------------------------------------------

void StretchedBend::fillStretchedSegments(bool untilNextSegment)
{
    if (m_bendSegments.empty()) {
        return;
    }

    double sp = spatium();
    double bendStart = m_bendSegments.at(0).src.x();
    double bendEnd = untilNextSegment ? nextSegmentX() : sp * 6;

    if (bendStart > bendEnd) {
        return;
    }

    m_bendSegmentsStretched = m_bendSegments;
    size_t segsSize = m_bendSegmentsStretched.size();

    m_bendSegmentsStretched.at(segsSize - 1).dest.setX(bendEnd);

    double step = (bendEnd - bendStart) / segsSize;

    for (auto i = segsSize - 1; i > 0; --i) {
        auto& lastSeg = m_bendSegmentsStretched.at(i);
        auto& prevSeg = m_bendSegmentsStretched.at(i - 1);
        if (lastSeg.type != BendSegmentType::LINE_UP && prevSeg.type != BendSegmentType::LINE_UP
            && lastSeg.type != BendSegmentType::LINE_STROKED) {
            bendEnd -= step;
            lastSeg.src.setX(bendEnd);
            prevSeg.dest.setX(bendEnd);
        }
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StretchedBend::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;

    setupPainter(painter);
    double sp = spatium();
    bool isTextDrawn = false;

    for (const BendSegment& bendSegment : m_bendSegmentsStretched) {
        if (!bendSegment.visible) {
            continue;
        }

        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = String::fromUtf8(label[bendSegment.tone]);

        switch (bendSegment.type) {
        case BendSegmentType::LINE_UP:
        {
            painter->drawLine(LineF(src, dest));
            painter->setBrush(curColor());
            painter->drawPolygon(m_arrows.up.translated(dest));
            /// TODO: remove substraction after fixing bRect
            drawText(painter, dest - PointF(0, sp * 0.5), text);
            break;
        }

        case BendSegmentType::CURVE_UP:
        case BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == BendSegmentType::CURVE_UP);
            double endY = dest.y() + m_arrows.width * (bendUp ? 1 : -1);

            PainterPath path = bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? m_arrows.up : m_arrows.down);

            painter->setBrush(BrushStyle::NoBrush);
            painter->drawPath(path);
            painter->setBrush(curColor());
            painter->drawPolygon(arrowPath.translated(dest));

            if (bendUp && !isTextDrawn) {
                /// TODO: remove subtraction after fixing bRect
                drawText(painter, dest - PointF(0, sp * 0.5), text);
                isTextDrawn = true;
            }

            break;
        }

        case BendSegmentType::LINE_STROKED:
        {
            PainterPath path;
            path.moveTo(src + PointF(m_arrows.width, 0));
            path.lineTo(dest);
            Pen p(painter->pen());
            p.setStyle(PenStyle::DashLine);
            painter->strokePath(path, p);
            break;
        }

        default:
            break;
        }
    }
}

mu::RectF StretchedBend::calculateBoundingRect() const
{
    RectF bRect;
    double sp = spatium();
    bool isTextDrawn = false;

    for (const BendSegment& bendSegment : m_bendSegmentsStretched) {
        const PointF& src = bendSegment.src;
        const PointF& dest = bendSegment.dest;
        const String& text = String::fromUtf8(label[bendSegment.tone]);

        switch (bendSegment.type) {
        case BendSegmentType::LINE_UP:
        {
            bRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
            bRect.unite(m_arrows.up.translated(dest).boundingRect());

            mu::draw::FontMetrics fm(font(sp));
            bRect.unite(textBoundingRect(fm, dest, text));
            /// TODO: remove after fixing bRect
            bRect.setHeight(bRect.height() + sp);
            bRect.setY(bRect.y() - sp);
            break;
        }

        case BendSegmentType::CURVE_UP:
        case BendSegmentType::CURVE_DOWN:
        {
            bool bendUp = (bendSegment.type == BendSegmentType::CURVE_UP);
            double endY = dest.y() + m_arrows.width * (bendUp ? 1 : -1);

            PainterPath path = bendCurveFromPoints(src, PointF(dest.x(), endY));
            const auto& arrowPath = (bendUp ? m_arrows.up : m_arrows.down);

            bRect.unite(path.boundingRect());
            bRect.unite(arrowPath.translated(dest).boundingRect());

            if (bendUp && !isTextDrawn) {
                mu::draw::FontMetrics fm(font(sp));
                bRect.unite(textBoundingRect(fm, dest - PointF(sp, 0), text));
                /// TODO: remove after fixing bRect
                bRect.setHeight(bRect.height() + sp);
                bRect.setY(bRect.y() - sp);
                isTextDrawn = true;
            }

            break;
        }

        case BendSegmentType::LINE_STROKED:
        {
            bRect.unite(RectF(src.x(), src.y(), dest.x() - src.x(), dest.y() - src.y()));
            break;
        }

        default:
            break;
        }
    }

    double lw = lineWidth();
    bRect.adjust(-lw, -lw, lw, lw);

    return bRect;
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
//   fillArrows
//---------------------------------------------------------

void StretchedBend::fillArrows(double width)
{
    if (m_arrows.width == width) {
        return;
    }

    double aw = 0;
    m_arrows.width = aw = width;

    m_arrows.up.clear();
    m_arrows.down.clear();

    m_arrows.up << PointF(0, 0) << PointF(aw * .5, aw) << PointF(-aw * .5, aw);
    m_arrows.down << PointF(0, 0) << PointF(aw * .5, -aw) << PointF(-aw * .5, -aw);
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
    Segment* nextSeg = toChord(parent())->segment()->nextInStaff(
        staffIdx(), SegmentType::ChordRest | SegmentType::BarLine | SegmentType::EndBarLine);
    if (!nextSeg) {
        return 0;
    }

    return nextSeg->pagePos().x() - pagePos().x() - spatium();
}

//---------------------------------------------------------
//   bendTone
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
    return spatium() * (bendIdx + 1) * BEND_HEIGHT_MULTIPLIER;
}

//---------------------------------------------------------
//   prepareBends
//---------------------------------------------------------

void StretchedBend::prepareBends(std::vector<StretchedBend*>& bends)
{
    for (StretchedBend* bend : bends) {
        bend->fillDrawPoints();
    }

    auto maxDrawPointIt = std::max_element(bends.begin(), bends.end(), [](StretchedBend* b1, StretchedBend* b2) {
        return b1->m_maxDrawPointRead < b2->m_maxDrawPointRead;
    });

    if (maxDrawPointIt != bends.end()) {
        int maxDrawPoint = (*maxDrawPointIt)->m_maxDrawPointRead;
        for (StretchedBend* bend: bends) {
            bend->m_maxDrawPointUpdated = maxDrawPoint;
        }
    }
}

//---------------------------------------------------------
//   backTiedStretchedBend
//---------------------------------------------------------

StretchedBend* StretchedBend::backTiedStretchedBend() const
{
    if (Tie* tie = m_note->tieBack()) {
        Note* backTied = tie->startNote();
        return backTied ? backTied->stretchedBend() : nullptr;
    }

    return nullptr;
}

//---------------------------------------------------------
//   firstPointShouldBeSkipped
//---------------------------------------------------------

bool StretchedBend::firstPointShouldBeSkipped() const
{
    StretchedBend* lastStretchedBend = backTiedStretchedBend();
    if (lastStretchedBend) {
        const auto& lastBendPoints = lastStretchedBend->m_drawPoints;
        if (!lastBendPoints.empty() && lastBendPoints.back() == m_drawPoints.at(0)) {
            return true;
        }
    }

    return false;
}
}
