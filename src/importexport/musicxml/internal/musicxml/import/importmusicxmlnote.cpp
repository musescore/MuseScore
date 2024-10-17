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
#include "dom/lyrics.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/stem.h"
#include "dom/sticking.h"
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
    m_delayedArps(delayedArps), m_lyricParser(m_pass1.getMusicXmlPart(m_partId).lyricNumberHandler(), m_e, m_score, m_logger,
                    m_pass1.isVocalStaff(m_partId)), m_notationsParser(m_e, m_score, m_logger, m_pass2),
    m_noteDuration(m_pass2.divs(), m_logger, &m_pass1), m_notePitch(m_logger)
{
}

/**
 Parse the /score-partwise/part/measure/note/beam node.
 Collects beamTypes, used in computeBeamMode.
 */

void MusicXmlParserNote::beam()
{
    bool hasBeamNo;
    int beamNo = m_e.asciiAttribute("number").toInt(&hasBeamNo);
    String s = m_e.readText();

    m_beamTypes.insert({ hasBeamNo ? beamNo : 1, s });
}

/**
 Calculate the beam mode based on the collected beamTypes.
 */

void MusicXmlParserNote::computeBeamMode()
{
    // Start with uniquely-handled beam modes
    if (muse::value(m_beamTypes, 1) == u"continue"
        && muse::value(m_beamTypes, 2) == u"begin") {
        m_beamMode = BeamMode::BEGIN16;
    } else if (muse::value(m_beamTypes, 1) == u"continue"
               && muse::value(m_beamTypes, 2) == u"continue"
               && muse::value(m_beamTypes, 3) == u"begin") {
        m_beamMode = BeamMode::BEGIN32;
    }
    // Generic beam modes are naive to all except the first beam
    else if (muse::value(m_beamTypes, 1) == u"begin") {
        m_beamMode = BeamMode::BEGIN;
    } else if (muse::value(m_beamTypes, 1) == u"continue") {
        m_beamMode = BeamMode::MID;
    } else if (muse::value(m_beamTypes, 1) == u"end") {
        m_beamMode = BeamMode::END;
    } else {
        // backward-hook, forward-hook, and other unknown combinations
        m_beamMode = BeamMode::AUTO;
    }
}

