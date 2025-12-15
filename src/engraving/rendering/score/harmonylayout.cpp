/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "harmonylayout.h"
#include "rendering/score/parenthesislayout.h"
#include "tlayout.h"
#include "textlayout.h"

#include "dom/fret.h"
#include "dom/harmony.h"
#include "draw/fontmetrics.h"
#include "dom/factory.h"
#include "style/textstyle.h"

#include "log.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

//---------------------------------------------------------
//   buildFontFromTextStyle
//    Build a Font from a TextStyleType's style properties
//    Pattern based on FretDiagram::fretNumFont() in fret.cpp
//---------------------------------------------------------

static muse::draw::Font buildFontFromTextStyle(TextStyleType textStyleType, const MStyle& style,
                                               double mag, double spatium)
{
    const TextStyle* ts = textStyle(textStyleType);
    if (!ts) {
        // Fallback to default font
        return muse::draw::Font();
    }

    // Find the Sid values for font properties
    Sid fontFaceSid = Sid::NOSTYLE;
    Sid fontSizeSid = Sid::NOSTYLE;
    Sid fontStyleSid = Sid::NOSTYLE;
    Sid spatiumDependentSid = Sid::NOSTYLE;

    for (const auto& p : *ts) {
        switch (p.type) {
        case TextStylePropertyType::FontFace: fontFaceSid = p.sid;
            break;
        case TextStylePropertyType::FontSize: fontSizeSid = p.sid;
            break;
        case TextStylePropertyType::FontStyle: fontStyleSid = p.sid;
            break;
        case TextStylePropertyType::SizeSpatiumDependent: spatiumDependentSid = p.sid;
            break;
        default: break;
        }
    }

    // Build the font
    String fontFamily = (fontFaceSid != Sid::NOSTYLE)
                        ? style.styleSt(fontFaceSid)
                        : String(u"Edwin");

    muse::draw::Font font(fontFamily, muse::draw::Font::Type::Harmony);

    double fontSize = (fontSizeSid != Sid::NOSTYLE)
                      ? style.styleD(fontSizeSid)
                      : 10.0;

    bool isSpatiumDependent = (spatiumDependentSid != Sid::NOSTYLE)
                              ? style.styleB(spatiumDependentSid)
                              : true;

    if (isSpatiumDependent) {
        // Scale by ratio of current spatium to default spatium
        fontSize *= spatium / style.defaultSpatium();
    }
    fontSize *= mag;
    font.setPointSizeF(fontSize);

    if (fontStyleSid != Sid::NOSTYLE) {
        FontStyle fStyle = style.styleV(fontStyleSid).value<FontStyle>();
        font.setBold(fStyle & FontStyle::Bold);
        font.setItalic(fStyle & FontStyle::Italic);
        font.setUnderline(fStyle & FontStyle::Underline);
        font.setStrike(fStyle & FontStyle::Strike);
    }

    return font;
}

void HarmonyLayout::layoutHarmony(Harmony* item, Harmony::LayoutData* ldata,
                                  const LayoutContext& ctx)
{
    if (!item->explicitParent()) {
        ldata->setPos(0.0, 0.0);
        const_cast<Harmony*>(item)->setOffset(0.0, 0.0);
    }

    if (!item->cursor()->editing()) {
        render(item, ldata, ctx);
    }

    if (ldata->layoutInvalid) {
        item->createBlocks(ldata);
    }

    if (ldata->blocks.empty()) {
        ldata->blocks.push_back(TextBlock());
    }

    PointF positionPoint = calculateBoundingRect(item, ldata, ctx);

    if (item->isPolychord()) {
        for (LineF& line : ldata->polychordDividerLines.mut_value()) {
            line.setP1(PointF(ldata->bbox().left(), line.y1()));
            line.setP2(PointF(ldata->bbox().right(), line.y2()));
        }
    }

    if (item->hasFrame()) {
        item->layoutFrame(ldata);
    }

    ldata->setPos(positionPoint);

    if (!item->cursor()->editing() && !ldata->renderItemList.value().empty()) {
        ParenthesisLayout::layoutParentheses(item, ctx);
    }
}

//---------------------------------------------------------
//   layoutCapoLabel
//    Add a "Capo N:" label to the left of the first harmony
//    in the score. The label is rendered as a TextSegment
//    prepended to the harmony's render item list.
//
//    This function is called AFTER calculateBoundingRect has
//    already set offsets on the existing render items, so we
//    must apply the same offset to our label segment.
//---------------------------------------------------------

