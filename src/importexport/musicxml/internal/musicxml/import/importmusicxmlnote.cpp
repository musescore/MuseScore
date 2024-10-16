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
#include "importmusicxmlnote.h"
#include "dom/beam.h"
#include "dom/chord.h"
#include "dom/chordrest.h"
#include "dom/clef.h"
#include "dom/drumset.h"
#include "dom/factory.h"
#include "dom/figuredbass.h"
#include "dom/instrument.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/tremolosinglechord.h"
#include "dom/tremolotwochord.h"
#include "dom/tuplet.h"
#include "dom/utils.h"
#include "types/typesconv.h"
#include "internal/musicxml/import/importmusicxmllogger.h"
#include "internal/musicxml/import/importmusicxmlnotepitch.h"
#include "serialization/xmlstreamreader.h"
#include "thirdparty/kors_logger/src/log_base.h"
#include "importmusicxmlpass1.h"
#include "importmusicxmlpass2.h"
#include "importmusicxmlnoteduration.h"
#include "global/types/string.h"
#include "global/types/bytearray.h"

using namespace mu::engraving;
using namespace mu::iex::musicxml;
using namespace muse;

MusicXmlParserNote::MusicXmlParserNote(muse::XmlStreamReader& e, engraving::Score* score, MusicXmlLogger* logger,
                                       MusicXmlParserPass1& pass1,
                                       MusicXmlParserPass2& pass2, const muse::String& partId, engraving::Measure* measure,
                                       const Fraction sTime, const Fraction prevSTime, Fraction& missingPrev, Fraction& dura,
                                       Fraction& missingCurr, String& currentVoice, GraceChordList& gcl, size_t& gac, Beams& currBeams,
                                       FiguredBassList& fbl, int& alt, MusicXmlTupletStates& tupletStates, Tuplets& tuplets,
                                       ArpeggioMap& arpMap, DelayedArpMap& delayedArps)
    : m_e(e), m_score(score), m_logger(logger), m_pass1(pass1), m_pass2(pass2), m_partId(partId), m_measure(measure), m_sTime(sTime),
    m_prevSTime(prevSTime), m_missingPrev(missingPrev), m_dura(dura), m_missingCurr(missingCurr), m_currentVoice(currentVoice), m_gcl(gcl),
    m_gac(gac), m_currBeams(currBeams), m_fbl(fbl), m_alt(alt), m_tupletStates(tupletStates), m_tuplets(tuplets), m_arpMap(arpMap),
    m_delayedArps(delayedArps)
{
}

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Collects beamTypes, used in computeBeamMode.
 */

void MusicXmlParserNote::beam(std::map<int, String>& beamTypes)
{
    bool hasBeamNo;
    int beamNo = m_e.asciiAttribute("number").toInt(&hasBeamNo);
    String s = m_e.readText();

    beamTypes.insert({ hasBeamNo ? beamNo : 1, s });
}

/**
 Calculate the beam mode based on the collected beamTypes.
 */

BeamMode MusicXmlParserNote::computeBeamMode(const std::map<int, String>& beamTypes)
{
    // Start with uniquely-handled beam modes
    if (muse::value(beamTypes, 1) == u"continue"
        && muse::value(beamTypes, 2) == u"begin") {
        return BeamMode::BEGIN16;
    } else if (muse::value(beamTypes, 1) == u"continue"
               && muse::value(beamTypes, 2) == u"continue"
               && muse::value(beamTypes, 3) == u"begin") {
        return BeamMode::BEGIN32;
    }
    // Generic beam modes are naive to all except the first beam
    else if (muse::value(beamTypes, 1) == u"begin") {
        return BeamMode::BEGIN;
    } else if (muse::value(beamTypes, 1) == u"continue") {
        return BeamMode::MID;
    } else if (muse::value(beamTypes, 1) == u"end") {
        return BeamMode::END;
    } else {
        // backward-hook, forward-hook, and other unknown combinations
        return BeamMode::AUTO;
    }
}

