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
#include "read410.h"

#include "../types/types.h"

#include "dom/anchors.h"
#include "dom/audio.h"
#include "dom/beam.h"
#include "dom/breath.h"
#include "dom/chord.h"
#include "dom/dynamic.h"
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/figuredbass.h"
#include "dom/fret.h"
#include "dom/hairpin.h"
#include "dom/harmony.h"
#include "dom/lyrics.h"
#include "dom/masterscore.h"
#include "dom/measurerepeat.h"
#include "dom/mscoreview.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/sticking.h"
#include "dom/text.h"
#include "dom/tie.h"
#include "dom/tremolotwochord.h"
#include "dom/tuplet.h"

#include "../compat/tremolocompat.h"
#include "staffread.h"
#include "tread.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::read410;

muse::Ret Read410::readScore(Score* score, XmlReader& e, rw::ReadInOutData* data)
{
    ReadContext ctx(score);
    if (data) {
        if (data->overriddenSpatium.has_value()) {
            ctx.setSpatium(data->overriddenSpatium.value());
        }

        ctx.setPropertiesToSkip(data->propertiesToSkip);
    }

    if (!score->isMaster() && data) {
        ctx.initLinks(data->links);
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "programVersion") {
            String ver = e.readText();
            if (score->isMaster()) {
                score->setMscoreVersion(ver);
            }
        } else if (tag == "programRevision") {
            int rev = e.readInt(nullptr, 16);
            if (score->isMaster()) {
                score->setMscoreRevision(rev);
            }
        } else if (tag == "Revision") {
            e.skipCurrentElement();
        } else if (tag == "LastEID") {
            e.skipCurrentElement();
        } else if (tag == "Score") {
            if (!readScore410(score, e, ctx)) {
                if (e.error() == muse::XmlStreamReader::CustomError) {
                    return make_ret(Err::FileCriticallyCorrupted, e.errorString());
                }
                return make_ret(Err::FileBadFormat, e.errorString());
            }
        } else if (tag == "museScore") {
            // pass
        } else {
            e.skipCurrentElement();
        }
    }

    if (!score->isMaster()) {
        Excerpt* ex = score->excerpt();
        ex->setTracksMapping(ctx.tracks());
    }

    ctx.clearOrphanedConnectors();

    if (data) {
        data->links = ctx.readLinks();
        data->settingsCompat = ctx.settingCompat();
    }

    return muse::make_ok();
}