void HarmonyLayout::layoutCapoLabel(Harmony* firstHarmony, const LayoutContext& ctx)
{
    if (!firstHarmony) {
        return;
    }

    // Only show label on the very first harmony in the score (tick 0)
    // This prevents the label from appearing on every system
    if (firstHarmony->tick() != Fraction(0, 1)) {
        return;
    }

    const MStyle& style = ctx.conf().style();

    // Check conditions for showing label
    int capoPosition = style.styleI(Sid::capoPosition);
    bool labelVisible = style.styleB(Sid::capoLabelVisible);
    DisplayCapoChordType displayCapo = style.styleV(Sid::displayCapoChords).value<DisplayCapoChordType>();

    // Only show label if:
    // 1. Capo position > 0
    // 2. Label visibility is enabled
    // 3. Display mode is not CONCERT (i.e., we're showing capo chords)
    if (capoPosition <= 0 || !labelVisible || displayCapo == DisplayCapoChordType::CONCERT) {
        return;
    }

    Harmony::LayoutData* ldata = firstHarmony->mutldata();
    std::vector<HarmonyRenderItem*>& renderItems = ldata->renderItemList.mut_value();

    if (renderItems.empty()) {
        return;
    }

    // Generate label text from format string
    String format = style.styleSt(Sid::capoLabelFormat);
    String labelText = format.arg(capoPosition);

    if (labelText.isEmpty()) {
        return;
    }

    // Get label text style (default: HARMONY_B which is italic)
    TextStyleType labelTextStyle = style.styleV(Sid::capoLabelTextStyle).value<TextStyleType>();

    // Build font for label from the selected text style
    muse::draw::Font labelFont = buildFontFromTextStyle(labelTextStyle, style,
                                                        firstHarmony->mag(), firstHarmony->spatium());

    // For stacked capo chords, find the Y position of the capo chord (which is above the base chord)
    // The capo chord Y position is calculated using: -lineHeight = -(capHeight + gap)
    // We need to match this calculation to align the label with the capo chord
    CapoChordDisplayMode displayMode = style.styleV(Sid::capoChordDisplayMode).value<CapoChordDisplayMode>();
    bool isStackedMode = (displayCapo == DisplayCapoChordType::BOTH && displayMode == CapoChordDisplayMode::STACKED);

    double labelY = 0.0;
    if (isStackedMode) {
        // Calculate the same Y offset used for capo chords in renderSingleHarmony
        double gapSpatium = style.styleS(Sid::capoChordStackedSpacing).val();
        double gap = gapSpatium * firstHarmony->spatium();
        double capHeight = FontMetrics::capHeight(firstHarmony->font()) * firstHarmony->mag();
        double lineHeight = capHeight + gap;
        labelY = -lineHeight;  // Negative = above baseline
    }

    // Find the leftmost x position from existing render items
    // We need to account for the offset that was applied by calculateBoundingRect
    double leftmostX = DBL_MAX;
    PointF existingOffset(0.0, 0.0);

    for (const HarmonyRenderItem* item : renderItems) {
        // Get the full position including offset
        double itemFullX = item->pos().x();
        if (itemFullX < leftmostX) {
            leftmostX = itemFullX;
        }
    }

    // Get the offset from an existing item (they all have the same offset)
    if (!renderItems.empty()) {
        // The offset is m_pos + m_offset - m_pos = the difference between pos() and x()
        existingOffset = renderItems.front()->pos() - PointF(renderItems.front()->x(), renderItems.front()->y());
    }

    if (leftmostX == DBL_MAX) {
        leftmostX = 0.0;
    }

    // Calculate padding between label and chord symbol (0.5 spatium)
    double padding = 0.5 * firstHarmony->spatium();

    // Create a TextSegment for the label
    TextSegment* labelSegment = new TextSegment(labelText, labelFont, 0, labelY, false);
    double labelWidth = labelSegment->width();

    // Position the label: its right edge + padding = leftmost existing item (including offset)
    // Since we'll apply the same offset, we work in the pre-offset coordinate system
    double labelX = leftmostX - existingOffset.x() - padding - labelWidth;
    labelSegment->setx(labelX);

    // Apply the same offset that was applied to other render items
    labelSegment->setOffset(existingOffset);

    // Insert the label at the beginning of the render item list
    renderItems.insert(renderItems.begin(), labelSegment);

    // Recalculate bounding box to include the label
    // Use the full position (including offset) for the bounding box
    RectF labelBbox = labelSegment->tightBoundingRect().translated(labelSegment->pos());
    RectF currentBbox = ldata->bbox();

    // Expand bbox to include label
    currentBbox.unite(labelBbox);
    ldata->setBbox(currentBbox);
}

PointF HarmonyLayout::calculateBoundingRect(const Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx)
{
    const double ypos = (item->placeBelow() && item->staff()) ? item->staff()->staffHeight(item->tick()) : 0.0;
    const FretDiagram* fd = (item->explicitParent() && item->explicitParent()->isFretDiagram())
                            ? toFretDiagram(item->explicitParent())
                            : nullptr;
    const bool alignToFretDiagram = fd && fd->visible();

    const double standardNoteWidth = item->symWidth(SymId::noteheadBlack);

    double newPosX = 0.0;
    double newPosY = 0.0;

    if (item->ldata()->renderItemList().empty()) {
        TextLayout::layoutBaseTextBase1(item, ldata);

        if (alignToFretDiagram) {
            newPosY = ldata->pos().y();
        } else {
            newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? -ldata->bbox().height() : 0.0);
        }
    } else {
        layoutModifierParentheses(item);
        RectF bb;
        RectF hAlignBox;
        for (HarmonyRenderItem* renderItem : item->ldata()->renderItemList()) {
            RectF tsBbox = renderItem->tightBoundingRect().translated(renderItem->x(), renderItem->y());
            bb.unite(tsBbox);

            if (renderItem->align()) {
                hAlignBox.unite(tsBbox);
            }
        }

        double xx = 0.0;
        if (alignToFretDiagram) {
            switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
            case AlignH::LEFT:
            case AlignH::JUSTIFY:
                xx = -hAlignBox.left();
                break;
            case AlignH::HCENTER:
                xx = -(hAlignBox.center().x());
                break;
            case AlignH::RIGHT:
                xx = -hAlignBox.right();
                break;
            }
        } else {
            switch (item->position()) {
            case AlignH::LEFT:
            case AlignH::JUSTIFY:
                xx = -hAlignBox.left();
                break;
            case AlignH::HCENTER:
                xx = -(hAlignBox.center().x());
                break;
            case AlignH::RIGHT:
                xx = -hAlignBox.right();
                break;
            }
        }

        double yy = -bb.y();      // Align::TOP
        if (item->align() == AlignV::VCENTER) {
            yy = -bb.y() / 2.0;
        } else if (item->align() == AlignV::BASELINE) {
            yy = item->baseLine();
        } else if (item->align() == AlignV::BOTTOM) {
            yy = -bb.height() - bb.y();
        }

        if (alignToFretDiagram) {
            double nutLineWidth = fd->ldata()->nutLineWidth;
            newPosY = yy - ctx.conf().styleMM(Sid::harmonyFretDist) - nutLineWidth;
        } else {
            newPosY = ypos;
        }

        for (HarmonyRenderItem* renderItem : item->ldata()->renderItemList()) {
            renderItem->setOffset(PointF(xx, yy));
        }

        ldata->polychordDividerOffset = yy;

        ldata->setBbox(bb.translated(xx, yy));
        ldata->harmonyHeight = ldata->bbox().height();
    }

    if (alignToFretDiagram) {
        switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
        case AlignH::LEFT:
        case AlignH::JUSTIFY:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = 0.5 * fd->mainWidth();
            break;
        case AlignH::RIGHT:
            newPosX = fd->mainWidth();
            break;
        }
    } else {
        switch (item->position()) {
        case AlignH::LEFT:
        case AlignH::JUSTIFY:
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = standardNoteWidth * 0.5;
            break;
        case AlignH::RIGHT:
            newPosX = standardNoteWidth;
            break;
        }
    }

    if (fd && !fd->visible()) {
        // Translate to base position around note
        newPosX -= fd->pos().x();
    }

    return PointF(newPosX, newPosY);
}