void MusicXmlParserNote::handleBeamAndStemDir(ChordRest* cr, const BeamMode bm, const DirectionV sd, Beam*& beam, bool hasBeamingInfo)
{
    if (!cr) {
        return;
    }
    // create a new beam
    if (bm == BeamMode::BEGIN) {
        // if currently in a beam, delete it
        if (beam) {
            LOGD("handleBeamAndStemDir() new beam, removing previous incomplete beam %p", beam);
            removeBeam(beam);
        }
        // create a new beam
        beam = Factory::createBeam(cr->score()->dummy()->system());
        beam->setTrack(cr->track());
        beam->setDirection(sd);
    }
    // add ChordRest to beam
    if (beam) {
        // verify still in the same track (if still in the same voice)
        // and in a beam ...
        // (note no check is done on correct order of beam begin/continue/end)
        // TODO: Some BEGINs are being skipped
        if (cr->track() != beam->track()) {
            LOGD("handleBeamAndStemDir() from track %zu to track %zu -> abort beam",
                 beam->track(), cr->track());
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (bm == BeamMode::NONE) {
            LOGD("handleBeamAndStemDir() in beam, bm BeamMode::NONE -> abort beam");
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (!(bm == BeamMode::BEGIN || bm == BeamMode::MID || bm == BeamMode::END || bm == BeamMode::BEGIN16
                     || bm == BeamMode::BEGIN32)) {
            LOGD("handleBeamAndStemDir() in beam, bm %d -> abort beam", static_cast<int>(bm));
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else {
            // actually add cr to the beam
            beam->add(cr);
        }
    }
    // if no beam, set stem direction on chord itself
    if (!beam) {
        static_cast<Chord*>(cr)->setStemDirection(sd);
        // set beam to none if score has beaming information and note can get beam, otherwise
        // set to auto
        bool canGetBeam = (cr->durationType().type() >= DurationType::V_EIGHTH
                           && cr->durationType().type() <= DurationType::V_1024TH);
        if (hasBeamingInfo && canGetBeam) {
            cr->setBeamMode(BeamMode::NONE);
        } else {
            cr->setBeamMode(BeamMode::AUTO);
        }
    }
    // terminate the current beam and add to the score
    if (beam && bm == BeamMode::END) {
        beam = nullptr;
    }
}

/**
 Convert a MusicXML notehead name to a MuseScore headgroup.
 */

NoteHeadGroup MusicXmlParserNote::convertNotehead(String mxmlName)
{
    // map MusicXML notehead name to a MuseScore headgroup
    static const std::map<String, NoteHeadGroup> map {
        { u"slash", NoteHeadGroup::HEAD_SLASH },
        { u"triangle", NoteHeadGroup::HEAD_TRIANGLE_UP },
        { u"diamond", NoteHeadGroup::HEAD_DIAMOND },
        { u"cross", NoteHeadGroup::HEAD_PLUS },
        { u"x", NoteHeadGroup::HEAD_CROSS },
        { u"circle-x", NoteHeadGroup::HEAD_XCIRCLE },
        { u"inverted triangle", NoteHeadGroup::HEAD_TRIANGLE_DOWN },
        { u"slashed", NoteHeadGroup::HEAD_SLASHED1 },
        { u"back slashed", NoteHeadGroup::HEAD_SLASHED2 },
        { u"normal", NoteHeadGroup::HEAD_NORMAL },
        { u"do", NoteHeadGroup::HEAD_DO },
        { u"re", NoteHeadGroup::HEAD_RE },
        { u"mi", NoteHeadGroup::HEAD_MI },
        { u"fa", NoteHeadGroup::HEAD_FA },
        { u"fa up", NoteHeadGroup::HEAD_FA },
        { u"so", NoteHeadGroup::HEAD_SOL },
        { u"la", NoteHeadGroup::HEAD_LA },
        { u"ti", NoteHeadGroup::HEAD_TI }
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    } else {
        LOGD("unknown notehead %s", muPrintable(mxmlName));      // TODO
    }

    // default: return 0
    return NoteHeadGroup::HEAD_NORMAL;
}

/**
 Parse the /score-partwise/part/measure/note/stem node.
 */

void MusicXmlParserNote::stem(DirectionV& stemDirection, bool& noStem)
{
    // defaults
    stemDirection = DirectionV::AUTO;
    noStem = false;

    String s = m_e.readText();

    if (s == u"up") {
        stemDirection = DirectionV::UP;
    } else if (s == u"down") {
        stemDirection = DirectionV::DOWN;
    } else if (s == u"none") {
        noStem = true;
    } else if (s == u"double") {
    } else {
        m_logger->logError(String(u"unknown stem direction %1").arg(s), &m_e);
    }
}

/**
 * Determine whole measure rest.
 */

// By convention, whole measure rests do not have a "type" element
// As of MusicXML 3.0, this can be indicated by an attribute "measure",
// but for backwards compatibility the "old" convention still has to be supported.
// Also verify the rest fits exactly in the measure, as some programs
// (e.g. Cakewalk SONAR X2 Studio [Version: 19.0.0.306]) leave out
// the type for all rests.
// Sibelius calls all whole-measure rests "whole", even if the duration != 4/4

bool MusicXmlParserNote::isWholeMeasureRest(const String& type, const Fraction dura, const Fraction mDura)
{
    if (!dura.isValid()) {
        return false;
    }

    if (!mDura.isValid()) {
        return false;
    }

    return (type.empty() && dura == mDura)
           || (type == u"whole" && dura == mDura);
}

/**
 * Determine duration for a note or rest.
 * This includes whole measure rest detection.
 */

TDuration MusicXmlParserNote::determineDuration(const bool rest, const bool measureRest, const String& type, const int dots,
                                                const Fraction dura, const Fraction mDura)
{
    //LOGD("determineDuration rest %d type '%s' dots %d dura %s mDura %s",
    //       rest, muPrintable(type), dots, muPrintable(dura.print()), muPrintable(mDura.print()));

    TDuration res;
    if (rest) {
        if (measureRest || isWholeMeasureRest(type, dura, mDura)) {
            res.setType(DurationType::V_MEASURE);
        } else if (type.empty()) {
            // If no type, set duration type based on duration.
            // Note that sometimes unusual duration (e.g. 261/256) are found.
            res.setVal(dura.ticks());
        } else {
            ByteArray ba = type.toAscii();
            res.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
            res.setDots(dots);
        }
    } else {
        ByteArray ba = type.toAscii();
        res.setType(TConv::fromXml(ba.constChar(), DurationType::V_INVALID));
        res.setDots(dots);
        if (res.type() == DurationType::V_INVALID) {
            res.setType(DurationType::V_QUARTER);        // default, TODO: use dura ?
        }
    }

    //LOGD("-> dur %hhd (%s) dots %d ticks %s",
    //       res.type(), muPrintable(res.name()), res.dots(), muPrintable(dura.print()));

    return res;
}

/**
 * Find (or create if not found) the chord at \a tick and \a track.
 * Note: staff move is a note property in MusicXML, but chord property in MuseScore
 * This is simply ignored here, effectively using the last chords value.
 */

Chord* MusicXmlParserNote::findOrCreateChord(Score*, Measure* m,
                                             const Fraction& tick, const int track, const int move,
                                             const TDuration duration, const Fraction dura,
                                             BeamMode bm, bool small)
{
    //LOGD("findOrCreateChord tick %d track %d dur ticks %d ticks %s bm %hhd",
    //       tick, track, duration.ticks(), muPrintable(dura.print()), bm);
    Chord* c = m->findChord(tick, track);
    if (c == 0) {
        Segment* s = m->getSegment(SegmentType::ChordRest, tick);
        c = Factory::createChord(s);
        // better not to force beam end, as the beam palette does not support it
        if (bm == BeamMode::END) {
            c->setBeamMode(BeamMode::AUTO);
        } else {
            c->setBeamMode(bm);
        }
        c->setTrack(track);
        // Chord is initialized with the smallness of its first note.
        // If a non-small note is added later, this is handled in handleSmallness.
        c->setSmall(small);

        setChordRestDuration(c, duration, dura);
        s->add(c);
    }
    c->setStaffMove(move);
    return c;
}

/**
 * convert duration and slash to grace note type
 */

NoteType MusicXmlParserNote::graceNoteType(const TDuration duration, const bool slash)
{
    NoteType nt = NoteType::APPOGGIATURA;
    if (slash) {
        nt = NoteType::ACCIACCATURA;
    } else {
        if (duration.type() == DurationType::V_QUARTER) {
            nt = NoteType::GRACE4;
        } else if (duration.type() == DurationType::V_16TH) {
            nt = NoteType::GRACE16;
        } else if (duration.type() == DurationType::V_32ND) {
            nt = NoteType::GRACE32;
        }
    }
    return nt;
}

Chord* MusicXmlParserNote::createGraceChord(Score* score, const int track,
                                            const TDuration duration, const bool slash, const bool small)
{
    Chord* c = Factory::createChord(score->dummy()->segment());
    c->setNoteType(graceNoteType(duration, slash));
    c->setTrack(track);
    // Chord is initialized with the smallness of its first note.
    // If a non-small note is added later, this is handled in handleSmallness.
    c->setSmall(small);
    // note grace notes have no durations, use default fraction 0/1
    setChordRestDuration(c, duration, Fraction());
    return c;
}

// TODO: refactor: optimize parameters

void MusicXmlParserNote::setPitch(Note* note, const MusicXmlInstruments& instruments, const String& instrumentId,
                                  const MusicXmlNotePitch& mnp,
                                  const int octaveShift, const Instrument* const instrument)
{
    if (mnp.unpitched()) {
        if (hasDrumset(instruments) && muse::contains(instruments, instrumentId)) {
            // step and oct are display-step and ...-oct
            // get pitch from instrument definition in drumset instead
            int unpitched = instruments.at(instrumentId).unpitched;
            note->setPitch(std::clamp(unpitched, 0, 127));
            // TODO - does this need to be key-aware?
            note->setTpc(pitch2tpc(unpitched, Key::C, Prefer::NEAREST));             // TODO: necessary ?
        } else {
            //LOGD("disp step %d oct %d", displayStep, displayOctave);
            xmlSetPitch(note, mnp.displayStep(), 0, 0.0, mnp.displayOctave(), 0, instrument);
        }
    } else {
        xmlSetPitch(note, mnp.step(), mnp.alter(), mnp.tuning(), mnp.octave(), octaveShift, instrument);
    }
}

/**
 * convert display-step and display-octave to staff line
 */

void MusicXmlParserNote::handleDisplayStep(ChordRest* cr, int step, int octave, const Fraction& tick, double spatium)
{
    if (0 <= step && step <= 6 && 0 <= octave && octave <= 9) {
        //LOGD("rest step=%d oct=%d", step, octave);
        ClefType clef = cr->staff()->clef(tick);
        int po = ClefInfo::pitchOffset(clef);
        //LOGD(" clef=%hhd po=%d step=%d", clef, po, step);
        int dp = 7 * (octave + 2) + step;
        //LOGD(" dp=%d po-dp=%d", dp, po-dp);
        cr->ryoffset() = (po - dp + 3) * spatium / 2;
    }
}

void MusicXmlParserNote::handleSmallness(bool cueOrSmall, Note* note, Chord* c)
{
    if (cueOrSmall) {
        note->setSmall(!c->isSmall()); // Avoid redundant smallness
    } else {
        note->setSmall(false);
        if (c->isSmall()) {
            // What was a small chord becomes small notes in a non-small chord
            c->setSmall(false);
            for (Note* otherNote : c->notes()) {
                if (note != otherNote) {
                    otherNote->setSmall(true);
                }
            }
        }
    }
}

/**
 Set the notehead parameters.
 */

void MusicXmlParserNote::setNoteHead(Note* note, const Color noteheadColor, const bool noteheadParentheses, const String& noteheadFilled)
{
    Score* const score = note->score();

    if (noteheadColor.isValid()) {
        note->setColor(noteheadColor);
    }
    if (noteheadParentheses) {
        Symbol* s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisLeft);
        s->setParent(note);
        score->addElement(s);
        s = new Symbol(note);
        s->setSym(SymId::noteheadParenthesisRight);
        s->setParent(note);
        score->addElement(s);
    }

    if (noteheadFilled == u"no") {
        note->setHeadType(NoteHeadType::HEAD_HALF);
    } else if (noteheadFilled == u"yes") {
        note->setHeadType(NoteHeadType::HEAD_QUARTER);
    }
}

void MusicXmlParserNote::addTremolo(ChordRest* cr,
                                    const int tremoloNr, const String& tremoloType, const String& tremoloSmufl,
                                    Chord*& tremStart,
                                    MusicXmlLogger* logger, const muse::XmlStreamReader* const xmlreader,
                                    Fraction& timeMod)
{
    if (!cr->isChord()) {
        return;
    }
    if (tremoloNr) {
        //LOGD("tremolo %d type '%s' ticks %d tremStart %p", tremoloNr, muPrintable(tremoloType), ticks, _tremStart);
        if (tremoloNr == 1 || tremoloNr == 2 || tremoloNr == 3 || tremoloNr == 4) {
            if (tremoloType.empty() || tremoloType == u"single") {
                TremoloType type = TremoloType::INVALID_TREMOLO;
                switch (tremoloNr) {
                case 1: type = TremoloType::R8;
                    break;
                case 2: type = TremoloType::R16;
                    break;
                case 3: type = TremoloType::R32;
                    break;
                case 4: type = TremoloType::R64;
                    break;
                }

                if (type != TremoloType::INVALID_TREMOLO) {
                    TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(cr));
                    tremolo->setTremoloType(type);
                    cr->add(tremolo);
                }
            } else if (tremoloType == u"start") {
                if (tremStart) {
                    logger->logError(u"MusicXml::import: double tremolo start", xmlreader);
                }
                tremStart = static_cast<Chord*>(cr);
                // timeMod takes into account also the factor 2 of a two-note tremolo
                if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                    timeMod.setDenominator(timeMod.denominator() / 2);
                }
            } else if (tremoloType == u"stop") {
                if (tremStart) {
                    TremoloType type = TremoloType::INVALID_TREMOLO;
                    switch (tremoloNr) {
                    case 1: type = TremoloType::C8;
                        break;
                    case 2: type = TremoloType::C16;
                        break;
                    case 3: type = TremoloType::C32;
                        break;
                    case 4: type = TremoloType::C64;
                        break;
                    }

                    if (type != TremoloType::INVALID_TREMOLO) {
                        TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(mu::engraving::toChord(cr));
                        tremolo->setTremoloType(type);
                        tremolo->setChords(tremStart, static_cast<Chord*>(cr));
                        // fixup chord duration and type
                        const Fraction tremDur = cr->ticks() * Fraction(1, 2);
                        tremolo->chord1()->setDurationType(tremDur);
                        tremolo->chord1()->setTicks(tremDur);
                        tremolo->chord2()->setDurationType(tremDur);
                        tremolo->chord2()->setTicks(tremDur);
                        // add tremolo to first chord (only)
                        tremStart->add(tremolo);
                    }
                    // timeMod takes into account also the factor 2 of a two-note tremolo
                    if (timeMod.isValid() && ((timeMod.denominator() % 2) == 0)) {
                        timeMod.setDenominator(timeMod.denominator() / 2);
                    }
                } else {
                    logger->logError(u"MusicXml::import: double tremolo stop w/o start", xmlreader);
                }
                tremStart = nullptr;
            }
        } else {
            logger->logError(String(u"unknown tremolo type %1").arg(tremoloNr), xmlreader);
        }
    } else if (tremoloNr == 0 && (tremoloType == u"unmeasured" || tremoloType.empty() || tremoloSmufl == u"buzzRoll")) {
        // Out of all the SMuFL unmeasured tremolos, we only support 'buzzRoll'
        TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(cr));
        tremolo->setTremoloType(TremoloType::BUZZ_ROLL);
        cr->add(tremolo);
    }
}

