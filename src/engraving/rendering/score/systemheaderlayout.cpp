/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "systemheaderlayout.h"

#include "systemlayout.h"
#include "tlayout.h"

#include "dom/bracket.h"
#include "dom/factory.h"
#include "dom/part.h"
#include "style/defaultstyle.h"
#include "dom/system.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

double SystemHeaderLayout::layoutBrackets(System* system, LayoutContext& ctx)
{
    size_t nstaves = system->staves().size();
    size_t columns = system->getBracketsColumnsCount();

    std::vector<double> bracketWidth(columns, 0.0);

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                Bracket* b = createBracket(system, ctx, bi, i, static_cast<int>(staffIdx), bl, system->firstMeasure());
                if (b != nullptr) {
                    b->mutldata()->bracketHeight.set_value(3.5 * b->spatium() * 2); // dummy
                    TLayout::layoutBracket(b, b->mutldata(), ctx.conf());
                    if (bi->bracketType() != BracketType::GROUP) { // Because Group brackets always stay outside
                        bracketWidth[i] = std::max(bracketWidth[i], b->ldata()->bracketWidth());
                    }
                }
            }
        }
    }

    for (Bracket* b : bl) {
        delete b;
    }

    double totalBracketWidth = 0.0;

    if (!system->brackets().empty()) {
        for (double w : bracketWidth) {
            totalBracketWidth += w;
        }
    }

    return totalBracketWidth;
}

Bracket* SystemHeaderLayout::createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                           std::vector<Bracket*>& bl, Measure* measure)
{
    if (!measure) {
        return nullptr;
    }

    size_t nstaves = system->staves().size();
    staff_idx_t firstStaff = staffIdx;
    staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
    if (lastStaff >= nstaves) {
        lastStaff = nstaves - 1;
    }

    for (; firstStaff <= lastStaff; ++firstStaff) {
        if (system->staff(firstStaff)->show()) {
            break;
        }
    }
    for (; lastStaff >= firstStaff; --lastStaff) {
        if (system->staff(lastStaff)->show()) {
            break;
        }
    }
    size_t span = lastStaff - firstStaff + 1;
    //
    // do not show bracket if it only spans one
    // system due to some invisible staves
    //
    if (span > 1
        || (bi->bracketSpan() == span)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() != BracketType::SQUARE)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowSquareBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() == BracketType::SQUARE)) {
        //
        // this bracket is visible
        //
        Bracket* b = 0;
        track_idx_t track = staffIdx * VOICES;
        for (size_t k = 0; k < bl.size(); ++k) {
            if (bl[k]->track() == track && bl[k]->column() == column && bl[k]->bracketType() == bi->bracketType()
                && bl[k]->measure() == measure) {
                b = muse::takeAt(bl, k);
                break;
            }
        }
        if (b == 0) {
            b = Factory::createBracket(ctx.mutDom().dummyParent());
            b->setBracketItem(bi);
            b->setGenerated(true);
            b->setTrack(track);
            b->setMeasure(measure);
        }
        system->add(b);

        if (bi->selected()) {
            bool needSelect = true;

            std::vector<EngravingItem*> brackets = ctx.selection().elements(ElementType::BRACKET);
            for (const EngravingItem* element : brackets) {
                if (toBracket(element)->bracketItem() == bi) {
                    needSelect = false;
                    break;
                }
            }

            if (needSelect) {
                ctx.select(b, SelectType::ADD);
            }
        }

        b->setStaffSpan(firstStaff, lastStaff);

        return b;
    }

    return nullptr;
}

void SystemHeaderLayout::addBrackets(System* system, Measure* measure, LayoutContext& ctx)
{
    if (system->staves().empty()) {                 // ignore vbox
        return;
    }

    size_t nstaves = system->staves().size();

    //---------------------------------------------------
    //  find x position of staves
    //    create brackets
    //---------------------------------------------------

    size_t columns = system->getBracketsColumnsCount();

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                createBracket(system, ctx, bi, i, staffIdx, bl, measure);
            }
        }
        if (!system->staff(staffIdx)->show()) {
            continue;
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------
    layoutBracketsVertical(system, ctx);

    setBracketsXPosition(system, measure->x());

    muse::join(system->brackets(), bl);
}

double SystemHeaderLayout::totalBracketOffset(LayoutContext& ctx)
{
    if (ctx.state().totalBracketsWidth() >= 0) {
        return ctx.state().totalBracketsWidth();
    }

    size_t columns = 0;
    for (const Staff* staff : ctx.dom().staves()) {
        for (const BracketItem* bi : staff->brackets()) {
            columns = std::max(columns, bi->column() + 1);
        }
    }

    size_t nstaves = ctx.dom().nstaves();
    std::vector < double > bracketWidth(nstaves, 0.0);
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* staff = ctx.dom().staff(staffIdx);
        for (auto bi : staff->brackets()) {
            if (bi->bracketType() == BracketType::NO_BRACKET || !bi->visible() || bi->bracketType() == BracketType::GROUP) {
                continue;
            }

            //! This logic is partially copied from System::createBracket.
            //! Of course, we don't need to worry about invisible staves,
            //! but we do need to worry about brackets that span past the
            //! last staff.
            staff_idx_t firstStaff = staffIdx;
            staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
            if (lastStaff >= nstaves) {
                lastStaff = nstaves - 1;
            }

            for (; firstStaff <= lastStaff; ++firstStaff) {
                if (ctx.dom().staff(firstStaff)->show()) {
                    break;
                }
            }
            for (; lastStaff >= firstStaff; --lastStaff) {
                if (ctx.dom().staff(lastStaff)->show()) {
                    break;
                }
            }

            size_t span = lastStaff - firstStaff + 1;
            if (span > 1
                || (bi->bracketSpan() == span)
                || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden))) {
                Bracket* dummyBr = Factory::createBracket(ctx.mutDom().dummyParent(), /*isAccessibleEnabled=*/ false);
                dummyBr->setBracketItem(bi);
                dummyBr->setStaffSpan(firstStaff, lastStaff);
                dummyBr->mutldata()->bracketHeight.set_value(3.5 * dummyBr->spatium() * 2); // default
                TLayout::layoutBracket(dummyBr, dummyBr->mutldata(), ctx.conf());
                for (staff_idx_t stfIdx = firstStaff; stfIdx <= lastStaff; ++stfIdx) {
                    bracketWidth[stfIdx] += dummyBr->ldata()->bracketWidth();
                }
                delete dummyBr;
            }
        }
    }

    double totalBracketsWidth = 0.0;
    for (double w : bracketWidth) {
        totalBracketsWidth = std::max(totalBracketsWidth, w);
    }
    ctx.mutState().setTotalBracketsWidth(totalBracketsWidth);

    return totalBracketsWidth;
}

