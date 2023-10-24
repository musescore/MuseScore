/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "arpeggiolayout.h"

#include "dom/arpeggio.h"
#include "dom/chord.h"
#include "dom/segment.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

//   layoutArpeggio2
//    called after layout of page

void ArpeggioLayout::layoutArpeggio2(Arpeggio* item, LayoutContext& ctx)
{
    if (!item) {
        return;
    }

    TLayout::layoutArpeggio(item, item->mutldata(), ctx.conf(), true);
}

void ArpeggioLayout::layoutOnEditDrag(Arpeggio* item, LayoutContext& ctx)
{
    TLayout::layoutArpeggio(item, item->mutldata(), ctx.conf());
}

void ArpeggioLayout::layoutOnEdit(Arpeggio* item, LayoutContext& ctx)
{
    Arpeggio::LayoutData* ldata = item->mutldata();
    TLayout::layoutArpeggio(item, ldata, ctx.conf(), true);

    ldata->setPosX(-(ldata->bbox().width() + item->spatium() * .5));

    Fraction tick = item->tick();

    ctx.setLayout(tick, tick, item->staffIdx(), item->staffIdx() + item->span(), item);
}

//
// INSET:
// Arpeggios have inset white space. For instance, the bracket
// "[" shape has whitespace inside of the "C". Symbols like
// accidentals can fit inside this whitespace. These inset
// functions are used to get the size of the inner dimensions
// for this area on all arpeggios.
//

//---------------------------------------------------------
//   insetTop
//---------------------------------------------------------

double ArpeggioLayout::insetTop(Arpeggio* item, Chord* c)
{
    double top = c->upNote()->y() - c->upNote()->height() / 2;

    // use wiggle width, not height, since it's rotated 90 degrees
    if (item->arpeggioType() == ArpeggioType::UP) {
        top += item->symBbox(SymId::wiggleArpeggiatoUpArrow).width();
    } else if (item->arpeggioType() == ArpeggioType::UP_STRAIGHT) {
        top += item->symBbox(SymId::arrowheadBlackUp).width();
    }

    return top;
}

//---------------------------------------------------------
//   insetBottom
//---------------------------------------------------------

double ArpeggioLayout::insetBottom(Arpeggio* item, Chord* c)
{
    double bottom = c->downNote()->y() + c->downNote()->height() / 2;

    // use wiggle width, not height, since it's rotated 90 degrees
    if (item->arpeggioType() == ArpeggioType::DOWN) {
        bottom -= item->symBbox(SymId::wiggleArpeggiatoUpArrow).width();
    } else if (item->arpeggioType() == ArpeggioType::DOWN_STRAIGHT) {
        bottom -= item->symBbox(SymId::arrowheadBlackDown).width();
    }

    return bottom;
}

//---------------------------------------------------------
//   insetWidth
//---------------------------------------------------------

double ArpeggioLayout::insetWidth(Arpeggio* item)
{
    switch (item->arpeggioType()) {
    case ArpeggioType::NORMAL:
    {
        return 0.0;
    }

    case ArpeggioType::UP:
    case ArpeggioType::DOWN:
    {
        // use wiggle height, not width, since it's rotated 90 degrees
        return (item->width() - item->symBbox(SymId::wiggleArpeggiatoUp).height()) / 2;
    }

    case ArpeggioType::UP_STRAIGHT:
    case ArpeggioType::DOWN_STRAIGHT:
    {
        return (item->width() - item->style().styleMM(Sid::ArpeggioLineWidth)) / 2;
    }

    case ArpeggioType::BRACKET:
    {
        return item->width() - item->style().styleMM(Sid::ArpeggioLineWidth) / 2;
    }
    }
    return 0.0;
}

//---------------------------------------------------------
//   insetDistance
//---------------------------------------------------------

