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
#include "dom/ledgerline.h"
#include "dom/part.h"
#include "dom/score.h"
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

//
// INSET:
// Arpeggios have inset white space. For instance, the bracket
// "[" shape has whitespace inside of the "C". Symbols like
// accidentals can fit inside this whitespace. These inset
// functions are used to get the size of the inner dimensions
// for this area on all arpeggios.
//

double ArpeggioLayout::insetTop(const Arpeggio* item, const Chord* c)
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

double ArpeggioLayout::insetBottom(const Arpeggio* item, const Chord* c)
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

double ArpeggioLayout::insetWidth(const Arpeggio* item)
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

void ArpeggioLayout::clearAccidentals(Arpeggio* item, LayoutContext& ctx)
{
    //   This function finds collisions with accidentals outside of an arpeggio's span
    //   Assumptions are made to deal with cross staff height, as we don't know this yet.
    //   Once a chord is found, its shape is stripped down to only accidentals which collide
    //   with the arpeggio.  The arpeggio's offset is calculated from the remaining shape

    if (!item || !item->chord()) {
        return;
    }

    const Segment* seg = item->chord()->segment();
    const EngravingItem* endEl = seg->elementAt(item->endTrack());
    const Chord* endChord = endEl && endEl->isChord() ? toChord(endEl) : item->chord();

    const Part* part = item->part();
    const track_idx_t partStartTrack = part->startTrack();
    const track_idx_t partEndTrack = part->endTrack();
    const PaddingTable& paddingTable = item->score()->paddingTable();

    double arpeggioAccidentalDistance = paddingTable.at(ElementType::ARPEGGIO).at(ElementType::ACCIDENTAL) * item->mag();
    double arpeggioLedgerDistance = paddingTable.at(ElementType::ARPEGGIO).at(ElementType::LEDGER_LINE) * item->mag();

    Shape arpShape = item->shape().translate(item->pagePos());
    double arpX = arpShape.right();
    double largestOverlap = 0.0;

    for (track_idx_t track = partStartTrack; track < partEndTrack; ++track) {
        EngravingItem* e = seg->element(track);
        if (!e || !e->isChord()) {
            continue;
        }

        Chord* chord = toChord(e);
        // Only check chords the arpeggio doesn't span on staves the arpeggios span
        // We already calculated the gap between chords it does span in ChordLayout::layoutPitched
        bool aboveStart
            = std::make_pair(chord->vStaffIdx(), chord->downLine()) < std::make_pair(item->vStaffIdx(), item->chord()->upLine());
        bool belowEnd = std::make_pair(chord->vStaffIdx(), chord->upLine()) > std::make_pair(endChord->vStaffIdx(), endChord->downLine());

        if ((chord->spanArpeggio() == item && !(aboveStart || belowEnd)) || chord->vStaffIdx() < item->vStaffIdx()
            || chord->vStaffIdx() > endChord->vStaffIdx()) {
            continue;
        }

        // We don't know the distance between staves, so our arpeggio's height is currently unknown if it spans staves
        // To make sure we spot collisions with non-arpeggio accidentals, extend arpeggio upwards/downwards a large amount
        // depending on if they are the end/start of the arpeggio respectively.
        // This is to find collisions between chords outside the arpeggios track span, but within its "visual span"
        Shape curArpShape = arpShape;
        staff_idx_t staffIdx = chord->vStaffIdx();
        if (staffIdx == item->chord()->vStaffIdx() && item->crossStaff()) {
            // Arpeggio doesn't end on this staff, so extend downwards
            curArpShape.addBBox(RectF(curArpShape.bbox().bottomLeft().x(), curArpShape.bbox().bottomLeft().y(),
                                      curArpShape.bbox().width(), ARBITRARY_ARPEGGIO_LENGTH));
        } else if (staffIdx == endChord->vStaffIdx() && item->crossStaff()) {
            // Arpeggio doesn't start on this staff so move to last note and extend upwards
            curArpShape.translate(PointF(0.0, endChord->downNote()->pagePos().y() - item->pagePos().y()));
            curArpShape.addBBox(RectF(curArpShape.bbox().topLeft().x(), curArpShape.bbox().topLeft().y() - ARBITRARY_ARPEGGIO_LENGTH,
                                      curArpShape.bbox().width(), ARBITRARY_ARPEGGIO_LENGTH));
        }

        Shape chordShape = chord->shape();
        chordShape.translate(chord->pagePos());
        // Remove any element which isn't an accidental the arpeggio intersects
        chordShape.remove_if([curArpShape, arpeggioAccidentalDistance, arpeggioLedgerDistance](ShapeElement& shapeElement) {
            if (!shapeElement.item()
                || !(shapeElement.item()->type() == ElementType::ACCIDENTAL || shapeElement.item()->type() == ElementType::LEDGER_LINE)) {
                return true;
            }
            if (shapeElement.item()->type() == ElementType::ACCIDENTAL) {
                // Pad accidentals with Sid::ArpeggioAccidentalDistance either side
                shapeElement.setTopLeft(PointF(shapeElement.topLeft().x() - arpeggioAccidentalDistance, shapeElement.topLeft().y()));
                shapeElement.setWidth(shapeElement.width() + 2 * arpeggioAccidentalDistance);
            } else if (shapeElement.item()->type() == ElementType::LEDGER_LINE) {
                shapeElement.setTopLeft(PointF(shapeElement.topLeft().x() - arpeggioLedgerDistance, shapeElement.topLeft().y()));
                shapeElement.setWidth(shapeElement.width() + 2 * arpeggioLedgerDistance);
            }
            return !curArpShape.intersects(shapeElement);
        });

        if (chordShape.empty()) {
            // No collisions with accidentals
            continue;
        }

        double chordX = -chordShape.left();
        double diff = chordX - arpX;

        if (!RealIsNull(diff)) {
            double inset = insetDistance(item, ctx, item->mag(), chord);
            largestOverlap = std::min(largestOverlap, diff + inset);
        }
    }
    if (!RealIsNull(largestOverlap)) {
        item->mutldata()->moveX(largestOverlap);
    }
}