/**
 Add the figured bass elements.
 */

void MusicXmlParserNote::addFiguredBassElements(FiguredBassList& fbl, const Fraction noteStartTime, const int msTrack,
                                                const Fraction dura, Measure* measure)
{
    if (!fbl.empty()) {
        Fraction sTick = noteStartTime;                  // starting tick
        for (FiguredBass* fb : fbl) {
            fb->setTrack(msTrack);
            // No duration tag defaults ticks() to 0; set to note value
            if (fb->ticks().isZero()) {
                fb->setTicks(dura);
            }
            // TODO: set correct onNote value
            Segment* s = measure->getSegment(SegmentType::ChordRest, sTick);
            s->add(fb);
            sTick += fb->ticks();
        }
        fbl.clear();
    }
}

/**
 set drumset information
 note that in MuseScore, the drumset contains defaults for notehead,
 line and stem direction, while a MusicXML file contains actuals.
 the MusicXML values for each note are simply copied to the defaults
 */

void MusicXmlParserNote::setDrumset(Chord* c, MusicXmlParserPass1& pass1, const String& partId, const String& instrumentId,
                                    const Fraction& noteStartTime, const MusicXmlNotePitch& mnp, const DirectionV stemDir,
                                    const NoteHeadGroup headGroup)
{
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = c->staff()->clef(noteStartTime);
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(mnp.displayStep(), 0, mnp.displayOctave());
    int line = po - absStep(pitch);

    // correct for number of staff lines
    // see ExportMusicXml::unpitch2xml for explanation
    // TODO handle other # staff lines ?
    int staffLines = c->staff()->lines(Fraction(0, 1));
    if (staffLines == 1) {
        line -= 8;
    }
    if (staffLines == 3) {
        line -= 2;
    }

    // the drum palette cannot handle stem direction AUTO,
    // overrule if necessary
    DirectionV overruledStemDir = stemDir;
    if (stemDir == DirectionV::AUTO) {
        if (line > 4) {
            overruledStemDir = DirectionV::DOWN;
        } else {
            overruledStemDir = DirectionV::UP;
        }
    }
    // this should be done in pass 1, would make _pass1 const here
    pass1.setDrumsetDefault(partId, instrumentId, headGroup, line, overruledStemDir);
}

