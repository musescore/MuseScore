/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "measureread.h"

#include "translation.h"

#include "../dom/ambitus.h"
#include "../dom/anchors.h"
#include "../dom/barline.h"
#include "../dom/beam.h"
#include "../dom/breath.h"
#include "../dom/chord.h"
#include "../dom/dynamic.h"
#include "../dom/expression.h"
#include "../dom/factory.h"
#include "../dom/fermata.h"
#include "../dom/fret.h"
#include "../dom/keysig.h"
#include "../dom/location.h"
#include "../dom/measure.h"
#include "../dom/measurenumber.h"
#include "../dom/measurerepeat.h"
#include "../dom/mmrest.h"
#include "../dom/mmrestrange.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/spacer.h"
#include "../dom/staff.h"
#include "../dom/stafflines.h"
#include "../dom/stafftext.h"
#include "../dom/systemdivider.h"
#include "../dom/timesig.h"
#include "../dom/tuplet.h"
#include "../dom/harmony.h"
#include "../dom/fret.h"
#include "../dom/tremolobar.h"
#include "../dom/tempotext.h"
#include "../dom/image.h"

#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read410;

void MeasureRead::readMeasure(Measure* measure, XmlReader& e, ReadContext& ctx, int staffIdx)
{
    IF_ASSERT_FAILED(ctx.isSameScore(measure)) {
        return;
    }

    double _spatium = measure->spatium();
    ctx.setCurrentMeasure(measure);
    int nextTrack = staffIdx * VOICES;
    ctx.setTrack(nextTrack);

    for (int n = int(measure->m_mstaves.size()); n <= staffIdx; ++n) {
        Staff* staff = ctx.staff(n);
        MStaff* s = new MStaff;
        s->setLines(Factory::createStaffLines(measure));
        s->lines()->setParent(measure);
        s->lines()->setTrack(n * VOICES);
        s->lines()->setVisible(!staff->isLinesInvisible(measure->tick()));
        measure->m_mstaves.push_back(s);
    }

    bool irregular;
    if (e.hasAttribute("len")) {
        StringList sl = e.attribute("len").split(u'/');
        if (sl.size() == 2) {
            measure->m_len = Fraction(sl.at(0).toInt(), sl.at(1).toInt());
        } else {
            LOGD("illegal measure size <%s>", muPrintable(e.attribute("len")));
        }
        irregular = true;
        if (measure->m_len.numerator() <= 0 || measure->m_len.denominator() <= 0 || measure->m_len.denominator() > 128) {
            e.raiseError(muse::mtrc("engraving",
                                    "MSCX error at line %1: invalid measure length: %2").arg(e.lineNumber()).arg(measure->m_len.toString()));
            return;
        }
        ctx.compatTimeSigMap()->add(measure->tick().ticks(), SigEvent(measure->m_len, measure->m_timesig));
        ctx.compatTimeSigMap()->add((measure->tick() + measure->ticks()).ticks(), SigEvent(measure->m_timesig));
    } else {
        irregular = false;
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "voice") {
            ctx.setTrack(nextTrack++);
            ctx.setTick(measure->tick());
            readVoice(measure, e, ctx, staffIdx, irregular);
        } else if (tag == "Marker" || tag == "Jump") {
            EngravingItem* el = Factory::createItemByName(tag, measure);
            el->setTrack(ctx.track());
            TRead::readItem(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            measure->add(el);
        } else if (tag == "stretch") {
            double val = e.readDouble();
            if (val < 0.0) {
                val = 0;
            }
            measure->setUserStretch(val);
        } else if (tag == "noOffset") {
            measure->setNoOffset(e.readInt());
        } else if (tag == "measureNumberMode") {
            measure->setMeasureNumberMode(MeasureNumberMode(e.readInt()));
        } else if (tag == "irregular") {
            measure->setIrregular(e.readBool());
        } else if (tag == "breakMultiMeasureRest") {
            measure->m_breakMultiMeasureRest = e.readBool();
        } else if (tag == "startRepeat") {
            measure->setRepeatStart(true);
            e.readNext();
        } else if (tag == "endRepeat") {
            measure->m_repeatCount = e.readInt();
            measure->setRepeatEnd(true);
        } else if (tag == "vspacer" || tag == "vspacerDown") {
            if (!measure->m_mstaves[staffIdx]->vspacerDown()) {
                Spacer* spacer = Factory::createSpacer(measure);
                spacer->setSpacerType(SpacerType::DOWN);
                spacer->setTrack(staffIdx * VOICES);
                measure->add(spacer);
            }
            measure->m_mstaves[staffIdx]->vspacerDown()->setGap(Millimetre(e.readDouble() * _spatium));
        } else if (tag == "vspacerFixed") {
            if (!measure->m_mstaves[staffIdx]->vspacerDown()) {
                Spacer* spacer = Factory::createSpacer(measure);
                spacer->setSpacerType(SpacerType::FIXED);
                spacer->setTrack(staffIdx * VOICES);
                measure->add(spacer);
            }
            measure->m_mstaves[staffIdx]->vspacerDown()->setGap(Millimetre(e.readDouble() * _spatium));
        } else if (tag == "vspacerUp") {
            if (!measure->m_mstaves[staffIdx]->vspacerUp()) {
                Spacer* spacer = Factory::createSpacer(measure);
                spacer->setSpacerType(SpacerType::UP);
                spacer->setTrack(staffIdx * VOICES);
                measure->add(spacer);
            }
            measure->m_mstaves[staffIdx]->vspacerUp()->setGap(Millimetre(e.readDouble() * _spatium));
        } else if (tag == "visible") {
            measure->m_mstaves[staffIdx]->setVisible(e.readInt());
        } else if ((tag == "slashStyle") || (tag == "stemless")) {
            measure->m_mstaves[staffIdx]->setStemless(e.readInt());
        } else if (tag == "measureRepeatCount") {
            measure->setMeasureRepeatCount(e.readInt(), staffIdx);
        } else if (tag == "SystemDivider") {
            SystemDivider* sd = new SystemDivider(ctx.dummy()->system());
            TRead::read(sd, e, ctx);
            //! TODO Looks like a bug.
            //! The SystemDivider parent must be System
            //! there is a method: `System* system() const { return (System*)parent(); }`,
            //! but when we add it to Measure, the parent will be rewritten.
            measure->add(sd);
        } else if (tag == "multiMeasureRest") {
            measure->m_mmRestCount = e.readInt();
            // set tick to previous measure
            measure->setTick(ctx.lastMeasure()->tick());
            ctx.setTick(ctx.lastMeasure()->tick());
        } else if (tag == "MeasureNumber") {
            MeasureNumber* noText = new MeasureNumber(measure);
            TRead::read(noText, e, ctx);
            noText->setTrack(ctx.track());
            measure->add(noText);
        } else if (tag == "MMRestRange") {
            MMRestRange* range = new MMRestRange(measure);
            TRead::read(range, e, ctx);
            range->setTrack(ctx.track());
            measure->add(range);
        } else if (TRead::readProperties(static_cast<MeasureBase*>(measure), e, ctx)) {
        } else {
            e.unknown();
        }
    }
    ctx.checkConnectors();
    if (measure->isMMRest()) {
        Measure* lm = ctx.lastMeasure();
        ctx.setTick(lm->tick() + lm->ticks());
    }
    ctx.setCurrentMeasure(nullptr);

    measure->connectTremolo();
}