double ArpeggioLayout::insetDistance(Arpeggio* item, LayoutContext& ctx, std::vector<Accidental*>& accidentals, double mag_,
                                     Chord* chord)
{
    if (accidentals.size() == 0) {
        return 0.0;
    }
    if (!chord) {
        return 0.0;
    }
    if (!item) {
        return 0.0;
    }

    // Only be concerned about the top/bottom of an arpeggio line if this is the start/end chord
    bool arpStartTrack = chord->track() == item->track();
    bool arpEndTrack = chord->track() == item->endTrack();

    double arpeggioTop = arpStartTrack ? insetTop(item, chord) * mag_ : -10000;
    double arpeggioBottom = arpEndTrack ? insetBottom(item, chord) * mag_ : 10000;

    ArpeggioType type = item->arpeggioType();
    bool hasTopArrow = type == ArpeggioType::UP
                       || type == ArpeggioType::UP_STRAIGHT
                       || type == ArpeggioType::BRACKET;
    bool hasBottomArrow = type == ArpeggioType::DOWN
                          || type == ArpeggioType::DOWN_STRAIGHT
                          || type == ArpeggioType::BRACKET;

    Accidental* furthestAccidental = nullptr;
    for (auto accidental : accidentals) {
        if (furthestAccidental) {
            bool currentIsFurtherX = accidental->x() < furthestAccidental->x();
            bool currentIsSameX = accidental->x() == furthestAccidental->x();
            auto accidentalBbox = item->symBbox(accidental->symId());
            double currentTop = accidental->note()->pos().y() + accidentalBbox.top() * mag_;
            double currentBottom = accidental->note()->pos().y() + accidentalBbox.bottom() * mag_;
            bool collidesWithTop = currentTop <= arpeggioTop && hasTopArrow;
            bool collidesWithBottom = currentBottom >= arpeggioBottom && hasBottomArrow;

            if (currentIsFurtherX || (currentIsSameX && (collidesWithTop || collidesWithBottom))) {
                furthestAccidental = accidental;
            }
        } else {
            furthestAccidental = accidental;
        }
    }

    IF_ASSERT_FAILED(furthestAccidental) {
        return 0.0;
    }

    // this cutout means the vertical lines for a ♯, ♭, and ♮ are in the same position
    // if an accidental does not have a cutout (e.g., ♭), this value is 0
    double accidentalCutOutX = item->symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutNW).x() * mag_;
    double accidentalCutOutYTop = item->symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutNW).y() * mag_;
    double accidentalCutOutYBottom = item->symSmuflAnchor(furthestAccidental->symId(), SmuflAnchorId::cutOutSW).y() * mag_;

    double maximumInset = (ctx.conf().styleMM(Sid::ArpeggioAccidentalDistance)
                           - ctx.conf().styleMM(Sid::ArpeggioAccidentalDistanceMin)) * mag_;

    if (accidentalCutOutX > maximumInset) {
        accidentalCutOutX = maximumInset;
    }

    RectF bbox = item->symBbox(furthestAccidental->symId());
    double center = furthestAccidental->note()->pos().y() * mag_;
    double top = center + bbox.top() * mag_;
    double bottom = center + bbox.bottom() * mag_;
    bool collidesWithTop = hasTopArrow && top <= arpeggioTop;
    bool collidesWithBottom = hasBottomArrow && bottom >= arpeggioBottom;
    bool cutoutCollidesWithTop = collidesWithTop && top - accidentalCutOutYTop >= arpeggioTop;
    bool cutoutCollidesWithBottom = collidesWithBottom && bottom - accidentalCutOutYBottom <= arpeggioBottom;

    if (collidesWithTop || collidesWithBottom) {
        // optical adjustment for one edge case
        if (accidentalCutOutX == 0.0 || cutoutCollidesWithTop || cutoutCollidesWithBottom) {
            return accidentalCutOutX + maximumInset;
        }
        return accidentalCutOutX;
    }

    return insetWidth(item) + accidentalCutOutX;
}