void HarmonyLayout::layoutModifierParentheses(const Harmony* item)
{
    const double spatium = item->spatium();
    const std::vector<HarmonyRenderItem*>& itemList = item->ldata()->renderItemList();
    // Layout parentheses
    std::vector<ChordSymbolParen*> openingParenStack;
    double lastTextSegHeight = 0.0;
    double lastTextSegTop = 0.0;
    double rootRefHeight = 0.0;
    double rootCapHeight = 0.0;
    double parenExtension = 0.1 * spatium * (item->size() / 10.0);
    for (HarmonyRenderItem* renderItem : itemList) {
        if (ChordSymbolParen* curParen = dynamic_cast<ChordSymbolParen*>(renderItem)) {
            if (curParen->parenItem->direction() == DirectionH::LEFT) {
                // Opening paren
                openingParenStack.push_back(curParen);
            } else {
                // Closing paren
                ChordSymbolParen* openingParen = openingParenStack.empty() ? nullptr : openingParenStack.back();
                if (!openingParen) {
                    continue;
                }
                openingParenStack.pop_back();
                curParen->top = openingParen->top;
                curParen->bottom = openingParen->bottom;

                // Layout parenthesis pair
                double startY = openingParen->top - parenExtension;
                double height = (openingParen->bottom - openingParen->top) + 2 * parenExtension;
                if (std::isinf(height)) {
                    height = lastTextSegHeight;
                }
                if (muse::RealIsEqual(DBL_MAX, startY)) {
                    startY = lastTextSegTop;
                }

                const double mag = openingParen->parenItem->ldata()->mag();
                const double scale = (height - 2 * parenExtension) / rootCapHeight;
                static constexpr double HEIGHT_TO_WIDTH_RATIO = 20;
                const double midPointThickness = height / HEIGHT_TO_WIDTH_RATIO * mag * 1 / std::sqrt(scale);
                const double endPointThickness = 0.03;
                const double shoulder = 0.2 * height * std::pow(mag, 0.1) * 1 / std::sqrt(scale);

                openingParen->parenItem->mutldata()->startY = startY;
                openingParen->sety(startY);
                openingParen->parenItem->mutldata()->height = height;
                openingParen->parenItem->mutldata()->midPointThickness.set_value(midPointThickness);
                openingParen->parenItem->mutldata()->endPointThickness.set_value(endPointThickness);
                openingParen->parenItem->mutldata()->shoulderWidth = shoulder;

                curParen->parenItem->mutldata()->startY = startY;
                curParen->sety(startY);
                curParen->parenItem->mutldata()->height = height;
                curParen->parenItem->mutldata()->midPointThickness.set_value(midPointThickness);
                curParen->parenItem->mutldata()->endPointThickness.set_value(endPointThickness);
                curParen->parenItem->mutldata()->shoulderWidth = shoulder;

                double closingPos = openingParen->closingParenPos;
                if (muse::RealIsEqual(-DBL_MAX, closingPos)) {
                    closingPos = openingParen->x() + openingParen->boundingRect().width();
                }
                curParen->setx(closingPos);

                ParenthesisLayout::createPathAndShape(openingParen->parenItem, openingParen->parenItem->mutldata());
                ParenthesisLayout::createPathAndShape(curParen->parenItem, curParen->parenItem->mutldata());

                // Outer parens must always be the same length or longer than inner parens
                for (ChordSymbolParen* outerParen : openingParenStack) {
                    outerParen->top = std::min(openingParen->top, outerParen->top);
                    outerParen->bottom = std::max(openingParen->bottom, outerParen->bottom);
                    outerParen->closingParenPos = std::max(openingParen->closingParenPos, outerParen->closingParenPos);
                }
            }
        } else if (TextSegment* textSeg = dynamic_cast<TextSegment*>(renderItem)) {
            // Set top paren height
            lastTextSegHeight = textSeg->height();
            lastTextSegTop = textSeg->tightBoundingRect().translated(textSeg->pos()).y();
            if (muse::RealIsNull(rootRefHeight)) {
                rootRefHeight = textSeg->height();
                double top = textSeg->tightBoundingRect().translated(textSeg->pos()).y();
                double bottom =  textSeg->pos().y();
                double height = (bottom - top) + 2 * parenExtension;

                rootRefHeight = height;
                rootCapHeight = textSeg->capHeight();
            }
            if (!openingParenStack.empty()) {
                ChordSymbolParen* topParen = openingParenStack.back();
                if (textSeg->font().type() != Font::Type::MusicSymbolText) {
                    topParen->top = std::min(topParen->top, textSeg->tightBoundingRect().translated(textSeg->pos()).y());
                    topParen->bottom = std::max(topParen->bottom, textSeg->pos().y());
                }
                topParen->closingParenPos = std::max(topParen->closingParenPos, textSeg->x() + textSeg->width());
                continue;
            }
        }
    }

    auto computePadding = [](HarmonyRenderItem* item1, HarmonyRenderItem* item2) {
        bool item1Paren = item1->type() == HarmonyRenderItemType::PAREN;
        bool item2Paren = item2->type() == HarmonyRenderItemType::PAREN;

        double padding = 0.0;
        if (item1Paren && item2Paren) {
            padding = std::min(item1->rightPadding(), item2->leftPadding());
        } else {
            padding = std::max(item1->rightPadding(), item2->leftPadding());
        }

        double scaling = (item1->height() + item2->height()) / 2;
        padding *= scaling;

        return padding;
    };

    // Create space in text for parentheses
    // TODO - STACKED C7((#11)#13)
    // C7(#11(#13))
    double additionalSpace = 0.0;
    bool prevParen = false;

    for (size_t i = 0; i < itemList.size(); i++) {
        HarmonyRenderItem* renderItem = itemList.at(i);
        double padding = i != 0 ? computePadding(itemList.at(i - 1), renderItem) : 0.0;

        if (ChordSymbolParen* paren = dynamic_cast<ChordSymbolParen*>(renderItem)) {
            if (paren->parenItem->direction() == DirectionH::LEFT) {
                double heightToRefRatio = std::max(paren->height(), rootRefHeight) / rootRefHeight;
                bool scaleParenWidth = !prevParen && !muse::RealIsEqual(heightToRefRatio, 1.0);
                const double PAREN_PADDING_SCALER = 0.6;
                double scaledParenWidth = paren->parenItem->width() * PAREN_PADDING_SCALER / heightToRefRatio;

                double parenWidth = scaleParenWidth ? scaledParenWidth : paren->parenItem->width();
                additionalSpace += parenWidth + padding;
                paren->movex(additionalSpace);
            }
            if (paren->parenItem->direction() == DirectionH::RIGHT) {
                paren->movex(additionalSpace + padding);
                additionalSpace += paren->parenItem->width() + padding;
            }
            prevParen = true;
        } else if (TextSegment* ts = dynamic_cast<TextSegment*>(renderItem)) {
            additionalSpace += padding;
            ts->movex(additionalSpace);
            prevParen = false;
        }
    }
}

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void HarmonyLayout::render(Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx)
{
    for (const HarmonyRenderItem* renderItem : ldata->renderItemList()) {
        delete renderItem;
    }
    ldata->renderItemList.mut_value().clear();
    if (item->harmonyType() == HarmonyType::ROMAN) {
        renderRomanNumeral(item, ldata);
        return;
    }

    // Render standard or Nashville chords

    ChordList* chordList = item->score()->chordList();

    ldata->fontList.mut_value().clear();
    for (const ChordFont& cf : chordList->fonts) {
        Font ff(item->font());
        ff.setFamily(item->family(), Font::Type::Harmony);

        double mag = item->mag() * cf.mag;
        ff.setPointSizeF(ff.pointSizeF() * mag);
        if (cf.musicSymbolText) {
            ff.setFamily(cf.family, Font::Type::MusicSymbolText);
        } else if (!(cf.family.isEmpty() || cf.family == "default")) {
            ff.setFamily(cf.family, Font::Type::Harmony);
        }
        ldata->fontList.mut_value().push_back(ff);
    }
    if (ldata->fontList.mut_value().empty()) {
        Font ff(item->font());
        ff.setFamily(item->family(), Font::Type::Harmony);

        ldata->fontList.mut_value().push_back(ff);
    }

    ldata->polychordDividerLines.reset();
    HarmonyRenderCtx harmonyCtx;

    // Map of text segments and their final width
    std::multimap<double, std::vector<HarmonyRenderItem*> > chordTextSegments;

    for (size_t i = item->chords().size(); i > 0; i--) {
        harmonyCtx.info = item->chords().at(i - 1);
        renderSingleHarmony(item, ldata, harmonyCtx, ctx);

        chordTextSegments.emplace(std::pair<double, std::vector<HarmonyRenderItem*> > { harmonyCtx.x(), harmonyCtx.renderItemList });
        ldata->renderItemList.mut_value().insert(ldata->renderItemList.mut_value().end(),
                                                 harmonyCtx.renderItemList.begin(), harmonyCtx.renderItemList.end());

        // Measure divider spacing from lowest baseline and highest cap-height in segments
        double rootBaseline = harmonyCtx.renderItemList.empty() ? -DBL_MAX : harmonyCtx.renderItemList.front()->y();
        double bottomBaseline = -DBL_MAX;
        for (const HarmonyRenderItem* renderItem : harmonyCtx.renderItemList) {
            if (const TextSegment* ts = dynamic_cast<const TextSegment*>(renderItem)) {
                bottomBaseline = std::max(bottomBaseline, ts->y());
            }
        }

        double diff = rootBaseline - bottomBaseline;
        if (bottomBaseline > rootBaseline) {
            for (HarmonyRenderItem* renderItem : harmonyCtx.renderItemList) {
                if (TextSegment* ts = dynamic_cast<TextSegment*>(renderItem)) {
                    ts->movey(diff);
                }
            }
        }
        if (i == item->chords().size()) {
            // Set baseline for bottom chord
            ldata->baseline = -diff;
        }

        double topCapHeight = DBL_MAX;
        for (const HarmonyRenderItem* renderItem : harmonyCtx.renderItemList) {
            if (const TextSegment* ts = dynamic_cast<const TextSegment*>(renderItem)) {
                topCapHeight = std::min(topCapHeight, ts->y() - ts->capHeight());
            }
        }

        harmonyCtx.renderItemList.clear();
        if (item->chords().size() == 1 || i == 1) {
            break;
        }

        assert(!muse::RealIsEqual(topCapHeight, DBL_MAX));

        harmonyCtx.setx(0);
        harmonyCtx.sety(topCapHeight);

        double lineY = harmonyCtx.y() - ctx.conf().style().styleS(Sid::polychordDividerSpacing).toMM(item->spatium())
                       - ctx.conf().style().styleS(Sid::polychordDividerThickness).toMM(item->spatium()) / 2;
        LineF line = LineF(PointF(0.0, lineY), PointF(0.0, lineY));
        ldata->polychordDividerLines.mut_value().push_back(line);

        harmonyCtx.movey(-ctx.conf().style().styleS(Sid::polychordDividerSpacing).toMM(item->spatium()) * 2.0);
        harmonyCtx.movey(-ctx.conf().style().styleS(Sid::polychordDividerThickness).toMM(item->spatium()));
    }

    // Align polychords

    if (item->align() == AlignH::LEFT) {
        return;
    }

    double longestLine = 0.0;

    for (const auto& [width, _] : chordTextSegments) {
        if (width > longestLine) {
            longestLine = width;
        }
    }

    for (auto& textSegs : chordTextSegments) {
        double width = textSegs.first;
        std::vector<HarmonyRenderItem*>& segs = textSegs.second;

        double diff = longestLine - width;

        if (muse::RealIsNull(diff)) {
            continue;
        }

        // For centre align adjust by .5* difference, for right align adjust by full difference
        if (item->align() == AlignH::HCENTER) {
            diff *= 0.5;
        }

        for (HarmonyRenderItem* seg : segs) {
            seg->movex(diff);
        }
    }
}

