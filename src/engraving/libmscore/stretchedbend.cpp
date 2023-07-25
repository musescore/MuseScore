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

StretchedBend::BendSegment::BendSegment()
{
}

StretchedBend::BendSegment::BendSegment(BendSegmentType bendType, int bendTone) : type(bendType), tone(bendTone)
{
}

void StretchedBend::BendSegment::setup(PointF s, PointF d, bool v)
{
    src = s;
    dest = d;
    visible = v;
}

void StretchedBend::addSegment(std::vector<StretchedBend::BendSegment>& bendSegments, StretchedBend::BendSegmentType type, int tone) const
{
    bendSegments.emplace_back(type, tone);
}

//---------------------------------------------------------
//   createBendSegments
//---------------------------------------------------------

void StretchedBend::createBendSegments()
{
    m_bendSegments.clear();

    if (m_pitchValues.size() < 2) {
        return;
    }

    IF_ASSERT_FAILED(m_note) {
        LOGE() << "note is not set";
        return;
    }

    bool isPrevBendUp = false;
    bool skipFirstPoint = firstPointShouldBeSkipped();

    BendSegmentType prevLineType = BendSegmentType::NO_TYPE;

    for (size_t pt = 0; pt < m_pitchValues.size() - 1; pt++) {
        int pitch = m_pitchValues.at(pt).pitch;
        int nextPitch = m_pitchValues.at(pt + 1).pitch;

        BendSegmentType type = BendSegmentType::NO_TYPE;

        /// PRE-BEND (+BEND, +RELEASE)
        if (pt == 0 && pitch != 0) {
            if (!skipFirstPoint) {
                addSegment(m_bendSegments, BendSegmentType::LINE_UP, bendTone(pitch));
            }
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
                int prevBendPitch = m_pitchValues.at(pt - 1).pitch;
                // remove prev segment if there are two bendup in a row
                if (!m_bendSegments.empty()) {
                    m_bendSegments.pop_back();
                    if (prevBendPitch > 0 && prevBendPitch < pitch && !m_bendSegments.empty()) {
                        m_bendSegments.back().tone = bendTone(0);
                    }
                }
            }

            isPrevBendUp = bendUp;
            type = (bendUp ? BendSegmentType::CURVE_UP : BendSegmentType::CURVE_DOWN);
        }

        // prevent double curve down rendering
        if (prevLineType == BendSegmentType::CURVE_DOWN && type == BendSegmentType::CURVE_DOWN) {
            continue;
        }

        if (type != BendSegmentType::NO_TYPE) {
            addSegment(m_bendSegments, type, bendTone(nextPitch));
        }

        prevLineType = type;
    }
}

//---------------------------------------------------------
//   fillSegments
//---------------------------------------------------------

void StretchedBend::fillSegments()
{
    IF_ASSERT_FAILED(!m_bendSegments.empty()) {
        LOGE() << "bend segments are empty";
        return;
    }

    double noteWidth = m_note->width();
    double noteHeight = m_note->height();
    PointF notePos = m_note->pos();
    double sp = spatium();
    PointF src;

    if (StretchedBend* tiedBend = backTiedStretchedBend()) {
        src = PointF(.0, tiedBend->m_bendSegments.back().dest.y());
    } else {
        PointF upBendDefaultSrc = PointF(noteWidth + sp * .8, 0) + notePos;
        PointF downBendDefaultSrc = PointF(noteWidth * .5, -noteHeight * .5 - sp * .2) + notePos;
        src = m_pitchValues.at(0).pitch == 0 ? upBendDefaultSrc : downBendDefaultSrc;
    }

    PointF dest(src);
    bool releasedToInitial = (0 == m_pitchValues.back().pitch);
    double baseBendHeight = sp * 1.5;

    for (size_t i = 0; i < m_bendSegments.size(); i++) {
        BendSegment& bendSegment = m_bendSegments.at(i);

        if (bendSegment.type == BendSegmentType::LINE_UP) {
            double minY = std::min(.0, src.y());
            dest = PointF(src.x(), minY - bendHeight(bendSegment.tone) - baseBendHeight);
            bendSegment.setup(src, dest);
            src.setY(dest.y());
            continue;
        }

        bool visible = true;

        if (bendSegment.type == BendSegmentType::CURVE_UP) {
            double minY = std::min(.0, src.y());
            dest.setY(minY - bendHeight(bendSegment.tone) - baseBendHeight);
        } else if (bendSegment.type == BendSegmentType::CURVE_DOWN) {
            // skipping extra curves down of bend-release in a chord
            if (false) { /// condition todo
                if (bendSegmentShouldBeHidden(this)) {
                    visible = false;
                }
            }

            if (releasedToInitial) {
                dest.setY(notePos.y());
            } else {
                int prevTone = ((i == 0) ? 0 : m_bendSegments.at(i - 1).tone);
                dest.setY(src.y() + bendHeight(prevTone) + baseBendHeight);
            }
        }

        bendSegment.setup(src, dest, visible);

        src = dest;
    }
}

//---------------------------------------------------------
//   updateHeights
//---------------------------------------------------------

void StretchedBend::updateHeights()
{
    IF_ASSERT_FAILED(!m_bendSegments.empty()) {
        LOGE() << "invalid bend data";
        return;
    }

    for (size_t i = 0; i < m_bendSegments.size(); i++) {

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

    double upNotePos = m_chord->upNote()->pageX() + m_chord->upNote()->width();
    double curNotePos = m_note->pageX() + m_note->width();

    bendEnd += upNotePos - curNotePos;

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

bool StretchedBend::bendSegmentShouldBeHidden(StretchedBend* bendSegment) const
{
    /// TODO: если пофикшу определение верхней ноты, возможно уйдет необходимость доп фиксов
    Note* currentNote = bendSegment->m_note;
    Note* upNote = bendSegment->m_chord->upNote();
    return (currentNote->pitch() != upNote->pitch()) || (currentNote->string() != upNote->string());
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
        // filling segments after all ties are connected
        bend->createBendSegments();
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
        const auto& lastBendPoints = lastStretchedBend->m_pitchValues;
        if (!lastBendPoints.empty() && lastBendPoints.back().pitch == m_pitchValues.at(0).pitch) {
            return true;
        }
    }

    return false;
}
}
