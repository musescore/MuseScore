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
#include "internal/musicxml/import/importmusicxmlpass2.h"
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
class MusicXmlParserLyric;
class MusicXmlParserNotations;
class MusicXmlNoteDuration;

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
private:
    void skipLogCurrElem();
    static engraving::NoteHeadGroup convertNotehead(muse::String mxmlName);

    void stem();
    void beam();
    void computeBeamMode();
    void handleBeamAndStemDir(engraving::Beam*& beam);

    static bool isWholeMeasureRest(const muse::String& type, const engraving::Fraction dura, const engraving::Fraction mDura);
    static engraving::TDuration determineDuration(const bool rest, const bool measureRest, const muse::String& type, const int dots,
                                                  const engraving::Fraction dura, const engraving::Fraction mDura);

    engraving::Chord* findOrCreateChord(const engraving::Fraction& tick, const int track, const int move,
                                        const engraving::TDuration duration, const engraving::Fraction dura);
    engraving::Chord* createGraceChord(const int track, const engraving::TDuration duration, const bool slash);
    static engraving::NoteType graceNoteType(const engraving::TDuration duration, const bool slash);

    void setPitch(const MusicXmlInstruments& instruments, const muse::String& instrumentId, const int octaveShift,
                  const engraving::Instrument* const instrument);

    void handleDisplayStep(int step, int octave, const engraving::Fraction& tick, double spatium);
    void handleSmallness();
    void setNoteHead(const engraving::Color noteheadColor, const bool noteheadParentheses, const muse::String& noteheadFilled);

    void addTremolo(const int tremoloNr, const muse::String& tremoloType, const muse::String& tremoloSmufl, engraving::Chord*& tremStart,
                    engraving::Fraction& timeMod);

    void addFiguredBassElements(const engraving::Fraction noteStartTime, const int msTrack, const engraving::Fraction dura);

    void setDrumset(const muse::String& instrumentId, const engraving::Fraction& noteStartTime, const engraving::NoteHeadGroup headGroup);
    void xmlSetDrumsetPitch(const engraving::Staff* staff, int step, int octave, engraving::NoteHeadGroup headGroup,
                            engraving::Instrument* instrument);

    void addInferredStickings() const;
    void addGraceNoteLyrics();
    void addLyrics();
    void addLyric(engraving::Lyrics* l, int lyricNo);

    bool isSmall() const { return m_cue || m_isSmall; }

    void notePrintSpacingNo(engraving::Fraction& dura);
    void addError(const muse::String& error);

    muse::XmlStreamReader& m_e;
    engraving::Score* m_score;
    MusicXmlLogger* m_logger;
    muse::String m_errors;
    MusicXmlParserPass1& m_pass1;
    MusicXmlParserPass2& m_pass2;
    muse::String m_partId;
    engraving::Chord* m_chord = nullptr;
    engraving::ChordRest* m_chordRest = nullptr;
    engraving::Note* m_note = nullptr;

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

    MusicXmlParserLyric m_lyricParser;
    MusicXmlParserNotations m_notationsParser;
    MusicXmlNoteDuration m_noteDuration;
    MusicXmlNotePitch m_notePitch;

    bool m_cue = false;
    bool m_isSmall = false;
    engraving::DirectionV m_stemDir = engraving::DirectionV::AUTO;
    bool m_noStem = false;

    std::map<int, muse::String> m_beamTypes;
    engraving::BeamMode m_beamMode;
};
}
#endif // IMPORTMUSICXMLNOTE_H
