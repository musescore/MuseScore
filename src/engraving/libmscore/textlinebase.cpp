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

#include "layout/v0/tlayout.h"

#include "factory.h"
#include "score.h"
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
    m_text    = Factory::createText(this, TextStyleType::DEFAULT, false);
    m_endText = Factory::createText(this, TextStyleType::DEFAULT, false);
    m_text->setParent(this);
    m_endText->setParent(this);
    m_text->setFlag(ElementFlag::MOVABLE, false);
    m_endText->setFlag(ElementFlag::MOVABLE, false);
}

TextLineBaseSegment::TextLineBaseSegment(const TextLineBaseSegment& seg)
    : LineSegment(seg)
{
    m_text    = seg.m_text->clone();
    m_endText = seg.m_endText->clone();
    m_text->setParent(this);
    m_endText->setParent(this);
    // set the right _text
    layout()->layoutTextLineBaseSegment(this);
}

TextLineBaseSegment::~TextLineBaseSegment()
{
    delete m_text;
    delete m_endText;
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineBaseSegment::setSelected(bool f)
{
    SpannerSegment::setSelected(f);
    m_text->setSelected(f);
    m_endText->setSelected(f);
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

    if (!m_text->empty()) {
        painter->translate(m_text->pos());
        m_text->setVisible(tl->visible());
        m_text->draw(painter);
        painter->translate(-m_text->pos());
    }

    if (!m_endText->empty()) {
        painter->translate(m_endText->pos());
        m_endText->setVisible(tl->visible());
        m_endText->draw(painter);
        painter->translate(-m_endText->pos());
    }

    if ((m_npoints == 0) || (score() && (score()->printing() || !score()->showInvisible()) && !tl->lineVisible())) {
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
    if (m_twoLines) { // hairpins
        if (isNonSolid) {
            pen.setDashPattern({ dash, gap });
        }

        painter->setPen(pen);
        painter->drawLines(&m_points[0], 1);
        painter->drawLines(&m_points[2], 1);
        return;
    }

    int start = 0, end = m_npoints;

    // Draw begin hook, if it needs to be drawn separately
    if (isSingleBeginType() && tl->beginHookType() != HookType::NONE) {
        bool isTHook = tl->beginHookType() == HookType::HOOK_90T;

        if (isNonSolid || isTHook) {
            const PointF& p1 = m_points[start++];
            const PointF& p2 = m_points[start++];

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
            const PointF& p1 = m_points[--end];
            const PointF& p2 = m_points[--end];

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
        pen.setDashPattern(distributedDashPattern(dash, gap, m_lineLength / lineWidth));
    }

    painter->setPen(pen);
    painter->drawPolyline(&m_points[start], end - start);
}

RectF TextLineBaseSegment::boundingBoxOfLine(const PointF& p1, const PointF& p2, double lw2, bool isDottedLine)
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
    if (!m_text->empty()) {
        shape.add(m_text->bbox().translated(m_text->pos()));
    }
    if (!m_endText->empty()) {
        shape.add(m_endText->bbox().translated(m_endText->pos()));
    }
    double lw2 = 0.5 * textLineBase()->lineWidth();
    bool isDottedLine = textLineBase()->lineStyle() == LineType::DOTTED;
    if (m_twoLines) {     // hairpins
        shape.add(boundingBoxOfLine(m_points[0], m_points[1], lw2, isDottedLine));
        shape.add(boundingBoxOfLine(m_points[2], m_points[3], lw2, isDottedLine));
    } else if (textLineBase()->lineVisible()) {
        for (int i = 0; i < m_npoints - 1; ++i) {
            shape.add(boundingBoxOfLine(m_points[i], m_points[i + 1], lw2, isDottedLine));
        }
    }
    return shape;
}

bool TextLineBaseSegment::setProperty(Pid id, const PropertyValue& v)
{
    if (id == Pid::COLOR) {
        mu::draw::Color color = v.value<mu::draw::Color>();

        if (m_text) {
            m_text->setColor(color);
        }

        if (m_endText) {
            m_endText->setColor(color);
        }
    }

    return LineSegment::setProperty(id, v);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineBaseSegment::spatiumChanged(double ov, double nv)
{
    LineSegment::spatiumChanged(ov, nv);

    textLineBase()->spatiumChanged(ov, nv);
    m_text->spatiumChanged(ov, nv);
    m_endText->spatiumChanged(ov, nv);
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