void HarmonyLayout::renderRomanNumeral(Harmony* item, Harmony::LayoutData* ldata)
{
    HarmonyRenderCtx harmonyCtx;
    if (item->chords().empty()) {
        return;
    }
    HarmonyInfo* info = item->chords().front();

    render(item, ldata, info->textName(), harmonyCtx);
    ldata->renderItemList.mut_value().insert(ldata->renderItemList.mut_value().end(), harmonyCtx.renderItemList.begin(),
                                             harmonyCtx.renderItemList.end());
    ldata->baseline = 0.0;
}

void HarmonyLayout::doRenderSingleHarmony(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx, int rootTpc, int bassTpc,
                                          const LayoutContext& ctx)
{
    HarmonyInfo* info = harmonyCtx.info;
    ChordList* chordList = info->chordList();
    if (!chordList) {
        return;
    }

    const MStyle& style = ctx.conf().style();

    NoteCaseType rootCase = item->rootRenderCase(info);
    NoteCaseType bassCase = item->bassRenderCase();

    NoteSpellingType spelling = style.styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    const ChordDescription* cd = info->getDescription();
    const bool stackModifiers = style.styleB(Sid::verticallyStackModifiers) && !item->doNotStackModifiers();

    if (item->harmonyType() == HarmonyType::STANDARD && tpcIsValid(rootTpc)) {
        // render root
        render(item, ldata, chordList->renderListRoot, harmonyCtx, ctx, rootTpc, spelling, rootCase);
        // render extension
        if (cd) {
            render(item, ldata, stackModifiers ? cd->renderListStacked : cd->renderList, harmonyCtx, ctx, 0);
        }
    } else if (item->harmonyType() == HarmonyType::NASHVILLE && tpcIsValid(rootTpc)) {
        // render function
        render(item, ldata, chordList->renderListFunction, harmonyCtx, ctx, rootTpc, spelling, bassCase);
        double adjust = chordList->nominalAdjust();
        harmonyCtx.movey(adjust * item->magS() * item->spatium() * .2);
        // render extension
        if (cd) {
            render(item, ldata, stackModifiers ? cd->renderListStacked : cd->renderList, harmonyCtx, ctx, 0);
        }
    } else {
        render(item, ldata, info->textName(), harmonyCtx);
    }

    // render bass
    if (tpcIsValid(bassTpc)) {
        const std::vector<RenderActionPtr>& bassNoteChordList
            = style.styleB(Sid::chordBassNoteStagger) ? chordList->renderListBassOffset : chordList->renderListBass;

        static const std::wregex PATTERN_69 = std::wregex(L"6[,/]?9");
        const bool is69 = info->textName().contains(PATTERN_69);
        const bool hasModifierStack = stackModifiers && (info->parsedChord() ? info->parsedChord()->modifierList().size() > 1 : false);

        if (hasModifierStack || is69) {
            render(item, ldata, { std::make_shared<RenderActionMove>(0.05, 0.0) }, harmonyCtx, ctx,
                   bassTpc, spelling, bassCase, item->bassScale());
        }
        render(item, ldata, bassNoteChordList, harmonyCtx, ctx, bassTpc, spelling, bassCase, item->bassScale());
    }
}

