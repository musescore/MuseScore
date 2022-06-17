/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "measurerw.h"

#include "rw/xml.h"
#include "rw/writecontext.h"

#include "../libmscore/factory.h"
#include "../libmscore/measure.h"
#include "../libmscore/stafflines.h"
#include "../libmscore/staff.h"
#include "../libmscore/spacer.h"
#include "../libmscore/measurenumber.h"
#include "../libmscore/mmrestrange.h"
#include "../libmscore/systemdivider.h"
#include "../libmscore/location.h"
#include "../libmscore/barline.h"
#include "../libmscore/tuplet.h"
#include "../libmscore/chord.h"
#include "../libmscore/segment.h"
#include "../libmscore/fermata.h"
#include "../libmscore/mmrest.h"
#include "../libmscore/breath.h"
#include "../libmscore/spanner.h"
#include "../libmscore/measurerepeat.h"
#include "../libmscore/timesig.h"
#include "../libmscore/keysig.h"
#include "../libmscore/stafftext.h"
#include "../libmscore/ambitus.h"
#include "../libmscore/dynamic.h"

#include "../libmscore/score.h"

#include "log.h"

using namespace mu::engraving::rw;
using namespace mu::engraving;

void MeasureRW::readMeasure(Measure* measure, XmlReader& e, ReadContext& ctx, int staffIdx)
{
    IF_ASSERT_FAILED(ctx.isSameScore(measure)) {
        return;
    }

    qreal _spatium = measure->spatium();
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
            measure->_len = Fraction(sl.at(0).toInt(), sl.at(1).toInt());
        } else {
            LOGD("illegal measure size <%s>", qPrintable(e.attribute("len")));
        }
        irregular = true;
        if (measure->_len.numerator() <= 0 || measure->_len.denominator() <= 0 || measure->_len.denominator() > 128) {
            e.raiseError(QObject::tr("MSCX error at line %1: invalid measure length: %2").arg(e.lineNumber()).arg(measure->_len.toString()));
            return;
        }
        ctx.sigmap()->add(measure->tick().ticks(), SigEvent(measure->_len, measure->m_timesig));
        ctx.sigmap()->add((measure->tick() + measure->ticks()).ticks(), SigEvent(measure->m_timesig));
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
            el->read(e);
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
            sd->read(e);
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
            noText->read(e);
            noText->setTrack(ctx.track());
            measure->add(noText);
        } else if (tag == "MMRestRange") {
            MMRestRange* range = new MMRestRange(measure);
            range->read(e);
            range->setTrack(ctx.track());
            measure->add(range);
        } else if (measure->MeasureBase::readProperties(e)) {
        } else {
            e.unknown();
        }
    }
    e.context()->checkConnectors();
    if (measure->isMMRest()) {
        Measure* lm = ctx.lastMeasure();
        ctx.setTick(lm->tick() + lm->ticks());
    }
    ctx.setCurrentMeasure(nullptr);

    measure->connectTremolo();
}