void MusicXmlParserNote::handleBeamAndStemDir(Beam*& beam)
{
    if (!m_chord) {
        return;
    }
    // create a new beam
    if (m_beamMode == BeamMode::BEGIN) {
        // if currently in a beam, delete it
        if (beam) {
            LOGD("handleBeamAndStemDir() new beam, removing previous incomplete beam %p", beam);
            removeBeam(beam);
        }
        // create a new beam
        beam = Factory::createBeam(m_chord->score()->dummy()->system());
        beam->setTrack(m_chord->track());
        beam->setDirection(m_stemDir);
    }
    // add Chord to beam
    if (beam) {
        // verify still in the same track (if still in the same voice)
        // and in a beam ...
        // (note no check is done on correct order of beam begin/continue/end)
        // TODO: Some BEGINs are being skipped
        if (m_chord->track() != beam->track()) {
            LOGD("handleBeamAndStemDir() from track %zu to track %zu -> abort beam",
                 beam->track(), m_chord->track());
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (m_beamMode == BeamMode::NONE) {
            LOGD("handleBeamAndStemDir() in beam, bm BeamMode::NONE -> abort beam");
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else if (!(m_beamMode == BeamMode::BEGIN || m_beamMode == BeamMode::MID || m_beamMode == BeamMode::END
                     || m_beamMode == BeamMode::BEGIN16
                     || m_beamMode == BeamMode::BEGIN32)) {
            LOGD("handleBeamAndStemDir() in beam, bm %d -> abort beam", static_cast<int>(m_beamMode));
            // reset beam mode for all elements and remove the beam
            removeBeam(beam);
        } else {
            // actually add cr to the beam
            beam->add(m_chord);
        }
    }
    // if no beam, set stem direction on chord itself
    if (!beam) {
        static_cast<Chord*>(m_chord)->setStemDirection(m_stemDir);
        // set beam to none if score has beaming information and note can get beam, otherwise
        // set to auto
        bool canGetBeam = (m_chord->durationType().type() >= DurationType::V_EIGHTH
                           && m_chord->durationType().type() <= DurationType::V_1024TH);
        if (m_pass1.hasBeamingInfo() && canGetBeam) {
            m_chord->setBeamMode(BeamMode::NONE);
        } else {
            m_chord->setBeamMode(BeamMode::AUTO);
        }
    }
    // terminate the current beam and add to the score
    if (beam && m_beamMode == BeamMode::END) {
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

void MusicXmlParserNote::stem()
{
    String s = m_e.readText();

    if (s == u"up") {
        m_stemDir = DirectionV::UP;
    } else if (s == u"down") {
        m_stemDir = DirectionV::DOWN;
    } else if (s == u"none") {
        m_noStem = true;
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

Chord* MusicXmlParserNote::findOrCreateChord(const TDuration duration) const
{
    //LOGD("findOrCreateChord tick %d track %d dur ticks %d ticks %s bm %hhd",
    //       tick, track, duration.ticks(), muPrintable(dura.print()), m_beamMode);
    Chord* c = m_measure->findChord(m_noteStartTime, track());
    if (c == 0) {
        Segment* s = m_measure->getSegment(SegmentType::ChordRest, m_noteStartTime);
        c = Factory::createChord(s);
        // better not to force beam end, as the beam palette does not support it
        if (m_beamMode == BeamMode::END) {
            c->setBeamMode(BeamMode::AUTO);
        } else {
            c->setBeamMode(m_beamMode);
        }
        c->setTrack(track());
        // Chord is initialized with the smallness of its first note.
        // If a non-small note is added later, this is handled in handleSmallness.
        c->setSmall(isSmall());

        setChordRestDuration(c, duration, m_dura);
        s->add(c);
    }
    c->setStaffMove(m_staffMove);
    return c;
}

/**
 * convert duration and slash to grace note type
 */

NoteType MusicXmlParserNote::graceNoteType(const TDuration duration) const
{
    NoteType nt = NoteType::APPOGGIATURA;
    if (m_graceSlash) {
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

Chord* MusicXmlParserNote::createGraceChord(const TDuration duration) const
{
    Chord* c = Factory::createChord(m_score->dummy()->segment());
    c->setNoteType(graceNoteType(duration));
    c->setTrack(track());
    // Chord is initialized with the smallness of its first note.
    // If a non-small note is added later, this is handled in handleSmallness.
    c->setSmall(isSmall());
    // note grace notes have no durations, use default fraction 0/1
    setChordRestDuration(c, duration, Fraction());
    return c;
}

// TODO: refactor: optimize parameters

void MusicXmlParserNote::setPitch(const MusicXmlInstruments& instruments, const int octaveShift, const Instrument* const instrument)
{
    if (m_notePitch.unpitched()) {
        if (hasDrumset(instruments) && muse::contains(instruments, m_instrumentId)) {
            // step and oct are display-step and ...-oct
            // get pitch from instrument definition in drumset instead
            int unpitched = instruments.at(m_instrumentId).unpitched;
            m_note->setPitch(std::clamp(unpitched, 0, 127));
            // TODO - does this need to be key-aware?
            m_note->setTpc(pitch2tpc(unpitched, Key::C, Prefer::NEAREST));             // TODO: necessary ?
        } else {
            //LOGD("disp step %d oct %d", displayStep, displayOctave);
            xmlSetPitch(m_note, m_notePitch.displayStep(), 0, 0.0, m_notePitch.displayOctave(), 0, instrument);
        }
    } else {
        xmlSetPitch(m_note, m_notePitch.step(), m_notePitch.alter(), m_notePitch.tuning(), m_notePitch.octave(), octaveShift, instrument);
    }
}

/**
 * convert display-step and display-octave to staff line
 */

void MusicXmlParserNote::handleDisplayStep()
{
    const int step = m_notePitch.displayStep();
    const int octave = m_notePitch.displayOctave();
    if (0 <= step && step <= 6 && 0 <= octave && octave <= 9) {
        //LOGD("rest step=%d oct=%d", step, octave);
        ClefType clef = m_chordRest->staff()->clef(m_noteStartTime);
        int po = ClefInfo::pitchOffset(clef);
        //LOGD(" clef=%hhd po=%d step=%d", clef, po, step);
        int dp = 7 * (octave + 2) + step;
        //LOGD(" dp=%d po-dp=%d", dp, po-dp);
        m_chordRest->ryoffset() = (po - dp + 3) * m_score->style().spatium() / 2;
    }
}

void MusicXmlParserNote::handleSmallness()
{
    if (isSmall()) {
        m_note->setSmall(!m_chord->isSmall()); // Avoid redundant smallness
    } else {
        m_note->setSmall(false);
        if (m_chord->isSmall()) {
            // What was a small chord becomes small notes in a non-small chord
            m_chord->setSmall(false);
            for (Note* otherNote : m_chord->notes()) {
                if (m_note != otherNote) {
                    otherNote->setSmall(true);
                }
            }
        }
    }
}

/**
 Set the notehead parameters.
 */

void MusicXmlParserNote::setNoteHead()
{
    Score* const score = m_note->score();

    if (m_noteheadColor.isValid()) {
        m_note->setColor(m_noteheadColor);
    }
    if (m_noteheadParentheses) {
        Symbol* s = new Symbol(m_note);
        s->setSym(SymId::noteheadParenthesisLeft);
        s->setParent(m_note);
        score->addElement(s);
        s = new Symbol(m_note);
        s->setSym(SymId::noteheadParenthesisRight);
        s->setParent(m_note);
        score->addElement(s);
    }

    if (m_noteheadFilled == u"no") {
        m_note->setHeadType(NoteHeadType::HEAD_HALF);
    } else if (m_noteheadFilled == u"yes") {
        m_note->setHeadType(NoteHeadType::HEAD_QUARTER);
    }
}

void MusicXmlParserNote::addTremolo(Chord*& tremStart)
{
    if (!m_chordRest->isChord()) {
        return;
    }
    const int tremoloNr = m_notationsParser.tremoloNr();
    const String& tremoloType = m_notationsParser.tremoloType();
    const String& tremoloSmufl = m_notationsParser.tremoloSmufl();
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
                    TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(m_chordRest));
                    tremolo->setTremoloType(type);
                    m_chordRest->add(tremolo);
                }
            } else if (tremoloType == u"start") {
                if (tremStart) {
                    m_logger->logError(u"MusicXml::import: double tremolo start", &m_e);
                }
                tremStart = static_cast<Chord*>(m_chordRest);
                // timeMod takes into account also the factor 2 of a two-note tremolo
                if (m_timeMod.isValid() && ((m_timeMod.denominator() % 2) == 0)) {
                    m_timeMod.setDenominator(m_timeMod.denominator() / 2);
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
                        TremoloTwoChord* tremolo = Factory::createTremoloTwoChord(mu::engraving::toChord(m_chordRest));
                        tremolo->setTremoloType(type);
                        tremolo->setChords(tremStart, static_cast<Chord*>(m_chordRest));
                        // fixup chord duration and type
                        const Fraction tremDur = m_chordRest->ticks() * Fraction(1, 2);
                        tremolo->chord1()->setDurationType(tremDur);
                        tremolo->chord1()->setTicks(tremDur);
                        tremolo->chord2()->setDurationType(tremDur);
                        tremolo->chord2()->setTicks(tremDur);
                        // add tremolo to first chord (only)
                        tremStart->add(tremolo);
                    }
                    // timeMod takes into account also the factor 2 of a two-note tremolo
                    if (m_timeMod.isValid() && ((m_timeMod.denominator() % 2) == 0)) {
                        m_timeMod.setDenominator(m_timeMod.denominator() / 2);
                    }
                } else {
                    m_logger->logError(u"MusicXml::import: double tremolo stop w/o start", &m_e);
                }
                tremStart = nullptr;
            }
        } else {
            m_logger->logError(String(u"unknown tremolo type %1").arg(tremoloNr), &m_e);
        }
    } else if (tremoloNr == 0 && (tremoloType == u"unmeasured" || tremoloType.empty() || tremoloSmufl == u"buzzRoll")) {
        // Out of all the SMuFL unmeasured tremolos, we only support 'buzzRoll'
        TremoloSingleChord* tremolo = Factory::createTremoloSingleChord(mu::engraving::toChord(m_chordRest));
        tremolo->setTremoloType(TremoloType::BUZZ_ROLL);
        m_chordRest->add(tremolo);
    }
}

/**
 Add the figured bass elements.
 */

void MusicXmlParserNote::addFiguredBassElements(const Fraction dura)
{
    if (!m_fbl.empty()) {
        Fraction sTick = m_noteStartTime;                  // starting tick
        for (FiguredBass* fb : m_fbl) {
            fb->setTrack(m_track);
            // No duration tag defaults ticks() to 0; set to note value
            if (fb->ticks().isZero()) {
                fb->setTicks(dura);
            }
            // TODO: set correct onNote value
            Segment* s = m_measure->getSegment(SegmentType::ChordRest, sTick);
            s->add(fb);
            sTick += fb->ticks();
        }
        m_fbl.clear();
    }
}

/**
 set drumset information
 note that in MuseScore, the drumset contains defaults for notehead,
 line and stem direction, while a MusicXML file contains actuals.
 the MusicXML values for each note are simply copied to the defaults
 */

void MusicXmlParserNote::setDrumset() const
{
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = m_chord->staff()->clef(m_noteStartTime);
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(m_notePitch.displayStep(), 0, m_notePitch.displayOctave());
    int line = po - absStep(pitch);

    // correct for number of staff lines
    // see ExportMusicXml::unpitch2xml for explanation
    // TODO handle other # staff lines ?
    int staffLines = m_chord->staff()->lines(Fraction(0, 1));
    if (staffLines == 1) {
        line -= 8;
    }
    if (staffLines == 3) {
        line -= 2;
    }

    // the drum palette cannot handle stem direction AUTO,
    // overrule if necessary
    DirectionV overruledStemDir = m_stemDir;
    if (m_stemDir == DirectionV::AUTO) {
        if (line > 4) {
            overruledStemDir = DirectionV::DOWN;
        } else {
            overruledStemDir = DirectionV::UP;
        }
    }
    // this should be done in pass 1, would make _pass1 const here
    m_pass1.setDrumsetDefault(m_partId, m_instrumentId, m_headGroup, line, overruledStemDir);
}

void MusicXmlParserNote::xmlSetDrumsetPitch(const Staff* staff, Instrument* instrument)
{
    const int step = m_notePitch.displayStep();
    const int octave = m_notePitch.displayOctave();
    Drumset* ds = instrument->drumset();
    // get line
    // determine staff line based on display-step / -octave and clef type
    const ClefType clef = staff->clef(m_chord->tick());
    const int po = ClefInfo::pitchOffset(clef);
    const int pitch = MusicXmlStepAltOct2Pitch(step, 0, octave);
    int line = po - absStep(pitch);
    const int staffLines = staff->lines(m_chord->tick());
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
            if (ds->noteHead(curDrum) == m_headGroup) {
                newPitch = curDrum;
                matchFound = true;
                break;
            }
        }
        curDrum = ds->nextPitch(curDrum);
    } while (curDrum != firstDrum);

    // Find inferred instruments at this tick
    if (configuration()->inferTextType()) {
        InferredPercInstr instr = m_pass2.inferredPercInstr(m_chord->tick(), m_chord->track());
        if (instr.track != muse::nidx) {
            // Clear old instrument
            ds->drum(newPitch) = DrumInstrument();

            newPitch = instr.pitch;
            ds->drum(newPitch) = ds->drum(newPitch) = DrumInstrument(
                instr.name.toStdString().c_str(), m_headGroup, line, m_stemDir, static_cast<int>(m_chord->voice()));
        }
    }

    // If there is no exact match add an entry to the drumkit with the XML line and notehead
    if (!matchFound) {
        // Create new instrument in drumkit
        if (m_stemDir == DirectionV::AUTO) {
            if (line > 4) {
                m_stemDir = DirectionV::DOWN;
            } else {
                m_stemDir = DirectionV::UP;
            }
        }

        ds->drum(newPitch) = DrumInstrument("drum", m_headGroup, line, m_stemDir, static_cast<int>(m_chord->voice()));
    } else if (m_stemDir == DirectionV::AUTO) {
        m_stemDir = ds->stemDirection(newPitch);
    }

    m_note->setPitch(newPitch);
    m_note->setTpcFromPitch();
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

/**
 Add a single lyric to the score or delete it (if number too high)
 */

void MusicXmlParserNote::addLyric(Lyrics* l, int lyricNo)
{
    if (lyricNo > MAX_LYRICS) {
        m_logger->logError(String(u"too much lyrics (>%1)")
                               .arg(MAX_LYRICS), &m_e);
        delete l;
    } else {
        l->setNo(lyricNo);
        m_chordRest->add(l);
        m_pass2.extendedLyrics().setExtend(lyricNo, m_chordRest->track(), m_chordRest->tick(), l);
    }
}

void MusicXmlParserNote::addLyrics()
{
    for (const int lyricNo : muse::keys(m_lyricParser.numberedLyrics())) {
        Lyrics* const lyric = m_lyricParser.numberedLyrics().at(lyricNo);
        addLyric(lyric, lyricNo);
        if (muse::contains(m_lyricParser.extendedLyrics(), lyric)) {
            m_pass2.extendedLyrics().addLyric(lyric);
        }
    }
}

void MusicXmlParserNote::addGraceNoteLyrics()
{
    for (const int lyricNo : muse::keys(m_lyricParser.numberedLyrics())) {
        Lyrics* const lyric = m_lyricParser.numberedLyrics().at(lyricNo);
        if (lyric) {
            bool extend = muse::contains(m_lyricParser.extendedLyrics(), lyric);
            const GraceNoteLyrics gnl = GraceNoteLyrics(lyric, extend, lyricNo);
            m_pass2.graceNoteLyrics().push_back(gnl);
        }
    }
}

void MusicXmlParserNote::addInferredStickings()const
{
    for (Sticking* sticking : m_lyricParser.inferredStickings()) {
        sticking->setParent(m_chordRest->segment());
        sticking->setTrack(m_chordRest->track());
        m_chordRest->score()->addElement(sticking);
    }
}

Note* MusicXmlParserNote::parse()
{
    if (m_e.asciiAttribute("print-spacing") == "no") {
        notePrintSpacingNo(m_dura);
        return 0;
    }

    bool chord = false;
    bool grace = false;
    bool rest = false;
    bool measureRest = false;
    int staff = 0;
    String type;
    String voice;
    bool hasHead = true;
    NoteHeadScheme headScheme = NoteHeadScheme::HEAD_AUTO;
    const Color noteColor = Color::fromString(m_e.asciiAttribute("color").ascii());
    Color stemColor;
    int velocity = round(m_e.doubleAttribute("dynamics") * 0.9);
    bool printObject = m_e.asciiAttribute("print-object") != "no";
    bool isSingleDrumset = false;
    String tieType;

    while (m_e.readNextStartElement()) {
        if (m_notePitch.readProperties(m_e, m_score)) {
            // element handled
        } else if (m_noteDuration.readProperties(m_e)) {
            // element handled
        } else if (m_e.name() == "beam") {
            beam();
        } else if (m_e.name() == "chord") {
            chord = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "cue") {
            m_cue = true;
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "grace") {
            grace = true;
            m_graceSlash = m_e.asciiAttribute("slash") == "yes";
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "instrument") {
            m_instrumentId = m_e.attribute("id");
            m_e.skipCurrentElement();  // skip but don't log
        } else if (m_e.name() == "lyric") {
            // lyrics on grace notes not (yet) supported by MuseScore
            // add to main note instead
            m_lyricParser.parse();
        } else if (m_e.name() == "notations") {
            m_notationsParser.parse();
            addError(m_notationsParser.errors());
        } else if (m_e.name() == "notehead") {
            m_noteheadColor = Color::fromString(m_e.asciiAttribute("color").ascii());
            m_noteheadParentheses = m_e.asciiAttribute("parentheses") == "yes";
            m_noteheadFilled = m_e.attribute("filled");
            String noteheadValue = m_e.readText();
            if (noteheadValue == "none") {
                hasHead = false;
            } else if (noteheadValue == "named" && m_pass1.exporterSoftware() == MusicXmlExporterSoftware::NOTEFLIGHT) {
                headScheme = NoteHeadScheme::HEAD_PITCHNAME;
            } else {
                m_headGroup = convertNotehead(noteheadValue);
            }
        } else if (m_e.name() == "rest") {
            rest = true;
            measureRest = m_e.asciiAttribute("measure") == "yes";
            m_notePitch.displayStepOctave(m_e);
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
            stem();
        } else if (m_e.name() == "tie") {
            tieType = m_e.attribute("type");
            m_e.skipCurrentElement();
        } else if (m_e.name() == "type") {
            m_isSmall = m_e.asciiAttribute("size") == "cue" || m_e.asciiAttribute("size") == "grace-cue";
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

    computeBeamMode();

    // check for timing error(s) and set dura
    // keep in this order as checkTiming() might change dura
    String errorStr = m_noteDuration.checkTiming(type, rest, grace);
    m_dura = m_noteDuration.duration();
    if (!errorStr.empty()) {
        m_logger->logError(errorStr, &m_e);
    }

    IF_ASSERT_FAILED(m_pass1.getPart(m_partId)) {
        return nullptr;
    }

    // At this point all checks have been done, the note should be added
    // note: in case of error exit from here, the postponed <note> children
    // must still be skipped

    int voiceInt = m_pass1.voiceToInt(voice);
    if (!m_pass1.determineStaffMoveVoice(m_partId, staff, voiceInt, m_staffMove, m_track, m_voice)) {
        m_logger->logDebugInfo(String(u"could not map staff %1 voice '%2'").arg(staff + 1).arg(voice), &m_e);
        addError(checkAtEndElement(m_e, u"note"));
        return 0;
    }

    // start time for note:
    // - sTime for non-chord / first chord note
    // - prevTime for others
    m_noteStartTime = chord ? m_prevSTime : m_sTime;
    m_timeMod = m_noteDuration.timeMod();

    // determine tuplet state, used twice (before and after note allocation)
    MusicXmlTupletFlags tupletAction;

    // handle tuplet state for the previous chord or rest
    if (!chord && !grace) {
        Tuplet* tuplet = m_tuplets[voice];
        MusicXmlTupletState& m_tupletState = m_tupletStates[voice];
        tupletAction = m_tupletState.determineTupletAction(m_noteDuration.duration(), m_timeMod, m_notationsParser.tupletDesc().type,
                                                           m_noteDuration.normalType(), m_missingPrev, m_missingCurr);
        if (tupletAction & MusicXmlTupletFlag::STOP_PREVIOUS) {
            // tuplet start while already in tuplet
            if (m_missingPrev.isValid() && m_missingPrev > Fraction(0, 1)) {
                Rest* const extraRest = addRest(m_score, m_measure, m_noteStartTime, track(), m_staffMove,
                                                TDuration { m_missingPrev* tuplet->ratio() }, m_missingPrev);
                if (extraRest) {
                    extraRest->setTuplet(tuplet);
                    tuplet->add(extraRest);
                    m_noteStartTime += m_missingPrev;
                }
            }
            // recover by simply stopping the current tuplet first
            const int normalNotes = m_timeMod.numerator();
            handleTupletStop(tuplet, normalNotes);
        }
    }

    TDuration duration = determineDuration(rest, measureRest, type, m_noteDuration.dots(), m_dura, m_measure->ticks());

    Part* part = m_pass1.getPart(m_partId);
    Instrument* instrument = part->instrument(m_noteStartTime);
    const MusicXmlInstruments& instruments = m_pass1.getInstruments(m_partId);
    isSingleDrumset = instrument->drumset() && instruments.size() == 1;
    // begin allocation
    if (rest) {
        m_chordRest = addRest(m_score, m_measure, m_noteStartTime, track(), m_staffMove,
                              duration, m_dura);
    } else {
        if (!grace) {
            // regular note
            // if there is already a chord just add to it
            // else create a new one
            // this basically ignores <chord/> errors
            m_chord = findOrCreateChord(duration);
        } else {
            // grace note
            // TODO: check if explicit stem direction should also be set for grace notes
            // (the DOM parser does that, but seems to have no effect on the autotester)
            if (!chord || m_gcl.empty()) {
                m_chord = createGraceChord(duration);
                // TODO FIX
                // the setStaffMove() below results in identical behaviour as 2.0:
                // grace note will be at the wrong staff with the wrong pitch,
                // seems to use the line value calculated for the right staff
                // leaving it places the note at the wrong staff with the right pitch
                // this affects only grace notes where staff move differs from
                // the main note, e.g. DebuMandSample.xml first grace in part 2
                // c->setStaffMove(msMove);
                // END TODO
                m_gcl.push_back(m_chord);
            } else {
                m_chord = m_gcl.back();
            }
        }
        m_note = Factory::createNote(m_chord);
        const staff_idx_t ottavaStaff = (m_track - m_pass1.trackForPart(m_partId)) / VOICES;
        const int octaveShift = m_pass1.octaveShift(m_partId, ottavaStaff, m_noteStartTime);
        const Staff* st = m_chord->staff();
        if (isSingleDrumset && m_notePitch.unpitched() && m_instrumentId.empty()) {
            xmlSetDrumsetPitch(st, instrument);
        } else {
            setPitch(instruments, octaveShift, instrument);
        }
        m_chord->add(m_note);
        m_chordRest = m_chord;
    }
    // end allocation

    if (rest) {
        if (m_chordRest) {
            if (currBeam) {
                if (currBeam->track() == track()) {
                    m_chordRest->setBeamMode(BeamMode::MID);
                    currBeam->add(m_chordRest);
                } else {
                    removeBeam(currBeam);
                }
            } else {
                m_chordRest->setBeamMode(BeamMode::NONE);
            }
            m_chordRest->setSmall(m_isSmall);
            if (noteColor.isValid()) {
                m_chordRest->setColor(noteColor);
            }
            m_chordRest->setVisible(printObject);
            handleDisplayStep();
        }
    } else {
        handleSmallness();
        m_note->setPlay(!m_cue);          // cue notes don't play
        m_note->setHeadGroup(m_headGroup);
        if (headScheme != NoteHeadScheme::HEAD_AUTO) {
            m_note->setHeadScheme(headScheme);
        }
        if (noteColor.isValid()) {
            m_note->setColor(noteColor);
        }
        Stem* stem = m_chord->stem();
        if (!stem) {
            stem = Factory::createStem(m_chord);
            if (stemColor.isValid()) {
                stem->setColor(stemColor);
            } else if (noteColor.isValid()) {
                stem->setColor(noteColor);
            }
            m_chord->add(stem);
        }
        setNoteHead();
        m_note->setVisible(hasHead && printObject);
        stem->setVisible(printObject);

        if (!grace) {
            // regular note
            // handle beam
            if (!chord) {
                handleBeamAndStemDir(currBeam);
            }

            // append any grace chord after chord to the previous chord
            Chord* const prevChord = m_measure->findChord(m_prevSTime, track());
            if (prevChord && prevChord != m_chord) {
                addGraceChordsAfter(prevChord, m_gcl, m_gac);
            }

            // append any grace chord
            addGraceChordsBefore(m_chord, m_gcl);
        }

        if (m_noteDuration.calculatedDuration().isValid()
            && m_noteDuration.specifiedDuration().isValid()
            && m_noteDuration.calculatedDuration().isNotZero()
            && m_noteDuration.calculatedDuration() != m_noteDuration.specifiedDuration()) {
            // convert duration into note length
            Fraction durationMult { (m_noteDuration.specifiedDuration() / m_noteDuration.calculatedDuration()).reduced() };
            durationMult = (1000 * durationMult).reduced();
            const int noteLen = durationMult.numerator() / durationMult.denominator();

            NoteEventList nel;
            NoteEvent ne;
            ne.setLen(noteLen);
            nel.push_back(ne);
            m_note->setPlayEvents(nel);
            if (m_chord) {
                m_chord->setPlayEventType(PlayEventType::User);
            }
        }

        if (velocity > 0) {
            m_note->setUserVelocity(velocity);
        }

        if (m_notePitch.unpitched() && !isSingleDrumset) {
            setDrumset();
        }

        // accidental handling
        //LOGD("note acc %p type %hhd acctype %hhd",
        //       acc, acc ? acc->accidentalType() : static_cast<mu::engraving::AccidentalType>(0), accType);
        Accidental* acc = m_notePitch.acc();
        if (!acc && m_notePitch.accType() != AccidentalType::NONE) {
            acc = Factory::createAccidental(m_score->dummy());
            acc->setAccidentalType(m_notePitch.accType());
        }

        if (acc) {
            acc->setVisible(printObject);
            m_note->add(acc);
            // save alter value for user accidental
            if (acc->accidentalType() != AccidentalType::NONE) {
                m_alt = m_notePitch.alter();
            }
        }

        m_chord->setNoStem(m_noStem);
    }

    // cr can be 0 here (if a rest cannot be added)
    // TODO: complete and cleanup handling this case
    if (m_chordRest) {
        m_chordRest->setVisible(printObject);
        m_notationsParser.addToScore(m_chordRest, m_note,
                                     m_noteStartTime.ticks(), m_pass2.slurs(), m_pass2.glissandi(), m_pass2.spanners(), m_pass2.trills(),
                                     m_pass2.ties(), m_pass2.unstartedTieNotes(), m_pass2.unendedTieNotes(), m_arpMap,
                                     m_delayedArps);

        // if no tie added yet, convert the "tie" into "tied" and add it.
        if (m_note && !m_note->tieFor() && !m_note->tieBack() && !tieType.empty()) {
            Notation notation = Notation(u"tied");
            const String type2 = u"type";
            notation.addAttribute(type2, tieType);
            addTie(notation, m_note, m_chordRest->track(), m_pass2.ties(), m_pass2.unstartedTieNotes(),
                   m_pass2.unendedTieNotes(), m_logger, &m_e);
        }
    }

    // handle tremolo before handling tuplet (two note tremolos modify timeMod)
    if (m_chordRest && m_notationsParser.hasTremolo()) {
        addTremolo(m_pass2.tremStart());
    }

    // handle tuplet state for the current chord or rest
    if (m_chordRest) {
        if (!chord && !grace) {
            Tuplet*& tuplet = m_tuplets[voice];
            // do tuplet if valid time-modification is not 1/1 and is not 1/2 (tremolo)
            // TODO: check interaction tuplet and tremolo handling
            if (m_timeMod.isValid() && m_timeMod != Fraction(1, 1) && m_timeMod != Fraction(1, 2)) {
                const int actualNotes = m_timeMod.denominator();
                const int normalNotes = m_timeMod.numerator();
                if (tupletAction & MusicXmlTupletFlag::START_NEW) {
                    // create a new tuplet
                    handleTupletStart(m_chordRest, tuplet, actualNotes, normalNotes, m_notationsParser.tupletDesc());
                }
                if (tupletAction & MusicXmlTupletFlag::ADD_CHORD) {
                    m_chordRest->setTuplet(tuplet);
                    tuplet->add(m_chordRest);
                }
                if (tupletAction & MusicXmlTupletFlag::STOP_CURRENT) {
                    if (m_missingCurr.isValid() && m_missingCurr > Fraction(0, 1)) {
                        LOGD("add missing %s to current tuplet", muPrintable(m_missingCurr.toString()));
                        Rest* const extraRest = addRest(m_score, m_measure, m_noteStartTime + m_dura, track(), m_staffMove,
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
    if (m_chord && !m_chord->graceNotes().empty() && !m_pass2.graceNoteLyrics().empty()) {
        for (GraceNoteLyrics gnl : m_pass2.graceNoteLyrics()) {
            if (gnl.lyric) {
                addLyric(gnl.lyric, gnl.no);
                if (gnl.extend) {
                    m_pass2.extendedLyrics().addLyric(gnl.lyric);
                }
            }
        }
        m_pass2.graceNoteLyrics().clear();
    }

    if (m_chordRest) {
        addInferredStickings();
    }

    // add lyrics found by lyric
    if (m_chordRest && !grace) {
        // add lyrics and stop corresponding extends
        addLyrics();
        if (rest) {
            // stop all extends
            m_pass2.extendedLyrics().setExtend(-1, m_chordRest->track(), m_chordRest->tick(), nullptr);
        }
    } else if (m_chord && grace) {
        // Add grace note lyrics to main chord later
        addGraceNoteLyrics();
    }

    // add figured bass element
    addFiguredBassElements(m_dura);

    // convert to slash or rhythmic notation if needed
    // TODO in the case of slash notation, we assume that given notes do in fact correspond to slash beats
    if (m_chord && m_pass2.measureStyleSlash() != MusicXmlSlash::NONE) {
        m_chord->setSlash(true, m_pass2.measureStyleSlash() == MusicXmlSlash::SLASH);
    }

    // don't count chord or grace note duration
    // note that this does not check the MusicXML requirement that notes in a chord
    // cannot have a duration longer than the first note in the chord
    if (chord || grace) {
        m_dura.set(0, 1);
    }

    addError(checkAtEndElement(m_e, u"note"));

    return m_note;
}