//---------------------------------------------------------
//   calculateCapoTpc
//    Calculate transposed TPC values for capo chord
//---------------------------------------------------------

static void calculateCapoTpc(int rootTpc, int bassTpc, int capo, int& capoRootTpc, int& capoBassTpc)
{
    capoRootTpc = Tpc::TPC_INVALID;
    capoBassTpc = Tpc::TPC_INVALID;

    if (!tpcIsValid(rootTpc) || capo <= 0 || capo >= 12) {
        return;
    }

    int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
    capoRootTpc = rootTpc + tpcOffset[capo];
    capoBassTpc = bassTpc;

    if (tpcIsValid(capoBassTpc)) {
        capoBassTpc += tpcOffset[capo];
    }

    // For guitarists, avoid x and bb in Root or Bass,
    // and also avoid E#, B#, Cb and Fb in Root.
    if (capoRootTpc < 8 || (tpcIsValid(capoBassTpc) && capoBassTpc < 6)) {
        capoRootTpc += 12;
        if (tpcIsValid(capoBassTpc)) {
            capoBassTpc += 12;
        }
    } else if (capoRootTpc > 24 || (tpcIsValid(capoBassTpc) && capoBassTpc > 26)) {
        capoRootTpc -= 12;
        if (tpcIsValid(capoBassTpc)) {
            capoBassTpc -= 12;
        }
    }
}