bool Read410::readScore410(Score* score, XmlReader& e, ReadContext& ctx)
{
    std::vector<int> sysStaves;
    while (e.readNextStartElement()) {
        ctx.setTrack(muse::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "eid") {
            AsciiStringView s = e.readAsciiText();
            EID eid = EID::fromStdString(s);
            if (eid.isValid()) {
                score->setEID(eid);
            }
        } else if (tag == "Staff") {
            StaffRead::readStaff(score, e, ctx);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            score->m_audio = new Audio;
            TRead::read(score->m_audio, e, ctx);
        } else if (tag == "showOmr") {
            e.skipCurrentElement();
        } else if (tag == "playMode") {
            score->m_playMode = PlayMode(e.readInt());
        } else if (tag == "LayerTag") {
            e.skipCurrentElement();
        } else if (tag == "Layer") {
            e.skipCurrentElement();
        } else if (tag == "currentLayer") {
            e.skipCurrentElement();
        } else if (tag == "Synthesizer") {
            score->m_synthesizerState.read(e);
        } else if (tag == "page-offset") {
            score->m_pageNumberOffset = e.readInt();
        } else if (tag == "Division") {
            score->m_fileDivision = e.readInt();
        } else if (tag == "open") {
            score->m_isOpen = e.readBool();
        } else if (tag == "showInvisible") {
            score->m_showInvisible = e.readInt();
        } else if (tag == "showUnprintable") {
            score->m_showUnprintable = e.readInt();
        } else if (tag == "showFrames") {
            score->m_showFrames = e.readInt();
        } else if (tag == "showMargins") {
            score->m_showPageborders = e.readInt();
        } else if (tag == "showSoundFlags") {
            score->m_showSoundFlags = e.readInt();
        } else if (tag == "markIrregularMeasures") {
            score->m_markIrregularMeasures = e.readInt();
        } else if (tag == "Style") {
            // Since version 400, the style is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "copyright" || tag == "rights") {
            score->setMetaTag(u"copyright", Text::readXmlText(e, score));
        } else if (tag == "movement-number") {
            score->setMetaTag(u"movementNumber", e.readText());
        } else if (tag == "movement-title") {
            score->setMetaTag(u"movementTitle", e.readText());
        } else if (tag == "work-number") {
            score->setMetaTag(u"workNumber", e.readText());
        } else if (tag == "work-title") {
            score->setMetaTag(u"workTitle", e.readText());
        } else if (tag == "source") {
            score->setMetaTag(u"source", e.readText());
        } else if (tag == "metaTag") {
            String name = e.attribute("name");
            score->setMetaTag(name, e.readText());
        } else if (tag == "Order") {
            ScoreOrder order;
            order.read(e);
            if (order.isValid()) {
                score->setScoreOrder(order);
            }
        } else if (tag == "SystemObjects") {
            // the staves to show system objects
            score->clearSystemObjectStaves();
            while (e.readNextStartElement()) {
                if (e.name() == "Instance") {
                    int staffIdx = e.attribute("staffId").toInt() - 1;
                    // TODO: read the other attributes from this element when we begin treating different classes
                    // of system objects differently. ex:
                    // bool showBarNumbers = !(e.hasAttribute("barNumbers") && e.attribute("barNumbers") == "false");
                    if (staffIdx > 0) {
                        sysStaves.push_back(staffIdx);
                    }
                    e.skipCurrentElement();
                } else {
                    e.skipCurrentElement();
                }
            }
        } else if (tag == "SystemLocks") {
            TRead::readSystemLocks(score, e);
        } else if (tag == "Part") {
            Part* part = new Part(score);
            TRead::read(part, e, ctx);
            score->appendPart(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
            TRead::readItem(s, e, ctx);
            score->addSpanner(s);
        } else if (tag == "Excerpt") {
            // Since version 400, the Excerpts are stored in a separate file
            e.skipCurrentElement();
        } else if (e.name() == "initialPartId") {
            if (score->excerpt()) {
                score->excerpt()->setInitialPartId(ID(e.readInt()));
            } else {
                e.skipCurrentElement();
            }
        } else if (e.name() == "Tracklist") {
            int strack = e.intAttribute("sTrack",   -1);
            int dtrack = e.intAttribute("dstTrack", -1);
            if (strack != -1 && dtrack != -1) {
                ctx.tracks().insert({ strack, dtrack });
            }
            e.skipCurrentElement();
        } else if (tag == "Score") {
            // Since version 400, the Excerpts is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "name") {
            String n = e.readText();
            if (!score->isMaster()) {     //ignore the name if it's not a child score
                score->excerpt()->setName(n, /*saveAndNotify=*/ false);
            }
        } else if (tag == "layoutMode") {
            String s = e.readText();
            if (s == "line") {
                score->setLayoutMode(LayoutMode::LINE);
            } else if (s == "system") {
                score->setLayoutMode(LayoutMode::SYSTEM);
            } else {
                LOGD("layoutMode: %s", muPrintable(s));
            }
        } else {
            e.unknown();
        }
    }
    ctx.reconnectBrokenConnectors();
    if (e.error() != muse::XmlStreamReader::NoError) {
        if (e.error() == muse::XmlStreamReader::CustomError) {
            LOGE() << e.errorString();
        } else {
            LOGE() << String(u"XML read error at line %1, column %2: %3").arg(e.lineNumber(), e.columnNumber())
                .arg(String::fromAscii(e.name().ascii()));
        }
        return false;
    }

    score->connectTies();

    score->m_fileDivision = Constants::DIVISION;

    // Make sure every instrument has an instrumentId set.
    for (Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            pair.second->updateInstrumentId();
        }
    }

    score->setUpTempoMap();

    for (Part* p : score->m_parts) {
        p->updateHarmonyChannels(false);
    }

    score->masterScore()->rebuildMidiMapping();
    score->masterScore()->updateChannel();

    for (Staff* staff : score->staves()) {
        staff->updateOttava();
    }
    for (int idx : sysStaves) {
        score->addSystemObjectStaff(score->staff(idx));
    }

    return true;
}

