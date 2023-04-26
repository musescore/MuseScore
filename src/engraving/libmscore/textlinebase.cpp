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

#include "textlinebase.h"

#include <cmath>

#include "draw/types/pen.h"

#include "style/style.h"

#include "factory.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   TextLineBaseSegment
//---------------------------------------------------------

TextLineBaseSegment::TextLineBaseSegment(const ElementType& type, Spanner* sp, System* parent, ElementFlags f)
    : LineSegment(type, sp, parent, f)
{
    _text    = Factory::createText(this, TextStyleType::DEFAULT, false);
    _endText = Factory::createText(this, TextStyleType::DEFAULT, false);
    _text->setParent(this);
    _endText->setParent(this);
    _text->setFlag(ElementFlag::MOVABLE, false);
    _endText->setFlag(ElementFlag::MOVABLE, false);
}

TextLineBaseSegment::TextLineBaseSegment(const TextLineBaseSegment& seg)
    : LineSegment(seg)
{
    _text    = seg._text->clone();
    _endText = seg._endText->clone();
    _text->setParent(this);
    _endText->setParent(this);
    layout();      // set the right _text
}

TextLineBaseSegment::~TextLineBaseSegment()
{
    delete _text;
    delete _endText;
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineBaseSegment::setSelected(bool f)
{
    SpannerSegment::setSelected(f);
    _text->setSelected(f);
    _endText->setSelected(f);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

static std::vector<double> distributedDashPattern(double dash, double gap, double lineLength)
{
    int numPairs = std::max(1.0, lineLength / (dash + gap));
    double newGap = (lineLength - dash * (numPairs + 1)) / numPairs;

    return { dash, newGap };
}

void TextLineBaseSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    TextLineBase* tl   = textLineBase();

    if (!_text->empty()) {
        painter->translate(_text->pos());
        _text->setVisible(tl->visible());
        _text->draw(painter);
        painter->translate(-_text->pos());
    }

    if (!_endText->empty()) {
        painter->translate(_endText->pos());
        _endText->setVisible(tl->visible());
        _endText->draw(painter);
        painter->translate(-_endText->pos());
    }

    if ((npoints == 0) || (score() && (score()->printing() || !score()->showInvisible()) && !tl->lineVisible())) {
        return;
    }

    // color for line (text color comes from the text properties)
    Color color = curColor(tl->visible() && tl->lineVisible(), tl->lineColor());

    double lineWidth = tl->lineWidth() * mag();

    const Pen solidPen(color, lineWidth, PenStyle::SolidLine, PenCapStyle::FlatCap, PenJoinStyle::MiterJoin);
    Pen pen(solidPen);

    double dash = 0;
    double gap = 0;

    switch (tl->lineStyle()) {
    case LineType::SOLID:
        break;
    case LineType::DASHED:
        dash = tl->dashLineLen(), gap = tl->dashGapLen();
        break;
    case LineType::DOTTED:
        dash = 0.01, gap = 1.99;
        pen.setCapStyle(PenCapStyle::RoundCap); // round dots
        break;
    }

    const bool isNonSolid = tl->lineStyle() != LineType::SOLID;

    // Draw lines
    if (twoLines) { // hairpins
        if (isNonSolid) {
            pen.setDashPattern({ dash, gap });
        }

        painter->setPen(pen);
        painter->drawLines(&points[0], 1);
        painter->drawLines(&points[2], 1);
        return;
    }

    int start = 0, end = npoints;

    // Draw begin hook, if it needs to be drawn separately
    if (isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
        bool isTHook = tl->beginHookType() == HookType::HOOK_90T;

        if (isNonSolid || isTHook) {
            const PointF& p1 = points[start++];
            const PointF& p2 = points[start++];

            if (isTHook) {
                painter->setPen(solidPen);
            } else {
                double hookLength = sqrt(PointF::dotProduct(p2 - p1, p2 - p1));
                pen.setDashPattern(distributedDashPattern(dash, gap, hookLength / lineWidth));
                painter->setPen(pen);
            }

            painter->drawLine(p1, p2);
        }
    }

    // Draw end hook, if it needs to be drawn separately
    if (isSingleEndType() && tl->endHookType() != HookType::NONE) {
        bool isTHook = tl->endHookType() == HookType::HOOK_90T;

        if (isNonSolid || isTHook) {
            const PointF& p1 = points[--end];
            const PointF& p2 = points[--end];

            if (isTHook) {
                painter->setPen(solidPen);
            } else {
                double hookLength = sqrt(PointF::dotProduct(p2 - p1, p2 - p1));
                pen.setDashPattern(distributedDashPattern(dash, gap, hookLength / lineWidth));
                painter->setPen(pen);
            }

            painter->drawLine(p1, p2);
        }
    }

    // Draw the rest
    if (isNonSolid) {
        pen.setDashPattern(distributedDashPattern(dash, gap, lineLength / lineWidth));
    }

    painter->setPen(pen);
    painter->drawPolyline(&points[start], end - start);
}

static RectF boundingBoxOfLine(const PointF& p1, const PointF& p2, double lw2, bool isDottedLine)
{
    if (isDottedLine) {
        return RectF(p1, p2).normalized().adjusted(-lw2, -lw2, lw2, lw2);
    }

    PointF a = lw2 * (p2 - p1).normalized();
    PointF b(-a.y(), a.x());
    return RectF(p1 - b, p1 + b).normalized().united(RectF(p2 - b, p2 + b).normalized());
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TextLineBaseSegment::shape() const
{
    Shape shape;
    if (!_text->empty()) {
        shape.add(_text->bbox().translated(_text->pos()));
    }
    if (!_endText->empty()) {
        shape.add(_endText->bbox().translated(_endText->pos()));
    }
    double lw2 = 0.5 * textLineBase()->lineWidth();
    bool isDottedLine = textLineBase()->lineStyle() == LineType::DOTTED;
    if (twoLines) {     // hairpins
        shape.add(boundingBoxOfLine(points[0], points[1], lw2, isDottedLine));
        shape.add(boundingBoxOfLine(points[2], points[3], lw2, isDottedLine));
    } else if (textLineBase()->lineVisible()) {
        for (int i = 0; i < npoints - 1; ++i) {
            shape.add(boundingBoxOfLine(points[i], points[i + 1], lw2, isDottedLine));
        }
    }
    return shape;
}

bool TextLineBaseSegment::setProperty(Pid id, const PropertyValue& v)
{
    if (id == Pid::COLOR) {
        mu::draw::Color color = v.value<mu::draw::Color>();

        if (_text) {
            _text->setColor(color);
        }

        if (_endText) {
            _endText->setColor(color);
        }
    }

    return LineSegment::setProperty(id, v);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

// Extends lines to fill the corner between them.
// Assumes that l1p2 == l2p1 is the intersection between the lines.
// If checkAngle is false, assumes that the lines are perpendicular,
// and some calculations are saved.
static inline void extendLines(const PointF& l1p1, PointF& l1p2, PointF& l2p1, const PointF& l2p2, double lineWidth, bool checkAngle)
{
    PointF l1UnitVector = (l1p2 - l1p1).normalized();
    PointF l2UnitVector = (l2p1 - l2p2).normalized();

    double addedLength = lineWidth * 0.5;

    if (checkAngle) {
        double angle = M_PI - acos(PointF::dotProduct(l1UnitVector, l2UnitVector));

        if (angle <= M_PI_2) {
            addedLength *= tan(0.5 * angle);
        }
    }

    l1p2 += l1UnitVector * addedLength;
    l2p1 += l2UnitVector * addedLength;
}

void TextLineBaseSegment::layout()
{
    npoints      = 0;
    TextLineBase* tl = textLineBase();
    double _spatium = tl->spatium();

    if (spanner()->placeBelow()) {
        setPosY(staff() ? staff()->height() : 0.0);
    }

    // adjust Y pos to staffType offset
    if (const StaffType* st = staffType()) {
        movePosY(st->yoffset().val() * spatium());
    }

    if (!tl->diagonal()) {
        _offset2.setY(0);
    }

    auto alignBaseLine = [tl](Text* text, PointF& pp1, PointF& pp2) {
        PointF widthCorrection(0.0, tl->lineWidth() / 2);
        switch (text->align().vertical) {
        case AlignV::TOP:
            pp1 += widthCorrection;
            pp2 += widthCorrection;
            break;
        case AlignV::VCENTER:
            break;
        case AlignV::BOTTOM:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        case AlignV::BASELINE:
            pp1 -= widthCorrection;
            pp2 -= widthCorrection;
            break;
        }
    };

    switch (spannerSegmentType()) {
    case SpannerSegmentType::SINGLE:
    case SpannerSegmentType::BEGIN:
        _text->setXmlText(tl->beginText());
        _text->setFamily(tl->beginFontFamily());
        _text->setSize(tl->beginFontSize());
        _text->setOffset(tl->beginTextOffset() * mag());
        _text->setAlign(tl->beginTextAlign());
        _text->setFontStyle(tl->beginFontStyle());
        break;
    case SpannerSegmentType::MIDDLE:
    case SpannerSegmentType::END:
        _text->setXmlText(tl->continueText());
        _text->setFamily(tl->continueFontFamily());
        _text->setSize(tl->continueFontSize());
        _text->setOffset(tl->continueTextOffset() * mag());
        _text->setAlign(tl->continueTextAlign());
        _text->setFontStyle(tl->continueFontStyle());
        break;
    }
    _text->setPlacement(PlacementV::ABOVE);
    _text->setTrack(track());
    _text->layout();

    if ((isSingleType() || isEndType())) {
        _endText->setXmlText(tl->endText());
        _endText->setFamily(tl->endFontFamily());
        _endText->setSize(tl->endFontSize());
        _endText->setOffset(tl->endTextOffset());
        _endText->setAlign(tl->endTextAlign());
        _endText->setFontStyle(tl->endFontStyle());
        _endText->setPlacement(PlacementV::ABOVE);
        _endText->setTrack(track());
        _endText->layout();
    } else {
        _endText->setXmlText(u"");
    }

    if (!textLineBase()->textSizeSpatiumDependent()) {
        _text->setSize(_text->size() * SPATIUM20 / spatium());
        _endText->setSize(_endText->size() * SPATIUM20 / spatium());
    }

    PointF pp1;
    PointF pp2(pos2());

    // line with no text or hooks - just use the basic rectangle for line
    if (_text->empty() && _endText->empty()
        && (!isSingleBeginType() || tl->beginHookType() == HookType::NONE)
        && (!isSingleEndType() || tl->endHookType() == HookType::NONE)) {
        npoints = 2;
        points[0] = pp1;
        points[1] = pp2;
        lineLength = sqrt(PointF::dotProduct(pp2 - pp1, pp2 - pp1));

        setbbox(boundingBoxOfLine(pp1, pp2, tl->lineWidth() / 2, tl->lineStyle() == LineType::DOTTED));
        return;
    }

    // line has text or hooks or is not diagonal - calculate reasonable bbox

    double x1 = std::min(0.0, pp2.x());
    double x2 = std::max(0.0, pp2.x());
    double y0 = -tl->lineWidth();
    double y1 = std::min(0.0, pp2.y()) + y0;
    double y2 = std::max(0.0, pp2.y()) - y0;

    double l = 0.0;
    if (!_text->empty()) {
        double gapBetweenTextAndLine = _spatium * tl->gapBetweenTextAndLine().val();
        if ((isSingleBeginType() && (tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO))
            || (!isSingleBeginType() && (tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO))) {
            l = _text->pos().x() + _text->bbox().width() + gapBetweenTextAndLine;
        }

        double h = _text->height();
        if (tl->beginTextPlace() == TextPlace::ABOVE) {
            y1 = std::min(y1, -h);
        } else if (tl->beginTextPlace() == TextPlace::BELOW) {
            y2 = std::max(y2, h);
        } else {
            y1 = std::min(y1, -h * .5);
            y2 = std::max(y2, h * .5);
        }
        x2 = std::max(x2, _text->width());
    }

    if (tl->endHookType() != HookType::NONE) {
        double h = pp2.y() + tl->endHookHeight().val() * _spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }

    if (tl->beginHookType() != HookType::NONE) {
        double h = tl->beginHookHeight().val() * _spatium;
        if (h > y2) {
            y2 = h;
        } else if (h < y1) {
            y1 = h;
        }
    }
    bbox().setRect(x1, y1, x2 - x1, y2 - y1);
    if (!_text->empty()) {
        bbox() |= _text->bbox().translated(_text->pos());      // DEBUG
    }
    // set end text position and extend bbox
    if (!_endText->empty()) {
        _endText->movePosX(bbox().right());
        bbox() |= _endText->bbox().translated(_endText->pos());
    }

    if (!(tl->lineVisible() || score()->showInvisible())) {
        return;
    }

    if (tl->lineVisible() || !score()->printing()) {
        pp1 = PointF(l, 0.0);

        // Make sure baseline of text and line are properly aligned (accounting for line thickness)
        bool alignBeginText = tl->beginTextPlace() == TextPlace::LEFT || tl->beginTextPlace() == TextPlace::AUTO;
        bool alignContinueText = tl->continueTextPlace() == TextPlace::LEFT || tl->continueTextPlace() == TextPlace::AUTO;
        bool alignEndText = tl->endTextPlace() == TextPlace::LEFT || tl->endTextPlace() == TextPlace::AUTO;
        bool isSingleOrBegin = isSingleBeginType();
        bool hasBeginText = !_text->empty() && isSingleOrBegin;
        bool hasContinueText = !_text->empty() && !isSingleOrBegin;
        bool hasEndText = !_endText->empty() && isSingleEndType();
        if ((hasBeginText && alignBeginText) || (hasContinueText && alignContinueText)) {
            alignBaseLine(_text, pp1, pp2);
        } else if (hasEndText && alignEndText) {
            alignBaseLine(_endText, pp1, pp2);
        }

        double beginHookHeight = tl->beginHookHeight().val() * _spatium;
        double endHookHeight = tl->endHookHeight().val() * _spatium;
        double beginHookWidth = 0.0;
        double endHookWidth = 0.0;

        if (tl->beginHookType() == HookType::HOOK_45) {
            beginHookWidth = fabs(beginHookHeight * .4);
            pp1.rx() += beginHookWidth;
        }

        if (tl->endHookType() == HookType::HOOK_45) {
            endHookWidth = fabs(endHookHeight * .4);
            pp2.rx() -= endHookWidth;
        }

        // don't draw backwards lines (or hooks) if text is longer than nominal line length
        if (!_text->empty() && pp1.x() > pp2.x() && !tl->diagonal()) {
            return;
        }

        if (isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
            // We use the term "endpoint" for the point that does not touch the main line.
            const PointF& beginHookEndpoint = points[npoints++] = PointF(pp1.x() - beginHookWidth, pp1.y() + beginHookHeight);

            if (tl->beginHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                points[npoints++] = PointF(pp1.x() - beginHookWidth, pp1.y() - beginHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& beginHookStartpoint = points[npoints++] = pp1;

                if (tl->lineStyle() == LineType::DASHED) {
                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    bool checkAngle = tl->beginHookType() == HookType::HOOK_45 || tl->diagonal();
                    extendLines(beginHookEndpoint, beginHookStartpoint, pp1, pp2, tl->lineWidth() * mag(), checkAngle);
                }
            }
        }

        points[npoints++] = pp1;
        PointF& pp22 = points[npoints++] = pp2; // Keep a reference so that we can modify later

        if (isSingleEndType() && tl->endHookType() != HookType::NONE) {
            const PointF endHookEndpoint = PointF(pp2.x() + endHookWidth, pp2.y() + endHookHeight);

            if (tl->endHookType() == HookType::HOOK_90T) {
                // A T-hook needs to be drawn separately, so we add an extra point
                points[npoints++] = PointF(pp2.x() + endHookWidth, pp2.y() - endHookHeight);
            } else if (tl->lineStyle() != LineType::SOLID) {
                // For non-solid lines, we also draw the hook separately,
                // so that we can distribute the dashes/dots for each linepiece individually
                PointF& endHookStartpoint = points[npoints++] = pp2;

                if (tl->lineStyle() == LineType::DASHED) {
                    bool checkAngle = tl->endHookType() == HookType::HOOK_45 || tl->diagonal();

                    // For dashes lines, we extend the lines somewhat,
                    // so that the corner between them gets filled
                    extendLines(pp1, pp22, endHookStartpoint, endHookEndpoint, tl->lineWidth() * mag(), checkAngle);
                }
            }

            points[npoints++] = endHookEndpoint;
        }

        lineLength = sqrt(PointF::dotProduct(pp22 - pp1, pp22 - pp1));
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineBaseSegment::spatiumChanged(double ov, double nv)
{
    LineSegment::spatiumChanged(ov, nv);

    textLineBase()->spatiumChanged(ov, nv);
    _text->spatiumChanged(ov, nv);
    _endText->spatiumChanged(ov, nv);
}

static constexpr std::array<Pid, 27> TextLineBasePropertyId = { {
    Pid::LINE_VISIBLE,
    Pid::BEGIN_HOOK_TYPE,
    Pid::BEGIN_HOOK_HEIGHT,
    Pid::END_HOOK_TYPE,
    Pid::END_HOOK_HEIGHT,
    Pid::GAP_BETWEEN_TEXT_AND_LINE,
    Pid::BEGIN_TEXT,
    Pid::BEGIN_TEXT_ALIGN,
    Pid::BEGIN_TEXT_PLACE,
    Pid::BEGIN_FONT_FACE,
    Pid::BEGIN_FONT_SIZE,
    Pid::BEGIN_FONT_STYLE,
    Pid::BEGIN_TEXT_OFFSET,
    Pid::CONTINUE_TEXT,
    Pid::CONTINUE_TEXT_ALIGN,
    Pid::CONTINUE_TEXT_PLACE,
    Pid::CONTINUE_FONT_FACE,
    Pid::CONTINUE_FONT_SIZE,
    Pid::CONTINUE_FONT_STYLE,
    Pid::CONTINUE_TEXT_OFFSET,
    Pid::END_TEXT,
    Pid::END_TEXT_ALIGN,
    Pid::END_TEXT_PLACE,
    Pid::END_FONT_FACE,
    Pid::END_FONT_SIZE,
    Pid::END_FONT_STYLE,
    Pid::END_TEXT_OFFSET,
} };

const std::array<Pid, 27>& TextLineBase::textLineBasePropertyIds()
{
    return TextLineBasePropertyId;
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* TextLineBaseSegment::propertyDelegate(Pid pid)
{
    for (Pid id : TextLineBasePropertyId) {
        if (pid == id) {
            return spanner();
        }
    }
    return LineSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   TextLineBase
//---------------------------------------------------------

TextLineBase::TextLineBase(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : SLine(type, parent, f)
{
    setBeginHookHeight(Spatium(1.9));
    setEndHookHeight(Spatium(1.9));
    setGapBetweenTextAndLine(Spatium(0.5));
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineBase::spatiumChanged(double /*ov*/, double /*nv*/)
{
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TextLineBase::getProperty(Pid id) const
{
    switch (id) {
    case Pid::BEGIN_TEXT:
        return beginText();
    case Pid::BEGIN_TEXT_ALIGN:
        return PropertyValue::fromValue(beginTextAlign());
    case Pid::CONTINUE_TEXT_ALIGN:
        return PropertyValue::fromValue(continueTextAlign());
    case Pid::END_TEXT_ALIGN:
        return PropertyValue::fromValue(endTextAlign());
    case Pid::BEGIN_TEXT_PLACE:
        return _beginTextPlace;
    case Pid::BEGIN_HOOK_TYPE:
        return _beginHookType;
    case Pid::BEGIN_HOOK_HEIGHT:
        return _beginHookHeight;
    case Pid::BEGIN_FONT_FACE:
        return _beginFontFamily;
    case Pid::BEGIN_FONT_SIZE:
        return _beginFontSize;
    case Pid::BEGIN_FONT_STYLE:
        return int(_beginFontStyle);
    case Pid::BEGIN_TEXT_OFFSET:
        return _beginTextOffset;
    case Pid::CONTINUE_TEXT:
        return continueText();
    case Pid::CONTINUE_TEXT_PLACE:
        return _continueTextPlace;
    case Pid::CONTINUE_FONT_FACE:
        return _continueFontFamily;
    case Pid::CONTINUE_FONT_SIZE:
        return _continueFontSize;
    case Pid::CONTINUE_FONT_STYLE:
        return int(_continueFontStyle);
    case Pid::CONTINUE_TEXT_OFFSET:
        return _continueTextOffset;
    case Pid::END_TEXT:
        return endText();
    case Pid::END_TEXT_PLACE:
        return _endTextPlace;
    case Pid::END_HOOK_TYPE:
        return _endHookType;
    case Pid::END_HOOK_HEIGHT:
        return _endHookHeight;
    case Pid::GAP_BETWEEN_TEXT_AND_LINE:
        return _gapBetweenTextAndLine;
    case Pid::END_FONT_FACE:
        return _endFontFamily;
    case Pid::END_FONT_SIZE:
        return _endFontSize;
    case Pid::END_FONT_STYLE:
        return int(_endFontStyle);
    case Pid::END_TEXT_OFFSET:
        return _endTextOffset;
    case Pid::LINE_VISIBLE:
        return lineVisible();
    case Pid::TEXT_SIZE_SPATIUM_DEPENDENT:
        return textSizeSpatiumDependent();
    default:
        return SLine::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TextLineBase::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::BEGIN_TEXT_PLACE:
        _beginTextPlace = v.value<TextPlace>();
        break;
    case Pid::BEGIN_TEXT_ALIGN:
        _beginTextAlign = v.value<Align>();
        break;
    case Pid::CONTINUE_TEXT_ALIGN:
        _continueTextAlign = v.value<Align>();
        break;
    case Pid::END_TEXT_ALIGN:
        _endTextAlign = v.value<Align>();
        break;
    case Pid::CONTINUE_TEXT_PLACE:
        _continueTextPlace = v.value<TextPlace>();
        break;
    case Pid::END_TEXT_PLACE:
        _endTextPlace = v.value<TextPlace>();
        break;
    case Pid::BEGIN_HOOK_HEIGHT:
        _beginHookHeight = v.value<Spatium>();
        break;
    case Pid::END_HOOK_HEIGHT:
        _endHookHeight = v.value<Spatium>();
        break;
    case Pid::BEGIN_HOOK_TYPE:
        _beginHookType = v.value<HookType>();
        break;
    case Pid::END_HOOK_TYPE:
        _endHookType = v.value<HookType>();
        break;
    case Pid::BEGIN_TEXT:
        setBeginText(v.value<String>());
        break;
    case Pid::BEGIN_TEXT_OFFSET:
        setBeginTextOffset(v.value<PointF>());
        break;
    case Pid::GAP_BETWEEN_TEXT_AND_LINE:
        _gapBetweenTextAndLine = v.value<Spatium>();
        break;
    case Pid::CONTINUE_TEXT_OFFSET:
        setContinueTextOffset(v.value<PointF>());
        break;
    case Pid::END_TEXT_OFFSET:
        setEndTextOffset(v.value<PointF>());
        break;
    case Pid::CONTINUE_TEXT:
        setContinueText(v.value<String>());
        break;
    case Pid::END_TEXT:
        setEndText(v.value<String>());
        break;
    case Pid::LINE_VISIBLE:
        setLineVisible(v.toBool());
        break;
    case Pid::BEGIN_FONT_FACE:
        setBeginFontFamily(v.value<String>());
        break;
    case Pid::BEGIN_FONT_SIZE:
        if (v.toReal() <= 0) {
            ASSERT_X(String(u"font size is %1").arg(v.toReal()));
        }
        setBeginFontSize(v.toReal());
        break;
    case Pid::BEGIN_FONT_STYLE:
        setBeginFontStyle(FontStyle(v.toInt()));
        break;
    case Pid::CONTINUE_FONT_FACE:
        setContinueFontFamily(v.value<String>());
        break;
    case Pid::CONTINUE_FONT_SIZE:
        setContinueFontSize(v.toReal());
        break;
    case Pid::CONTINUE_FONT_STYLE:
        setContinueFontStyle(FontStyle(v.toInt()));
        break;
    case Pid::END_FONT_FACE:
        setEndFontFamily(v.value<String>());
        break;
    case Pid::END_FONT_SIZE:
        setEndFontSize(v.toReal());
        break;
    case Pid::END_FONT_STYLE:
        setEndFontStyle(FontStyle(v.toInt()));
        break;
    case Pid::TEXT_SIZE_SPATIUM_DEPENDENT:
        setTextSizeSpatiumDependent(v.toBool());
        break;
    default:
        return SLine::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

mu::engraving::PropertyValue TextLineBase::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GAP_BETWEEN_TEXT_AND_LINE:
        return Spatium(0.5);
    default:
        return SLine::propertyDefault(propertyId);
    }
}
}