void SystemHeaderLayout::setBracketsXPosition(System* system, double xPosition)
{
    std::vector<Bracket*> brackets = system->brackets();
    for (Bracket* b1 : brackets) {
        BracketType bracketType = b1->bracketType();
        // For brackets that are drawn, we must correct for half line width
        double lineWidthCorrection = 0.0;
        if (bracketType == BracketType::NORMAL || bracketType == BracketType::LINE) {
            lineWidthCorrection = system->style().styleAbsolute(Sid::bracketWidth) / 2;
        }
        // Compute offset cause by other stacked brackets
        double xOffset = 0;
        for (const Bracket* b2 : brackets) {
            if (!b2->bracketItem()->visible() || b2->bracketType() == BracketType::GROUP) {
                continue;
            }
            if (b1->column() > b2->column() && b1->intersects(b2)) {
                xOffset += b2->ldata()->bracketWidth();
            }
        }
        // Set position
        double x = xPosition - xOffset - b1->ldata()->bracketWidth() + lineWidthCorrection;
        b1->mutldata()->setPosX(x);
    }
}

void SystemHeaderLayout::layoutBracketsVertical(System* system, LayoutContext& ctx)
{
    for (Bracket* b : system->brackets()) {
        int staffIdx1 = static_cast<int>(b->firstStaff());
        int staffIdx2 = static_cast<int>(b->lastStaff());
        double sy = 0;                           // assume bracket not visible
        double ey = 0;
        // if start staff not visible, try next staff
        while (staffIdx1 <= staffIdx2 && !system->staves().at(staffIdx1)->show()) {
            ++staffIdx1;
        }
        // if end staff not visible, try prev staff
        while (staffIdx1 <= staffIdx2 && !system->staves().at(staffIdx2)->show()) {
            --staffIdx2;
        }
        // if the score doesn't have "alwaysShowBracketsWhenEmptyStavesAreHidden" as true,
        // the bracket will be shown IF:
        // it spans at least 2 visible staves (staffIdx1 < staffIdx2) OR
        // it spans just one visible staff (staffIdx1 == staffIdx2) but it is required to do so
        // (the second case happens at least when the bracket is initially dropped)
        bool notHidden = ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
                         ? (staffIdx1 <= staffIdx2) : (staffIdx1 < staffIdx2) || (b->span() == 1 && staffIdx1 == staffIdx2);
        if (notHidden) {                        // set vert. pos. and height to visible spanned staves
            sy = system->staves().at(staffIdx1)->bbox().top();
            ey = system->staves().at(staffIdx2)->bbox().bottom();
        }

        Bracket::LayoutData* bldata = b->mutldata();
        bldata->setPosY(sy);
        bldata->bracketHeight = ey - sy;
        TLayout::layoutBracket(b, bldata, ctx.conf());
    }
}

bool SystemHeaderLayout::stackLabelsVertically(System* system)
{
    const MStyle& style = system->style();
    bool longName = system->ldata()->useLongNames();
    InstrumentNamesAlign align
        = style.styleV(longName ? Sid::instrumentNamesAlignLong : Sid::instrumentNamesAlignShort).value<InstrumentNamesAlign>();
    return align == InstrumentNamesAlign::CENTER_CENTER
           || (align == InstrumentNamesAlign::RIGHT_RIGHT && style.styleB(Sid::instrumentNamesStackVertically));
}

void SystemHeaderLayout::computeInstrumentNameOffset(System* system, LayoutContext& ctx)
{
    // Get standard instrument name distance
    double instrumentNameOffset = ctx.conf().styleAbsolute(Sid::instrumentNameOffset);
    // Now scale it depending on the text size (which also may not follow staff scaling)
    double textSizeScaling = 1.0;
    double actualSize = 0.0;
    double defaultSize = 0.0;
    bool followStaffSize = true;
    if (ctx.state().startWithLongNames()) {
        actualSize = ctx.conf().styleD(Sid::longInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::longInstrumentFontSize).toDouble();
        followStaffSize = ctx.conf().styleB(Sid::longInstrumentFontSpatiumDependent);
    } else {
        actualSize = ctx.conf().styleD(Sid::shortInstrumentFontSize);
        defaultSize = DefaultStyle::defaultStyle().value(Sid::shortInstrumentFontSize).toDouble();
        followStaffSize = ctx.conf().styleB(Sid::shortInstrumentFontSpatiumDependent);
    }
    textSizeScaling = actualSize / defaultSize;
    if (!followStaffSize) {
        textSizeScaling *= DefaultStyle::defaultStyle().value(Sid::spatium).toDouble() / ctx.conf().styleD(Sid::spatium);
    }
    textSizeScaling = std::max(textSizeScaling, 1.0);
    instrumentNameOffset *= textSizeScaling;

    system->mutldata()->setInstrumentNameOffset(instrumentNameOffset);
}