bool Read410::pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale)
{
    assert(dst->isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE));

    Score* score = dst->score();
    ReadContext ctx(score);
    ctx.setPasteMode(true);

    std::vector<Harmony*> pastedHarmony;
    std::vector<Chord*> graceNotes;
    Beam* startingBeam = nullptr;
    Tuplet* tuplet = nullptr;
    TremoloTwoChord* prevTremolo = nullptr;
    Fraction dstTick = dst->tick();
    bool pasted = false;
    Fraction tickLen = Fraction(0, 1);
    int staves  = 0;
    bool done   = false;
    bool doScale = (scale != Fraction(1, 1));

    while (e.readNextStartElement()) {
        if (done) {
            break;
        }
        if (e.name() != "StaffList") {
            e.unknown();
            break;
        }
        String version = e.attribute("version", u"NONE");
        if (!MScore::testMode) {
            if (version != Constants::MSC_VERSION_STR) {
                LOGD("pasteStaff: bad version");
                break;
            }
        }
        Fraction tickStart = Fraction::fromString(e.attribute("tick"));
        Fraction oTickLen = Fraction::fromString(e.attribute("len"));
        tickLen = oTickLen * scale;
        int staffStart = e.intAttribute("staff", 0);
        staves = e.intAttribute("staves", 0);

        if (tickLen.isZero() || staves == 0) {
            break;
        }

        Fraction oEndTick = dstTick + oTickLen;
        auto oSpanner = score->spannerMap().findContained(dstTick.ticks(), oEndTick.ticks());
        bool spannerFound = false;

        ctx.setTickOffset(dstTick - tickStart);
        ctx.setTick(Fraction(0, 1));

        while (e.readNextStartElement()) {
            if (done) {
                break;
            }
            if (e.name() != "Staff") {
                e.unknown();
                break;
            }
            ctx.setTransposeChromatic(0);
            ctx.setTransposeDiatonic(0);

            int srcStaffIdx = e.intAttribute("id", 0);
            ctx.setTrack(srcStaffIdx * static_cast<int>(VOICES));
            ctx.setTrackOffset(static_cast<int>((dstStaff - staffStart) * VOICES));
            size_t dstStaffIdx = ctx.track() / VOICES;
            if (dstStaffIdx >= dst->score()->nstaves()) {
                LOGD("paste beyond staves");
                done = true;
                break;
            }

            while (e.readNextStartElement()) {
                pasted = true;
                const AsciiStringView tag(e.name());

                if (tag == "transposeChromatic") {
                    ctx.setTransposeChromatic(static_cast<int8_t>(e.readInt()));
                } else if (tag == "transposeDiatonic") {
                    ctx.setTransposeDiatonic(static_cast<int8_t>(e.readInt()));
                } else if (tag == "voiceOffset") {
                    int voiceOffset[VOICES];
                    std::fill(voiceOffset, voiceOffset + VOICES, -1);
                    while (e.readNextStartElement()) {
                        if (e.name() != "voice") {
                            e.unknown();
                        }
                        voice_idx_t voiceId = static_cast<voice_idx_t>(e.intAttribute("id", -1));
                        assert(voiceId < VOICES);
                        voiceOffset[voiceId] = e.readInt();
                    }
                    if (!score->makeGap1(dstTick, dstStaffIdx, tickLen, voiceOffset)) {
                        LOGD() << "cannot make gap in staff " << dstStaffIdx << " at tick " << dstTick.ticks();
                        done = true;             // break main loop, cannot make gap
                        break;
                    }
                } else if (tag == "location") {
                    Location loc = Location::relative();
                    TRead::read(&loc, e, ctx);
                    ctx.setLocation(loc);
                    if (loc.isTimeTick()) {
                        Measure* measure = score->tick2measure(ctx.tick());
                        EditTimeTickAnchors::createTimeTickAnchor(measure, ctx.tick() - measure->tick(), track2staff(ctx.track()));
                    }
                } else if (tag == "Tuplet") {
                    Tuplet* oldTuplet = tuplet;
                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    // no paste into local time signature
                    if (score->staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                        MScore::setError(MsError::DEST_LOCAL_TIME_SIGNATURE);
                        if (oldTuplet && oldTuplet->elements().empty()) {
                            delete oldTuplet;
                        }
                        return false;
                    }
                    Measure* measure = score->tick2measure(tick);
                    tuplet = Factory::createTuplet(measure);
                    tuplet->setTrack(ctx.track());
                    TRead::read(tuplet, e, ctx);
                    if (doScale) {
                        tuplet->setTicks(tuplet->ticks() * scale);
                        tuplet->setBaseLen(tuplet->baseLen().fraction() * scale);
                    }
                    tuplet->setParent(measure);
                    tuplet->setTick(tick);
                    tuplet->setTuplet(oldTuplet);
                    if (tuplet->rtick() + tuplet->actualTicks() > measure->ticks()) {
                        delete tuplet;
                        if (oldTuplet && oldTuplet->elements().empty()) {
                            delete oldTuplet;
                        }
                        MScore::setError(MsError::TUPLET_CROSSES_BAR);
                        return false;
                    }
                    if (oldTuplet) {
                        tuplet->readAddTuplet(oldTuplet);
                    }
                } else if (tag == "endTuplet") {
                    if (!tuplet) {
                        LOGD("Score::pasteStaff: encountered <endTuplet/> when no tuplet was started");
                        e.skipCurrentElement();
                        continue;
                    }
                    Tuplet* oldTuplet = tuplet;
                    tuplet = tuplet->tuplet();
                    if (oldTuplet->elements().empty()) {
                        LOGD("Score::pasteStaff: ended tuplet is empty");
                        if (tuplet) {
                            tuplet->remove(oldTuplet);
                        }
                        delete oldTuplet;
                    } else {
                        oldTuplet->sortElements();
                    }
                    e.readNext();
                } else if (tag == "Chord" || tag == "Rest" || tag == "MeasureRepeat") {
                    ChordRest* cr = toChordRest(Factory::createItemByName(tag, score->dummy()));
                    cr->setTrack(ctx.track());
                    TRead::readItem(cr, e, ctx);
                    cr->setSelected(false);
                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    // no paste into local time signature
                    if (score->staff(dstStaffIdx)->isLocalTimeSignature(tick)) {
                        MScore::setError(MsError::DEST_LOCAL_TIME_SIGNATURE);
                        return false;
                    }
                    if (score->tick2measure(tick)->isMeasureRepeatGroup(dstStaffIdx)) {
                        MeasureRepeat* mr = score->tick2measure(tick)->measureRepeatElement(dstStaffIdx);
                        score->deleteItem(mr);    // resets any measures related to mr
                    }
                    if (startingBeam) {
                        startingBeam->add(cr);             // also calls cr->setBeam(startingBeam)
                        startingBeam = nullptr;
                    }
                    if (cr->isGrace()) {
                        graceNotes.push_back(toChord(cr));
                    } else {
                        if (tuplet) {
                            cr->readAddTuplet(tuplet);
                        }
                        ctx.incTick(cr->actualTicksAt(tick));
                        if (doScale) {
                            Fraction d = cr->durationTypeTicks();
                            cr->setTicks(cr->ticks() * scale);
                            cr->setDurationType(d * scale);
                            for (Lyrics* l : cr->lyrics()) {
                                l->setTicks(l->ticks() * scale);
                            }
                        }
                        if (cr->isChord()) {
                            Chord* chord = toChord(cr);
                            // disallow tie across barline within two-note tremolo
                            // tremolos can potentially still straddle the barline if no tie is required
                            // but these will be removed later

                            if (chord->tremoloTwoChord()) {
                                prevTremolo = chord->tremoloTwoChord();
                                prevTremolo->setChord1(chord);
                                chord->setTremoloTwoChord(prevTremolo);
                            } else if (!chord->tremoloTwoChord() && prevTremolo) {
                                prevTremolo->setChord2(chord);
                                chord->setTremoloTwoChord(prevTremolo);
                                prevTremolo = nullptr;
                            }

                            if (TremoloTwoChord* tremolo = chord->tremoloTwoChord()) {
                                if (doScale) {
                                    Fraction d = tremolo->durationType().ticks();
                                    tremolo->setDurationType(d * scale);
                                }
                                Measure* m = score->tick2measure(tick);
                                Fraction ticks = cr->actualTicks();
                                Fraction rticks = m->endTick() - tick;
                                if (rticks < ticks || (rticks != ticks && rticks < ticks * 2)) {
                                    MScore::setError(MsError::DEST_TREMOLO);
                                    return false;
                                }
                            }
                            for (size_t i = 0; i < graceNotes.size(); ++i) {
                                Chord* gc = graceNotes.at(i);
                                gc->setGraceIndex(i);
                                Score::transposeChord(gc, tick);
                                chord->add(gc);
                            }
                            graceNotes.clear();
                        }
                        // delete pending ties, they are not selected when copy
                        if ((tick - dstTick) + cr->actualTicks() >= tickLen) {
                            if (cr->isChord()) {
                                Chord* c = toChord(cr);
                                for (Note* note: c->notes()) {
                                    if (note->laissezVib()) {
                                        continue;
                                    }
                                    Tie* tie = note->tieFor();
                                    if (tie) {
                                        note->setTieFor(0);
                                        delete tie;
                                    }
                                }
                            }
                        }
                        // shorten last cr to fit in the space made by makeGap
                        if ((tick - dstTick) + cr->actualTicks() > tickLen) {
                            Fraction newLength = tickLen - (tick - dstTick);
                            // check previous CR on same track, if it has tremolo, delete the tremolo
                            // we don't want a tremolo and two different chord durations
                            if (cr->isChord()) {
                                Segment* s = score->tick2leftSegment(tick - Fraction::fromTicks(1));
                                if (s) {
                                    ChordRest* crt = toChordRest(s->element(cr->track()));
                                    if (!crt) {
                                        crt = s->nextChordRest(cr->track(), true);
                                    }
                                    if (crt && crt->isChord()) {
                                        Chord* chrt = toChord(crt);
                                        TremoloTwoChord* tr = chrt->tremoloTwoChord();
                                        if (tr) {
                                            tr->setChords(chrt, toChord(cr));
                                            chrt->remove(tr);
                                            delete tr;
                                        }
                                    }
                                }
                            }
                            if (!cr->tuplet()) {
                                // shorten duration
                                // exempt notes in tuplets, since we don't allow copy of partial tuplet anyhow
                                // TODO: figure out a reasonable fudge factor to make sure shorten tuplets appropriately if we do ever copy a partial tuplet
                                cr->setTicks(newLength);
                                cr->setDurationType(newLength);
                            }
                        }
                        score->pasteChordRest(cr, tick);
                    }
                } else if (tag == "Spanner") {
                    TRead::readSpanner(e, ctx, score, ctx.track());
                    spannerFound = true;
                } else if (tag == "Harmony") {
                    // transpose
                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    Measure* m = score->tick2measure(tick);
                    Segment* seg = m->undoGetSegment(SegmentType::ChordRest, tick);
                    Harmony* harmony = Factory::createHarmony(seg);
                    harmony->setTrack(ctx.track());
                    TRead::read(harmony, e, ctx);
                    harmony->setTrack(ctx.track());

                    Staff* staffDest = score->staff(ctx.track() / VOICES);
                    Interval interval = staffDest->transpose(tick);
                    if (!ctx.style().styleB(Sid::concertPitch) && !interval.isZero()) {
                        interval.flip();
                        int rootTpc = transposeTpc(harmony->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(harmony->baseTpc(), interval, true);
                        score->undoTransposeHarmony(harmony, rootTpc, baseTpc);
                    }

                    // remove pre-existing chords on this track
                    // but be sure not to remove any we just added
                    for (EngravingItem* el : seg->findAnnotations(ElementType::HARMONY, ctx.track(), ctx.track())) {
                        if (std::find(pastedHarmony.begin(), pastedHarmony.end(), el) == pastedHarmony.end()) {
                            score->undoRemoveElement(el);
                        }
                    }
                    harmony->setParent(seg);
                    score->undoAddElement(harmony);
                    pastedHarmony.push_back(harmony);
                } else if (tag == "Dynamic"
                           || tag == "Expression"
                           || tag == "Symbol"
                           || tag == "FretDiagram"
                           || tag == "TremoloBar"
                           || tag == "Marker"
                           || tag == "Jump"
                           || tag == "Image"
                           || tag == "Text"
                           || tag == "StaffText"
                           || tag == "PlayTechAnnotation"
                           || tag == "Capo"
                           || tag == "StringTunings"
                           || tag == "TempoText"
                           || tag == "FiguredBass"
                           || tag == "Sticking"
                           || tag == "Fermata"
                           || tag == "HarpPedalDiagram"
                           ) {
                    EngravingItem* el = Factory::createItemByName(tag, score->dummy());
                    el->setTrack(ctx.track());                // a valid track might be necessary for el->read() to work
                    if (el->isFermata()) {
                        el->setPlacement(el->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
                    }
                    TRead::readItem(el, e, ctx);

                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    Measure* m = score->tick2measure(tick);
                    Segment* seg = m->undoGetChordRestOrTimeTickSegment(tick);
                    el->setParent(seg);

                    // be sure to paste the element in the destination track;
                    // setting track needs to be repeated, as it might have been overwritten by el->read()
                    // preserve *voice* from source, though
                    el->setStaffIdx(ctx.track() / VOICES);
                    score->undoAddElement(el);
                } else if (tag == "Clef") {
                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    Measure* m = score->tick2measure(tick);
                    if (m->tick().isNotZero() && m->tick() == tick) {
                        m = m->prevMeasure();
                    }
                    Segment* segment = m->undoGetSegment(SegmentType::Clef, tick);
                    Clef* clef = Factory::createClef(segment);
                    TRead::read(clef, e, ctx);
                    clef->setTrack(ctx.track());
                    clef->setParent(segment);
                    score->undoChangeElement(segment->element(ctx.track()), clef);
                } else if (tag == "Breath") {
                    Fraction tick = doScale ? (ctx.tick() - dstTick) * scale + dstTick : ctx.tick();
                    Measure* m = score->tick2measure(tick);
                    if (m->tick() == tick) {
                        m = m->prevMeasure();
                    }
                    Segment* segment = m->undoGetSegment(SegmentType::Breath, tick);
                    Breath* breath = Factory::createBreath(segment);
                    breath->setTrack(ctx.track());
                    breath->setPlacement(breath->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
                    TRead::read(breath, e, ctx);
                    breath->setParent(segment);
                    score->undoChangeElement(segment->element(ctx.track()), breath);
                } else if (tag == "Beam") {
                    Beam* beam = Factory::createBeam(score->dummy()->system());
                    beam->setTrack(ctx.track());
                    TRead::read(beam, e, ctx);
                    beam->resetExplicitParent();
                    if (startingBeam) {
                        LOGD("The read beam was not used");
                        delete startingBeam;
                    }
                    startingBeam = beam;
                } else if (tag == "BarLine") {
                    e.skipCurrentElement();              // ignore bar line
                } else {
                    LOGD("PasteStaff: element %s not handled", tag.ascii());
                    e.skipCurrentElement();              // ignore
                }
            }

            ctx.checkConnectors();
            if (startingBeam) {
                LOGD("The read beam was not used");
                delete startingBeam;
            }
            if (tuplet) {
                LOGD("<endTuplet/> not found");
                if (tuplet->elements().empty()) {
                    if (tuplet->tuplet()) {
                        tuplet->tuplet()->remove(tuplet);
                    }
                    delete tuplet;
                }
            }
        }
        // fix up spanners
        if (doScale && spannerFound) {
            // build list of original spanners
            std::vector<Spanner*> oSpannerList;
            for (auto interval : oSpanner) {
                Spanner* sp = interval.value;
                oSpannerList.push_back(sp);
            }
            auto nSpanner = score->spannerMap().findContained(dstTick.ticks(), oEndTick.ticks());
            for (auto interval : nSpanner) {
                Spanner* sp = interval.value;
                // skip if not in this staff list
                if (sp->staffIdx() < dstStaff || sp->staffIdx() >= dstStaff + staves) {
                    continue;
                }
                // CHORD and NOTE spanners are normally handled already
                if (sp->anchor() == Spanner::Anchor::CHORD || sp->anchor() == Spanner::Anchor::NOTE) {
                    continue;
                }
                // skip if present originally
                auto i = std::find(oSpannerList.begin(), oSpannerList.end(), sp);
                if (i != oSpannerList.end()) {
                    continue;
                }
                Fraction tick = (sp->tick() - dstTick) * scale + dstTick;
                sp->undoChangeProperty(Pid::SPANNER_TICK, tick);
                sp->undoChangeProperty(Pid::SPANNER_TICKS, sp->ticks() * scale);
            }
        }
    }

    for (Score* s : score->scoreList()) {     // for all parts
        s->connectTies();
    }

    if (pasted) {                         //select only if we pasted something
        staff_idx_t endStaff = dstStaff + staves;
        if (endStaff > score->nstaves()) {
            endStaff = score->nstaves();
        }

        if (score->cmdState().layoutRange()) {
            score->cmdState().reset();
            score->setLayout(dstTick, dstTick + tickLen, dstStaff, endStaff, dst);
        }

        //TODO: look if this could be done different
        Measure* dstM = score->tick2measure(dstTick);
        Measure* endM = score->tick2measure(dstTick + tickLen);
        for (staff_idx_t i = dstStaff; i < endStaff; i++) {
            for (Measure* m = dstM; m && m != endM->nextMeasure(); m = m->nextMeasure()) {
                m->checkMeasure(i);
            }
        }
        score->m_selection.setRangeTicks(dstTick, dstTick + tickLen, dstStaff, endStaff);

        //finding the first element that has a track
        //the canvas position will be set to this element
        EngravingItem* el = nullptr;
        Segment* s = score->tick2segmentMM(dstTick);
        Segment* s2 = score->tick2segmentMM(dstTick + tickLen);
        bool found = false;
        if (s2) {
            s2 = s2->next1MM();
        }
        while (!found && s != s2) {
            for (size_t i = dstStaff * VOICES; i < (endStaff + 1) * VOICES; i++) {
                el = s->element(i);
                if (el) {
                    found = true;
                    break;
                }
            }
            s = s->next1MM();
        }

        for (MuseScoreView* v : score->m_viewer) {
            v->adjustCanvasPosition(el);
        }
        if (!score->selection().isRange()) {
            score->m_selection.setState(SelState::RANGE);
        }
    }
    return true;
}

void Read410::pasteSymbols(XmlReader& e, ChordRest* dst)
{
    Score* score = dst->score();
    ReadContext ctx(score);
    ctx.setPasteMode(true);

    Segment* currSegm = dst->segment();
    Fraction destTick = Fraction(0, 1);                // the tick and track to place the pasted element at
    track_idx_t destTrack = 0;
    bool done        = false;
    int segDelta    = 0;
    Segment* startSegm = currSegm;
    Fraction startTick   = dst->tick();        // the initial tick and track where to start pasting
    track_idx_t startTrack  = dst->track();
    track_idx_t maxTrack    = score->ntracks();

    while (e.readNextStartElement()) {
        if (done) {
            break;
        }
        if (e.name() != "SymbolList") {
            e.unknown();
            break;
        }
        String version = e.attribute("version", u"NONE");
        if (version != Constants::MSC_VERSION_STR) {
            break;
        }

        while (e.readNextStartElement()) {
            if (done) {
                break;
            }
            const AsciiStringView tag(e.name());

            if (tag == "trackOffset") {
                destTrack = startTrack + e.readInt();
                currSegm  = startSegm;
            } else if (tag == "tickOffset") {
                destTick = startTick + Fraction::fromTicks(e.readInt());
            } else if (tag == "segDelta") {
                segDelta = e.readInt();
            } else if (tag == "Articulation"
                       || tag == "Ornament"
                       || tag == "Arpeggio"
                       || tag == "TremoloSingleChord"
                       || tag == "Sticking"
                       || tag == "Lyrics") {
                // Elements that can be attached only to a Chord
                Measure* meas = score->tick2measure(destTick);
                Segment* seg = meas ? meas->undoGetSegment(SegmentType::ChordRest, destTick) : nullptr;
                if (!seg) {
                    LOGD() << "No ChordRest segment for " << tag << " at tick " << destTick.ticks();
                    e.skipCurrentElement();
                    continue;
                }

                ChordRest* cr = seg->cr(destTrack);
                if (!cr || !cr->isChord()) {
                    LOGD() << "No Chord for " << tag << " at tick " << destTick.ticks();
                    e.skipCurrentElement();
                    continue;
                }

                EngravingItem* el = Factory::createItemByName(tag, score->dummy());
                el->setTrack(destTrack);
                TRead::readItem(el, e, ctx);
                el->setTrack(destTrack);
                el->setParent(cr);
                score->undoAddElement(el);
            } else if (tag == "Fermata") {
                Measure* meas = score->tick2measure(destTick);
                Segment* seg = meas ? meas->undoGetSegment(SegmentType::ChordRest, destTick) : nullptr;
                if (!seg) {
                    LOGD() << "No ChordRest segment for Fermata at tick " << destTick.ticks();
                    e.skipCurrentElement();
                    continue;
                }

                Fermata* b = Factory::createFermata(score->dummy()->segment());
                b->setTrack(destTrack);
                TRead::read(b, e, ctx);
                b->setTrack(destTrack);
                b->setParent(seg);
                score->undoAddElement(b);
            } else if (tag == "Breath") {
                Measure* meas = score->tick2measure(destTick);
                Segment* seg = meas ? meas->undoGetSegment(SegmentType::Breath, destTick) : nullptr;
                if (!seg) {
                    LOGD() << "No Breath segment for Breath at tick " << destTick.ticks();
                    e.skipCurrentElement();
                    continue;
                }

                Breath* b = Factory::createBreath(score->dummy()->segment());
                b->setTrack(destTrack);
                TRead::read(b, e, ctx);
                b->setTrack(destTrack);
                b->setParent(seg);
                score->undoAddElement(b);
            } else if (tag == "Dynamic"
                       || tag == "Expression"
                       || tag == "StaffText"
                       || tag == "PlayTechAnnotation"
                       || tag == "Capo"
                       || tag == "HarpPedalDiagram"
                       || tag == "StringTunings") {
                // Text elements that can be attached to ChordRest or TimeTick segments
                Measure* meas = score->tick2measure(destTick);
                Segment* seg = meas ? meas->undoGetChordRestOrTimeTickSegment(destTick) : nullptr;
                if (!seg) {
                    LOGD() << "No segment for " << tag << " at tick " << destTick.ticks();
                    e.skipCurrentElement();
                    continue;
                }

                EngravingItem* el = Factory::createItemByName(tag, score->dummy());
                el->setTrack(destTrack);
                TRead::readItem(el, e, ctx);
                el->setTrack(destTrack);
                el->setParent(seg);
                score->undoAddElement(el);
            } else if (tag == "Slur"
                       || tag == "HairPin"
                       || tag == "Ottava"
                       || tag == "Trill"
                       || tag == "LetRing"
                       || tag == "Vibrato"
                       || tag == "PalmMute"
                       || tag == "WhammyBar"
                       || tag == "Rasgueado"
                       || tag == "HarmonicMark"
                       || tag == "PickScrape"
                       || tag == "TextLine"
                       || tag == "Pedal") {
                // Spanners
                if (destTrack >= maxTrack) {
                    LOGD("PasteSymbols: no track for %s", tag.ascii());
                    e.skipCurrentElement();
                    continue;
                }
                Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
                s->setTrack(destTrack);
                TRead::readItem(s, e, ctx);
                s->setTrack(destTrack);
                s->setTrack2(destTrack);
                s->setTick(destTick);
                score->undoAddElement(s);
            } else if (tag == "Harmony" || tag == "FretDiagram") {
                //
                // Harmony elements (= chord symbols) are positioned respecting
                // the original tickOffset: advance to destTick (or near)
                // same for FretDiagram elements
                //
                Measure* meas = score->tick2measure(destTick);
                Segment* seg = meas ? meas->undoGetChordRestOrTimeTickSegment(destTick) : nullptr;

                if (destTrack >= maxTrack || seg == nullptr) {
                    LOGD() << "No track or segment for " << tag << " at tick " << destTick.ticks();
                    e.skipCurrentElement(); // ignore
                    continue;
                }
                if (tag == "Harmony") {
                    Harmony* el = Factory::createHarmony(seg);
                    el->setTrack(trackZeroVoice(destTrack));
                    TRead::read(el, e, ctx);
                    el->setTrack(trackZeroVoice(destTrack));
                    // transpose
                    Staff* staffDest = score->staff(track2staff(destTrack));
                    Interval interval = staffDest->transpose(destTick);
                    if (!ctx.style().styleB(Sid::concertPitch) && !interval.isZero()) {
                        interval.flip();
                        int rootTpc = transposeTpc(el->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(el->baseTpc(), interval, true);
                        score->undoTransposeHarmony(el, rootTpc, baseTpc);
                    }
                    el->setParent(seg);
                    score->undoAddElement(el);
                } else {
                    FretDiagram* el = Factory::createFretDiagram(seg);
                    el->setTrack(trackZeroVoice(destTrack));
                    TRead::read(el, e, ctx);
                    el->setTrack(trackZeroVoice(destTrack));
                    el->setParent(seg);
                    score->undoAddElement(el);
                }
            } else if (tag == "FiguredBass") {
                // TODO: when overhauling FiguredBass, clean this up
                //
                // FiguredBass is currently positioned respecting the distance in chords, while
                // it may be more appropriate to respect time distance, using TimeTick segments.
                for (; currSegm && segDelta > 0; segDelta--) {
                    currSegm = currSegm->nextCR(destTrack);
                }
                // check the intended dest. track and segment exist
                if (destTrack >= maxTrack || currSegm == nullptr) {
                    LOGD("PasteSymbols: no track or segment for %s", tag.ascii());
                    e.skipCurrentElement();                           // ignore
                    continue;
                }
                // check there is a segment element in the required track
                if (currSegm->element(destTrack) == nullptr) {
                    LOGD("PasteSymbols: no track element for %s", tag.ascii());
                    e.skipCurrentElement();
                    continue;
                }

                // FiguredBass always belongs to first staff voice
                destTrack = trackZeroVoice(destTrack);
                Fraction ticks;
                FiguredBass* el = Factory::createFiguredBass(currSegm);
                el->setTrack(destTrack);
                TRead::read(el, e, ctx);
                el->setTrack(destTrack);
                // if f.b. is off-note, we have to locate a place before currSegm
                // where an on-note f.b. element could (potentially) be
                // (while having an off-note f.b. without an on-note one before it
                // is un-idiomatic, possible mismatch in rhythmic patterns between
                // copy source and paste destination does not allow to be too picky)
                if (!el->onNote()) {
                    FiguredBass* onNoteFB = nullptr;
                    Segment* prevSegm = currSegm;
                    bool done1    = false;
                    while (prevSegm) {
                        if (done1) {
                            break;
                        }
                        prevSegm = prevSegm->prev1(SegmentType::ChordRest);
                        // if there is a ChordRest in the dest. track
                        // this segment is a (potential) f.b. location
                        if (prevSegm->element(destTrack) != nullptr) {
                            done1 = true;
                        }
                        // in any case, look for a f.b. in annotations:
                        // if there is a f.b. element in the right track,
                        // this is an (actual) f.b. location
                        for (EngravingItem* a : prevSegm->annotations()) {
                            if (a->isFiguredBass() && a->track() == destTrack) {
                                onNoteFB = toFiguredBass(a);
                                done1 = true;
                            }
                        }
                    }
                    if (!prevSegm) {
                        LOGD("PasteSymbols: can't place off-note FiguredBass");
                        delete el;
                        continue;
                    }
                    // by default, split on-note duration in half: half on-note and half off-note
                    Fraction totTicks  = currSegm->tick() - prevSegm->tick();
                    Fraction destTick1 = prevSegm->tick() + (totTicks * Fraction(1, 2));
                    ticks         = totTicks * Fraction(1, 2);
                    if (onNoteFB) {
                        onNoteFB->setTicks(totTicks * Fraction(1, 2));
                    }
                    // look for a segment at this tick; if none, create one
                    Segment* nextSegm = prevSegm;
                    while (nextSegm && nextSegm->tick() < destTick1) {
                        nextSegm = nextSegm->next1(SegmentType::ChordRest);
                    }
                    if (!nextSegm || nextSegm->tick() > destTick1) {                            // no ChordRest segm at this tick
                        nextSegm = Factory::createSegment(prevSegm->measure(), SegmentType::ChordRest, destTick1);
                        if (!nextSegm) {
                            LOGD("PasteSymbols: can't find or create destination segment for FiguredBass");
                            delete el;
                            continue;
                        }
                        score->undoAddElement(nextSegm);
                    }
                    currSegm = nextSegm;
                } else {
                    // by default, assign to FiguredBass element the duration of the chord it refers to
                    ticks = toChordRest(currSegm->element(destTrack))->ticks();
                }
                // in both cases, look for an existing f.b. element in segment and remove it, if found
                FiguredBass* oldFB = nullptr;
                for (EngravingItem* a : currSegm->annotations()) {
                    if (a->isFiguredBass() && a->track() == destTrack) {
                        oldFB = toFiguredBass(a);
                        break;
                    }
                }
                if (oldFB) {
                    score->undoRemoveElement(oldFB);
                }
                el->setParent(currSegm);
                el->setTicks(ticks);
                score->undoAddElement(el);
            } else {
                LOGD("PasteSymbols: element %s not handled", tag.ascii());
                e.skipCurrentElement();     // ignore
            }
        } // outer while readNextstartElement()
    } // inner while readNextstartElement()
}

void Read410::readTremoloCompat(compat::TremoloCompat* tc, XmlReader& xml)
{
    IF_ASSERT_FAILED(tc->parent) {
        return;
    }

    ReadContext ctx(tc->parent->score());
    ctx.setPasteMode(true);
    TRead::read(tc, xml, ctx);
}

void Read410::doReadItem(EngravingItem* item, XmlReader& xml)
{
    ReadContext ctx(item->score());
    ctx.setPasteMode(true);
    TRead::readItem(item, xml, ctx);
}
