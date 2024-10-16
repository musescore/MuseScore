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

#pragma once

#include "framework/global/types/string.h"
#include "imusicxmlconfiguration.h"
#include "internal/musicxml/shared/musicxmltypes.h"
#include "serialization/xmlstreamreader.h"
#include "types/types.h"

namespace mu::engraving {
enum class AccidentalType;
class Articulation;
class Beam;
class Chord;
class ChordRest;
class Fraction;
class Instrument;
class Lyrics;
class Measure;
class Note;
class Rest;
enum class SymId;
class Score;
class Sticking;
class TDuration;
}

namespace mu::iex::musicxml {
class Notation;
class MusicXmlLogger;
class MusicXmlLyricsExtend;
struct GraceNoteLyrics;
//---------------------------------------------------------
//   MusicXmlSupport -- MusicXML import support functions
//---------------------------------------------------------

class MusicXmlSupport
{
public:
    static int stringToInt(const muse::String& s, bool* ok);
    static engraving::Fraction noteTypeToFraction(const muse::String& type);
    static engraving::Fraction calculateFraction(const muse::String& type, int dots, int normalNotes, int actualNotes);
};

extern std::shared_ptr<mu::iex::musicxml::IMusicXmlConfiguration> configuration();
extern muse::String accSymId2MusicXmlString(const engraving::SymId id);
extern muse::String accSymId2SmuflMusicXmlString(const engraving::SymId id);
extern muse::String accidentalType2MusicXmlString(const engraving::AccidentalType type);
extern muse::String accidentalType2SmuflMusicXmlString(const engraving::AccidentalType type);
extern engraving::AccidentalType musicXmlString2accidentalType(const muse::String mxmlName, const muse::String smufl);
extern muse::String musicXmlAccidentalTextToChar(const muse::String mxmlName);
extern engraving::SymId musicXmlString2accSymId(const muse::String mxmlName, const muse::String smufl = {});
extern engraving::AccidentalType microtonalGuess(double val);
extern bool isLaissezVibrer(const engraving::SymId id);
extern const engraving::Articulation* findLaissezVibrer(const engraving::Chord* chord);
extern muse::String errorStringWithLocation(int line, int col, const muse::String& error);
extern muse::String checkAtEndElement(const muse::XmlStreamReader& e, const muse::String& expName);
extern void removeBeam(engraving::Beam*& beam);
extern void setChordRestDuration(engraving::ChordRest* cr, engraving::TDuration duration, const engraving::Fraction dura);
extern engraving::Rest* addRest(engraving::Score*, engraving::Measure* m, const engraving::Fraction& tick,
                                const engraving::track_idx_t track, const int move, const engraving::TDuration duration,
                                const engraving::Fraction dura);
extern bool hasDrumset(const MusicXmlInstruments& instruments);
extern void addGraceChordsAfter(engraving::Chord* c, GraceChordList& gcl, size_t& gac);
extern void addGraceChordsBefore(engraving::Chord* c, GraceChordList& gcl);
extern void coerceGraceCue(engraving::Chord* mainChord, engraving::Chord* graceChord);
extern void xmlSetPitch(engraving::Note* n, int step, int alter, double tuning, int octave, const int octaveShift,
                        const engraving::Instrument* const instr);
extern int MusicXmlStepAltOct2Pitch(int step, int alter, int octave);
extern void handleTupletStart(const engraving::ChordRest* const cr, engraving::Tuplet*& tuplet, const int actualNotes,
                              const int normalNotes, const MusicXmlTupletDesc& tupletDesc);
extern void handleTupletStop(engraving::Tuplet*& tuplet, const int normalNotes);
extern void addTie(const Notation& notation, engraving::Note* note, const engraving::track_idx_t track, MusicXmlTieMap& ties,
                   std::vector<engraving::Note*>& unstartedTieNotes, std::vector<engraving::Note*>& unendedTieNotes, MusicXmlLogger* logger,
                   const muse::XmlStreamReader* const xmlreader);
extern void addLyric(MusicXmlLogger* logger, const muse::XmlStreamReader* const xmlreader, engraving::ChordRest* cr, engraving::Lyrics* l,
                     int lyricNo, MusicXmlLyricsExtend& extendedLyrics);
extern void addLyrics(MusicXmlLogger* logger, const muse::XmlStreamReader* const xmlreader, engraving::ChordRest* cr, const std::map<int,
                                                                                                                                     engraving::Lyrics*>& numbrdLyrics, const std::set<engraving::Lyrics*>& extLyrics, MusicXmlLyricsExtend& extendedLyrics);
extern void addGraceNoteLyrics(const std::map<int, engraving::Lyrics*>& numberedLyrics, std::set<engraving::Lyrics*> extendedLyrics,
                               std::vector<GraceNoteLyrics>& gnLyrics);
extern void addInferredStickings(engraving::ChordRest* cr, const std::vector<engraving::Sticking*>& numberedStickings);
} // namespace Ms