void MeasureRead::readVoice(Measure* measure, XmlReader& e, ReadContext& ctx, int staffIdx, bool irregular)
{
    Segment* segment = nullptr;
    std::vector<Chord*> graceNotes;
    Beam* startingBeam = nullptr;
    Tuplet* tuplet = nullptr;
    Fermata* fermata = nullptr;

    Staff* staff = ctx.staff(staffIdx);
    Fraction timeStretch(staff->timeStretch(measure->tick()));

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "location") {
            Location loc = Location::relative();
            TRead::read(&loc, e, ctx);
            ctx.setLocation(loc);
            if (loc.isTimeTick()) {
                EditTimeTickAnchors::createTimeTickAnchor(measure, ctx.tick() - measure->tick(), track2staff(ctx.track()));
            }
        } else if (tag == "tick") {             // obsolete?
            LOGD() << "read midi tick";
            ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
        } else if (tag == "BarLine") {
            BarLine* barLine = Factory::createBarLine(ctx.dummy()->segment());
            barLine->setTrack(ctx.track());
            TRead::read(barLine, e, ctx);
            //
            //  StartRepeatBarLine: at rtick == 0, always BarLineType::START_REPEAT
            //  BarLine:            in the middle of a measure, has no semantic
            //  EndBarLine:         at the end of a measure
            //  BeginBarLine:       first segment of a measure, systemic barline

            SegmentType st = SegmentType::Invalid;
            Fraction t = ctx.tick() - measure->tick();
            if (t.isNotZero() && (t != measure->ticks())) {
                st = SegmentType::BarLine;
            } else if (barLine->barLineType() == BarLineType::START_REPEAT && t.isZero()) {
                st = SegmentType::StartRepeatBarLine;
            } else if (barLine->barLineType() == BarLineType::START_REPEAT && t == measure->ticks()) {
                // old version, ignore
                delete barLine;
                barLine = 0;
            } else if (t.isZero() && segment == 0) {
                st = SegmentType::BeginBarLine;
            } else {
                st = SegmentType::EndBarLine;
            }
            if (barLine) {
                segment = measure->getSegmentR(st, t);
                segment->add(barLine);
                barLine->renderer()->layoutItem(barLine);
            }
            if (fermata) {
                segment->add(fermata);
                fermata = nullptr;
            }
        } else if (tag == "Chord") {
            Chord* chord = Factory::createChord(ctx.dummy()->segment());
            chord->setTrack(ctx.track());
            TRead::read(chord, e, ctx);
            if (startingBeam) {
                startingBeam->add(chord);         // also calls chord->setBeam(startingBeam)
                startingBeam = nullptr;
            }
//                  if (tuplet && !chord->isGrace())
//                        chord->readAddTuplet(tuplet);
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            if (chord->noteType() != NoteType::NORMAL) {
                graceNotes.push_back(chord);
            } else {
                segment->add(chord);
                for (size_t i = 0; i < graceNotes.size(); ++i) {
                    Chord* gc = graceNotes[i];
                    gc->setGraceIndex(static_cast<int>(i));
                    chord->add(gc);
                }
                graceNotes.clear();
                if (tuplet) {
                    tuplet->add(chord);
                }
                ctx.incTick(chord->actualTicks());
            }
            if (fermata) {
                segment->add(fermata);
                fermata = nullptr;
            }
        } else if (tag == "Rest") {
            if (measure->isMMRest()) {
                segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
                MMRest* mmr = Factory::createMMRest(segment);
                mmr->setTrack(ctx.track());
                mmr->setParent(segment);
                TRead::read(mmr, e, ctx);
                segment->add(mmr);
                ctx.incTick(mmr->actualTicks());
            } else {
                segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
                Rest* rest = Factory::createRest(segment);
                rest->setDurationType(DurationType::V_MEASURE);
                rest->setTicks(measure->timesig() / timeStretch);
                rest->setTrack(ctx.track());
                TRead::read(rest, e, ctx);
                if (startingBeam) {
                    startingBeam->add(rest); // also calls rest->setBeam(startingBeam)
                    startingBeam = nullptr;
                }

                segment->add(rest);
                if (fermata) {
                    segment->add(fermata);
                    fermata = nullptr;
                }

                if (!rest->ticks().isValid()) {    // hack
                    rest->setTicks(measure->timesig() / timeStretch);
                }

                if (tuplet) {
                    tuplet->add(rest);
                }
                ctx.incTick(rest->actualTicks());
            }
        } else if (tag == "Breath") {
            segment = measure->getSegment(SegmentType::Breath, ctx.tick());
            Breath* breath = Factory::createBreath(segment);
            breath->setTrack(ctx.track());
            breath->setPlacement(breath->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
            TRead::read(breath, e, ctx);
            segment->add(breath);
        } else if (tag == "Spanner") {
            TRead::readSpanner(e, ctx, measure, ctx.track());
        } else if (tag == "MeasureRepeat" || tag == "RepeatMeasure") {
            //             4.x                       3.x
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            MeasureRepeat* mr = Factory::createMeasureRepeat(segment);
            mr->setTrack(ctx.track());
            TRead::read(mr, e, ctx);
            if (!mr->numMeasures()) {
                mr->setNumMeasures(1); // 3.x doesn't have any other possibilities
            }
            if (!measure->measureRepeatCount(staffIdx)) {
                measure->setMeasureRepeatCount(1, staffIdx);
            }
            segment->add(mr);
            ctx.incTick(measure->ticks());
        } else if (tag == "Clef") {
            Clef* clef = Factory::createClef(ctx.dummy()->segment());
            clef->setTrack(ctx.track());
            TRead::read(clef, e, ctx);
            clef->setGenerated(false);

            bool header = false;
            if (ctx.score()->mscVersion() < 410) {
                /***********************************************************************
                 * LEGACY: we used to try to guess if the clef is a header based
                 * on context, which is very unreliable. After 4.1, we just TAG it.
                 * *********************************************************************/
                // there may be more than one clef segment for same tick position
                // the first clef may be missing and is added later in layout
                if (ctx.tick() != measure->tick()) {
                    header = false;
                } else if (!segment) {
                    header = true;
                } else {
                    header = true;
                    for (Segment* s = measure->m_segments.first(); s && s->rtick().isZero(); s = s->next()) {
                        if (s->isKeySigType() || s->isTimeSigType()) {
                            // hack: there may be other segment types which should
                            // generate a clef at current position
                            header = false;
                            break;
                        }
                    }
                }
            } else {
                header = clef->isHeader();
            }

            segment = measure->getSegment(header ? SegmentType::HeaderClef : SegmentType::Clef, ctx.tick());
            segment->add(clef);
            clef->setIsHeader(header);
        } else if (tag == "TimeSig") {
            TimeSig* ts = Factory::createTimeSig(ctx.dummy()->segment());
            ts->setTrack(ctx.track());
            TRead::read(ts, e, ctx);
            // if time sig not at beginning of measure => courtesy time sig
            Fraction currTick = ctx.tick();
            bool courtesySig = (currTick > measure->tick());
            if (courtesySig) {
                // if courtesy sig., just add it without map processing
                segment = measure->getSegment(SegmentType::TimeSigAnnounce, currTick);
                segment->add(ts);
            } else {
                // if 'real' time sig., do full process
                segment = measure->getSegment(SegmentType::TimeSig, currTick);
                segment->add(ts);

                timeStretch = ts->stretch().reduced();
                measure->m_timesig = ts->sig() / timeStretch;

                if (!irregular) {
                    measure->m_len = measure->m_timesig;
                }
            }
        } else if (tag == "KeySig") {
            KeySig* ks = Factory::createKeySig(ctx.dummy()->segment());
            ks->setTrack(ctx.track());
            TRead::read(ks, e, ctx);
            Fraction curTick = ctx.tick();
            // if key sig not at beginning of measure => courtesy key sig
            bool courtesySig = (curTick == measure->endTick());
            segment = measure->getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
            segment->add(ks);
            if (!courtesySig) {
                staff->setKey(curTick, ks->keySigEvent());
            }
        } else if (tag == "Text") {
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            StaffText* t = Factory::createStaffText(segment);
            t->setTrack(ctx.track());
            TRead::read(t, e, ctx);
            if (t->empty()) {
                LOGD("==reading empty text: deleted");
                delete t;
            } else {
                segment->add(t);
            }
        }
        //----------------------------------------------------
        // Annotation
        else if (tag == "Dynamic") {
            segment = measure->getChordRestOrTimeTickSegment(ctx.tick());
            Dynamic* dyn = Factory::createDynamic(segment);
            dyn->setTrack(ctx.track());
            TRead::read(dyn, e, ctx);
            segment->add(dyn);
        } else if (tag == "Expression") {
            segment = measure->getChordRestOrTimeTickSegment(ctx.tick());
            Expression* expr = Factory::createExpression(segment);
            expr->setTrack(ctx.track());
            TRead::read(expr, e, ctx);
            segment->add(expr);
        } else if (tag == "Harmony") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            Harmony* el = Factory::createHarmony(segment);

            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "FretDiagram") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            FretDiagram* el = Factory::createFretDiagram(segment);

            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "TremoloBar") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            TremoloBar* el = Factory::createTremoloBar(segment);
            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "Symbol") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            Symbol* el = Factory::createSymbol(segment);

            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "Tempo") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            TempoText* el = Factory::createTempoText(segment);

            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "StaffText") {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            StaffText* el = Factory::createStaffText(segment);
            el->setTrack(ctx.track());
            TRead::read(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0);     // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "Sticking"
                   || tag == "SystemText"
                   || tag == "PlayTechAnnotation"
                   || tag == "Capo"
                   || tag == "StringTunings"
                   || tag == "RehearsalMark"
                   || tag == "InstrumentChange"
                   || tag == "StaffState"
                   || tag == "FiguredBass"
                   || tag == "HarpPedalDiagram"
                   ) {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            EngravingItem* el = Factory::createItemByName(tag, segment);

            el->setTrack(ctx.track());
            TRead::readItem(el, e, ctx);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "Fermata") {
            fermata = Factory::createFermata(ctx.dummy());
            fermata->setTrack(ctx.track());
            fermata->setPlacement(fermata->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
            TRead::read(fermata, e, ctx);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
                Image* el = Factory::createImage(segment);
                el->setTrack(ctx.track());
                TRead::read(el, e, ctx);
                segment->add(el);
            }
        }
        //----------------------------------------------------
        else if (tag == "Tuplet") {
            Tuplet* oldTuplet = tuplet;
            tuplet = Factory::createTuplet(measure);
            tuplet->setTrack(ctx.track());
            tuplet->setTick(ctx.tick());
            tuplet->setParent(measure);
            TRead::read(tuplet, e, ctx);
            if (oldTuplet) {
                oldTuplet->add(tuplet);
            }
        } else if (tag == "endTuplet") {
            if (!tuplet) {
                LOGD("Measure::read: encountered <endTuplet/> when no tuplet was started");
                e.skipCurrentElement();
                continue;
            }
            Tuplet* oldTuplet = tuplet;
            tuplet = tuplet->tuplet();
            if (oldTuplet->elements().empty()) {
                // this should not happen and is a sign of input file corruption
                LOGD("Measure:read: empty tuplet in measure index=%d, input file corrupted?", ctx.currentMeasureIndex());
                if (tuplet) {
                    tuplet->remove(oldTuplet);
                }
                delete oldTuplet;
            }
            e.readNext();
        } else if (tag == "Beam") {
            Beam* beam = Factory::createBeam(ctx.dummy()->system());
            beam->setTrack(ctx.track());
            TRead::read(beam, e, ctx);
            beam->resetExplicitParent();
            if (startingBeam) {
                LOGD("The read beam was not used");
                delete startingBeam;
            }
            startingBeam = beam;
        } else if (tag == "Segment" && segment) {
            TRead::read(segment, e, ctx);
        } else if (tag == "Ambitus") {
            segment = measure->getSegment(SegmentType::Ambitus, ctx.tick());
            Ambitus* range = Factory::createAmbitus(segment);
            TRead::read(range, e, ctx);
            range->setParent(segment);                // a parent segment is needed for setTrack() to work
            range->setTrack(trackZeroVoice(ctx.track()));
            segment->add(range);
        } else {
            e.unknown();
        }
    }
    if (startingBeam) {
        LOGD("The read beam was not used");
        delete startingBeam;
    }
    if (tuplet) {
        LOGD("Measure:readVoice: measure index=%d, <endTuplet/> not found", ctx.currentMeasureIndex());
        if (tuplet->elements().empty()) {
            if (tuplet->tuplet()) {
                tuplet->tuplet()->remove(tuplet);
            }
            delete tuplet;
        }
    }
    if (fermata) {
        SegmentType st = (ctx.tick() == measure->endTick() ? SegmentType::EndBarLine : SegmentType::ChordRest);
        segment = measure->getSegment(st, ctx.tick());
        segment->add(fermata);
        fermata = nullptr;
    }
}
