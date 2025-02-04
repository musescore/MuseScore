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
#include "segmentlayout.h"

#include "dom/part.h"
#include "dom/drumset.h"
#include "dom/parenthesis.h"

#include "tlayout.h"
#include "chordlayout.h"
#include "horizontalspacing.h"

using namespace mu::engraving::rendering::score;

void SegmentLayout::layoutMeasureIndependentElements(const Segment& segment, track_idx_t track, const LayoutContext& ctx)
{
    LAYOUT_CALL() << segment.typeName() << "(" << segment.eid() << ")";

    if (segment.isJustType(SegmentType::KeySig)) {
        KeySig* ks = toKeySig(segment.element(track));
        if (ks) {
            TLayout::layoutKeySig(ks, ks->mutldata(), ctx.conf());         // LD_INDEPENDENT
        }
    } else if (segment.isJustType(SegmentType::Clef)) {
        Clef* cl = item_cast<Clef*>(segment.element(track));
        if (cl) {
            cl->setSmall(true);
            TLayout::layoutClef(cl, cl->mutldata(), ctx.conf());         // LD_INDEPENDENT
        }
    } else if (segment.isType(SegmentType::HeaderClef)) {
        Clef* cl = item_cast<Clef*>(segment.element(track));
        if (cl) {
            TLayout::layoutClef(cl, cl->mutldata(), ctx.conf());         // LD_INDEPENDENT
        }
    } else if (segment.isType(SegmentType::TimeSig)) {
        TimeSig* ts = item_cast<TimeSig*>(segment.element(track));
        if (ts) {
            TLayout::layoutTimeSig(ts, ts->mutldata(), ctx);         // LD_INDEPENDENT
        }
    } else if (segment.isType(SegmentType::Ambitus)) {
        Ambitus* am = item_cast<Ambitus*>(segment.element(track));
        if (am) {
            TLayout::layoutAmbitus(am, am->mutldata(), ctx);         // LD_INDEPENDENT
        }
    } else if (segment.isType(SegmentType::BarLine)) {
        BarLine* bl = toBarLine(segment.element(track));
        if (bl) {
            // check conditions (see TLayout::layoutBarLine)
            {
                for (const EngravingItem* e : *bl->el()) {
                    if (e->isType(ElementType::ARTICULATION)) {
                        LD_CONDITION(item_cast<const Articulation*>(e)->ldata()->symId.has_value());
                    }
                }
            }
            TLayout::layoutBarLine(bl, bl->mutldata(), ctx);
        }
    }
}

void SegmentLayout::setChordMag(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                                const LayoutConfiguration& conf)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(staff) << " " << segment.typeName() << "(" << segment.eid() << ")";

    IF_ASSERT_FAILED(segment.isJustType(SegmentType::ChordRest)) {
        return;
    }

    const double staffMag = staff->staffMag(&segment);
    const double smallNoteMag = conf.styleD(Sid::smallNoteMag);
    const double graceNoteMag = conf.styleD(Sid::graceNoteMag);

    for (track_idx_t t = startTrack; t < endTrack; ++t) {
        ChordRest* cr = segment.cr(t);
        if (!cr) {
            continue;
        }

        double m = staffMag;
        if (cr->isSmall()) {
            m *= smallNoteMag;
        }

        if (cr->isChord()) {
            double graceMag = m * graceNoteMag;
            Chord* chord = toChord(cr);
            for (Chord* c : chord->graceNotes()) {
                c->mutldata()->setMag(graceMag);
            }
        }
        cr->mutldata()->setMag(m);
    }
}

void SegmentLayout::checkStaffMoveValidity(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack)
{
    LAYOUT_CALL() << segment.typeName() << "(" << segment.eid() << ")";

    IF_ASSERT_FAILED(segment.isJustType(SegmentType::ChordRest)) {
        return;
    }

    for (track_idx_t t = startTrack; t < endTrack; ++t) {
        ChordRest* cr = toChordRest(segment.element(t));
        if (cr) {
            // Check if requested cross-staff is possible
            if (cr->staffMove() || cr->storedStaffMove()) {
                cr->checkStaffMoveValidity();
            }
        }
    }
}

void SegmentLayout::layoutChordDrumset(const Staff* staff, const Segment& segment, track_idx_t startTrack, track_idx_t endTrack,
                                       const LayoutConfiguration& conf)
{
    LAYOUT_CALL() << LAYOUT_ITEM_INFO(staff) << " " << segment.typeName() << "(" << segment.eid() << ")";

    IF_ASSERT_FAILED(segment.isJustType(SegmentType::ChordRest)) {
        return;
    }

    const Instrument* ins = staff->part()->instrument(segment.tick());
    if (!ins->useDrumset()) {
        return;
    }

    const Drumset* drumset = ins->drumset();
    IF_ASSERT_FAILED(drumset) {
        return;
    }

    const StaffType* st = staff->staffTypeForElement(&segment);

    auto layoutDrumset = [](Chord* c, const Drumset* drumset, const StaffType* st, double spatium)
    {
        for (Note* note : c->notes()) {
            int pitch = note->pitch();
            if (!drumset->isValid(pitch)) {
                // LOGD("unmapped drum note %d", pitch);
            } else if (!note->fixed()) {
                note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
                int line = drumset->line(pitch);
                note->setLine(line);

                int off  = st->stepOffset();
                double ld = st->lineDistance().val();
                note->mutldata()->setPosY((line + off * 2.0) * spatium * .5 * ld);
            }
        }
    };

    double spatium = conf.spatium();
    for (track_idx_t t = startTrack; t < endTrack; ++t) {
        Chord* chord = item_cast<Chord*>(segment.element(t), CastMode::MAYBE_BAD); // maybe Rest
        if (!chord) {
            continue;
        }

        for (Chord* c : chord->graceNotes()) {
            layoutDrumset(c, drumset, st, spatium);
        }

        layoutDrumset(chord, drumset, st, spatium);
    }
}

