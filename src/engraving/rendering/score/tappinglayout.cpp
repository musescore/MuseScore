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

#include "slurtielayout.h"
#include "tappinglayout.h"
#include "tlayout.h"

#include "dom/chord.h"
#include "dom/slur.h"
#include "dom/staff.h"
#include "dom/system.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void TappingLayout::layoutTapping(Tapping* item, Tapping::LayoutData* ldata, LayoutContext& ctx)
{
    IF_ASSERT_FAILED(item->hand() != TappingHand::INVALID) {
        return;
    }

    const MStyle& style = item->style();
    bool tabStaff = item->staffType()->isTabStaff();

    if (item->hand() == TappingHand::LEFT) {
        layoutLeftHandTapping(item, ldata, style, tabStaff, ctx);
    } else {
        layoutRightHandTapping(item, ldata, style, tabStaff, ctx);
    }

    if (ldata->symId != SymId::noSym) {
        ldata->setShape(Shape(item->symBbox(ldata->symId), item));
    } else if (TappingText* text = item->text()) {
        RectF textBbox = text->ldata()->bbox();
        text->setPos(PointF(-textBbox.x(), 0.0));
        ldata->setShape(Shape(textBbox.translated(text->pos()), item));
    }

    if (tabStaff) {
        // Slightly increase vertical padding on TAB staves
        item->setMinDistance(item->staff()->lineDistance(item->tick()) * style.styleS(Sid::articulationMinDistance));
    }
}

void TappingLayout::layoutLeftHandTapping(Tapping* item, Tapping::LayoutData* ldata, const MStyle& style, bool tabStaff, LayoutContext& ctx)
{
    updateHalfSlurs(item, style, tabStaff, ctx);

    LHTappingSymbol lhSym = tabStaff ? style.styleV(Sid::lhTappingSymbolTab).value<LHTappingSymbol>()
                            : style.styleV(Sid::lhTappingSymbolNormalStave).value<LHTappingSymbol>();

    bool dontShowSymbols = item->lhShowItems() == LHTappingShowItems::HALF_SLUR;

    if (lhSym != LHTappingSymbol::DOT || dontShowSymbols) {
        ldata->symId = SymId::noSym;
    }
    if (lhSym != LHTappingSymbol::CIRCLED_T || dontShowSymbols) {
        delete item->text();
        item->setText(nullptr);
    }

    if (dontShowSymbols) {
        return;
    }

    if (lhSym == LHTappingSymbol::DOT) {
        ldata->symId = SymId::windClosedHole;
        ldata->setMag(0.5);
    } else if (lhSym == LHTappingSymbol::CIRCLED_T) {
        ldata->setMag(1.0);
        TappingText* text = item->text();
        if (!text) {
            text = new TappingText(item);
        }
        item->setText(text);
        text->setParent(item);
        text->setTrack(item->track());
        text->setXmlText("T");
        text->setBorderType(BorderType::CIRCLE);
        text->setAlign(Align(AlignH::LEFT, item->up() ? AlignV::BASELINE : AlignV::TOP));
        TLayout::layoutBaseTextBase(text, ctx);
        // Move the circle up very slightly to look better centered on the T
        text->mutldata()->borderRect.translate(0.0, -0.02 * text->height());
    }
}

