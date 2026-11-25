/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <algorithm>
#include <cmath>

#include "rendering/iscorerenderer.h"

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

TextLineBaseSegment::TextLineBaseSegment(const ElementType& type, TextLineBase* sp, ElementFlags f)
    : LineSegment(type, sp, f)
{
    m_text    = Factory::createText(this, TextStyleType::DEFAULT, false);
    m_middleText = Factory::createText(this, TextStyleType::DEFAULT, false);
    m_endText = Factory::createText(this, TextStyleType::DEFAULT, false);
    m_text->setFlag(ElementFlag::MOVABLE, false);
    m_middleText->setFlag(ElementFlag::MOVABLE, false);
    m_endText->setFlag(ElementFlag::MOVABLE, false);
}

TextLineBaseSegment::TextLineBaseSegment(const TextLineBaseSegment& seg)
    : LineSegment(seg)
{
    m_text    = seg.m_text->clone();
    m_middleText = seg.m_middleText->clone();
    m_endText = seg.m_endText->clone();
    m_text->setOwnershipParent(this);
    m_middleText->setOwnershipParent(this);
    m_endText->setOwnershipParent(this);
    // set the right _text
    renderer()->layoutTextLineBaseSegment(this);
}

TextLineBaseSegment::~TextLineBaseSegment()
{
    delete m_text;
    delete m_middleText;
    delete m_endText;
}

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineBaseSegment::setSelected(bool f)
{
    SpannerSegment::setSelected(f);
    m_text->setSelected(f);
    m_middleText->setSelected(f);
    m_endText->setSelected(f);
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

void TextLineBaseSegment::layoutMiddleText(PointF& start, PointF& end, Shape& shape)
{
    LayoutData* ldata = mutldata();
    ldata->setMask(Shape());
    ldata->gapIndex = -1;
    ldata->splitMiddleLine = false;
    ldata->hideBegin = false;
    ldata->hideEnd = false;
    if (m_middleText->empty()) {
        return;
    }

    const TextLineBase* tl = textLineBase();
    m_middleText->mutldata()->setPos((start + end) * 0.5 - m_middleText->ldata()->bbox().center());
    const RectF textBounds = m_middleText->ldata()->bbox().translated(m_middleText->pos());
    shape.add(textBounds, m_middleText);
    if (!tl->middleTextCutThrough()) {
        return;
    }

    const double padding = tl->absoluteFromSpatium(tl->gapBetweenTextAndLine());
    const RectF mask = textBounds.padded(padding);
    // For middle text cutting through lines:
    // We normally use two additional points to creat a break at the center,
    // but it doesn't work for hairpins, so we use masks.
    if (twoLines()) {
        ldata->setMask(Shape(mask));
    }

    // Find the part of the line inside the mask, in unit of fraction of the line.
    const PointF lineVector = end - start;
    double maskEntryFraction = 0.0;
    double maskExitFraction = 1.0;
    auto isClipping = [&maskEntryFraction, &maskExitFraction](double lineOrigin, double lineDelta,
                                                              double maskStart, double maskEnd) -> bool {
        if (muse::RealIsNull(lineDelta)) {
            return lineOrigin >= maskStart && lineOrigin <= maskEnd;
        }
        double axisEntryFraction = (maskStart - lineOrigin) / lineDelta;
        double axisExitFraction = (maskEnd - lineOrigin) / lineDelta;
        if (axisEntryFraction > axisExitFraction) {
            std::swap(axisEntryFraction, axisExitFraction);
        }
        // only count a segment as masked when both axes report it masked.
        maskEntryFraction = std::max(maskEntryFraction, axisEntryFraction);
        maskExitFraction = std::min(maskExitFraction, axisExitFraction);
        return maskEntryFraction <= maskExitFraction;
    };
    if (!isClipping(start.x(), lineVector.x(), mask.left(), mask.right())
        || !isClipping(start.y(), lineVector.y(), mask.top(), mask.bottom())) {
        return;
    }

    // Hide the hook if needed, and move the line start/end point (thus also hiding the start/end segment).
    const double hideThreshold = std::max(0.0, tl->absoluteFromSpatium(tl->middleTextMinLineLength()));
    const double lineLength = std::hypot(lineVector.x(), lineVector.y());
    ldata->hideBegin = maskEntryFraction == 0.0 || lineLength * maskEntryFraction < hideThreshold;
    ldata->hideEnd = maskExitFraction == 1.0 || lineLength * (1.0 - maskExitFraction) < hideThreshold;
    const PointF origin = start;
    ldata->middleGapStart = origin + lineVector * maskEntryFraction;
    ldata->middleGapEnd = origin + lineVector * maskExitFraction;
    ldata->splitMiddleLine = !twoLines() && !ldata->hideBegin && !ldata->hideEnd;
    if (ldata->hideBegin) {
        start = twoLines() ? ldata->middleGapStart : ldata->middleGapEnd;
    }
    if (ldata->hideEnd) {
        end = twoLines() ? ldata->middleGapEnd : ldata->middleGapStart;
    }
}

void TextLineBaseSegment::splitMiddleTextLine(int mainLineStart)
{
    LayoutData* ldata = mutldata();
    if (!ldata->splitMiddleLine) {
        return;
    }

    const int insert = mainLineStart + 1;
    for (int i = ldata->npoints - 1; i >= insert; --i) {
        ldata->points[i + 2] = ldata->points[i];
    }
    ldata->points[insert] = ldata->middleGapStart;
    ldata->points[insert + 1] = ldata->middleGapEnd;
    ldata->npoints += 2;
    ldata->gapIndex = insert;
}

bool TextLineBaseSegment::isMiddleTextCramped() const
{
    if (m_middleText->empty()) {
        return false;
    }

    const TextLineBase* tl = textLineBase();
    const double padding = tl->absoluteFromSpatium(tl->gapBetweenTextAndLine());
    const RectF middleBounds = m_middleText->ldata()->bbox().translated(m_middleText->pos()).adjusted(-padding, 0.0, padding, 0.0);
    return (!m_text->empty() && middleBounds.intersects(m_text->ldata()->bbox().translated(m_text->pos())))
           || (!m_endText->empty() && middleBounds.intersects(m_endText->ldata()->bbox().translated(m_endText->pos())));
}

double TextLineBaseSegment::layoutEndText(Shape& shape)
{
    if (m_endText->empty()) {
        return 0.0;
    }

    const TextLineBase* tl = textLineBase();
    double clearance = 0.0;
    if (tl->endTextPlace() == TextPlace::LEFT || tl->endTextPlace() == TextPlace::AUTO) {
        clearance = tl->absoluteFromSpatium(tl->gapBetweenTextAndLine());
        switch (m_endText->position()) {
        case AlignH::RIGHT:
            clearance += m_endText->ldata()->bbox().width();
            break;
        case AlignH::HCENTER:
            clearance += m_endText->ldata()->bbox().width() / 2;
            break;
        default:
            break;
        }
    }
    const double endTextX = std::max(pos2().x(), shape.bbox().right() + clearance);
    m_endText->mutldata()->setPosX(endTextX);
    shape.add(m_endText->ldata()->bbox().translated(m_endText->pos()), this);
    return clearance;
}

void TextLineBaseSegment::layoutCrampedTexts(Shape& shape)
{
    if (m_middleText->empty() || (m_text->empty() && m_endText->empty())) {
        return;
    }

    const TextLineBase* tl = textLineBase();
    const double padding = tl->absoluteFromSpatium(tl->gapBetweenTextAndLine());
    // Reset the shape, since now we need to move the end text further away to make room for mid text
    shape = Shape();
    if (!m_text->empty()) {
        shape.add(m_text->ldata()->bbox().translated(m_text->pos()), this);
    }
    const RectF middleBounds = m_middleText->ldata()->bbox();
    m_middleText->mutldata()->setPos(PointF(shape.bbox().right() + padding - middleBounds.left(), -middleBounds.center().y()));
    mutldata()->setMask(Shape());
    shape.add(m_middleText->ldata()->bbox().translated(m_middleText->pos()), m_middleText);
    layoutEndText(shape);
}

bool TextLineBaseSegment::setProperty(Pid id, const PropertyValue& v)
{
    if (id == Pid::COLOR) {
        Color color = v.value<Color>();

        if (m_text) {
            m_text->setColor(color);
        }

        if (m_middleText) {
            m_middleText->setColor(color);
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
    m_middleText->spatiumChanged(ov, nv);
    m_endText->spatiumChanged(ov, nv);
}

static constexpr std::array<Pid, 49> TextLineBasePropertyId = { {
    Pid::MIDDLE_TEXT,
    Pid::MIDDLE_TEXT_OFFSET,
    Pid::MIDDLE_TEXT_CUT_THROUGH,
    Pid::MIDDLE_TEXT_MIN_LINE_LENGTH,
    Pid::LINE_VISIBLE,
    Pid::BEGIN_HOOK_TYPE,
    Pid::BEGIN_HOOK_HEIGHT,
    Pid::BEGIN_LINE_ARROW_HEIGHT,
    Pid::BEGIN_LINE_ARROW_WIDTH,
    Pid::BEGIN_FILLED_ARROW_HEIGHT,
    Pid::BEGIN_FILLED_ARROW_WIDTH,
    Pid::END_HOOK_TYPE,
    Pid::END_HOOK_HEIGHT,
    Pid::END_LINE_ARROW_HEIGHT,
    Pid::END_LINE_ARROW_WIDTH,
    Pid::END_FILLED_ARROW_HEIGHT,
    Pid::END_FILLED_ARROW_WIDTH,
    Pid::GAP_BETWEEN_TEXT_AND_LINE,
    Pid::BEGIN_TEXT,
    Pid::BEGIN_TEXT_ALIGN,
    Pid::BEGIN_TEXT_POSITION,
    Pid::BEGIN_TEXT_PLACE,
    Pid::BEGIN_FONT_FACE,
    Pid::BEGIN_FONT_SIZE,
    Pid::BEGIN_FONT_STYLE,
    Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE,
    Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE,
    Pid::CONTINUE_TEXT_MUSICAL_SYMBOLS_SCALE,
    Pid::CONTINUE_TEXT_MUSIC_SYMBOLS_SIZE,
    Pid::END_TEXT_MUSICAL_SYMBOLS_SCALE,
    Pid::END_TEXT_MUSIC_SYMBOLS_SIZE,
    Pid::BEGIN_TEXT_OFFSET,
    Pid::CONTINUE_TEXT,
    Pid::CONTINUE_TEXT_ALIGN,
    Pid::CONTINUE_TEXT_POSITION,
    Pid::CONTINUE_TEXT_PLACE,
    Pid::CONTINUE_FONT_FACE,
    Pid::CONTINUE_FONT_SIZE,
    Pid::CONTINUE_FONT_STYLE,
    Pid::CONTINUE_TEXT_OFFSET,
    Pid::END_TEXT,
    Pid::END_TEXT_ALIGN,
    Pid::END_TEXT_POSITION,
    Pid::END_TEXT_PLACE,
    Pid::END_FONT_FACE,
    Pid::END_FONT_SIZE,
    Pid::END_FONT_STYLE,
    Pid::END_TEXT_OFFSET,
    Pid::TEXT_SIZE_SPATIUM_DEPENDENT
} };

const std::array<Pid, 49>& TextLineBase::textLineBasePropertyIds()
{
    return TextLineBasePropertyId;
}

void TextLineBase::reset()
{
    String beginText = _beginText;
    String middleText = _middleText;
    String contText = _continueText;
    String endText = _endText;
    bool beginStyled = isStyled(Pid::BEGIN_TEXT);
    bool middleStyled = isStyled(Pid::MIDDLE_TEXT);
    bool contStyled = isStyled(Pid::CONTINUE_TEXT);
    bool endStyled = isStyled(Pid::END_TEXT);

    SLine::reset();
    if (!beginStyled) {
        undoChangeProperty(Pid::BEGIN_TEXT, beginText, PropertyFlags::UNSTYLED);
    }
    if (!middleStyled) {
        undoChangeProperty(Pid::MIDDLE_TEXT, middleText);
    }
    if (!contStyled) {
        undoChangeProperty(Pid::CONTINUE_TEXT, contText, PropertyFlags::UNSTYLED);
    }
    if (!endStyled) {
        undoChangeProperty(Pid::END_TEXT, endText, PropertyFlags::UNSTYLED);
    }
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

PropertyDelegate TextLineBaseSegment::resolvePropertyDelegate(Pid pid) const
{
    if (pid == Pid::TEXT_STYLE) {
        return PropertyDelegate(spanner());
    }

    for (Pid id : TextLineBasePropertyId) {
        if (pid == id) {
            return PropertyDelegate(spanner());
        }
    }
    return LineSegment::resolvePropertyDelegate(pid);
}

//---------------------------------------------------------
//   TextLineBase
//---------------------------------------------------------

TextLineBase::TextLineBase(const ElementType& type, EngravingObject* parent, ElementFlags f)
    : SLine(type, parent, f)
{
    setBeginHookHeight(1.9_sp);
    setEndHookHeight(1.9_sp);
    setGapBetweenTextAndLine(0.5_sp);
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
    case Pid::ALIGN: // Redirect to begin text...
    case Pid::BEGIN_TEXT_ALIGN:
        return PropertyValue::fromValue(beginTextAlign());
    case Pid::MIDDLE_TEXT:
        return middleText();
    case Pid::MIDDLE_TEXT_OFFSET:
        return middleTextOffset();
    case Pid::MIDDLE_TEXT_CUT_THROUGH:
        return middleTextCutThrough();
    case Pid::MIDDLE_TEXT_MIN_LINE_LENGTH:
        return middleTextMinLineLength();
    case Pid::CONTINUE_TEXT_ALIGN:
        return PropertyValue::fromValue(continueTextAlign());
    case Pid::END_TEXT_ALIGN:
        return PropertyValue::fromValue(endTextAlign());
    case Pid::POSITION: // Redirect to begin text...
    case Pid::BEGIN_TEXT_POSITION:
        return PropertyValue::fromValue(beginTextPosition());
    case Pid::CONTINUE_TEXT_POSITION:
        return PropertyValue::fromValue(continueTextPosition());
    case Pid::END_TEXT_POSITION:
        return PropertyValue::fromValue(endTextPosition());
    case Pid::BEGIN_TEXT_PLACE:
        return _beginTextPlace;
    case Pid::BEGIN_HOOK_TYPE:
        return _beginHookType;
    case Pid::BEGIN_HOOK_HEIGHT:
        return _beginHookHeight;
    case Pid::BEGIN_LINE_ARROW_HEIGHT:
        return _beginLineArrowHeight;
    case Pid::BEGIN_LINE_ARROW_WIDTH:
        return _beginLineArrowWidth;
    case Pid::BEGIN_FILLED_ARROW_HEIGHT:
        return _beginFilledArrowHeight;
    case Pid::BEGIN_FILLED_ARROW_WIDTH:
        return _beginFilledArrowWidth;
    case Pid::FONT_FACE: // Redirect to begin text...
    case Pid::BEGIN_FONT_FACE:
        return _beginFontFamily;
    case Pid::FONT_SIZE: // Redirect to begin text...
    case Pid::BEGIN_FONT_SIZE:
        return _beginFontSize;
    case Pid::FONT_STYLE: // Redirect to begin text...
    case Pid::BEGIN_FONT_STYLE:
        return int(_beginFontStyle);
    case Pid::MUSIC_SYMBOL_SIZE: // Redirect to begin text...
    case Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE:
        return _beginTextMusicSymbolsSize;
    case Pid::MUSICAL_SYMBOLS_SCALE: // Redirect to begin text...
    case Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE:
        return _beginTextMusicalSymbolsScale;
    case Pid::CONTINUE_TEXT_MUSICAL_SYMBOLS_SCALE:
        return _continueTextMusicalSymbolsScale;
    case Pid::CONTINUE_TEXT_MUSIC_SYMBOLS_SIZE:
        return _continueTextMusicSymbolsSize;
    case Pid::END_TEXT_MUSICAL_SYMBOLS_SCALE:
        return _endTextMusicalSymbolsScale;
    case Pid::END_TEXT_MUSIC_SYMBOLS_SIZE:
        return _endTextMusicSymbolsSize;
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
    case Pid::END_LINE_ARROW_HEIGHT:
        return _endLineArrowHeight;
    case Pid::END_LINE_ARROW_WIDTH:
        return _endLineArrowWidth;
    case Pid::END_FILLED_ARROW_HEIGHT:
        return _endFilledArrowHeight;
    case Pid::END_FILLED_ARROW_WIDTH:
        return _endFilledArrowWidth;
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
    case Pid::BEGIN_TEXT_POSITION:
        _beginTextPosition = v.value<AlignH>();
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
    case Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE:
        setBeginTextMusicalSymbolsScale(v.toReal());
        break;
    case Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE:
        setBeginTextMusicSymbolsSize(v.toReal());
        break;
    case Pid::MIDDLE_TEXT:
        setMiddleText(v.value<String>());
        break;
    case Pid::MIDDLE_TEXT_OFFSET:
        setMiddleTextOffset(v.value<PointF>());
        break;
    case Pid::MIDDLE_TEXT_CUT_THROUGH:
        setMiddleTextCutThrough(v.toBool());
        break;
    case Pid::MIDDLE_TEXT_MIN_LINE_LENGTH:
        setMiddleTextMinLineLength(v.value<Spatium>());
        break;
    case Pid::CONTINUE_TEXT_MUSICAL_SYMBOLS_SCALE:
        setContinueTextMusicalSymbolsScale(v.toReal());
        break;
    case Pid::CONTINUE_TEXT_MUSIC_SYMBOLS_SIZE:
        setContinueTextMusicSymbolsSize(v.toReal());
        break;
    case Pid::END_TEXT_MUSICAL_SYMBOLS_SCALE:
        setEndTextMusicalSymbolsScale(v.toReal());
        break;
    case Pid::END_TEXT_MUSIC_SYMBOLS_SIZE:
        setEndTextMusicSymbolsSize(v.toReal());
        break;

    case Pid::CONTINUE_TEXT_ALIGN:
        _continueTextAlign = v.value<Align>();
        break;
    case Pid::END_TEXT_ALIGN:
        _endTextAlign = v.value<Align>();
        break;
    case Pid::CONTINUE_TEXT_POSITION:
        _continueTextPosition = v.value<AlignH>();
        break;
    case Pid::END_TEXT_POSITION:
        _endTextPosition = v.value<AlignH>();
        break;
    case Pid::CONTINUE_TEXT_PLACE:
        _continueTextPlace = v.value<TextPlace>();
        break;
    case Pid::END_TEXT_PLACE:
        _endTextPlace = v.value<TextPlace>();
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

    case Pid::BEGIN_FILLED_ARROW_HEIGHT:
        _beginFilledArrowHeight = v.value<Spatium>();
        break;
    case Pid::BEGIN_FILLED_ARROW_WIDTH:
        _beginFilledArrowWidth = v.value<Spatium>();
        break;
    case Pid::END_FILLED_ARROW_HEIGHT:
        _endFilledArrowHeight = v.value<Spatium>();
        break;
    case Pid::END_FILLED_ARROW_WIDTH:
        _endFilledArrowWidth = v.value<Spatium>();
        break;
    case Pid::BEGIN_LINE_ARROW_HEIGHT:
        _beginLineArrowHeight = v.value<Spatium>();
        break;
    case Pid::BEGIN_LINE_ARROW_WIDTH:
        _beginLineArrowWidth = v.value<Spatium>();
        break;
    case Pid::END_LINE_ARROW_HEIGHT:
        _endLineArrowHeight = v.value<Spatium>();
        break;
    case Pid::END_LINE_ARROW_WIDTH:
        _endLineArrowWidth = v.value<Spatium>();
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
    case Pid::TEXT_SIZE_SPATIUM_DEPENDENT:
        setTextSizeSpatiumDependent(v.toBool());
        break;

    /// In >=4.6 text line begin, continue, and end text should all have the same style. This preserves backwards
    /// compatibility and will allow us to style these text items separately in the future if desired

    // These are TextBase properties - updating these should update begin, continue, and end text...
    case Pid::ALIGN: {
        const Align align(v.value<Align>());
        _beginTextAlign = align;
        _continueTextAlign = align;
        _endTextAlign = align;
        break;
    }
    case Pid::POSITION: {
        const AlignH alignH(v.value<AlignH>());
        _beginTextPosition = alignH;
        _continueTextPosition = alignH;
        _endTextPosition = alignH;
        break;
    }
    case Pid::FONT_FACE: {
        const String face(v.value<String>());
        setBeginFontFamily(face);
        setContinueFontFamily(face);
        setEndFontFamily(face);
        break;
    }
    case Pid::FONT_SIZE: {
        const double size(v.toReal());
        if (size <= 0) {
            ASSERT_X(String(u"font size is %1").arg(size));
        }
        setBeginFontSize(size);
        setContinueFontSize(size);
        setEndFontSize(size);
        break;
    }
    case Pid::FONT_STYLE: {
        const FontStyle style = FontStyle(v.toInt());
        setBeginFontStyle(style);
        setContinueFontStyle(style);
        setEndFontStyle(style);
        break;
    }
    case Pid::MUSIC_SYMBOL_SIZE: {
        const double size = v.toReal();
        setBeginTextMusicSymbolsSize(size);
        setContinueTextMusicSymbolsSize(size);
        setEndTextMusicSymbolsSize(size);
        break;
    }
    case Pid::MUSICAL_SYMBOLS_SCALE: {
        const double scale = v.toReal();
        setBeginTextMusicalSymbolsScale(scale);
        setContinueTextMusicalSymbolsScale(scale);
        setEndTextMusicalSymbolsScale(scale);
        break;
    }

    default:
        return SLine::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

PropertyValue TextLineBase::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MIDDLE_TEXT:
        return String();
    case Pid::MIDDLE_TEXT_OFFSET:
        return PointF();
    case Pid::MIDDLE_TEXT_CUT_THROUGH:
        return true;
    case Pid::MIDDLE_TEXT_MIN_LINE_LENGTH:
        return Spatium(0.75);
    case Pid::GAP_BETWEEN_TEXT_AND_LINE:
        return 0.5_sp;

    // Redirect to begin text for these...
    case Pid::ALIGN:
        return propertyDefault(Pid::BEGIN_TEXT_ALIGN);
    case Pid::POSITION:
        return propertyDefault(Pid::BEGIN_TEXT_POSITION);
    case Pid::FONT_FACE:
        return propertyDefault(Pid::BEGIN_FONT_FACE);
    case Pid::FONT_SIZE:
        return propertyDefault(Pid::BEGIN_FONT_SIZE);
    case Pid::FONT_STYLE:
        return propertyDefault(Pid::BEGIN_FONT_STYLE);
    case Pid::MUSICAL_SYMBOLS_SCALE:
        return propertyDefault(Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE);
    case Pid::MUSIC_SYMBOL_SIZE:
        return propertyDefault(Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE);

    default:
        return SLine::propertyDefault(propertyId);
    }
}

PropertyFlags TextLineBase::propertyFlags(Pid id) const
{
    switch (id) {
    case Pid::ALIGN:
        return propertyFlags(Pid::BEGIN_TEXT_ALIGN);
    case Pid::POSITION:
        return propertyFlags(Pid::BEGIN_TEXT_POSITION);
    case Pid::FONT_FACE:
        return propertyFlags(Pid::BEGIN_FONT_FACE);
    case Pid::FONT_SIZE:
        return propertyFlags(Pid::BEGIN_FONT_SIZE);
    case Pid::FONT_STYLE:
        return propertyFlags(Pid::BEGIN_FONT_STYLE);
    case Pid::MUSICAL_SYMBOLS_SCALE:
        return propertyFlags(Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE);
    case Pid::MUSIC_SYMBOL_SIZE:
        return propertyFlags(Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE);

    default:
        return SLine::propertyFlags(id);
    }
}

void TextLineBase::setPropertyFlags(Pid id, PropertyFlags f)
{
    switch (id) {
    case Pid::ALIGN: {
        setPropertyFlags(Pid::BEGIN_TEXT_ALIGN, f);
        setPropertyFlags(Pid::CONTINUE_TEXT_ALIGN, f);
        setPropertyFlags(Pid::END_TEXT_ALIGN, f);
        break;
    }
    case Pid::POSITION: {
        setPropertyFlags(Pid::BEGIN_TEXT_POSITION, f);
        setPropertyFlags(Pid::CONTINUE_TEXT_POSITION, f);
        setPropertyFlags(Pid::END_TEXT_POSITION, f);
        break;
    }
    case Pid::FONT_FACE: {
        setPropertyFlags(Pid::BEGIN_FONT_FACE, f);
        setPropertyFlags(Pid::CONTINUE_FONT_FACE, f);
        setPropertyFlags(Pid::END_FONT_FACE, f);
        break;
    }
    case Pid::FONT_SIZE: {
        setPropertyFlags(Pid::BEGIN_FONT_SIZE, f);
        setPropertyFlags(Pid::CONTINUE_FONT_SIZE, f);
        setPropertyFlags(Pid::END_FONT_SIZE, f);
        break;
    }
    case Pid::FONT_STYLE: {
        setPropertyFlags(Pid::BEGIN_FONT_STYLE, f);
        setPropertyFlags(Pid::CONTINUE_FONT_STYLE, f);
        setPropertyFlags(Pid::END_FONT_STYLE, f);
        break;
    }
    case Pid::MUSICAL_SYMBOLS_SCALE: {
        setPropertyFlags(Pid::BEGIN_TEXT_MUSICAL_SYMBOLS_SCALE, f);
        setPropertyFlags(Pid::CONTINUE_TEXT_MUSICAL_SYMBOLS_SCALE, f);
        setPropertyFlags(Pid::END_TEXT_MUSICAL_SYMBOLS_SCALE, f);
        break;
    }
    case Pid::MUSIC_SYMBOL_SIZE: {
        setPropertyFlags(Pid::BEGIN_TEXT_MUSIC_SYMBOLS_SIZE, f);
        setPropertyFlags(Pid::CONTINUE_TEXT_MUSIC_SYMBOLS_SIZE, f);
        setPropertyFlags(Pid::END_TEXT_MUSIC_SYMBOLS_SIZE, f);
        break;
    }

    default:
        SLine::setPropertyFlags(id, f);
    }
}
}