void SystemHeaderLayout::computeGroupBracketsWidths(System* system, LayoutContext& ctx)
{
    System::LayoutData* ldata = system->mutldata();
    ldata->clearGroupBracketsWidth();

    std::vector<Bracket*> groupBrackets;
    for (Bracket* bracket : system->brackets()) {
        if (bracket->bracketType() == BracketType::GROUP) {
            groupBrackets.push_back(bracket);
        }
    }

    if (groupBrackets.empty()) {
        return;
    }

    std::sort(groupBrackets.begin(), groupBrackets.end(), [](Bracket* b1, Bracket* b2) { return b1->column() < b2->column(); });

    bool hangIntoMargin = ctx.conf().styleB(Sid::groupBracketHangTextIntoMargin)
                          && ctx.conf().styleV(Sid::groupBracketTextOrientation).value<Orientation>() == Orientation::VERTICAL;
    double bracketDistToNames = ctx.conf().styleAbsolute(Sid::groupBracketDistanceToNames);

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        Bracket* firstBracketOnThisStaff = nullptr;
        for (Bracket* b : groupBrackets) {
            if (b->contains(staffIdx)) {
                firstBracketOnThisStaff = b;
                break;
            }
        }

        if (!firstBracketOnThisStaff) {
            continue;
        }

        std::vector<Bracket*> stack;
        double bracketWidth = 0.0;
        computeStackedBracketsWidth(firstBracketOnThisStaff, groupBrackets, bracketWidth, stack);
        if (hangIntoMargin) {
            bracketWidth += stack.back()->ldata()->bbox().left();
        }

        ldata->setGroupBracketsWidthAtStaffIdx(staffIdx, bracketWidth + bracketDistToNames);
    }

    if (!ctx.conf().styleB(Sid::instrumentNamesAlignIncludeGroupBrackets)) {
        double maxGroupBracketWidth = 0.0;
        for (const auto& pair : ldata->groupBracketsWidth()) {
            maxGroupBracketWidth = std::max(maxGroupBracketWidth, pair.second);
        }
        for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
            ldata->setGroupBracketsWidthAtStaffIdx(staffIdx, maxGroupBracketWidth);
        }
    }
}

void SystemHeaderLayout::computeStackedBracketsWidth(Bracket* first, const std::vector<Bracket*>& allGroupBracketsOrderedByColumn,
                                                     double& width, std::vector<Bracket*>& stack)
{
    stack.push_back(first);

    width += first->ldata()->bbox().width();
    double bracketDist = first->style().styleAbsolute(Sid::groupBracketDistanceToGroupBracket);

    for (Bracket* b2 : allGroupBracketsOrderedByColumn) {
        if (muse::contains(stack, b2)) {
            continue;
        }
        if (b2->column() > first->column() && b2->intersects(first)) {
            stack.push_back(b2);
            computeStackedBracketsWidth(b2, allGroupBracketsOrderedByColumn, width, stack);
            width += bracketDist;
        }
    }
}

double SystemHeaderLayout::nameWidthIncludingGroupBrackets(InstrumentName* name, System* system)
{
    const System::LayoutData* ldata = system->ldata();
    double groupBracketsWidth = ldata->groupBracketsWidthAtStaffIdx(name->effectiveStaffIdx());
    double result = name->width();
    if (!muse::RealIsNull(groupBracketsWidth)) {
        result += groupBracketsWidth;
    }
    return result;
}

void SystemHeaderLayout::computeInstrumentNamesWidth(System* system, LayoutContext& ctx)
{
    computeGroupBracketsWidths(system, ctx);

    System::LayoutData* ldata = system->mutldata();
    ldata->setFirstColumnWidth(0.0);
    ldata->setSecondColumnWidth(0.0);
    ldata->setTotalNamesWidth(0.0);

    std::unordered_set<Part*> partsWithIndividualStaffNames;
    std::unordered_set<Part*> partsWithGroupNames;

    std::vector<InstrumentName*> groupNames;
    std::vector<InstrumentName*> instrumentNames;

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* sysStaff = system->staff(staffIdx);
        const Staff* staff = ctx.dom().staff(staffIdx);

        if (InstrumentName* groupName = sysStaff->groupName) {
            if (ldata->useLongNames() && groupName->effectiveStaffIdx() == muse::nidx) {
                // NOTE: We can only do this for long-name systems because they don't need to have
                // the left barline aligned with the other systems across the page.
                groupName->mutldata()->setIsSkipDraw(true);
                continue;
            }
            groupName->mutldata()->setIsSkipDraw(false);

            groupNames.push_back(groupName);
            groupName->mutldata()->setColumn(1);
            for (staff_idx_t groupIdx = groupName->staffIdx(); groupIdx < groupName->ldata()->endIdxOfGroup(); ++groupIdx) {
                partsWithGroupNames.insert(ctx.dom().staff(groupIdx)->part());
            }
            ldata->setSecondColumnWidth(std::max(ldata->secondColumnWidth(), nameWidthIncludingGroupBrackets(groupName, system)));
            ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), nameWidthIncludingGroupBrackets(groupName, system)));
        }

        if (!staff->show()) {
            // We know that the staff is hidden in the entire score so safe to skip
            continue;
        }

        if (InstrumentName* name = sysStaff->individualStaffName) {
            if (ldata->useLongNames() && !sysStaff->show()) {
                // NOTE: We can only do this for long-name systems because they don't need to have
                // the left barline aligned with the other systems across the page.
                name->mutldata()->setIsSkipDraw(true);
                continue;
            }
            name->mutldata()->setIsSkipDraw(false);

            name->mutldata()->setColumn(0);
            partsWithIndividualStaffNames.insert(ctx.dom().staff(staffIdx)->part());
            ldata->setFirstColumnWidth(std::max(ldata->firstColumnWidth(), name->width()));
            ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), nameWidthIncludingGroupBrackets(name, system)));
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* sysStaff = system->staff(staffIdx);
        const Staff* staff = ctx.dom().staff(staffIdx);
        Part* part = staff->part();

        if (!part->show() || part->visibleStavesCount() == 0) {
            // We know that the part is hidden in the entire score so safe to skip
            continue;
        }

        InstrumentName* instrName = sysStaff->instrumentName;
        if (!instrName) {
            continue;
        }

        if (ldata->useLongNames() && instrName->effectiveStaffIdx() == muse::nidx) {
            // NOTE: We can only do this for long-name systems because they don't need to have
            // the left barline aligned with the other systems across the page.
            instrName->mutldata()->setIsSkipDraw(true);
            continue;
        }
        instrName->mutldata()->setIsSkipDraw(false);

        instrumentNames.push_back(instrName);
        ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), nameWidthIncludingGroupBrackets(instrName, system)));

        if (partsWithGroupNames.count(part)) {
            instrName->mutldata()->setColumn(0);
            ldata->setFirstColumnWidth(std::max(ldata->firstColumnWidth(), instrName->width()));
        } else {
            instrName->mutldata()->setColumn(1);
            ldata->setSecondColumnWidth(std::max(ldata->secondColumnWidth(), nameWidthIncludingGroupBrackets(instrName, system)));
        }
    }

    if (stackLabelsVertically(system)) {
        return;
    }

    auto sumWidth = [&](InstrumentName* outerName, InstrumentName* innerName) {
        AlignH staffNameAlign = innerName->position();
        if (staffNameAlign == AlignH::LEFT || staffNameAlign == AlignH::JUSTIFY) {
            return ldata->firstColumnWidth() + nameWidthIncludingGroupBrackets(outerName, system) + ldata->instrumentNameOffset();
        } else if (staffNameAlign == AlignH::HCENTER) {
            double sumWidth = innerName->width() + nameWidthIncludingGroupBrackets(outerName, system) + ldata->instrumentNameOffset();
            double move = 0.5 * (ldata->firstColumnWidth() - innerName->width());
            return sumWidth + move;
        } else {
            return innerName->width() + ldata->instrumentNameOffset() + nameWidthIncludingGroupBrackets(outerName, system);
        }
    };

    for (InstrumentName* groupName : groupNames) {
        staff_idx_t startStaff = groupName->staffIdx();
        staff_idx_t endStaff = groupName->ldata()->endIdxOfGroup();
        if (ldata->useLongNames() && system->visiblePartsOfGroup(startStaff, endStaff).size() % 2 == 0) {
            continue;
        }
        for (staff_idx_t idx = startStaff; idx < endStaff; ++idx) {
            if (InstrumentName* n = system->staff(idx)->individualStaffName; n && !n->ldata()->isSkipDraw()) {
                ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), sumWidth(groupName, n)));
            }
            if (InstrumentName* n = system->staff(idx)->instrumentName; n && !n->ldata()->isSkipDraw()) {
                ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), sumWidth(groupName, n)));
            }
        }
    }

    for (InstrumentName* instrName : instrumentNames) {
        if (instrName->ldata()->column() == 0) {
            continue;
        }
        Part* part = ctx.dom().staff(instrName->staffIdx())->part();
        if (ldata->useLongNames() && system->visibleStavesOfPart(part).size() % 2 == 0) {
            continue;
        }
        for (staff_idx_t idx : part->staveIdxList()) {
            if (InstrumentName* n = system->staff(idx)->individualStaffName; n && !n->ldata()->isSkipDraw()) {
                ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), sumWidth(instrName, n)));
            }
        }
    }
}

