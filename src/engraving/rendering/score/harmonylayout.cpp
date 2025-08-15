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

#include "dom/fret.h"
#include "dom/harmony.h"
#include "draw/fontmetrics.h"
#include "dom/factory.h"

using namespace muse::draw;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

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

    if (!item->cursor()->editing()) {
        ParenthesisLayout::layoutParentheses(item, ctx);
    }
}

PointF HarmonyLayout::calculateBoundingRect(const Harmony* item, Harmony::LayoutData* ldata, const LayoutContext& ctx)
{
    const double ypos = (item->placeBelow() && item->staff()) ? item->staff()->staffHeight(item->tick()) : 0.0;
    const FretDiagram* fd = (item->explicitParent() && item->explicitParent()->isFretDiagram())
                            ? toFretDiagram(item->explicitParent())
                            : nullptr;
    const bool alignToFretDiagram = fd && fd->visible();

    const double cw = item->symWidth(SymId::noteheadBlack);

    double newPosX = 0.0;
    double newPosY = 0.0;

    if (item->ldata()->renderItemList().empty()) {
        TLayout::layoutBaseTextBase1(item, ldata);

        if (alignToFretDiagram) {
            newPosY = ldata->pos().y();
        } else {
            newPosY = ypos - ((item->align() == AlignV::BOTTOM) ? -ldata->bbox().height() : 0.0);
        }
    } else {
        layoutModifierParentheses(item);
        RectF bb;
        RectF hAlignBox;
        double segBl = 0.0;
        for (HarmonyRenderItem* renderItem : item->ldata()->renderItemList()) {
            RectF tsBbox = renderItem->tightBoundingRect().translated(renderItem->x(), renderItem->y());
            bb.unite(tsBbox);

            if (renderItem->align()) {
                hAlignBox.unite(tsBbox);
            }
            if (TextSegment* ts = dynamic_cast<TextSegment*>(renderItem)) {
                segBl = ts->bboxBaseLine();
            }
        }

        double xx = 0.0;
        if (alignToFretDiagram) {
            switch (ctx.conf().styleV(Sid::chordAlignmentToFretboard).value<AlignH>()) {
            case AlignH::LEFT:
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
            newPosY = ypos - yy - ctx.conf().styleMM(Sid::harmonyFretDist);
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
            newPosX = 0.0;
            break;
        case AlignH::HCENTER:
            newPosX = cw * 0.5;
            break;
        case AlignH::RIGHT:
            newPosX = cw;
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
    const std::vector<HarmonyRenderItem*>& itemList = item->ldata()->renderItemList();
    // Layout parentheses
    std::vector<ChordSymbolParen*> openingParenStack;
    for (HarmonyRenderItem* renderItem : itemList) {
        if (ChordSymbolParen* paren = dynamic_cast<ChordSymbolParen*>(renderItem)) {
            if (paren->paren->direction() == DirectionH::LEFT) {
                // Opening paren
                openingParenStack.push_back(paren);
            } else {
                // Closing paren
                ChordSymbolParen* openingParen = openingParenStack.empty() ? nullptr : openingParenStack.back();
                if (!openingParen) {
                    continue;
                }
                openingParenStack.pop_back();
                paren->top = openingParen->top;
                paren->bottom = openingParen->bottom;

                // Layout parenthesis pair
                double startY = openingParen->top;
                double height = openingParen->bottom - openingParen->top;
                double midPointThickness = height / 30 * openingParen->paren->ldata()->mag();
                double endPointThickness = 0.05;
                openingParen->paren->mutldata()->startY = startY;
                openingParen->sety(startY);
                openingParen->paren->mutldata()->height = height;
                openingParen->paren->mutldata()->midPointThickness.set_value(midPointThickness);
                openingParen->paren->mutldata()->endPointThickness.set_value(endPointThickness);

                paren->paren->mutldata()->startY = startY;
                paren->sety(startY);
                paren->paren->mutldata()->height = height;
                paren->paren->mutldata()->midPointThickness.set_value(midPointThickness);
                paren->paren->mutldata()->endPointThickness.set_value(endPointThickness);
                paren->setx(openingParen->closingParenPos);

                ParenthesisLayout::createPathAndShape(openingParen->paren, openingParen->paren->mutldata());
                ParenthesisLayout::createPathAndShape(paren->paren, paren->paren->mutldata());

                // Outer parens must always be the same length or longer than inner parens
                for (ChordSymbolParen* outerParen : openingParenStack) {
                    outerParen->top = std::min(openingParen->top, outerParen->top);
                    outerParen->bottom = std::max(openingParen->bottom, outerParen->bottom);
                }
            }
        } else if (TextSegment* ts = dynamic_cast<TextSegment*>(renderItem)) {
            // Set top paren height
            if (!openingParenStack.empty()) {
                ChordSymbolParen* topParen = openingParenStack.back();
                topParen->top = std::min(topParen->top, ts->boundingRect().translated(ts->pos()).y());
                topParen->bottom = std::max(topParen->bottom, ts->boundingRect().translated(ts->pos()).y() + ts->boundingRect().height());
                topParen->closingParenPos = std::max(topParen->closingParenPos, ts->x() + ts->width());
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
    for (size_t i = 0; i < itemList.size(); i++) {
        HarmonyRenderItem* renderItem = itemList.at(i);
        double padding = i != 0 ? computePadding(itemList.at(i - 1), renderItem) : 0.0;

        if (ChordSymbolParen* paren = dynamic_cast<ChordSymbolParen*>(renderItem)) {
            if (paren->paren->direction() == DirectionH::LEFT) {
                additionalSpace += paren->paren->width() + padding;
                paren->movex(additionalSpace);
            }
            if (paren->paren->direction() == DirectionH::RIGHT) {
                paren->movex(additionalSpace + padding);
                additionalSpace += paren->paren->width() + padding;
            }
        } else if (TextSegment* ts = dynamic_cast<TextSegment*>(renderItem)) {
            additionalSpace += padding;
            ts->movex(additionalSpace);
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
        ldata->fontList.mut_value().push_back(item->font());
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
    for (double width : muse::keys(chordTextSegments)) {
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

void HarmonyLayout::renderSingleHarmony(Harmony* item, Harmony::LayoutData* ldata, HarmonyRenderCtx& harmonyCtx,
                                        const LayoutContext& ctx)
{
    harmonyCtx.hAlign = true;
    HarmonyInfo* info = harmonyCtx.info;

    const MStyle& style = ctx.conf().style();

    int capo = style.styleI(Sid::capoPosition);

    ChordList* chordList = info->chordList();
    if (!chordList) {
        return;
    }

    NoteCaseType rootCase = item->rootRenderCase(info);
    NoteCaseType bassCase = item->bassRenderCase();

    NoteSpellingType spelling = style.styleV(Sid::chordSymbolSpelling).value<NoteSpellingType>();

    if (item->harmonyType() == HarmonyType::STANDARD && tpcIsValid(info->rootTpc())) {
        // render root
        render(item, ldata, chordList->renderListRoot, harmonyCtx, ctx, info->rootTpc(), spelling, rootCase);
        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            const bool stackModifiers = style.styleB(Sid::verticallyStackModifiers) && !item->doNotStackModifiers();
            render(item, ldata, stackModifiers ? cd->renderListStacked : cd->renderList, harmonyCtx, ctx, 0);
        }
    } else if (item->harmonyType() == HarmonyType::NASHVILLE && tpcIsValid(info->rootTpc())) {
        // render function
        render(item, ldata, chordList->renderListFunction, harmonyCtx, ctx, info->rootTpc(), spelling, bassCase);
        double adjust = chordList->nominalAdjust();
        harmonyCtx.movey(adjust * item->magS() * item->spatium() * .2);
        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            const bool stackModifiers = style.styleB(Sid::verticallyStackModifiers) && !item->doNotStackModifiers();
            render(item, ldata, stackModifiers ? cd->renderListStacked : cd->renderList, harmonyCtx, ctx, 0);
        }
    } else {
        render(item, ldata, info->textName(), harmonyCtx);
    }

    // render bass
    if (tpcIsValid(info->bassTpc())) {
        std::list<RenderActionPtr >& bassNoteChordList
            = style.styleB(Sid::chordBassNoteStagger) ? chordList->renderListBassOffset : chordList->renderListBass;
        render(item, ldata, bassNoteChordList, harmonyCtx, ctx, info->bassTpc(), spelling, bassCase, item->bassScale());
    }

    if (tpcIsValid(info->rootTpc()) && capo > 0 && capo < 12) {
        int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
        int capoRootTpc = info->rootTpc() + tpcOffset[capo];
        int capoBassTpc = info->bassTpc();

        if (tpcIsValid(capoBassTpc)) {
            capoBassTpc += tpcOffset[capo];
        }

        /*
         * For guitarists, avoid x and bb in Root or Bass,
         * and also avoid E#, B#, Cb and Fb in Root.
         */
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

        render(item, ldata, SymId::csymParensLeftTall, harmonyCtx, ctx);
        render(item, ldata, chordList->renderListRoot, harmonyCtx, ctx, capoRootTpc, spelling, rootCase);

        // render extension
        const ChordDescription* cd = info->getDescription();
        if (cd) {
            const bool stackModifiers = style.styleB(Sid::verticallyStackModifiers) && !item->doNotStackModifiers();
            render(item, ldata, stackModifiers ? cd->renderListStacked : cd->renderList, harmonyCtx, ctx,  0);
        }

        if (tpcIsValid(capoBassTpc)) {
            std::list<RenderActionPtr >& bassNoteChordList
                = style.styleB(Sid::chordBassNoteStagger) ? chordList->renderListBassOffset : chordList->renderListBass;
            render(item, ldata, bassNoteChordList, harmonyCtx, ctx, capoBassTpc, spelling, bassCase, item->bassScale());
        }
        render(item, ldata, SymId::csymParensRightTall, harmonyCtx, ctx);
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

void HarmonyLayout::render(Harmony* item, Harmony::LayoutData* ldata, const std::list<RenderActionPtr>& renderList,
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
        renderActionSet(item, ldata, std::static_pointer_cast<RenderActionSet>(a), harmonyCtx);
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
    font.setNoFontMerging(true);

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
    p->setDirection(a->direction());

    ChordSymbolParen* parenItem = new ChordSymbolParen(p, harmonyCtx.hAlign, harmonyCtx.x(), harmonyCtx.y());
    harmonyCtx.renderItemList.push_back(parenItem);
}

void HarmonyLayout::renderActionSet(Harmony* item, Harmony::LayoutData* ldata, const RenderActionSetPtr& a, HarmonyRenderCtx& harmonyCtx)
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
    const FontMetrics fm = FontMetrics(item->font());
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