double ArpeggioLayout::insetDistance(const Arpeggio* item, const LayoutContext& ctx, double mag_, const Chord* chord)
{
    if (!item || !chord) {
        return 0.0;
    }

    std::vector<Accidental*> _accidentals;

    // generate list of accidentals if none provided
    for (Note* note : chord->notes()) {
        Accidental* accidental = note->accidental();
        if (accidental && accidental->visible()) {
            _accidentals.push_back(accidental);
        }
    }

    if (_accidentals.empty()) {
        return 0.0;
    }

    return insetDistance(item, ctx, mag_, chord, _accidentals);
}

double ArpeggioLayout::insetDistance(const Arpeggio* item, const LayoutContext& ctx, double mag_, const Chord* chord,
                                     const std::vector<Accidental*>& accidentals)
{
    if (!item || !chord) {
        return 0.0;
    }

    if (accidentals.empty()) {
        return 0.0;
    }

    // Only be concerned about the top/bottom of an arpeggio line if this is the start/end chord

    const Segment* seg = item->chord()->segment();
    Chord* endChord = item->chord();
    const PaddingTable paddingTable = item->score()->paddingTable();
    if (EngravingItem* e = seg->element(item->endTrack())) {
        endChord = e->isChord() ? toChord(e) : endChord;
    }
    bool arpStartStave = chord->vStaffIdx() == item->vStaffIdx();
    bool arpEndStave = chord->vStaffIdx() == endChord->vStaffIdx();

    double arpeggioTop = arpStartStave ? insetTop(item, item->chord()) * mag_ : -ARBITRARY_ARPEGGIO_LENGTH;
    double arpeggioBottom = arpEndStave ? insetBottom(item, endChord) * mag_ : ARBITRARY_ARPEGGIO_LENGTH;

    ArpeggioType type = item->arpeggioType();
    bool hasTopArrow = type == ArpeggioType::UP
                       || type == ArpeggioType::UP_STRAIGHT
                       || type == ArpeggioType::BRACKET;
    bool hasBottomArrow = type == ArpeggioType::DOWN
                          || type == ArpeggioType::DOWN_STRAIGHT
                          || type == ArpeggioType::BRACKET;

    const Accidental* furthestAccidental = nullptr;
    for (const Accidental* accidental : accidentals) {
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

    double maximumInset = (paddingTable.at(ElementType::ARPEGGIO).at(ElementType::ACCIDENTAL)
                           - ctx.conf().styleMM(Sid::ArpeggioAccidentalDistanceMin)) * mag_;

    RectF bbox = item->symBbox(furthestAccidental->symId());
    double center = furthestAccidental->note()->pos().y() * mag_;
    double top = center + bbox.top() * mag_;
    double bottom = center + bbox.bottom() * mag_;
    bool collidesWithTop = hasTopArrow && top <= arpeggioTop;
    bool collidesWithBottom = hasBottomArrow && bottom >= arpeggioBottom;

    if (collidesWithTop || collidesWithBottom) {
        return maximumInset;
    }

    return insetWidth(item);
}