void SystemHeaderLayout::setInstrumentNamesVerticalPos(System* system, LayoutContext& ctx)
{
    std::set<const Part*> partsWithIndividualStaffNames;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        SysStaff* s = system->staff(staffIdx);
        if (InstrumentName* individualName = s->individualStaffName) {
            const RectF& staffBBox = s->bbox();
            const RectF& nameBBox = individualName->ldata()->bbox();
            individualName->mutldata()->setPosY(0.5 * (staffBBox.top() + staffBBox.bottom() - nameBBox.bottom() - nameBBox.top()));
            partsWithIndividualStaffNames.insert(ctx.dom().staff(staffIdx)->part());
        }
    }

    bool stackVertically = stackLabelsVertically(system);

    staff_idx_t staffIdx = 0;
    for (const Part* p : ctx.dom().parts()) {
        size_t nstaves = p->nstaves();

        SysStaff* s = system->staff(staffIdx);

        InstrumentName* t = s->instrumentName;
        if (!t || t->effectiveStaffIdx() == muse::nidx) {
            staffIdx += nstaves;
            continue;
        }

        const RectF& bbox = t->ldata()->bbox();
        double yCenter = 0.5 * (bbox.bottom() + bbox.top());

        std::vector<staff_idx_t> visibleStavesOfPart = system->visibleStavesOfPart(p);
        size_t visibleStavesCount = visibleStavesOfPart.size();
        DO_ASSERT(visibleStavesCount > 0);

        double y1 = 0;
        double y2 = 0;

        if (!muse::contains(partsWithIndividualStaffNames, p)) {
            SysStaff* topSt = system->staff(visibleStavesOfPart.front());
            SysStaff* bottomSt = system->staff(visibleStavesOfPart.back());
            y1 = topSt->bbox().top();
            y2 = bottomSt->bbox().bottom();
            t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);

            staffIdx += nstaves;
            continue;
        }

        if (visibleStavesCount % 2 == 0) {
            SysStaff* staffAboveMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2 - 1]);
            SysStaff* staffBelowMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
            y1 = staffAboveMid->bbox().top();
            y2 = staffBelowMid->bbox().bottom();
            t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);

            staffIdx += nstaves;
            continue;
        }

        SysStaff* midStaff = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
        y1 = midStaff->bbox().top();
        y2 = midStaff->bbox().bottom();
        if (InstrumentName* staffName = midStaff->individualStaffName) {
            if (stackVertically || t->ldata()->column() == 0) {
                double lineSpacing = t->lineSpacing();
                double instrNameBottom = t->ldata()->blocks.back().y();
                double centerY = 0.5 * (midStaff->bbox().top() + midStaff->bbox().bottom());
                t->mutldata()->setPosY(centerY - instrNameBottom - 0.25 * lineSpacing);
                double staffNameTop = staffName->ldata()->blocks.front().y();
                staffName->mutldata()->setPosY(centerY - staffNameTop + 0.75 * lineSpacing);
            } else if (staffName->ldata()->rows() == 1 && t->ldata()->rows() == 1) {
                t->mutldata()->setPosY(staffName->y());
            } else {
                t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            }
        }

        staffIdx += nstaves;
    }

    for (staff_idx_t idx = 0; idx < system->staves().size(); ++idx) {
        InstrumentName* groupName = system->staff(idx)->groupName;
        if (!groupName || groupName->effectiveStaffIdx() == muse::nidx) {
            continue;
        }

        const RectF& bbox = groupName->ldata()->bbox();
        double yCenter = 0.5 * (bbox.bottom() + bbox.top());

        std::vector<Part*> visibleParts = system->visiblePartsOfGroup(idx, groupName->ldata()->endIdxOfGroup());
        size_t visiblePartsCount = visibleParts.size();
        DO_ASSERT(visiblePartsCount > 0);

        double y1 = 0;
        double y2 = 0;

        if (visiblePartsCount % 2 == 0) {
            Part* partAboveMid = visibleParts[visiblePartsCount / 2 - 1];
            staff_idx_t staffIdxAboveMid = system->lastVisibleSysStaffOfPart(partAboveMid);
            DO_ASSERT(staffIdxAboveMid != muse::nidx);
            SysStaff* staffAboveMid = system->staff(staffIdxAboveMid);

            Part* partBelowMid = visibleParts[visiblePartsCount / 2];
            staff_idx_t staffIdxBelowMid = system->firstVisibleSysStaffOfPart(partBelowMid);
            DO_ASSERT(staffIdxBelowMid != muse::nidx);
            SysStaff* staffBelowMid = system->staff(staffIdxBelowMid);

            y1 = staffAboveMid->bbox().top();
            y2 = staffBelowMid->bbox().bottom();
            groupName->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);

            continue;
        }

        Part* midPart = visibleParts[visiblePartsCount / 2];
        InstrumentName* instrName = system->staff(*midPart->staveIdxList().begin())->instrumentName;
        std::vector<staff_idx_t> visibleStaves = system->visibleStavesOfPart(midPart);
        size_t visibleStavesCount = visibleStaves.size();

        if (visibleStavesCount % 2) {
            SysStaff* midStaff = system->staff(visibleStaves[visibleStavesCount / 2]);
            y1 = midStaff->bbox().top();
            y2 = midStaff->bbox().bottom();
        } else {
            SysStaff* staffAboveMid = system->staff(visibleStaves[visibleStavesCount / 2 - 1]);
            SysStaff* staffBelowMid = system->staff(visibleStaves[visibleStavesCount / 2]);
            y1 = staffAboveMid->bbox().top();
            y2 = staffBelowMid->bbox().bottom();
        }

        if (instrName) {
            if (stackVertically) {
                double lineSpacing = groupName->lineSpacing();
                double groupNameBottom = groupName->ldata()->blocks.back().y();
                if (visibleStavesCount % 2 && system->staff(visibleStaves[visibleStavesCount / 2])->individualStaffName) {
                    groupName->mutldata()->setPosY(instrName->y() - groupNameBottom - lineSpacing);
                } else {
                    double centerY = 0.5 * (y1 + y2);
                    groupName->mutldata()->setPosY(centerY - groupNameBottom - 0.25 * lineSpacing);
                    double instrNameTop = instrName->ldata()->blocks.front().y();
                    instrName->mutldata()->setPosY(centerY - instrNameTop + 0.75 * lineSpacing);
                }
            } else if (instrName->ldata()->rows() == 1 && groupName->ldata()->rows() == 1) {
                groupName->mutldata()->setPosY(instrName->y());
            } else {
                groupName->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            }

            continue;
        }

        if (visibleStavesCount % 2 == 0) {
            groupName->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            continue;
        }

        SysStaff* midStaff = system->staff(visibleStaves[visibleStavesCount / 2]);
        InstrumentName* staffName = midStaff->individualStaffName;
        if (!staffName) {
            groupName->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            continue;
        }

        if (stackVertically) {
            double lineSpacing = groupName->lineSpacing();
            double groupNameBottom = groupName->ldata()->blocks.back().y();
            double centerY = 0.5 * (y1 + y2);
            groupName->mutldata()->setPosY(centerY - groupNameBottom - 0.25 * lineSpacing);
            double staffNameTop = staffName->ldata()->blocks.front().y();
            staffName->mutldata()->setPosY(centerY - staffNameTop + 0.75 * lineSpacing);
        } else if (staffName->ldata()->rows() == 1 && groupName->ldata()->rows() == 1) {
            groupName->mutldata()->setPosY(staffName->y());
        } else {
            groupName->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
        }
    }
}