void HarmonyLayout::renderSingleHarmony(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx,
                                        const LayoutContext& ctx)
{
    harmonyCtx.hAlign = true;
    const MStyle& style = ctx.conf().style();
    HarmonyInfo* info = harmonyCtx.info;

    int rootTpc = info->rootTpc();
    int bassTpc = info->bassTpc();

    DisplayCapoChordType displayCapo = style.styleV(Sid::displayCapoChords).value<DisplayCapoChordType>();
    int capo = style.styleI(Sid::capoPosition);

    // Early exit if no capo or not showing both - render single chord
    if (capo == 0 || displayCapo != DisplayCapoChordType::BOTH) {
        if (displayCapo == DisplayCapoChordType::CONCERT || displayCapo == DisplayCapoChordType::BOTH) {
            doRenderSingleHarmony(item, ldata, harmonyCtx, rootTpc, bassTpc, ctx);
        }
        if (displayCapo == DisplayCapoChordType::TRANSPOSED && capo > 0) {
            int capoRootTpc, capoBassTpc;
            calculateCapoTpc(rootTpc, bassTpc, capo, capoRootTpc, capoBassTpc);
            doRenderSingleHarmony(item, ldata, harmonyCtx, capoRootTpc, capoBassTpc, ctx);
        }
        return;
    }

    // We have a capo and displayCapo == BOTH
    CapoChordDisplayMode displayMode = style.styleV(Sid::capoChordDisplayMode).value<CapoChordDisplayMode>();

    int capoRootTpc, capoBassTpc;
    calculateCapoTpc(rootTpc, bassTpc, capo, capoRootTpc, capoBassTpc);

    if (displayMode == CapoChordDisplayMode::STACKED) {
        // STACKED MODE: Capo chord above, base chord below
        // Key principle: The chord symbol position is INVARIANT - always centered over base chord.
        // Parentheses (if enabled) simply extend to the left and right of the chord symbol.
        // Toggling parentheses should NOT move the chord symbol.
        //
        // IMPORTANT: layoutModifierParentheses will later shift all items RIGHT to make space
        // for parentheses. We must measure the actual shift and pre-compensate.
        bool showParens = style.styleB(Sid::capoChordParenthesized);

        // Get the configurable gap between stacked chords (in spatium)
        double gapSpatium = style.styleS(Sid::capoChordStackedSpacing).val();
        double gap = gapSpatium * item->spatium();

        // Get capo chord text style (default: HARMONY_B which is italic)
        TextStyleType capoTextStyle = style.styleV(Sid::capoChordTextStyle).value<TextStyleType>();

        // Build font for capo chord from the selected text style
        muse::draw::Font capoFont = buildFontFromTextStyle(capoTextStyle, style, item->mag(), item->spatium());

        // Calculate line height for vertical offset
        // The offset is: capHeight (text height) + gap
        double capHeight = FontMetrics::capHeight(item->font()) * item->mag();
        double lineHeight = capHeight + gap;

        // Save original fontList - we'll replace it for capo chord rendering
        std::vector<muse::draw::Font> originalFontList = ldata->fontList.value();

        // Step 1: Render base chord first (needed for accurate paren shift measurement)
        doRenderSingleHarmony(item, ldata, harmonyCtx, rootTpc, bassTpc, ctx);
        double baseChordWidth = harmonyCtx.x();
        size_t baseChordItemCount = harmonyCtx.renderItemList.size();

        // Step 2: Measure capo chord symbol width (without parens)
        // Replace fontList with capo font for capo chord rendering
        std::vector<muse::draw::Font> capoFontList;
        capoFontList.push_back(capoFont);
        // Copy any additional fonts (for chord modifications, etc.) with capo styling applied
        for (size_t i = 1; i < originalFontList.size(); ++i) {
            muse::draw::Font modFont = originalFontList[i];
            // Apply capo font style to modification fonts
            modFont.setBold(capoFont.bold());
            modFont.setItalic(capoFont.italic());
            capoFontList.push_back(modFont);
        }
        ldata->fontList.mut_value() = capoFontList;

        harmonyCtx.setx(0);  // Reset x for measurement
        harmonyCtx.sety(-lineHeight);
        harmonyCtx.hAlign = false;

        size_t capoStartIdx = harmonyCtx.renderItemList.size();
        doRenderSingleHarmony(item, ldata, harmonyCtx, capoRootTpc, capoBassTpc, ctx);
        double capoChordSymbolWidth = harmonyCtx.x();

        // Step 3: If parens enabled, measure the shift that layoutModifierParentheses will apply
        double parenShift = 0.0;
        if (showParens && harmonyCtx.renderItemList.size() > capoStartIdx) {
            // Get the first capo text segment's original x position
            double originalFirstCapoTextX = harmonyCtx.renderItemList[capoStartIdx]->x();

            // Add parens around the capo chord items (after base chord items)
            // Insert left paren before capo text
            Parenthesis* tempPL = Factory::createParenthesis(item);
            tempPL->setParent(item);
            tempPL->setDirection(DirectionH::LEFT);
            tempPL->setGenerated(true);
            ChordSymbolParen* tempLeftParen = new ChordSymbolParen(tempPL, false, 0, -lineHeight);
            harmonyCtx.renderItemList.insert(harmonyCtx.renderItemList.begin() + capoStartIdx, tempLeftParen);

            // Add right paren at end
            Parenthesis* tempPR = Factory::createParenthesis(item);
            tempPR->setParent(item);
            tempPR->setDirection(DirectionH::RIGHT);
            tempPR->setGenerated(true);
            ChordSymbolParen* tempRightParen = new ChordSymbolParen(tempPR, false, capoChordSymbolWidth, -lineHeight);
            harmonyCtx.renderItemList.push_back(tempRightParen);

            // Temporarily set ldata to run layoutModifierParentheses with full context
            std::vector<HarmonyRenderItem*> savedItems = ldata->renderItemList.value();
            ldata->renderItemList.mut_value() = harmonyCtx.renderItemList;

            // Run layout - this will process base chord AND capo chord items
            layoutModifierParentheses(item);

            // Measure shift: how much did the first capo text segment move?
            // It's now at index capoStartIdx + 1 (after left paren was inserted)
            if (harmonyCtx.renderItemList.size() > capoStartIdx + 1) {
                HarmonyRenderItem* firstCapoText = harmonyCtx.renderItemList[capoStartIdx + 1];
                parenShift = firstCapoText->x() - originalFirstCapoTextX;
            }

            // Restore original ldata
            ldata->renderItemList.mut_value() = savedItems;

            // Remove temp items from harmonyCtx (keep base chord items, remove capo items)
            // We'll re-render capo items with correct offset
            while (harmonyCtx.renderItemList.size() > baseChordItemCount) {
                delete harmonyCtx.renderItemList.back();
                harmonyCtx.renderItemList.pop_back();
            }
        } else {
            // No parens - just clean up the temp capo items
            while (harmonyCtx.renderItemList.size() > baseChordItemCount) {
                delete harmonyCtx.renderItemList.back();
                harmonyCtx.renderItemList.pop_back();
            }
        }

        // Step 4: Calculate where chord symbol should be centered
        double baseChordCenter = baseChordWidth / 2.0;
        double capoChordSymbolCenter = capoChordSymbolWidth / 2.0;
        double capoChordSymbolStartX = baseChordCenter - capoChordSymbolCenter;

        // Step 5: Render capo chord
        harmonyCtx.sety(-lineHeight);
        harmonyCtx.hAlign = false;

        if (showParens) {
            // Start further LEFT to compensate for the rightward shift that layout will apply
            double adjustedStartX = capoChordSymbolStartX - parenShift;

            // Left paren
            harmonyCtx.setx(adjustedStartX);
            RenderActionParenPtr pLeft = std::make_shared<RenderActionParenLeft>();
            renderActionParen(item, pLeft, harmonyCtx);

            // Chord symbol (layout will shift it right by parenShift, landing at capoChordSymbolStartX)
            doRenderSingleHarmony(item, ldata, harmonyCtx, capoRootTpc, capoBassTpc, ctx);

            // Right paren
            RenderActionParenPtr pRight = std::make_shared<RenderActionParenRight>();
            renderActionParen(item, pRight, harmonyCtx);
        } else {
            // No parens - render chord symbol directly at centered position
            harmonyCtx.setx(capoChordSymbolStartX);
            doRenderSingleHarmony(item, ldata, harmonyCtx, capoRootTpc, capoBassTpc, ctx);
        }

        // Restore original fontList after capo chord rendering
        ldata->fontList.mut_value() = originalFontList;
    } else {
        // INLINE MODE: Current behavior - base chord then (capo chord)
        doRenderSingleHarmony(item, ldata, harmonyCtx, rootTpc, bassTpc, ctx);

        RenderActionParenPtr pLeft = std::make_shared<RenderActionParenLeft>();
        renderActionParen(item, pLeft, harmonyCtx);

        doRenderSingleHarmony(item, ldata, harmonyCtx, capoRootTpc, capoBassTpc, ctx);

        RenderActionParenPtr pRight = std::make_shared<RenderActionParenRight>();
        renderActionParen(item, pRight, harmonyCtx);
    }
}

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void HarmonyLayout::render(Harmony* item, Harmony::LayoutData* ldata, const String& s, HarmonyRenderCtx& harmonyCtx)
{
    if (s.isEmpty()) {
        return;
    }

    Font f = item->harmonyType() != HarmonyType::ROMAN ? ldata->fontList.value().front() : item->font();
    TextSegment* ts = new TextSegment(s, f, harmonyCtx.x(), harmonyCtx.y(), harmonyCtx.hAlign);
    harmonyCtx.renderItemList.push_back(ts);
    harmonyCtx.movex(ts->width());
}

