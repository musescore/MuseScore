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
                    bracketWidth[i] = std::max(bracketWidth[i], b->ldata()->bracketWidth());
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
    SystemLayout::layoutBracketsVertical(system, ctx);

    system->setBracketsXPosition(measure->x());

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
            if (bi->bracketType() == BracketType::NO_BRACKET || !bi->visible()) {
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

bool SystemHeaderLayout::stackVertically(InstrumentName* n)
{
    const MStyle& style = n->style();
    bool longName = n->instrumentNameType() == InstrumentNameType::LONG;
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

void SystemHeaderLayout::computeInstrumentNamesWidth(System* system, LayoutContext& ctx)
{
    System::LayoutData* ldata = system->mutldata();
    ldata->setStaffNamesWidth(0.0);
    ldata->setInstrumentNamesWidth(0.0);
    ldata->setTotalNamesWidth(0.0);

    std::set<Part*> partsWithIndividualStaffNames;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (!ctx.dom().staff(staffIdx)->show()) {
            // We know that the staff is hidden in the entire score so safe to skip
            continue;
        }

        const SysStaff* staff = system->staff(staffIdx);
        if (ldata->useLongNames() && !staff->show()) {
            // NOTE: We can only do this for long-name systems because they don't need to have
            // the left barline aligned with the other systems across the page.
            continue;
        }

        if (InstrumentName* name = staff->individualStaffName) {
            TLayout::layoutInstrumentName(name, name->mutldata());
            ldata->setStaffNamesWidth(std::max(ldata->staffNamesWidth(), name->width()));
            ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), name->width()));
            partsWithIndividualStaffNames.insert(ctx.dom().staff(staffIdx)->part());
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        Part* part = ctx.dom().staff(staffIdx)->part();
        if (!part->show() || part->visibleStavesCount() == 0) {
            // We know that the part is hidden in the entire score so safe to skip
            continue;
        }

        const SysStaff* staff = system->staff(staffIdx);

        InstrumentName* name = staff->instrumentName;
        if (!name) {
            continue;
        }

        if (ldata->useLongNames() && name->effectiveStaffIdx() == muse::nidx) {
            continue;
        }

        TLayout::layoutInstrumentName(name, name->mutldata());
        ldata->setInstrumentNamesWidth(std::max(ldata->instrumentNamesWidth(), name->width()));
        ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), name->width()));

        if (stackVertically(name)) {
            continue;
        }

        size_t visibleStaveCount = system->visibleStavesOfPart(part).size();
        if (ldata->useLongNames() && visibleStaveCount % 2 == 0) {
            continue;
        }

        if (muse::contains(partsWithIndividualStaffNames, part)) {
            staff_idx_t startStaff = system->firstSysStaffOfPart(part);
            staff_idx_t endStaff = startStaff + part->nstaves();
            for (staff_idx_t idx = startStaff; idx < endStaff; ++idx) {
                if (InstrumentName* staffName = system->staff(idx)->individualStaffName) {
                    double sumWidth = 0.0;
                    AlignH staffNameAlign = staffName->position();
                    if (staffNameAlign == AlignH::LEFT || staffNameAlign == AlignH::JUSTIFY) {
                        sumWidth = ldata->staffNamesWidth() + name->width() + ldata->instrumentNameOffset();
                    } else if (staffNameAlign == AlignH::HCENTER) {
                        sumWidth = staffName->width() + name->width() + ldata->instrumentNameOffset();
                        double move = 0.5 * (ldata->staffNamesWidth() - staffName->width());
                        sumWidth += move;
                    } else {
                        sumWidth = staffName->width() + name->width() + ldata->instrumentNameOffset();
                    }
                    ldata->setTotalNamesWidth(std::max(ldata->totalNamesWidth(), sumWidth));
                }
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
        } else {
            if (visibleStavesCount % 2) {
                SysStaff* midStaff = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
                y1 = midStaff->bbox().top();
                y2 = midStaff->bbox().bottom();
                if (InstrumentName* staffName = midStaff->individualStaffName) {
                    if (stackVertically(t)) {
                        double lineSpacing = t->fontMetrics().lineSpacing();
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
                } else {
                    t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
                }
            } else {
                SysStaff* staffAboveMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2 - 1]);
                SysStaff* staffBelowMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
                y1 = staffAboveMid->bbox().top();
                y2 = staffBelowMid->bbox().bottom();
                t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            }
        }

        staffIdx += nstaves;
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
    double staffNamesWidth = ldata->staffNamesWidth();

    for (const SysStaff* s : system->staves()) {
        if (InstrumentName* t = s->individualStaffName) {
            bool longName = t->instrumentNameType() == InstrumentNameType::LONG;
            InstrumentNamesAlign align
                = t->style().styleV(longName ? Sid::instrumentNamesAlignLong : Sid::instrumentNamesAlignShort).value<InstrumentNamesAlign>();
            const RectF& bbox = t->ldata()->bbox();
            if (align == InstrumentNamesAlign::CENTER_CENTER) {
                t->mutldata()->setPosX(totalNamesWidth * .5 - (bbox.right() + bbox.left()) * .5);
            } else {
                switch (t->position()) {
                case AlignH::JUSTIFY:
                case AlignH::LEFT:
                    t->mutldata()->setPosX(totalNamesWidth - staffNamesWidth - bbox.left());
                    break;
                case AlignH::HCENTER:
                    t->mutldata()->setPosX(totalNamesWidth - 0.5 * staffNamesWidth - 0.5 * (bbox.right() + bbox.left()));
                    break;
                case AlignH::RIGHT:
                    t->mutldata()->setPosX(totalNamesWidth - bbox.right());
                }
            }
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* s = system->staff(staffIdx);
        const Part* p = system->score()->staff(staffIdx)->part();
        if (InstrumentName* t = s->instrumentName; t && t->effectiveStaffIdx() != muse::nidx) {
            const RectF& bbox = t->ldata()->bbox();
            bool longName = t->instrumentNameType() == InstrumentNameType::LONG;
            InstrumentNamesAlign align
                = t->style().styleV(longName ? Sid::instrumentNamesAlignLong : Sid::instrumentNamesAlignShort).value<InstrumentNamesAlign>();

            if (align == InstrumentNamesAlign::LEFT_RIGHT) {
                t->mutldata()->setPosX(0 - bbox.left());
            } else if (align == InstrumentNamesAlign::CENTER_CENTER) {
                t->mutldata()->setPosX(0.5 * totalNamesWidth - 0.5 * (bbox.right() + bbox.left()));
            } else if (align == InstrumentNamesAlign::CENTER_RIGHT) {
                t->mutldata()->moveX(0.5 * ldata->instrumentNamesWidth() - 0.5 * (bbox.right() + bbox.left()));
            } else {
                std::vector<staff_idx_t> visibleStavesForPart = system->visibleStavesOfPart(p);
                size_t visibleStaveCount = visibleStavesForPart.size();
                if (visibleStaveCount % 2 && !stackVertically(t)) {
                    staff_idx_t centerStaff = visibleStavesForPart[visibleStaveCount / 2];
                    if (InstrumentName* staffName = system->staff(centerStaff)->individualStaffName) {
                        t->mutldata()->setPosX(
                            staffName->x() + staffName->ldata()->bbox().left() - bbox.right() - ldata->instrumentNameOffset());
                    } else {
                        t->mutldata()->setPosX(totalNamesWidth - bbox.right());
                    }
                } else {
                    t->mutldata()->setPosX(totalNamesWidth - bbox.right());
                }
            }
        }
    }
}