void SegmentLayout::computeChordsUp(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack, const LayoutContext& ctx)
{
    IF_ASSERT_FAILED(segment.isJustType(SegmentType::ChordRest)) {
        return;
    }

    for (track_idx_t t = startTrack; t < endTrack; ++t) {
        ChordRest* cr = segment.cr(t);
        if (!cr) {
            continue;
        }

        if (cr->isChord()) {
            Chord* chord = toChord(cr);

            for (Chord* c : chord->graceNotes()) {
                ChordLayout::computeUp(c, ctx);
            }

            ChordLayout::computeUp(chord, ctx);
        }
    }
}

void SegmentLayout::layoutChordsStem(const Segment& segment, track_idx_t startTrack, track_idx_t endTrack, const LayoutContext& ctx)
{
    IF_ASSERT_FAILED(segment.isJustType(SegmentType::ChordRest)) {
        return;
    }

    for (track_idx_t t = startTrack; t < endTrack; ++t) {
        ChordRest* cr = segment.cr(t);
        if (!cr) {
            continue;
        }

        if (cr->isChord()) {
            Chord* chord = toChord(cr);

            for (Chord* c : chord->graceNotes()) {
                ChordLayout::layoutStem(c, ctx);
            }

            ChordLayout::layoutStem(chord, ctx);
            // stem direction can change later during beam processing
        }
    }
}

void SegmentLayout::placeParentheses(const Segment* segment, staff_idx_t staffIdx)
{
    const EngravingItem* segItem = segment->elementAt(staff2track(staffIdx));
    const std::vector<EngravingItem*> parens = segment->findAnnotations(ElementType::PARENTHESIS, staffIdx * VOICES, staffIdx * VOICES);
    assert(parens.size() <= 2);
    if (parens.empty() || !segItem) {
        return;
    }

    if (parens.size() == 1) {
        // 1 parenthesis
        Parenthesis* paren = toParenthesis(parens.front());
        const bool leftBracket = paren->direction() == DirectionH::LEFT;
        segment->renderer()->layoutItem(paren);
        if (!leftBracket) {
            // Space against existing segment shape
            const double minDist = HorizontalSpacing::minHorizontalDistance(segment->staffShape(paren->staffIdx()),
                                                                            paren->shape().translated(paren->pos()), paren->spatium());
            paren->mutldata()->moveX(minDist);
        } else {
            // Space following segment shape against this
            const double minDist = HorizontalSpacing::minHorizontalDistance(paren->shape().translated(paren->pos()),
                                                                            segment->staffShape(paren->staffIdx()), paren->spatium());
            paren->mutldata()->moveX(-minDist);
        }
        return;
    }

    // 2 parentheses
    Parenthesis* leftParen = nullptr;
    Parenthesis* rightParen = nullptr;
    for (EngravingItem* paren : parens) {
        if (toParenthesis(paren)->direction() == DirectionH::LEFT) {
            leftParen = toParenthesis(paren);
            continue;
        }

        rightParen = toParenthesis(paren);
    }

    assert(leftParen && rightParen);

    segment->renderer()->layoutItem(toParenthesis(leftParen));
    segment->renderer()->layoutItem(toParenthesis(rightParen));
    Shape dummySegShape = segment->staffShape(leftParen->staffIdx());

    const double itemLeftX = segItem->pos().x();
    const double itemRightX = itemLeftX + segItem->width();

    const double leftParenPadding = HorizontalSpacing::minHorizontalDistance(leftParen->shape().translated(leftParen->pos()),
                                                                             dummySegShape, leftParen->spatium());
    leftParen->mutldata()->moveX(-leftParenPadding);
    dummySegShape.add(leftParen->shape().translate(leftParen->pos() + leftParen->staffOffset()));

    const double rightParenPadding = HorizontalSpacing::minHorizontalDistance(dummySegShape, rightParen->shape().translated(
                                                                                  rightParen->pos()), rightParen->spatium());
    rightParen->mutldata()->moveX(rightParenPadding);

    // If the right parenthesis has been padded against the left parenthesis, this means the parenthesis -> parenthesis padding distance
    // is larger than the width of the item the parentheses surrounds. In this case, the result is visually unbalanced.  Move both parens
    // to the left (relative to the segment) in order to centre the item: (b  ) -> ( b )
    const double itemWidth = segItem->width();
    const double parenPadding = segment->score()->paddingTable().at(ElementType::PARENTHESIS).at(ElementType::PARENTHESIS);

    if (itemWidth >= parenPadding) {
        return;
    }

    // Move parentheses to place item in the middle
    const double leftParenX = leftParen->pos().x() + leftParen->ldata()->bbox().x() + leftParen->ldata()->thickness;
    const double rightParenX = rightParen->pos().x() + rightParen->ldata()->bbox().x() + rightParen->width()
                               - rightParen->ldata()->thickness;

    const double leftParenToItem = itemLeftX - leftParenX;
    const double itemToRightParen = rightParenX - itemRightX;
    const double parenToItemDist = (leftParenToItem + itemToRightParen) / 2;

    leftParen->mutldata()->moveX(-std::abs(parenToItemDist - leftParenToItem));
    rightParen->mutldata()->moveX(-std::abs(itemToRightParen - parenToItemDist));
}