void MeasureRW::readVoice(Measure* measure, XmlReader& e, ReadContext& ctx, int staffIdx, bool irregular)
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
            loc.read(e);
            ctx.setLocation(loc);
        } else if (tag == "tick") {             // obsolete?
            LOGD() << "read midi tick";
            ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
        } else if (tag == "BarLine") {
            BarLine* barLine = Factory::createBarLine(ctx.dummy()->segment());
            barLine->setTrack(ctx.track());
            barLine->read(e);
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
                barLine->layout();
            }
            if (fermata) {
                segment->add(fermata);
                fermata = nullptr;
            }
        } else if (tag == "Chord") {
            Chord* chord = Factory::createChord(ctx.dummy()->segment());
            chord->setTrack(ctx.track());
            chord->read(e);
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
                mmr->read(e);
                segment->add(mmr);
                ctx.incTick(mmr->actualTicks());
            } else {
                segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
                Rest* rest = Factory::createRest(segment);
                rest->setDurationType(DurationType::V_MEASURE);
                rest->setTicks(measure->timesig() / timeStretch);
                rest->setTrack(ctx.track());
                rest->read(e);
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
            breath->read(e);
            segment->add(breath);
        } else if (tag == "Spanner") {
            Spanner::readSpanner(e, measure, ctx.track());
        } else if (tag == "MeasureRepeat" || tag == "RepeatMeasure") {
            //             4.x                       3.x
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            MeasureRepeat* mr = Factory::createMeasureRepeat(segment);
            mr->setTrack(ctx.track());
            mr->read(e);
            if (!mr->numMeasures()) {
                mr->setNumMeasures(1); // 3.x doesn't have any other possibilities
            }
            if (!measure->measureRepeatCount(staffIdx)) {
                measure->setMeasureRepeatCount(1, staffIdx);
            }
            segment->add(mr);
            ctx.incTick(measure->ticks());
        } else if (tag == "Clef") {
            // there may be more than one clef segment for same tick position
            // the first clef may be missing and is added later in layout

            bool header;
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
            segment = measure->getSegment(header ? SegmentType::HeaderClef : SegmentType::Clef, ctx.tick());
            Clef* clef = Factory::createClef(segment);
            clef->setTrack(ctx.track());
            clef->read(e);
            clef->setGenerated(false);

            segment->add(clef);
        } else if (tag == "TimeSig") {
            TimeSig* ts = Factory::createTimeSig(ctx.dummy()->segment());
            ts->setTrack(ctx.track());
            ts->read(e);
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

                if (irregular) {
                    ctx.sigmap()->add(measure->tick().ticks(), SigEvent(measure->_len, measure->m_timesig));
                    ctx.sigmap()->add((measure->tick() + measure->ticks()).ticks(), SigEvent(measure->m_timesig));
                } else {
                    measure->_len = measure->m_timesig;
                    ctx.sigmap()->add(measure->tick().ticks(), SigEvent(measure->m_timesig));
                }
            }
        } else if (tag == "KeySig") {
            KeySig* ks = Factory::createKeySig(ctx.dummy()->segment());
            ks->setTrack(ctx.track());
            ks->read(e);
            Fraction curTick = ctx.tick();
            if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick.isZero()) {
                // ignore empty key signature
                LOGD("remove keysig c at tick 0");
            } else {
                // if key sig not at beginning of measure => courtesy key sig
                bool courtesySig = (curTick == measure->endTick());
                segment = measure->getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
                segment->add(ks);
                if (!courtesySig) {
                    staff->setKey(curTick, ks->keySigEvent());
                }
            }
        } else if (tag == "Text") {
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            StaffText* t = Factory::createStaffText(segment);
            t->setTrack(ctx.track());
            t->read(e);
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
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            Dynamic* dyn = Factory::createDynamic(segment);
            dyn->setTrack(ctx.track());
            dyn->read(e);
            segment->add(dyn);
        } else if (tag == "Harmony"
                   || tag == "FretDiagram"
                   || tag == "TremoloBar"
                   || tag == "Symbol"
                   || tag == "Tempo"
                   || tag == "StaffText"
                   || tag == "Sticking"
                   || tag == "SystemText"
                   || tag == "PlayTechAnnotation"
                   || tag == "RehearsalMark"
                   || tag == "InstrumentChange"
                   || tag == "StaffState"
                   || tag == "FiguredBass"
                   ) {
            // hack - getSegment needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
            EngravingItem* el = Factory::createItemByName(tag, segment);

            el->setTrack(ctx.track());
            el->read(e);
            if (el->systemFlag() && el->isTopSystemObject()) {
                el->setTrack(0); // original system object always goes on top
            }
            segment->add(el);
        } else if (tag == "Fermata") {
            fermata = Factory::createFermata(ctx.dummy());
            fermata->setTrack(ctx.track());
            fermata->setPlacement(fermata->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
            fermata->read(e);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                segment = measure->getSegment(SegmentType::ChordRest, ctx.tick());
                EngravingItem* el = Factory::createItemByName(tag, segment);
                el->setTrack(ctx.track());
                el->read(e);
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
            tuplet->read(e);
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
            beam->read(e);
            beam->resetExplicitParent();
            if (startingBeam) {
                LOGD("The read beam was not used");
                delete startingBeam;
            }
            startingBeam = beam;
        } else if (tag == "Segment" && segment) {
            segment->read(e);
        } else if (tag == "Ambitus") {
            segment = measure->getSegment(SegmentType::Ambitus, ctx.tick());
            Ambitus* range = Factory::createAmbitus(segment);
            range->read(e);
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

void MeasureRW::writeMeasure(const Measure* measure, XmlWriter& xml, staff_idx_t staff, bool writeSystemElements, bool forceTimeSig)
{
    if (MScore::debugMode) {
        const int mno = measure->no() + 1;
        xml.comment(String("Measure %1").arg(mno));
    }
    if (measure->_len != measure->m_timesig) {
        // this is an irregular measure
        xml.startElement(measure, { { "len", measure->_len.toString() } });
    } else {
        xml.startElement(measure);
    }

    xml.context()->setCurTick(measure->tick());
    xml.context()->setCurTrack(staff * VOICES);

    if (measure->m_mmRestCount > 0) {
        xml.tag("multiMeasureRest", measure->m_mmRestCount);
    }
    if (writeSystemElements) {
        if (measure->repeatStart()) {
            xml.tag("startRepeat");
        }
        if (measure->repeatEnd()) {
            xml.tag("endRepeat", measure->m_repeatCount);
        }
        measure->writeProperty(xml, Pid::IRREGULAR);
        measure->writeProperty(xml, Pid::BREAK_MMR);
        measure->writeProperty(xml, Pid::USER_STRETCH);
        measure->writeProperty(xml, Pid::NO_OFFSET);
        measure->writeProperty(xml, Pid::MEASURE_NUMBER_MODE);
    }
    qreal _spatium = measure->spatium();
    MStaff* mstaff = measure->m_mstaves[staff];
    if (mstaff->noText() && !mstaff->noText()->generated()) {
        mstaff->noText()->write(xml);
    }

    if (mstaff->mmRangeText() && !mstaff->mmRangeText()->generated()) {
        mstaff->mmRangeText()->write(xml);
    }

    if (mstaff->vspacerUp()) {
        xml.tag("vspacerUp", mstaff->vspacerUp()->gap().val() / _spatium);
    }
    if (mstaff->vspacerDown()) {
        if (mstaff->vspacerDown()->spacerType() == SpacerType::FIXED) {
            xml.tag("vspacerFixed", mstaff->vspacerDown()->gap().val() / _spatium);
        } else {
            xml.tag("vspacerDown", mstaff->vspacerDown()->gap().val() / _spatium);
        }
    }
    if (!mstaff->visible()) {
        xml.tag("visible", mstaff->visible());
    }
    if (mstaff->stemless()) {
        xml.tag("slashStyle", mstaff->stemless());     // for backwards compatibility
        xml.tag("stemless", mstaff->stemless());
    }
    if (mstaff->measureRepeatCount()) {
        xml.tag("measureRepeatCount", mstaff->measureRepeatCount());
    }

    track_idx_t strack = staff * VOICES;
    track_idx_t etrack = strack + VOICES;
    for (const EngravingItem* e : measure->el()) {
        if (e->generated()) {
            continue;
        }

        bool writeSystem = writeSystemElements;
        if (e->systemFlag()) {
            ElementType et = e->type();
            if ((et == ElementType::REHEARSAL_MARK)
                || (et == ElementType::SYSTEM_TEXT)
                || (et == ElementType::TRIPLET_FEEL)
                || (et == ElementType::PLAYTECH_ANNOTATION)
                || (et == ElementType::JUMP)
                || (et == ElementType::MARKER)
                || (et == ElementType::TEMPO_TEXT)
                || (et == ElementType::VOLTA)
                || (et == ElementType::TEXTLINE && e->systemFlag())) {
                writeSystem = (e->staffIdx() == staff); // always show these on appropriate staves
            }
        }

        if (e->staffIdx() != staff) {
            if (!e->systemFlag() || (e->systemFlag() && !writeSystem)) {
                continue;
            }
        }

        e->write(xml);
    }
    Q_ASSERT(measure->first());
    Q_ASSERT(measure->last());
    if (measure->first() && measure->last()) {
        measure->score()->writeSegments(xml, strack, etrack, measure->first(), measure->last()->next1(), writeSystemElements, forceTimeSig);
    }

    xml.endElement();
}