void SystemHeaderLayout::updateName(System* system, staff_idx_t staffIdx, LayoutContext& ctx, const String& name,
                                    InstrumentNameType type, InstrumentNameRole role)
{
    SysStaff* sysStaff = system->staff(staffIdx);
    InstrumentName* iname = role == InstrumentNameRole::PART ? sysStaff->instrumentName : sysStaff->individualStaffName;
    if (name.empty()) {
        if (iname) {
            ctx.mutDom().removeElement(iname);
        }
        return;
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
}

void SystemHeaderLayout::setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick)
{
    if (system->vbox()) {
        return;
    }

    system->mutldata()->setUseLongNames(longName);

    if (!ctx.conf().isShowInstrumentNames()
        || (ctx.conf().styleB(Sid::hideInstrumentNameIfOneInstrument) && ctx.dom().visiblePartCount() <= 1)
        || (ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE)
        || (!ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>()
            == InstrumentLabelVisibility::HIDE)) {
        for (SysStaff* staff : system->staves()) {
            if (InstrumentName* iName = staff->instrumentName) {
                ctx.mutDom().removeElement(iName);
            }
            if (InstrumentName* sName = staff->individualStaffName) {
                ctx.mutDom().removeElement(sName);
            }
        }
        return;
    }

    InstrumentNameType type = longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT;

    for (size_t staffIdx = 0; staffIdx < system->staves().size(); /*empty*/) {
        Part* part = ctx.dom().staff(staffIdx)->part();

        if (!part->show() || part->visibleStavesCount() == 0) {
            for (size_t i = 0; i < part->nstaves(); ++i) {
                SysStaff* sysStaff = system->staff(staffIdx + i);
                if (InstrumentName* iName = sysStaff->instrumentName) {
                    ctx.mutDom().removeElement(iName);
                }
                if (InstrumentName* sName = sysStaff->individualStaffName) {
                    ctx.mutDom().removeElement(sName);
                }
            }
            staffIdx += part->nstaves();
            continue;
        }

        for (size_t i = 0; i < part->nstaves(); ++i) {
            staff_idx_t idx = staffIdx + i;
            SysStaff* sysStaff = system->staff(idx);
            if (i == 0) {
                const String& instrName = longName ? part->longName(tick) : part->shortName(tick);
                updateName(system, idx, ctx, instrName, type, InstrumentNameRole::PART);
            } else {
                if (sysStaff->instrumentName) {
                    ctx.mutDom().removeElement(sysStaff->instrumentName);
                }
            }
            const Staff* staff = ctx.dom().staff(idx);
            if (staff->show()) {
                const String& staffName = longName ? staff->individualStaffNameLong(tick) : staff->individualStaffNameShort(tick);
                updateName(system, idx, ctx, staffName, type, InstrumentNameRole::STAFF);
            }
        }

        staffIdx += part->nstaves();
    }
}
