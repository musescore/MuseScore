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
#include "modifydom.h"

#include "dom/measure.h"
#include "dom/staff.h"
#include "dom/spannermap.h"
#include "dom/trill.h"
#include "dom/ornament.h"
#include "dom/note.h"
#include "dom/utils.h"
#include "dom/chord.h"
#include "dom/keysig.h"
#include "dom/hook.h"
#include "dom/part.h"
#include "rendering/score/chordlayout.h"

using namespace mu::engraving::rendering::score;

void ModifyDom::setCrossMeasure(const Measure* measure, LayoutContext& ctx)
{
    bool crossMeasure = ctx.conf().styleB(Sid::crossMeasureValues);
    const DomAccessor& dom = ctx.dom();
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }
                if (cr->isChord()) {
                    Chord* chord = toChord(cr);
                    if (!chord->isGrace()) {
                        ChordLayout::crossMeasureSetup(chord, crossMeasure, ctx);
                    }
                }
            }
        }
    }
}

void ModifyDom::connectTremolo(Measure* m)
{
    m->connectTremolo();
}

void ModifyDom::cmdUpdateNotes(const Measure* measure, const DomAccessor& dom)
{
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        AccidentalState as;          // list of already set accidentals for this measure
        // initAccidentalState
        {
            as.init(staff->keySigEvent(measure->tick()));

            // Trills may carry an accidental into this measure that requires a force-restate
            int ticks = measure->tick().ticks();
            auto spanners = dom.spannerMap().findOverlapping(ticks, ticks, true);
            for (auto iter : spanners) {
                Spanner* spanner = iter.value;
                if (spanner->staffIdx() != staffIdx || !spanner->isTrill()
                    || spanner->tick() == measure->tick() || spanner->tick2() == measure->tick()) {
                    continue;
                }
                Ornament* ornament = toTrill(spanner)->ornament();
                Note* trillNote = ornament ? ornament->noteAbove() : nullptr;
                if (trillNote && trillNote->accidental() && ornament->showAccidental() == OrnamentShowAccidental::DEFAULT) {
                    int line = absStep(trillNote->tpc(), trillNote->epitch());
                    as.setForceRestateAccidental(line, true);
                }
            }
        }

        track_idx_t startTrack = staff->part()->startTrack();
        track_idx_t mainTrack = staffIdx * VOICES;
        track_idx_t endTrack = staff->part()->endTrack();

        for (const Segment& segment : measure->segments()) {
            if (segment.isJustType(SegmentType::KeySig)) {
                KeySig* ks = item_cast<KeySig*>(segment.element(mainTrack));
                if (ks) {
                    Fraction tick = segment.tick();
                    as.init(staff->keySigEvent(tick));
                }
            } else if (segment.isJustType(SegmentType::ChordRest)) {
                for (track_idx_t t = startTrack; t < endTrack; ++t) {
                    Chord* chord = item_cast<Chord*>(segment.element(t), CastMode::MAYBE_BAD); // maybe Rest
                    if (chord) {
                        chord->cmdUpdateNotes(&as, staffIdx);
                    }
                }
            }
        }
    }
}

void ModifyDom::createStems(const Measure* measure, LayoutContext& ctx)
{
    const DomAccessor& dom = ctx.dom();
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        const Staff* staff = dom.staff(staffIdx);
        if (!staff->show()) {
            continue;
        }

        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }

                auto createStems = [](Chord* chord) {
                    if (!chord->shouldHaveStem()) {
                        chord->removeStem();
                        return;
                    }

                    if (!chord->stem()) {
                        chord->createStem();
                    }
                };

                if (cr->isChord()) {
                    Chord* chord = toChord(cr);

                    for (Chord* c : chord->graceNotes()) {
                        createStems(c);
                    }

                    createStems(chord);     // create stems needed to calculate spacing
                    // stem direction can change later during beam processing
                }
            }
        }
    }
}

void ModifyDom::setTrackForChordGraceNotes(Measure* measure, const DomAccessor& dom)
{
    for (staff_idx_t staffIdx = 0; staffIdx < dom.nstaves(); ++staffIdx) {
        track_idx_t startTrack = staffIdx * VOICES;
        track_idx_t endTrack  = startTrack + VOICES;

        for (const Segment& segment : measure->segments()) {
            if (!segment.isJustType(SegmentType::ChordRest)) {
                continue;
            }

            for (track_idx_t t = startTrack; t < endTrack; ++t) {
                ChordRest* cr = segment.cr(t);
                if (!cr) {
                    continue;
                }

                if (cr->isChord()) {
                    Chord* chord = toChord(cr);
                    for (Chord* c : chord->graceNotes()) {
                        c->setTrack(t);
                    }
                }
            }
        }
    }
}

void ModifyDom::sortMeasureBeginSegments(Measure* measure, LayoutContext& ctx)
{
    if (!measure->repeatStart()) {
        return;
    }

    Segment* blSeg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
    Segment* ksSeg = measure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    Segment* tsSeg = measure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));

    if (!blSeg || (!ksSeg && !tsSeg)) {
        return;
    }

    Segment* s1 = nullptr; // leftmost, could be null if there only a time sig or only a key sig
    Segment* s2 = nullptr;
    Segment* s3 = nullptr; // rightmost

    std::vector<Segment*> segments;
    if (ctx.conf().styleB(Sid::changesBetweenEndStartRepeat)) {
        s3 = blSeg;
        s2 = tsSeg ? tsSeg : ksSeg;
        s1 = tsSeg ? ksSeg : nullptr;
    } else {
        s3 = tsSeg ? tsSeg : ksSeg;
        s2 = ksSeg ? ksSeg : blSeg;
        s1 = ksSeg ? blSeg : nullptr;
    }

    if (s2->next() != s3) {
        measure->segments().remove(s2);
        measure->segments().insert(s2, s3);
    }

    if (s1 && s1->next() != s2) {
        measure->segments().remove(s1);
        measure->segments().insert(s1, s2);
    }
}