void SystemHeaderLayout::setInstrumentNamesHorizontalPos(System* system)
{
    //---------------------------------------------------
    //  layout instrument names x position
    //     at this point it is not clear which staves will
    //     be hidden, so layout all instrument names
    //---------------------------------------------------

    System::LayoutData* ldata = system->mutldata();
    double totalNamesWidth = ldata->totalNamesWidth();
    double firstColumnWidth = ldata->firstColumnWidth();
    InstrumentNamesAlign align = system->style().styleV(
        ldata->useLongNames() ? Sid::instrumentNamesAlignLong : Sid::instrumentNamesAlignShort).value<InstrumentNamesAlign>();

    auto placeFirstColumnName = [&](InstrumentName* name) {
        const RectF& bbox = name->ldata()->bbox();
        if (align == InstrumentNamesAlign::CENTER_CENTER) {
            double groupBracketsWidth = ldata->groupBracketsWidthAtStaffIdx(name->effectiveStaffIdx());
            name->mutldata()->setPosX(0.5 * totalNamesWidth - 0.5 * (bbox.right() + bbox.left()) + groupBracketsWidth);
        } else {
            switch (name->position()) {
            case AlignH::JUSTIFY:
            case AlignH::LEFT:
                name->mutldata()->setPosX(totalNamesWidth - firstColumnWidth - bbox.left());
                break;
            case AlignH::HCENTER:
                name->mutldata()->setPosX(totalNamesWidth - 0.5 * firstColumnWidth - 0.5 * (bbox.right() + bbox.left()));
                break;
            case AlignH::RIGHT:
                name->mutldata()->setPosX(totalNamesWidth - bbox.right());
            }
        }
    };

    for (const SysStaff* s : system->staves()) {
        if (InstrumentName* n = s->individualStaffName) {
            placeFirstColumnName(n);
        }
        if (InstrumentName* n = s->instrumentName; n && n->ldata()->column() == 0 && n->effectiveStaffIdx() != muse::nidx) {
            placeFirstColumnName(n);
        }
    }

    bool stackVertically = stackLabelsVertically(system);

    auto placeSecondColumnName = [&](InstrumentName* name, staff_idx_t staffIdx) {
        const RectF& bbox = name->ldata()->bbox();
        double groupBracketsWidth = ldata->groupBracketsWidthAtStaffIdx(name->effectiveStaffIdx());

        if (align == InstrumentNamesAlign::LEFT_RIGHT) {
            name->mutldata()->setPosX(groupBracketsWidth - bbox.left());
            return;
        }

        if (align == InstrumentNamesAlign::CENTER_CENTER) {
            name->mutldata()->setPosX(0.5 * totalNamesWidth - 0.5 * (bbox.right() + bbox.left()) + 0.5 * groupBracketsWidth);
            return;
        }

        if (align == InstrumentNamesAlign::CENTER_RIGHT) {
            name->mutldata()->setPosX(0.5 * ldata->secondColumnWidth() - 0.5 * (bbox.right() + bbox.left()) + 0.5 * groupBracketsWidth);
            return;
        }

        if (stackVertically) {
            name->mutldata()->setPosX(totalNamesWidth - bbox.right());
            return;
        }

        if (name->instrumentNameRole() == InstrumentNameRole::PART) {
            const Part* p = system->score()->staff(staffIdx)->part();
            std::vector<staff_idx_t> visibleStavesForPart = system->visibleStavesOfPart(p);
            size_t visibleStaveCount = visibleStavesForPart.size();
            if (visibleStaveCount % 2 == 0) {
                name->mutldata()->setPosX(totalNamesWidth - bbox.right());
                return;
            }

            staff_idx_t centerStaff = visibleStavesForPart[visibleStaveCount / 2];
            if (InstrumentName* staffName = system->staff(centerStaff)->individualStaffName) {
                name->mutldata()->setPosX(
                    staffName->x() + staffName->ldata()->bbox().left() - bbox.right() - ldata->instrumentNameOffset());
            } else {
                name->mutldata()->setPosX(totalNamesWidth - bbox.right());
            }

            return;
        }

        std::vector<Part*> visiblePartsOfGroup = system->visiblePartsOfGroup(staffIdx, name->ldata()->endIdxOfGroup());
        size_t visiblePartsCount = visiblePartsOfGroup.size();
        if (visiblePartsCount % 2 == 0) {
            name->mutldata()->setPosX(totalNamesWidth - bbox.right());
            return;
        }

        Part* centerPart = visiblePartsOfGroup[visiblePartsCount / 2];
        if (InstrumentName* instrName = system->staff(*centerPart->staveIdxList().begin())->instrumentName) {
            name->mutldata()->setPosX(
                instrName->x() + instrName->ldata()->bbox().left() - bbox.right() - ldata->instrumentNameOffset());
            return;
        }

        std::vector<staff_idx_t> visibleStavesOfPart = system->visibleStavesOfPart(centerPart);
        size_t visibleStavesCount = visibleStavesOfPart.size();
        if (visibleStavesCount % 2 == 0) {
            name->mutldata()->setPosX(totalNamesWidth - bbox.right());
            return;
        }

        staff_idx_t centerStaff = visibleStavesOfPart[visibleStavesCount / 2];
        if (InstrumentName* staffName = system->staff(centerStaff)->individualStaffName) {
            name->mutldata()->setPosX(
                staffName->x() + staffName->ldata()->bbox().left() - bbox.right() - ldata->instrumentNameOffset());
        } else {
            name->mutldata()->setPosX(totalNamesWidth - bbox.right());
        }
    };

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* s = system->staff(staffIdx);
        InstrumentName* instrName = s->instrumentName;
        if (instrName && instrName->effectiveStaffIdx() != muse::nidx && instrName->ldata()->column() > 0) {
            placeSecondColumnName(instrName, staffIdx);
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* s = system->staff(staffIdx);
        InstrumentName* groupName = s->groupName;
        if (groupName && groupName->effectiveStaffIdx() != muse::nidx) {
            placeSecondColumnName(groupName, staffIdx);
        }
    }
}