void MusicXmlParserNote::xmlSetDrumsetPitch(Note* note, const Chord* chord, const Staff* staff, int step, int octave,
                                            NoteHeadGroup headGroup, DirectionV& stemDir, Instrument* instrument)
{
    Drumset* ds = instrument->drumset();
    // get line
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = staff->clef(chord->tick());
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(step, 0, octave);
    int line = po - absStep(pitch);
    const int staffLines = staff->lines(chord->tick());
    if (staffLines == 1) {
        line -= 8;
    }
    if (staffLines == 3) {
        line -= 2;
    }

    const int firstDrum = ds->nextPitch(0);
    int curDrum = firstDrum;
    int newPitch = pitch;
    bool matchFound = false;
    do {
        if (ds->line(curDrum) == line) {
            if (ds->noteHead(curDrum) == headGroup) {
                newPitch = curDrum;
                matchFound = true;
                break;
            }
        }
        curDrum = ds->nextPitch(curDrum);
    } while (curDrum != firstDrum);

    // Find inferred instruments at this tick
    if (configuration()->inferTextType()) {
        InferredPercInstr instr = m_pass2.inferredPercInstr(chord->tick(), chord->track());
        if (instr.track != muse::nidx) {
            // Clear old instrument
            ds->drum(newPitch) = DrumInstrument();

            newPitch = instr.pitch;
            ds->drum(newPitch) = ds->drum(newPitch) = DrumInstrument(
                instr.name.toStdString().c_str(), headGroup, line, stemDir, static_cast<int>(chord->voice()));
        }
    }

    // If there is no exact match add an entry to the drumkit with the XML line and notehead
    if (!matchFound) {
        // Create new instrument in drumkit
        if (stemDir == DirectionV::AUTO) {
            if (line > 4) {
                stemDir = DirectionV::DOWN;
            } else {
                stemDir = DirectionV::UP;
            }
        }

        ds->drum(newPitch) = DrumInstrument("drum", headGroup, line, stemDir, static_cast<int>(chord->voice()));
    } else if (stemDir == DirectionV::AUTO) {
        stemDir = ds->stemDirection(newPitch);
    }

    note->setPitch(newPitch);
    note->setTpcFromPitch();
}

