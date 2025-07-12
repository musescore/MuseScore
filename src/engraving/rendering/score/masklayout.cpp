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

#include "masklayout.h"

#include "dom/barline.h"
#include "dom/chord.h"
#include "dom/lyrics.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/note.h"
#include "dom/segment.h"
#include "dom/staff.h"
#include "dom/stafflines.h"
#include "dom/system.h"
#include "dom/page.h"
#include "dom/textlinebase.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void MaskLayout::computeMasks(LayoutContext& ctx, Page* page)
{
    TRACEFUNC;

    bool maskBarlines = ctx.conf().styleB(Sid::maskBarlinesForText);

    for (const System* system : page->systems()) {
        std::vector<TextBase*> allSystemText = collectAllSystemText(system);

        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* measure = toMeasure(mb);

            if (maskBarlines) {
                for (const Segment& seg : measure->segments()) {
                    if (seg.isType(SegmentType::BarLineType)) {
                        computeBarlineMasks(&seg, system, allSystemText, ctx);
                    }
                }
            }
        }
    }
}

void MaskLayout::computeBarlineMasks(const Segment* barlineSement, const System* system, const std::vector<TextBase*>& allSystemText,
                                     LayoutContext& ctx)
{
    if (barlineSement->measure()->isLastInSystem() && barlineSement == barlineSement->measure()->lastEnabled()) {
        return;
    }

    staff_idx_t nstaves = ctx.dom().nstaves();

    std::vector<BarLine*> barlines;
    barlines.reserve(nstaves);

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (!system->staff(staffIdx)->show()) {
            continue;
        }
        BarLine* barline = toBarLine(barlineSement->element(staff2track(staffIdx)));
        if (!barline || barline->spanStaff() == 0) {
            continue;
        }
        maskBarlineForText(barline, allSystemText);
    }
}

void MaskLayout::maskBarlineForText(BarLine* barline, const std::vector<TextBase*>& allSystemText)
{
    TRACEFUNC;

    PointF barlinePos = barline->pagePos();
    Shape barlineShape = barline->shape().translated(barlinePos);

    Shape mask;
    const double spatium = barline->spatium();

    for (TextBase* text : allSystemText) {
        const double fontSizeScaleFactor = text->size() / 10.0;
        const double collisionPadding = 0.2 * spatium * fontSizeScaleFactor;
        const bool hasFrame = text->frameType() != FrameType::NO_FRAME;
        const bool useHighResShape = !text->isDynamic() && !text->hasFrame();
        const double maskPadding = hasFrame ? 0.0 : std::clamp(0.5 * spatium * fontSizeScaleFactor, 0.1 * spatium, spatium);

        PointF textPos = text->pagePos();
        if (!barlineShape.intersects(text->ldata()->bbox().translated(textPos).padded(collisionPadding))) {
            continue;
        }

        Shape textShape = (useHighResShape ? text->ldata()->highResShape() : text->ldata()->shape()).translated(textPos);

        Shape filteredTextShape;
        filteredTextShape.elements().reserve(textShape.elements().size());
        for (const ShapeElement& el : textShape.elements()) {
            if (barlineShape.intersects(el.padded(collisionPadding))) {
                filteredTextShape.add(el);
            }
        }
        if (filteredTextShape.empty()) {
            continue;
        }

        filteredTextShape = filteredTextShape.bbox();
        filteredTextShape.pad(maskPadding);

        mask.add(filteredTextShape.translate(-barlinePos));
    }

    if (mask.empty()) {
        barline->mutldata()->setMask(mask);
        return;
    }

    // Ensure that we don't leave tiny barline fragments. If two masking
    // elements are too close to each other we extend them to join.
    barlineShape.translate(-barlinePos);
    const double minFragmentLengh = 0.5 * spatium;
    cleanupMask(barlineShape, mask, minFragmentLengh);

    barline->mutldata()->setMask(mask);
}