void SystemHeaderLayout::setGroupBracketsHorizontalPos(System* system)
{
    std::vector<Bracket*> groupBrackets;
    for (Bracket* b : system->brackets()) {
        if (b->bracketType() == BracketType::GROUP) {
            groupBrackets.push_back(b);
        }
    }
    std::sort(groupBrackets.begin(), groupBrackets.end(), [](Bracket* b1, Bracket* b2) { return b1->column() > b2->column(); });

    const MStyle& style = system->style();
    bool intoMargin = style.styleB(Sid::groupBracketHangTextIntoMargin)
                      && style.styleV(Sid::groupBracketTextOrientation).value<Orientation>() == Orientation::VERTICAL;
    double bracketsDist = style.styleAbsolute(Sid::groupBracketDistanceToGroupBracket);

    for (Bracket* b : groupBrackets) {
        b->mutldata()->setPosX(intoMargin ? 0.0 : -b->mutldata()->bbox().left());
        for (Bracket* bb : groupBrackets) {
            if (bb == b) {
                break;
            }
            if (bb->column() > b->column() && bb->intersects(b)) {
                b->mutldata()->setPosX(bb->x() + bb->ldata()->bbox().right() - b->ldata()->bbox().left() + bracketsDist);
            }
        }
    }
}

InstrumentName* SystemHeaderLayout::updateName(System* system, staff_idx_t staffIdx, LayoutContext& ctx, const String& name,
                                               InstrumentNameType type, InstrumentNameRole role)
{
    SysStaff* sysStaff = system->staff(staffIdx);
    InstrumentName* iname = role == InstrumentNameRole::GROUP ? sysStaff->groupName
                            : role == InstrumentNameRole::PART ? sysStaff->instrumentName : sysStaff->individualStaffName;
    if (name.empty()) {
        if (iname) {
            ctx.mutDom().removeElement(iname);
        }
        return nullptr;
    }

    if (!iname) {
        iname = new InstrumentName(system);
        iname->setGenerated(true);
        iname->setParent(system);
        iname->setSysStaff(sysStaff);
        iname->setTrack(staffIdx * VOICES);
        iname->setInstrumentNameType(type);
        iname->setInstrumentNameRole(role);
        ctx.mutDom().addElement(iname);
    }

    iname->setAlign(Align(iname->align().horizontal, AlignV::BASELINE));
    iname->setXmlText(name);

    iname->mutldata()->setColumn(0); // Reset here, will be computed later
    TLayout::layoutInstrumentName(iname, iname->mutldata());

    return iname;
}