//---------------------------------------------------------
//   notePrintSpacingNo
//---------------------------------------------------------

/**
 Parse the /score-partwise/part/measure/note node for a note with print-spacing="no".
 These are handled like a forward: only moving the time forward.
 */

void MusicXmlParserNote::notePrintSpacingNo(Fraction& dura)
{
    //_logger->logDebugTrace("MusicXmlParserPass1::notePrintSpacingNo", &_e);

    bool chord = false;
    bool grace = false;

    while (m_e.readNextStartElement()) {
        if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "duration") {
            m_pass2.duration(dura);
        } else if (m_e.name() == "grace") {
            grace = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else {
            m_e.skipCurrentElement();              // skip but don't log
        }
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        dura.set(0, 1);
    }

    addError(checkAtEndElement(m_e, u"note"));
}

void MusicXmlParserNote::addError(const String& error)
{
    if (!error.empty()) {
        m_logger->logError(error, &m_e);
        m_errors += errorStringWithLocation(m_e.lineNumber(), m_e.columnNumber(), error) + u'\n';
    }
}

void MusicXmlParserNote::skipLogCurrElem()
{
    //_logger->logDebugInfo(String("skipping '%1'").arg(_e.name().toString()), &_e);
    m_e.skipCurrentElement();
}

Note* MusicXmlParserNote::parse()
{
    if (m_e.asciiAttribute("print-spacing") == "no") {
        notePrintSpacingNo(m_dura);
        return 0;
    }

    bool chord = false;
    bool cue = false;
    bool isSmall = false;
    bool grace = false;
    bool rest = false;
    bool measureRest = false;
    int staff = 0;
    String type;
    String voice;
    DirectionV stemDir = DirectionV::AUTO;
    bool noStem = false;
    bool hasHead = true;
    NoteHeadGroup headGroup = NoteHeadGroup::HEAD_NORMAL;
    NoteHeadScheme headScheme = NoteHeadScheme::HEAD_AUTO;
    const Color noteColor = Color::fromString(m_e.asciiAttribute("color").ascii());
    Color noteheadColor;
    Color stemColor;
    bool noteheadParentheses = false;
    String noteheadFilled;
    int velocity = round(m_e.doubleAttribute("dynamics") * 0.9);
    bool graceSlash = false;
    bool printObject = m_e.asciiAttribute("print-object") != "no";
    bool isSingleDrumset = false;
    BeamMode bm;
    std::map<int, String> beamTypes;
    String instrumentId;
    String tieType;
    MusicXmlParserLyric lyric { m_pass1.getMusicXmlPart(m_partId).lyricNumberHandler(), m_e, m_score, m_logger,
                                m_pass1.isVocalStaff(m_partId) };
    MusicXmlParserNotations notations { m_e, m_score, m_logger, m_pass2 };

    MusicXmlNoteDuration mnd { m_pass2.divs(), m_logger, &m_pass1 };
    MusicXmlNotePitch mnp { m_logger };

    while (m_e.readNextStartElement()) {
        if (mnp.readProperties(m_e, m_score)) {
            // element handled
        } else if (mnd.readProperties(m_e)) {
            // element handled
        } else if (m_e.name() == "beam") {
            beam(beamTypes);
        } else if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "cue") {
            cue = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "grace") {
            grace = true;
            graceSlash = m_e.asciiAttribute("slash") == "yes";
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "instrument") {
            instrumentId = m_e.attribute("id");
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "lyric") {
            // lyrics on grace notes not (yet) supported by MuseScore
            // add to main note instead
            lyric.parse();
        } else if (m_e.name() == "notations") {
            notations.parse();
            addError(notations.errors());
        } else if (m_e.name() == "notehead") {
            noteheadColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            noteheadParentheses = m_e.asciiAttribute("parentheses") == "yes";
            noteheadFilled = m_e.attribute("filled");
            String noteheadValue = m_e.readText();
            if (noteheadValue == "none") {
                hasHead = false;
            } else if (noteheadValue == "named" && m_pass1.exporterSoftware() == MusicXmlExporterSoftware::NOTEFLIGHT) {
                headScheme = NoteHeadScheme::HEAD_PITCHNAME;
            } else {
                headGroup = convertNotehead(noteheadValue);
            }
        } else if (m_e.name() == "rest") {
            rest = true;
            measureRest = m_e.asciiAttribute("measure") == "yes";
            mnp.displayStepOctave(m_e);
        } else if (m_e.name() == "staff") {
            bool ok = false;
            String strStaff = m_e.readText();
            staff = m_pass1.getMusicXmlPart(m_partId).staffNumberToIndex(strStaff.toInt(&ok));
            if (!ok) {
                // error already reported in pass 1
                staff = -1;
            }
        } else if (m_e.name() == "stem") {
            stemColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            stem(stemDir, noStem);
        } else if (m_e.name() == "tie") {
            tieType = m_e.attribute("type");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "type") {
            isSmall = m_e.asciiAttribute("size") == "cue" || m_e.asciiAttribute("size") == "grace-cue";
            type = m_e.readText();
        } else if (m_e.name() == "voice") {
            voice = m_e.readText();
        } else {
            skipLogCurrElem();
        }
    }

    // Bug fix for Sibelius 7.1.3 which does not write <voice> for notes with <chord>
    if (!chord) {
        // remember voice
        m_currentVoice = voice;
    } else if (voice.empty()) {
        // use voice from last note w/o <chord>
        voice = m_currentVoice;
    }

    // Assume voice 1 if voice is empty (legal in a single voice part)
    if (voice.empty()) {
        voice = u"1";
    }

    // Define currBeam based on currentVoice to handle multi-voice beaming (and instantiate if not already)
    if (!muse::contains(m_currBeams, m_currentVoice)) {
        m_currBeams.insert({ m_currentVoice, (Beam*)nullptr });
    }
    Beam*& currBeam = m_currBeams[m_currentVoice];

    bm = computeBeamMode(beamTypes);

    // check for timing error(s) and set dura
    // keep in this order as checkTiming() might change dura
    String errorStr = mnd.checkTiming(type, rest, grace);
    m_dura = mnd.duration();
    if (!errorStr.empty()) {
        m_logger->logError(errorStr, &m_e);
    }

    IF_ASSERT_FAILED(m_pass1.getPart(m_partId)) {
        return nullptr;
    }

    // At this point all checks have been done, the note should be added
    // note: in case of error exit from here, the postponed <note> children
    // must still be skipped

    int msMove = 0;
    int msTrack = 0;
    int msVoice = 0;

    int voiceInt = m_pass1.voiceToInt(voice);
    if (!m_pass1.determineStaffMoveVoice(m_partId, staff, voiceInt, msMove, msTrack, msVoice)) {
        m_logger->logDebugInfo(String(u"could not map staff %1 voice '%2'").arg(staff + 1).arg(voice), &m_e);
        addError(checkAtEndElement(m_e, u"note"));
        return 0;
    }

    // start time for note:
    // - sTime for non-chord / first chord note
    // - prevTime for others
    Fraction noteStartTime = chord ? m_prevSTime : m_sTime;
    Fraction timeMod = mnd.timeMod();

    // determine tuplet state, used twice (before and after note allocation)
    MusicXmlTupletFlags tupletAction;

    // handle tuplet state for the previous chord or rest
    if (!chord && !grace) {
        Tuplet* tuplet = m_tuplets[voice];
        MusicXmlTupletState& m_tupletState = m_tupletStates[voice];
        tupletAction = m_tupletState.determineTupletAction(mnd.duration(), timeMod, notations.tupletDesc().type,
                                                           mnd.normalType(), m_missingPrev, m_missingCurr);
        if (tupletAction & MusicXmlTupletFlag::STOP_PREVIOUS) {
            // tuplet start while already in tuplet
            if (m_missingPrev.isValid() && m_missingPrev > Fraction(0, 1)) {
                const int track = msTrack + msVoice;
                Rest* const extraRest = addRest(m_score, m_measure, noteStartTime, track, msMove,
                                                TDuration { m_missingPrev* tuplet->ratio() }, m_missingPrev);
                if (extraRest) {
                    extraRest->setTuplet(tuplet);
                    tuplet->add(extraRest);
                    noteStartTime += m_missingPrev;
                }
            }
            // recover by simply stopping the current tuplet first
            const int normalNotes = timeMod.numerator();
            handleTupletStop(tuplet, normalNotes);
        }
    }

    Chord* c { nullptr };
    ChordRest* cr { nullptr };
    Note* note { nullptr };

    TDuration duration = determineDuration(rest, measureRest, type, mnd.dots(), m_dura, m_measure->ticks());

    Part* part = m_pass1.getPart(m_partId);
    Instrument* instrument = part->instrument(noteStartTime);
    const MusicXmlInstruments& instruments = m_pass1.getInstruments(m_partId);
    isSingleDrumset = instrument->drumset() && instruments.size() == 1;
    // begin allocation
    if (rest) {
        const int track = msTrack + msVoice;
        cr = addRest(m_score, m_measure, noteStartTime, track, msMove,
                     duration, m_dura);
    } else {
        if (!grace) {
            // regular note
            // if there is already a chord just add to it
            // else create a new one
            // this basically ignores <chord/> errors
            c = findOrCreateChord(m_score, m_measure,
                                  noteStartTime,
                                  msTrack + msVoice, msMove,
                                  duration, m_dura, bm, isSmall || cue);
        } else {
            // grace note
            // TODO: check if explicit stem direction should also be set for grace notes
            // (the DOM parser does that, but seems to have no effect on the autotester)
            if (!chord || m_gcl.empty()) {
                c = createGraceChord(m_score, msTrack + msVoice, duration, graceSlash, isSmall || cue);
                // TODO FIX
                // the setStaffMove() below results in identical behaviour as 2.0:
                // grace note will be at the wrong staff with the wrong pitch,
                // seems to use the line value calculated for the right staff
                // leaving it places the note at the wrong staff with the right pitch
                // this affects only grace notes where staff move differs from
                // the main note, e.g. DebuMandSample.xml first grace in part 2
                // c->setStaffMove(msMove);
                // END TODO
                m_gcl.push_back(c);
            } else {
                c = m_gcl.back();
            }
        }
        note = Factory::createNote(c);
        const staff_idx_t ottavaStaff = (msTrack - m_pass1.trackForPart(m_partId)) / VOICES;
        const int octaveShift = m_pass1.octaveShift(m_partId, ottavaStaff, noteStartTime);
        const Staff* st = c->staff();
        if (isSingleDrumset && mnp.unpitched() && instrumentId.empty()) {
            xmlSetDrumsetPitch(note, c, st, mnp.displayStep(), mnp.displayOctave(), headGroup, stemDir, instrument);
        } else {
            setPitch(note, instruments, instrumentId, mnp, octaveShift, instrument);
        }
        c->add(note);
        cr = c;
    }
    // end allocation

    if (rest) {
        const track_idx_t track = msTrack + msVoice;
        if (cr) {
            if (currBeam) {
                if (currBeam->track() == track) {
                    cr->setBeamMode(BeamMode::MID);
                    currBeam->add(cr);
                } else {
                    removeBeam(currBeam);
                }
            } else {
                cr->setBeamMode(BeamMode::NONE);
            }
            cr->setSmall(isSmall);
            if (noteColor.isValid()) {
                cr->setColor(noteColor);
            }
            cr->setVisible(printObject);
            handleDisplayStep(cr, mnp.displayStep(), mnp.displayOctave(), noteStartTime, m_score->style().spatium());
        }
    } else {
        handleSmallness(cue || isSmall, note, c);
        note->setPlay(!cue);          // cue notes don't play
        note->setHeadGroup(headGroup);
        if (headScheme != NoteHeadScheme::HEAD_AUTO) {
            note->setHeadScheme(headScheme);
        }
        if (noteColor.isValid()) {
            note->setColor(noteColor);
        }
        Stem* stem = c->stem();
        if (!stem) {
            stem = Factory::createStem(c);
            if (stemColor.isValid()) {
                stem->setColor(stemColor);
            } else if (noteColor.isValid()) {
                stem->setColor(noteColor);
            }
            c->add(stem);
        }
        setNoteHead(note, noteheadColor, noteheadParentheses, noteheadFilled);
        note->setVisible(hasHead && printObject);
        stem->setVisible(printObject);

        if (!grace) {
            // regular note
            // handle beam
            if (!chord) {
                handleBeamAndStemDir(c, bm, stemDir, currBeam, m_pass1.hasBeamingInfo());
            }

            // append any grace chord after chord to the previous chord
            Chord* const prevChord = m_measure->findChord(m_prevSTime, msTrack + msVoice);
            if (prevChord && prevChord != c) {
                addGraceChordsAfter(prevChord, m_gcl, m_gac);
            }

            // append any grace chord
            addGraceChordsBefore(c, m_gcl);
        }

        if (mnd.calculatedDuration().isValid()
            && mnd.specifiedDuration().isValid()
            && mnd.calculatedDuration().isNotZero()
            && mnd.calculatedDuration() != mnd.specifiedDuration()) {
            // convert duration into note length
            Fraction durationMult { (mnd.specifiedDuration() / mnd.calculatedDuration()).reduced() };
            durationMult = (1000 * durationMult).reduced();
            const int noteLen = durationMult.numerator() / durationMult.denominator();

            NoteEventList nel;
            NoteEvent ne;
            ne.setLen(noteLen);
            nel.push_back(ne);
            note->setPlayEvents(nel);
            if (c) {
                c->setPlayEventType(PlayEventType::User);
            }
        }

        if (velocity > 0) {
            note->setUserVelocity(velocity);
        }

        if (mnp.unpitched() && !isSingleDrumset) {
            setDrumset(c, m_pass1, m_partId, instrumentId, noteStartTime, mnp, stemDir, headGroup);
        }

        // accidental handling
        //LOGD("note acc %p type %hhd acctype %hhd",
        //       acc, acc ? acc->accidentalType() : static_cast<mu::engraving::AccidentalType>(0), accType);
        Accidental* acc = mnp.acc();
        if (!acc && mnp.accType() != AccidentalType::NONE) {
            acc = Factory::createAccidental(m_score->dummy());
            acc->setAccidentalType(mnp.accType());
        }

        if (acc) {
            acc->setVisible(printObject);
            note->add(acc);
            // save alter value for user accidental
            if (acc->accidentalType() != AccidentalType::NONE) {
                m_alt = mnp.alter();
            }
        }

        c->setNoStem(noStem);
    }

    // cr can be 0 here (if a rest cannot be added)
    // TODO: complete and cleanup handling this case
    if (cr) {
        cr->setVisible(printObject);
    }

    // handle notations
    if (cr) {
        notations.addToScore(cr, note,
                             noteStartTime.ticks(), m_pass2.slurs(), m_pass2.glissandi(), m_pass2.spanners(), m_pass2.trills(),
                             m_pass2.ties(), m_pass2.unstartedTieNotes(), m_pass2.unendedTieNotes(), m_arpMap,
                             m_delayedArps);

        // if no tie added yet, convert the "tie" into "tied" and add it.
        if (note && !note->tieFor() && !note->tieBack() && !tieType.empty()) {
            Notation notation = Notation(u"tied");
            const String type2 = u"type";
            notation.addAttribute(type2, tieType);
            addTie(notation, note, cr->track(), m_pass2.ties(), m_pass2.unstartedTieNotes(), m_pass2.unendedTieNotes(), m_logger, &m_e);
        }
    }

    // handle grace after state: remember current grace list size
    if (grace && notations.mustStopGraceAFter()) {
        m_gac = m_gcl.size();
    }

    // handle tremolo before handling tuplet (two note tremolos modify timeMod)
    if (cr && notations.hasTremolo()) {
        addTremolo(cr, notations.tremoloNr(), notations.tremoloType(), notations.tremoloSmufl(),
                   m_pass2.tremStart(), m_logger, &m_e, timeMod);
    }

    // handle tuplet state for the current chord or rest
    if (cr) {
        if (!chord && !grace) {
            Tuplet*& tuplet = m_tuplets[voice];
            // do tuplet if valid time-modification is not 1/1 and is not 1/2 (tremolo)
            // TODO: check interaction tuplet and tremolo handling
            if (timeMod.isValid() && timeMod != Fraction(1, 1) && timeMod != Fraction(1, 2)) {
                const int actualNotes = timeMod.denominator();
                const int normalNotes = timeMod.numerator();
                if (tupletAction & MusicXmlTupletFlag::START_NEW) {
                    // create a new tuplet
                    handleTupletStart(cr, tuplet, actualNotes, normalNotes, notations.tupletDesc());
                }
                if (tupletAction & MusicXmlTupletFlag::ADD_CHORD) {
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);
                }
                if (tupletAction & MusicXmlTupletFlag::STOP_CURRENT) {
                    if (m_missingCurr.isValid() && m_missingCurr > Fraction(0, 1)) {
                        LOGD("add missing %s to current tuplet", muPrintable(m_missingCurr.toString()));
                        const int track = msTrack + msVoice;
                        Rest* const extraRest = addRest(m_score, m_measure, noteStartTime + m_dura, track, msMove,
                                                        TDuration { m_missingCurr* tuplet->ratio() }, m_missingCurr);
                        if (extraRest) {
                            extraRest->setTuplet(tuplet);
                            tuplet->add(extraRest);
                        }
                    }
                    handleTupletStop(tuplet, normalNotes);
                }
            } else if (tuplet) {
                // stop any still incomplete tuplet
                handleTupletStop(tuplet, 2);
            }
        }
    }

    // Add all lyrics from grace notes attached to this chord
    if (c && !c->graceNotes().empty() && !m_pass2.graceNoteLyrics().empty()) {
        for (GraceNoteLyrics gnl : m_pass2.graceNoteLyrics()) {
            if (gnl.lyric) {
                addLyric(m_logger, &m_e, cr, gnl.lyric, gnl.no, m_pass2.extendedLyrics());
                if (gnl.extend) {
                    m_pass2.extendedLyrics().addLyric(gnl.lyric);
                }
            }
        }
        m_pass2.graceNoteLyrics().clear();
    }

    if (cr) {
        addInferredStickings(cr, lyric.inferredStickings());
    }

    // add lyrics found by lyric
    if (cr && !grace) {
        // add lyrics and stop corresponding extends
        addLyrics(m_logger, &m_e, cr, lyric.numberedLyrics(), lyric.extendedLyrics(), m_pass2.extendedLyrics());
        if (rest) {
            // stop all extends
            m_pass2.extendedLyrics().setExtend(-1, cr->track(), cr->tick(), nullptr);
        }
    } else if (c && grace) {
        // Add grace note lyrics to main chord later
        addGraceNoteLyrics(lyric.numberedLyrics(), lyric.extendedLyrics(), m_pass2.graceNoteLyrics());
    }

    // add figured bass element
    addFiguredBassElements(m_fbl, noteStartTime, msTrack, m_dura, m_measure);

    // convert to slash or rhythmic notation if needed
    // TODO in the case of slash notation, we assume that given notes do in fact correspond to slash beats
    if (c && m_pass2.measureStyleSlash() != MusicXmlSlash::NONE) {
        c->setSlash(true, m_pass2.measureStyleSlash() == MusicXmlSlash::SLASH);
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        m_dura.set(0, 1);
    }

    addError(checkAtEndElement(m_e, u"note"));

    return note;
}