void HarmonyLayout::render(Harmony* item, Harmony::LayoutData* ldata, SymId sym, HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx)
{
    if (sym == SymId::noSym) {
        return;
    }

    Font f = item->harmonyType() != HarmonyType::ROMAN ? ldata->fontList.value().front() : item->font();
    f.setFamily(ctx.conf().style().styleSt(Sid::musicalTextFont), Font::Type::MusicSymbolText);

    String s = ctx.conf().engravingFont()->toString(sym);

    TextSegment* ts = new TextSegment(s, f, harmonyCtx.x(), harmonyCtx.y(), harmonyCtx.hAlign);
    harmonyCtx.renderItemList.push_back(ts);
    harmonyCtx.movex(ts->width());
}

void HarmonyLayout::render(Harmony* item, Harmony::LayoutData* ldata, const std::vector<RenderActionPtr>& renderList,
                           HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx,
                           int tpc,
                           NoteSpellingType noteSpelling,
                           NoteCaseType noteCase, double noteMag)
{
    harmonyCtx.stack = {};
    harmonyCtx.tpc = tpc;
    harmonyCtx.noteSpelling = noteSpelling;
    harmonyCtx.noteCase = noteCase;
    harmonyCtx.scale = noteMag;

    for (const RenderActionPtr& a : renderList) {
        renderAction(item, ldata, a, harmonyCtx, ctx);
    }
}

void HarmonyLayout::renderAction(Harmony* item, Harmony::LayoutData* ldata, const RenderActionPtr& a, HarmonyRenderCtx& harmonyCtx,
                                 const LayoutContext& ctx)
{
    switch (a->actionType()) {
    case RenderAction::RenderActionType::SET:
        renderActionSet(item, ldata, std::static_pointer_cast<RenderActionSet>(a), harmonyCtx, ctx);
        break;
    case RenderAction::RenderActionType::MOVE:
        renderActionMove(item, std::static_pointer_cast<RenderActionMove>(a), harmonyCtx);
        break;
    case RenderAction::RenderActionType::MOVEXHEIGHT:
        renderActionMoveXHeight(item, std::static_pointer_cast<RenderActionMoveXHeight>(a), harmonyCtx);
        break;
    case RenderAction::RenderActionType::PUSH:
        renderActionPush(harmonyCtx);
        break;
    case RenderAction::RenderActionType::POP:
        renderActionPop(std::static_pointer_cast<RenderActionPop>(a), harmonyCtx);
        break;
    case RenderAction::RenderActionType::NOTE:
        renderActionNote(item, ldata, harmonyCtx);
        break;
    case RenderAction::RenderActionType::ACCIDENTAL:
        renderActionAcc(item, ldata, harmonyCtx, ctx);
        break;
    case RenderAction::RenderActionType::STOPHALIGN:
        renderActionAlign(harmonyCtx);
        break;
    case RenderAction::RenderActionType::SCALE:
        renderActionScale(std::static_pointer_cast<RenderActionScale>(a), harmonyCtx);
        break;
    case RenderAction::RenderActionType::PAREN:
        renderActionParen(item, std::static_pointer_cast<RenderActionParen>(a), harmonyCtx);
        break;
    default:
        LOGD("unknown render action %d", static_cast<int>(a->actionType()));
    }
}

void HarmonyLayout::renderActionPush(HarmonyRenderCtx& harmonyCtx)
{
    harmonyCtx.stack.push(harmonyCtx.pos);
}

void HarmonyLayout::renderActionPop(const RenderActionPopPtr& a, HarmonyRenderCtx& harmonyCtx)
{
    if (harmonyCtx.stack.empty()) {
        LOGD("RenderAction::RenderActionType::POP: stack empty");
        return;
    }

    PointF pt = harmonyCtx.stack.top();
    harmonyCtx.stack.pop();
    harmonyCtx.pos = PointF(a->popX() ? pt.x() : harmonyCtx.x(), a->popY() ? pt.y() : harmonyCtx.y());
}

void HarmonyLayout::renderActionNote(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx)
{
    if (!tpcIsValid(harmonyCtx.tpc)) {
        return;
    }
    const ChordList* chordList = item->score()->chordList();
    const Staff* st = item->staff();
    const Key key = st ? st->key(item->tick()) : Key::INVALID;

    String c;
    AccidentalVal acc;

    if (item->harmonyType() == HarmonyType::STANDARD) {
        tpc2name(harmonyCtx.tpc, harmonyCtx.noteSpelling, harmonyCtx.noteCase, c, acc);
    } else if (item->harmonyType() == HarmonyType::NASHVILLE) {
        String accStr;
        tpc2Function(harmonyCtx.tpc, key, accStr, c);
    }

    if (c.empty()) {
        return;
    }

    String lookup = u"note" + c;
    ChordSymbol cs = chordList->symbol(lookup);
    if (!cs.isValid()) {
        cs = chordList->symbol(c);
    }
    String text = cs.isValid() ? cs.value : c;
    muse::draw::Font font = cs.isValid() ? ldata->fontList.value()[cs.fontIdx] : ldata->fontList.value().front();
    font.setPointSizeF(font.pointSizeF() * harmonyCtx.scale);

    TextSegment* ts = new TextSegment(text, font, harmonyCtx.x(), harmonyCtx.y(), harmonyCtx.hAlign);
    harmonyCtx.renderItemList.push_back(ts);
    harmonyCtx.movex(ts->width());
}

void HarmonyLayout::renderActionAcc(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx)
{
    if (!tpcIsValid(harmonyCtx.tpc)) {
        return;
    }
    const ChordList* chordList = item->score()->chordList();
    const Staff* st = item->staff();
    const Key key = st ? st->key(item->tick()) : Key::INVALID;

    String c;
    String acc;
    String context = u"accidental";

    if (item->harmonyType() == HarmonyType::STANDARD) {
        tpc2name(harmonyCtx.tpc, harmonyCtx.noteSpelling, harmonyCtx.noteCase, c, acc);
    } else if (item->harmonyType() == HarmonyType::NASHVILLE) {
        tpc2Function(harmonyCtx.tpc, key, acc, c);
    }

    if (acc.empty()) {
        return;
    }

    // Try to find token & execute renderlist
    ChordToken tok = chordList->token(acc, ChordTokenClass::ACCIDENTAL);
    if (tok.isValid()) {
        for (const RenderActionPtr& a : tok.renderList) {
            renderAction(item, ldata, a, harmonyCtx, ctx);
        }
        return;
    }

    // No valid token, find symbol

    // German spelling - use special symbol for accidental in TPC_B_B
    // to allow it to be rendered as either Bb or B
    if (harmonyCtx.tpc == Tpc::TPC_B_B && harmonyCtx.noteSpelling == NoteSpellingType::GERMAN) {
        context = u"german_B";
    }
    String lookup = context + acc;
    ChordSymbol cs = chordList->symbol(lookup);
    if (!cs.isValid()) {
        cs = chordList->symbol(acc);
    }
    String text = cs.isValid() ? cs.value : c;
    muse::draw::Font font = cs.isValid() ? ldata->fontList.value()[cs.fontIdx] : ldata->fontList.value().front();
    font.setPointSizeF(font.pointSizeF() * harmonyCtx.scale);

    TextSegment* ts = new TextSegment(text, font, harmonyCtx.x(), harmonyCtx.y(), harmonyCtx.hAlign);
    harmonyCtx.renderItemList.push_back(ts);
    harmonyCtx.movex(ts->width());
}