String SystemHeaderLayout::formattedInstrumentName(System* system, Part* part, const Fraction& tick)
{
    bool longNames = system->ldata()->useLongNames();
    Instrument* instr = part->instrument(tick);
    const InstrumentLabel& label = instr->instrumentLabel();

    if (muse::contains(system->ldata()->partsWithGroupName(), part)) {
        if (label.useCustomIndividualName()) {
            return longNames ? label.customNameLongIndividual() : label.customNameShortIndividual();
        }

        int number = part->number(tick);
        return number > 0 ? String::number(number) : String();
    }

    if (label.useCustomName()) {
        return longNames ? label.customNameLong() : label.customNameShort();
    }

    const MStyle& style = system->style();

    String instrName = longNames ? instr->longName() : instr->shortName();

    bool showNumber = longNames ? label.showNumberLong() : label.showNumberShort();
    String number = instr->number() > 0 && showNumber ? String::number(instr->number()) : String();

    bool showTranspo = longNames ? label.showTranspositionLong() : label.showTranspositionShort();
    showTranspo &= style.styleB(longNames ? Sid::instrumentNamesShowTranspositionLong : Sid::instrumentNamesShowTranspositionShort);
    String transposition = showTranspo ? instr->transposition() : String();

    if (transposition.empty()) {
        String result = instrName;
        if (!number.empty()) {
            result += u" " + number;
        }
        return result;
    }

    InstrumentNamesFormat nameFormat
        = style.styleV(longNames ? Sid::instrumentNamesFormatLong : Sid::instrumentNamesFormatShort).value<InstrumentNamesFormat>();

    //: For instrument transposition, e.g. Horn in F
    String in = TranslatableString("notation", "in").translated();

    switch (nameFormat) {
    case InstrumentNamesFormat::NAME_IN_TRANSP_NUM:
        return instrName + u" " + in + u" " + transposition + (number.empty() ? String() : u" " + number);
    case InstrumentNamesFormat::NAME_NUM_IN_TRANSP:
        return instrName + (number.empty() ? String() : u" " + number) + u" " + in + u" " + transposition;
    case InstrumentNamesFormat::TRANSP_NAME_NUM:
        return transposition + u" " + instrName + (number.empty() ? String() : u" " + number);
    default:
    {
        String result = style.styleSt(longNames ? Sid::instrumentNamesCustomFormatLong : Sid::instrumentNamesCustomFormatShort);
        return resolveTokens(result, instrName, transposition, number);
    }
    }
}

String SystemHeaderLayout::formattedGroupName(System* system, Part* part, const Fraction& tick)
{
    const MStyle& style = system->style();

    bool longNames = system->ldata()->useLongNames();
    Instrument* instr = part->instrument(tick);
    const InstrumentLabel& label = instr->instrumentLabel();

    if (label.useCustomGroupName()) {
        return longNames ? label.customNameLongGroup() : label.customNameShortGroup();
    }

    String instrName = longNames ? instr->longName() : instr->shortName();

    bool showTranspo = longNames ? label.showTranspositionLong() : label.showTranspositionShort();
    showTranspo &= style.styleB(longNames ? Sid::instrumentNamesShowTranspositionLong : Sid::instrumentNamesShowTranspositionShort);

    String transposition = showTranspo ? instr->transposition() : String();

    if (transposition.empty()) {
        return instrName;
    }

    InstrumentNamesFormat nameFormat
        = style.styleV(longNames ? Sid::instrumentNamesFormatLong : Sid::instrumentNamesFormatShort).value<InstrumentNamesFormat>();

    //: For instrument transposition, e.g. Horn in F
    String in = TranslatableString("notation", "in").translated();

    switch (nameFormat) {
    case InstrumentNamesFormat::NAME_IN_TRANSP_NUM:
        return instrName + u" " + in + u" " + transposition;
    case InstrumentNamesFormat::NAME_NUM_IN_TRANSP:
        return instrName + u" " + in + u" " + transposition;
    case InstrumentNamesFormat::TRANSP_NAME_NUM:
        return transposition + u" " + instrName;
    default:
    {
        String result = style.styleSt(longNames ? Sid::instrumentNamesCustomFormatLong : Sid::instrumentNamesCustomFormatShort);
        return resolveTokens(result, instrName, transposition, /*number*/ String());
    }
    }
}

String& SystemHeaderLayout::resolveTokens(String& str, const String& name, const String& transposition, const String& number)
{
    static const String NAME = u"$name";
    static const String TRANSP = u"$transposition";
    static const String NUMBER = u"$number";

    str.replace(NAME, name);
    str.replace(TRANSP, transposition);

    if (!number.empty()) {
        str.replace(NUMBER, number);
    } else if (str.contains(u" $number")) {
        str.remove(u" $number");
    } else if (str.contains(u"$number ")) {
        str.remove(u"$number ");
    } else if (str.contains(u"$number")) {
        str.remove(u"$number");
    }

    return str;
}