void MaskLayout::cleanupMask(const Shape& itemShape, Shape& mask, double minFragmentLength)
{
    for (size_t i = 0; i < mask.size(); ++i) {
        ShapeElement& el = mask.elements()[i];

        if (el.top() - itemShape.top() < minFragmentLength) {
            el.adjust(0.0, -minFragmentLength, 0.0, 0.0);
        }
        if (itemShape.bottom() - el.bottom() < minFragmentLength) {
            el.adjust(0.0, 0.0, 0.0, minFragmentLength);
        }

        if (el.left() + itemShape.left() < minFragmentLength) {
            el.adjust(-minFragmentLength, 0.0, 0.0, 0.0);
        }
        if (itemShape.right() - el.right() < minFragmentLength) {
            el.adjust(0.0, 0.0, minFragmentLength, 0.0);
        }

        for (size_t j = i + 1; j < mask.size(); ++j) {
            ShapeElement& otherEl = mask.elements()[j];
            if (intersects(el.left(), el.right(), otherEl.left(), otherEl.right())) {
                bool otherIsBelow = otherEl.y() > el.y();
                double vertClearance = otherIsBelow ? otherEl.top() - el.bottom() : el.top() - otherEl.bottom();
                if (vertClearance < minFragmentLength) {
                    (otherIsBelow ? el : otherEl).adjust(0.0, 0.0, 0.0, minFragmentLength);
                }
            }
            if (intersects(el.top(), el.bottom(), otherEl.top(), el.bottom())) {
                bool otherIsRight = otherEl.x() > el.x();
                double horClearance = otherIsRight ? otherEl.left() - el.right() : el.left() - otherEl.right();
                if (horClearance < minFragmentLength) {
                    (otherIsRight ? el : otherEl).adjust(0.0, 0.0, minFragmentLength, 0.0);
                }
            }
        }
    }
}

std::vector<TextBase*> MaskLayout::collectAllSystemText(const System* system)
{
    TRACEFUNC;

    std::vector<TextBase*> allText;

    for (const MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (const MStaff* mstaff : toMeasure(mb)->mstaves()) {
            if (MeasureNumber* measureNumber = mstaff->measureNumber()) {
                allText.push_back(measureNumber);
            }
        }
        for (const Segment& s : toMeasure(mb)->segments()) {
            if (!s.isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE) || !s.enabled()) {
                continue;
            }
            for (EngravingItem* annotation : s.annotations()) {
                if (annotation->isTextBase() && annotation->visible() && system->staff(annotation->staffIdx())->show()) {
                    allText.push_back(toTextBase(annotation));
                }
            }
            if (!s.isChordRestType()) {
                continue;
            }
            for (EngravingItem* chordRest : s.elist()) {
                if (!chordRest || !system->staff(chordRest->staffIdx())->show()) {
                    continue;
                }
                for (Lyrics* lyr : toChordRest(chordRest)->lyrics()) {
                    if (lyr->visible()) {
                        allText.push_back(lyr);
                    }
                }
            }
        }
    }

    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (!spannerSegment->isTextLineBaseSegment() || !system->staff(spannerSegment->staffIdx())->show()
            || !spannerSegment->getProperty(Pid::VISIBLE).toBool()) {
            continue;
        }
        TextLineBaseSegment* textLineBaseSegment = static_cast<TextLineBaseSegment*>(spannerSegment);
        Text* beginText = textLineBaseSegment->text();
        Text* endText = textLineBaseSegment->endText();
        if (beginText && !beginText->empty()) {
            allText.push_back(beginText);
        }
        if (endText && !endText->empty()) {
            allText.push_back(endText);
        }
    }

    return allText;
}

void MaskLayout::maskTABStringLinesForFrets(StaffLines* staffLines, const LayoutContext& ctx)
{
    staff_idx_t staffIdx = staffLines->staffIdx();
    bool linesThrough = ctx.dom().staff(staffIdx)->staffType(Fraction())->linesThrough();

    PointF staffLinesPos = staffLines->pagePos();

    double padding = ctx.conf().styleMM(Sid::tabFretPadding);

    track_idx_t startTrack = staff2track(staffIdx);
    track_idx_t endTrack = startTrack + VOICES;

    Shape mask;

    auto maskFret = [&mask, linesThrough, padding, staffLinesPos] (Chord* chord) {
        for (Note* note : chord->notes()) {
            if (note->visible() && !note->shouldHideFret() && (!linesThrough || note->fretConflict())) {
                RectF noteShape = note->ldata()->bbox().translated(note->pagePos());
                noteShape.pad(padding);
                mask.add(noteShape.translated(-staffLinesPos));
            }
        }
    };

    const Measure* measure = staffLines->measure();
    for (Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            EngravingItem* el = seg->element(track);
            if (!el || !el->isChord()) {
                continue;
            }
            maskFret(toChord(el));
            for (Chord* grace : toChord(el)->graceNotes()) {
                maskFret(grace);
            }
        }
    }

    staffLines->mutldata()->setMask(mask);
}