void TappingLayout::updateHalfSlurs(Tapping* item, const MStyle& style, bool tabStaff, LayoutContext& ctx)
{
    Chord* chord = item->parent() && item->parent()->isChord() ? toChord(item->parent()) : nullptr;
    if (!chord) {
        return;
    }

    bool showHalfSlurAbove = item->lhShowItems() != LHTappingShowItems::SYMBOL;
    if (showHalfSlurAbove) {
        TappingHalfSlur* halfSlurAbove = item->halfSlurAbove();
        if (!halfSlurAbove) {
            halfSlurAbove = new TappingHalfSlur(item);
            item->setHalfSlurAbove(halfSlurAbove);
            halfSlurAbove->setSelected(item->selected());
        }

        halfSlurAbove->setParent(item);
        halfSlurAbove->setIsHalfSlurAbove(true);
        halfSlurAbove->setTrack(item->track());
        halfSlurAbove->setTick(item->tick());
        halfSlurAbove->setTicks(Fraction(0, 1));
        halfSlurAbove->setStartElement(chord);
        halfSlurAbove->setEndElement(chord);
        if (tabStaff) {
            halfSlurAbove->setSlurDirection(DirectionV::UP);
        }

        layoutHalfSlur(item, halfSlurAbove, ctx);
    } else if (item->halfSlurAbove()) {
        delete item->halfSlurAbove();
        item->setHalfSlurAbove(nullptr);
    }

    bool showHalfSlurBelow = tabStaff && showHalfSlurAbove && style.styleB(Sid::lhTappingSlurTopAndBottomNoteOnTab)
                             && toChord(item->parent())->notes().size() > 1;
    if (showHalfSlurBelow) {
        TappingHalfSlur* halfSlurBelow = item->halfSlurBelow();
        if (!halfSlurBelow) {
            halfSlurBelow = new TappingHalfSlur(item);
            item->setHalfSlurBelow(halfSlurBelow);
        }

        halfSlurBelow->setParent(item);
        halfSlurBelow->setIsHalfSlurAbove(false);
        halfSlurBelow->setTrack(item->track());
        halfSlurBelow->setTick(item->tick());
        halfSlurBelow->setTicks(Fraction(0, 1));
        halfSlurBelow->setStartElement(chord);
        halfSlurBelow->setEndElement(chord);
        halfSlurBelow->setSlurDirection(DirectionV::DOWN);

        layoutHalfSlur(item, halfSlurBelow, ctx);
    } else if (item->halfSlurBelow()) {
        delete item->halfSlurBelow();
        item->setHalfSlurBelow(nullptr);
    }
}

void TappingLayout::layoutHalfSlur(Tapping* item, TappingHalfSlur* slur, LayoutContext& ctx)
{
    System* system = toSystem(item->findAncestor(ElementType::SYSTEM));
    IF_ASSERT_FAILED(system) {
        return;
    }

    Skyline& skyline = system->staff(item->staffIdx())->skyline();
    TappingHalfSlurSegment* slurSeg = toTappingHalfSlurSegment(SlurTieLayout::layoutSystem(slur, system, ctx));

    Shape segShape = slurSeg->shape();

    RectF mask(segShape.bbox());
    mask.setWidth(0.5 * mask.width());
    const double maskPad = 0.25 * item->spatium();
    mask.adjust(-maskPad, -maskPad, 0.0, maskPad);
    slurSeg->mutldata()->setMask(mask);

    Shape maskedShape;
    double roundingMargin = 0.05 * item->spatium(); // otherwise rounding errors may affect the threshold
    for (ShapeElement& el : segShape.elements()) {
        if (el.left() > mask.right() - roundingMargin) {
            maskedShape.add(el);
        }
    }
    slurSeg->mutldata()->setShape(maskedShape);

    skyline.add(maskedShape.translated(slurSeg->pos()));
}

void TappingLayout::layoutRightHandTapping(Tapping* item, Tapping::LayoutData* ldata, const MStyle& style, bool tabStaff,
                                           LayoutContext& ctx)
{
    RHTappingSymbol rhSym = tabStaff ? style.styleV(Sid::rhTappingSymbolTab).value<RHTappingSymbol>()
                            : style.styleV(Sid::rhTappingSymbolNormalStave).value<RHTappingSymbol>();

    if (rhSym != RHTappingSymbol::PLUS) {
        ldata->symId = SymId::noSym;
    }
    if (rhSym != RHTappingSymbol::T) {
        delete item->text();
        item->setText(nullptr);
    }

    if (rhSym == RHTappingSymbol::PLUS) {
        ldata->symId = SymId::pluckedLeftHandPizzicato;
    } else if (rhSym == RHTappingSymbol::T) {
        TappingText* text = item->text();
        if (!text) {
            text = new TappingText(item);
        }
        item->setText(text);
        text->setParent(item);
        text->setTrack(item->track());
        text->setXmlText("T");
        text->setAlign(Align(AlignH::LEFT, item->up() ? AlignV::BASELINE : AlignV::TOP));
        TLayout::layoutBaseTextBase(text, ctx);
    }
}