bool SystemHeaderLayout::showNames(LayoutContext& ctx)
{
    if (!ctx.conf().isShowInstrumentNames()) {
        return false;
    }

    if (ctx.conf().styleB(Sid::hideInstrumentNameIfOneInstrument) && ctx.dom().visiblePartCount() <= 1) {
        return false;
    }

    if (ctx.state().firstSystem()
        && ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE) {
        return false;
    }

    if (!ctx.state().firstSystem()
        && ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE) {
        return false;
    }

    return true;
}

void SystemHeaderLayout::setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick)
{
    if (system->vbox()) {
        return;
    }

    System::LayoutData* ldata = system->mutldata();
    ldata->setUseLongNames(longName);

    InstrumentNameType type = longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT;

    if (!showNames(ctx)) {
        for (staff_idx_t idx = 0; idx < system->staves().size(); ++idx) {
            updateName(system, idx, ctx, String(), type, InstrumentNameRole::STAFF);
            updateName(system, idx, ctx, String(), type, InstrumentNameRole::PART);
            updateName(system, idx, ctx, String(), type, InstrumentNameRole::GROUP);
        }
        return;
    }

    updateGroupNames(system, ctx, tick);

    for (size_t staffIdx = 0; staffIdx < system->staves().size(); /*empty*/) {
        Part* part = ctx.dom().staff(staffIdx)->part();
        size_t partNstaves = part->nstaves();
        size_t visibleStavesCount = part->visibleStavesCount();

        for (size_t idxInPart = 0; idxInPart < partNstaves; ++idxInPart) {
            staff_idx_t globalIdx = staffIdx + idxInPart;

            String instrumentName;
            if (idxInPart == 0 && part->show() && visibleStavesCount > 0) {
                instrumentName = formattedInstrumentName(system, part, tick);
            }
            updateName(system, globalIdx, ctx, instrumentName, type, InstrumentNameRole::PART);

            const Staff* staff = ctx.dom().staff(globalIdx);
            String staffName;
            if (staff->show()) {
                staffName = longName ? staff->individualStaffNameLong(tick) : staff->individualStaffNameShort(tick);
            }
            updateName(system, globalIdx, ctx, staffName, type, InstrumentNameRole::STAFF);
        }

        staffIdx += partNstaves;
    }
}

void SystemHeaderLayout::updateGroupNames(System* system, LayoutContext& ctx, const Fraction& tick)
{
    const MStyle& style = ctx.conf().style();
    InstrumentNameType type = system->ldata()->useLongNames() ? InstrumentNameType::LONG : InstrumentNameType::SHORT;

    System::LayoutData* ldata = system->mutldata();
    ldata->clearPartsWithGroupNames();

    auto useGroupNames = [&](String instrumentGroup) {
        if (instrumentGroup == "woodwinds" || instrumentGroup == "brass") {
            return style.styleB(Sid::windsNameByGroup);
        }
        if (instrumentGroup == "vocals") {
            return style.styleB(Sid::vocalsNameByGroup);
        }
        if (instrumentGroup == "strings") {
            return style.styleB(Sid::stringsNameByGroup);
        }
        return style.styleB(Sid::othersNameByGroup);
    };

    for (staff_idx_t startOfGroup = 0; startOfGroup < system->staves().size();) {
        Part* curPart = ctx.dom().staff(startOfGroup)->part();
        const Instrument* curInstrument = curPart->instrument(tick);
        const InstrumentLabel& curLabel = curInstrument->instrumentLabel();
        if (!curInstrument->instrumentLabel().allowGroupName()) {
            ++startOfGroup;
            continue;
        }

        std::vector<Part*> partsInThisGroup;

        staff_idx_t endOfGroup = startOfGroup;
        while (endOfGroup < system->staves().size()) {
            Part* nextPart = ctx.dom().staff(endOfGroup)->part();
            Instrument* nextInstrument = nextPart->instrument(tick);
            InstrumentLabel& nextLabel = nextInstrument->instrumentLabel();
            if (nextPart != curPart && (nextInstrument->id() != curInstrument->id() || !nextLabel.allowGroupName())) {
                break;
            }
            if (nextLabel.useCustomGroupName() && (nextLabel.customNameLongGroup() != curLabel.customNameLongGroup()
                                                   || nextLabel.customNameShortGroup() != curLabel.customNameShortGroup())) {
                break;
            }

            if (type == InstrumentNameType::LONG && system->visibleStavesOfPart(nextPart).size() == 0) {
                // NOTE: We can only do this for long-name systems because they don't need to have
                // the left barline aligned with the other systems across the page.
                endOfGroup += nextPart->nstaves();
                continue;
            }

            if (nextPart->show() && nextPart->visibleStavesCount() > 0) {
                partsInThisGroup.push_back(nextPart);
                nextLabel.setUseCustomGroupName(curLabel.useCustomGroupName());
                nextLabel.setCustomNameLongGroup(curLabel.customNameLongGroup());
                nextLabel.setCustomNameShortGroup(curLabel.customNameShortGroup());
            }

            endOfGroup += nextPart->nstaves();
        }

        if (partsInThisGroup.size() > 1 && useGroupNames(curInstrument->group())) {
            String name = formattedGroupName(system, curPart, tick);
            InstrumentName* groupName = updateName(system, startOfGroup, ctx, name, type, InstrumentNameRole::GROUP);

            if (groupName) {
                groupName->mutldata()->setEndIdxOfGroup(endOfGroup);

                for (Part* p : partsInThisGroup) {
                    ldata->addPartWithGroupNames(p, groupName);
                }
            }

            for (staff_idx_t idx = startOfGroup + 1; idx < endOfGroup; ++idx) {
                updateName(system, idx, ctx, String(), type, InstrumentNameRole::GROUP);
            }
        } else {
            for (staff_idx_t idx = startOfGroup; idx < endOfGroup; ++idx) {
                updateName(system, idx, ctx, String(), type, InstrumentNameRole::GROUP);
            }
        }

        startOfGroup = endOfGroup;
    }
}