void HarmonyLayout::renderActionAlign(HarmonyRenderCtx& harmonyCtx)
{
    harmonyCtx.hAlign = false;
}

void HarmonyLayout::renderActionScale(const RenderActionScalePtr& a, HarmonyRenderCtx& harmonyCtx)
{
    harmonyCtx.scale *= a->scale();
}

void HarmonyLayout::renderActionParen(Harmony* item, const RenderActionParenPtr& a, HarmonyRenderCtx& harmonyCtx)
{
    Parenthesis* p = Factory::createParenthesis(item);
    p->setParent(item);
    p->setDirection(a->direction());
    p->setColor(item->color());
    p->setFollowParentColor(true);
    p->setGenerated(true);

    ChordSymbolParen* parenItem = new ChordSymbolParen(p, harmonyCtx.hAlign, harmonyCtx.x(), harmonyCtx.y());
    harmonyCtx.renderItemList.push_back(parenItem);
}

void HarmonyLayout::kernCharacters(const Harmony* item, const String& text, HarmonyRenderCtx& harmonyCtx, const LayoutContext& ctx)
{
    if (harmonyCtx.renderItemList.empty()) {
        return;
    }
    // Character pair and distance to move the second
    // Blank strings will apply the kerning no matter the following character
    // TODO - move this information to the XML file
    static const std::map<std::pair<String, String>, double> KERNED_CHARACTERS {
        { { u"A", u"\uE870" }, -0.4 },  // dim
        { { u"A", u"\uE871" }, -0.3 },  // half-dim
        { { u"\uE873", u"\uE870" }, -0.4 }, // triangle - dim
        { { u"\uE873", u"\uE871" }, -0.3 }, // triangle - half-dim
        { { u"A", u"/" }, 0.1 },

        { { u"A", u"\uE18E" }, -0.15 },  // dim JAZZ
        { { u"A", u"\uE18F" }, -0.15 },  // half-dim JAZZ
        { { u"\uE18A", u"\uE18E" }, -0.15 },  // triangle - dim JAZZ
        { { u"\uE18A", u"\uE18F" }, -0.15 },  // triangle - half-dim JAZZ

        { { u"\u266D", u"" }, -0.15 },  // b JAZZ
        { { u"\u266E", u"" }, -0.15 },  // natural JAZZ
        { { u"\u266F", u"" }, -0.15 },  // # JAZZ
        { { u"\u1D12A", u"" }, -0.15 }, // ## JAZZ
        { { u"\u1D12B", u"" }, -0.15 }, // bb JAZZ
    };

    HarmonyRenderItem* prevSeg = harmonyCtx.renderItemList.back();
    TextSegment* ts = dynamic_cast<TextSegment*>(prevSeg);
    if (!ts) {
        return;
    }

    for (auto& kernInfo : KERNED_CHARACTERS) {
        const std::pair<String, String> kernPair = kernInfo.first;
        bool endChar = ts->text().endsWith(kernPair.first);
        bool startChar = text.startsWith(kernPair.second);
        // VERY DIRTY HACK ALERT
        // "Match any" should only be applied to the Jazz preset currently.
        // Remove the jazz condition when these kern values are moved to the XML files
        if (kernPair.second.isEmpty() && ctx.conf().styleV(Sid::chordStyle).value<ChordStylePreset>() != ChordStylePreset::JAZZ) {
            continue;
        }
        bool startMatchAny = kernPair.second.isEmpty();
        if ((endChar && startChar) || (endChar && startMatchAny)) {
            const FontMetrics fm = FontMetrics(item->font());
            const double scale = harmonyCtx.scale * item->mag();
            harmonyCtx.pos = harmonyCtx.pos + PointF(kernInfo.second, 0.0) * FontMetrics::capHeight(item->font()) * scale;
            break;
        }
    }
}

void HarmonyLayout::renderActionSet(Harmony* item, Harmony::LayoutData* ldata, const RenderActionSetPtr& a, HarmonyRenderCtx& harmonyCtx,
                                    const LayoutContext& ctx)
{
    const ChordList* chordList = item->score()->chordList();
    const ChordSymbol cs = chordList->symbol(a->text());
    const String text = cs.isValid() ? cs.value : a->text();
    muse::draw::Font font = cs.isValid() ? ldata->fontList.value()[cs.fontIdx] : ldata->fontList.value().front();
    font.setPointSizeF(font.pointSizeF() * harmonyCtx.scale);
    if (item->harmonyType() == HarmonyType::NASHVILLE) {
        double nmag = chordList->nominalMag();
        font.setPointSizeF(font.pointSizeF() * nmag);
    }

    kernCharacters(item, text, harmonyCtx, ctx);

    TextSegment* ts = new TextSegment(text, font, harmonyCtx.x(), harmonyCtx.y(), harmonyCtx.hAlign);
    harmonyCtx.movex(ts->width());

    if (a->renderText()) {
        harmonyCtx.renderItemList.push_back(ts);
        return;
    }

    delete ts;
}

void HarmonyLayout::renderActionMove(Harmony* item, const RenderActionMovePtr& a, HarmonyRenderCtx& harmonyCtx)
{
    const double scale = (a->scaled() ? harmonyCtx.scale : 1.0) * item->mag();
    harmonyCtx.pos = harmonyCtx.pos + a->vec() * FontMetrics::capHeight(item->font()) * scale;
}

void HarmonyLayout::renderActionMoveXHeight(Harmony* item, const RenderActionMoveXHeightPtr& a, HarmonyRenderCtx& harmonyCtx)
{
    const int direction = a->up() ? -1 : 1;
    const double scale = a->scaled() ? harmonyCtx.scale : 1.0;
    const FontMetrics fm = FontMetrics(item->font());
    harmonyCtx.movey(direction * fm.xHeight() * scale);
}
