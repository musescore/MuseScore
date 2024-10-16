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
#ifndef IMPORTMUSICXMLNOTE_H
#define IMPORTMUSICXMLNOTE_H
#include "types/fraction.h"
#include "../shared/musicxmltypes.h"
#include "musicxmltupletstate.h"

namespace mu::engraving {
class Beam;
class Chord;
class ChordRest;
enum class DirectionV;
class FiguredBass;
class Instrument;
class Measure;
class Note;
enum class NoteType;
class Score;
class Staff;
class TDuration;
class Tuplet;
}

namespace muse {
class XmlStreamReader;
}

namespace mu::iex::musicxml {
class MusicXmlParserPass1;
class MusicXmlParserPass2;
class MusicXmlLogger;
class MusicXmlNotePitch;

using DelayedArpMap = std::map<int, DelayedArpeggio>;

class MusicXmlParserNote
{
public:
    MusicXmlParserNote(muse::XmlStreamReader& e, engraving::Score* score, MusicXmlLogger* logger, MusicXmlParserPass1& pass1,
                       MusicXmlParserPass2& pass2, const muse::String& partId, engraving::Measure* measure, const engraving::Fraction sTime,
                       const engraving::Fraction prevSTime, engraving::Fraction& missingPrev, engraving::Fraction& dura,
                       engraving::Fraction& missingCurr, muse::String& currentVoice, GraceChordList& gcl, size_t& gac, Beams& currBeams,
                       FiguredBassList& fbl, int& alt, MusicXmlTupletStates& tupletStates, Tuplets& tuplets, ArpeggioMap& arpMap,
                       DelayedArpMap& delayedArps);
    engraving::Note* parse();
    muse::String errors() const { return m_errors; }
    void skipLogCurrElem();
private:
    static engraving::NoteHeadGroup convertNotehead(muse::String mxmlName);

    void stem(engraving::DirectionV& stemDirection, bool& noStem);

    void beam(std::map<int, muse::String>& beamTypes);
    static engraving::BeamMode computeBeamMode(const std::map<int, muse::String>& beamTypes);
    static void handleBeamAndStemDir(engraving::ChordRest* cr, const engraving::BeamMode bm, const engraving::DirectionV sd,
                                     engraving::Beam*& beam, bool hasBeamingInfo);
    // static void removeBeam(engraving::Beam*& beam);

    static bool isWholeMeasureRest(const muse::String& type, const engraving::Fraction dura, const engraving::Fraction mDura);
    static engraving::TDuration determineDuration(const bool rest, const bool measureRest, const muse::String& type, const int dots,
                                                  const engraving::Fraction dura, const engraving::Fraction mDura);

    static engraving::Chord* findOrCreateChord(engraving::Score*, engraving::Measure* m, const engraving::Fraction& tick, const int track,
                                               const int move, const engraving::TDuration duration, const engraving::Fraction dura,
                                               engraving::BeamMode bm, bool small);
    static engraving::Chord* createGraceChord(engraving::Score* score, const int track, const engraving::TDuration duration,
                                              const bool slash, const bool small);
    static engraving::NoteType graceNoteType(const engraving::TDuration duration, const bool slash);

    static void setPitch(engraving::Note* note, const MusicXmlInstruments& instruments, const muse::String& instrumentId,
                         const MusicXmlNotePitch& mnp, const int octaveShift, const engraving::Instrument* const instrument);

    static void handleDisplayStep(engraving::ChordRest* cr, int step, int octave, const engraving::Fraction& tick, double spatium);
    static void handleSmallness(bool cueOrSmall, engraving::Note* note, engraving::Chord* c);
    static void setNoteHead(engraving::Note* note, const engraving::Color noteheadColor, const bool noteheadParentheses,
                            const muse::String& noteheadFilled);

    static void addTremolo(engraving::ChordRest* cr, const int tremoloNr, const muse::String& tremoloType, const muse::String& tremoloSmufl,
                           engraving::Chord*& tremStart, MusicXmlLogger* logger, const muse::XmlStreamReader* const xmlreader,
                           engraving::Fraction& timeMod);

    static void addFiguredBassElements(FiguredBassList& fbl, const engraving::Fraction noteStartTime, const int msTrack,
                                       const engraving::Fraction dura, engraving::Measure* measure);

    static void setDrumset(engraving::Chord* c, MusicXmlParserPass1& pass1, const muse::String& partId, const muse::String& instrumentId,
                           const engraving::Fraction& noteStartTime, const MusicXmlNotePitch& mnp, const engraving::DirectionV stemDir,
                           const engraving::NoteHeadGroup headGroup);
    void xmlSetDrumsetPitch(engraving::Note* note, const engraving::Chord* chord, const engraving::Staff* staff, int step, int octave,
                            engraving::NoteHeadGroup headGroup, engraving::DirectionV& stemDir, engraving::Instrument* instrument);

    void notePrintSpacingNo(engraving::Fraction& dura);

    void addError(const muse::String& error);

    muse::XmlStreamReader& m_e;
    engraving::Score* m_score;
    MusicXmlLogger* m_logger;
    muse::String m_errors;
    MusicXmlParserPass1& m_pass1;
    MusicXmlParserPass2& m_pass2;
    muse::String m_partId;
    // engraving::Measure* m_measure = nullptr;
    // engraving::Chord* m_chord = nullptr;
    // engraving::ChordRest* m_chordRest = nullptr;
    // engraving::Note* m_note = nullptr;

    // ARGS FROM MusicXmlParserPass2::note
    //TODO reduce
    engraving::Measure* m_measure;
    const engraving::Fraction m_sTime;
    const engraving::Fraction m_prevSTime;
    engraving::Fraction& m_missingPrev;
    engraving::Fraction& m_dura;
    engraving::Fraction& m_missingCurr;
    muse::String& m_currentVoice;
    GraceChordList& m_gcl;
    size_t& m_gac;
    Beams& m_currBeams;
    FiguredBassList& m_fbl;
    int& m_alt;
    MusicXmlTupletStates& m_tupletStates;
    Tuplets& m_tuplets;
    ArpeggioMap& m_arpMap;
    DelayedArpMap& m_delayedArps;
};
}
#endif // IMPORTMUSICXMLNOTE_H
