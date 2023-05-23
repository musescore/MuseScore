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
#ifndef MU_BRAILLE_BRAILLE_H
#define MU_BRAILLE_BRAILLE_H

#include <QIODevice>

#include "engraving/types/types.h"
#include "engraving/libmscore/types.h"

namespace mu::engraving {
class Arpeggio;
class Articulation;
class BarLine;
class Breath;
class Chord;
class ChordRest;
class Clef;
class DurationElement;
class Dynamic;
class Fermata;
class Fingering;
class Hairpin;
class Jump;
class KeySig;
class Marker;
class Measure;
class MeasureRepeat;
class MMRest;
class Note;
class Rest;
class Score;
class Slur;
class TempoText;
class TimeSig;
class Tuplet;
class Volta;

//This class currently supports just a limited conversion from text to braille
//TODO: enhance it to have full support from text to UEB, including contractions
//http://www.brailleauthority.org/learn/braillebasic.pdf
//https://www.teachingvisuallyimpaired.com/uploads/1/4/1/2/14122361/ueb_braille_chart.pdf
class TextToUEBBraille
{
public:
    TextToUEBBraille();
    QString braille(QChar c);
    QString braille(QString text);

private:
    const QString ASCII_PREFIX_CAPITAL_LETTER   = QString(",");
    const QString ASCII_PREFIX_CAPITAL_WORD     = QString(",,");
    const QString ASCII_PREFIX_NUMBER           = QString("#");
    const QString ASCII_END_OF_CAPITALISATION   = QString(",");
    const QString ASCII_END_OF_NUMBER           = QString(";");
    QMap<QString, QString> textToBrailleASCII;
};

// Braille export is implemented according to Music Braille Code 2015
// published by the Braille Authority of North America
// http://www.brailleauthority.org/music/Music_Braille_Code_2015.pdf
// This class is not thread safe.
class Braille
{
public:
    Braille(Score* s);
    bool write(QIODevice& device);

private:
    static constexpr int MAX_CHARS_PER_LINE = 40;

    Score* score;

    /* ------------ Context ------------ */
    std::vector<Note*> previousNote;
    std::vector<ClefType> currentCleffType;
    std::vector<Key> currentKey;
    /* --------------------------------- */

    void resetOctave(size_t stave);
    void resetOctaves();

    void credits(QIODevice& device);
    void instruments(QIODevice& device);

    /* --------------- Utils. Move these to libmscore? --------------- */
    int computeInterval(Note* rootNote, Note* note, bool ignoreOctaves);
    std::vector<Slur*> slurs(ChordRest* chordRest);
    std::vector<Hairpin*> hairpins(ChordRest* chordRest);
    int notesInSlur(Slur* slur);
    bool isShortSlur(Slur* slur);
    bool isLongSlur(Slur* slur);
    bool isShortShortSlurConvergence(const std::vector<Slur*>& slurs);
    bool isLongLongSlurConvergence(const std::vector<Slur*>& slurs);
    bool hasTies(ChordRest* chordRest);
    bool ascendingChords(ClefType clefType);
    BarLine* firstBarline(Measure* measure, track_idx_t track);
    BarLine* lastBarline(Measure* measure, track_idx_t track);
    /* --------------------------------------------------------------- */

    QString brailleAccidentalType(AccidentalType accidental);
    QString brailleArpeggio(Arpeggio* arpeggio);
    QString brailleArticulation(Articulation* articulation);
    QString brailleBarline(BarLine* barline);
    QString brailleBreath(Breath* breath);
    QString brailleChord(Chord* chord);
    QString brailleChordInterval(Note* rootNote, const std::vector<Note*>& notes, Note* note);
    QString brailleChordRootNote(Chord* chord, Note* rootNote);
    QString brailleClef(Clef* clef);
    QString brailleDynamic(Dynamic* dynamic);
    QString brailleFermata(Fermata* fermata);
    QString brailleFingeringAfter(Fingering* fingering);
    QString brailleGlissando(Note* note);
    QString brailleGraceNoteMarking(Chord* chord);
    QString brailleJump(Jump* jump);
    QString brailleKeySig(KeySig* keySig);
    QString brailleMarker(Marker* marker);
    QString brailleMeasure(Measure* measure, int staffCount);
    QString brailleMeasureRepeat(MeasureRepeat* measureRepeat);
    QString brailleMMRest(MMRest* mmRest);
    QString brailleNote(const QString& pitchName, DurationType durationType, int dots);
    QString brailleOctave(int octave);
    QString brailleRest(Rest* rest);
    QString brailleTempoText(TempoText* tempoText, int staffIdx);
    QString brailleTie(Chord* chord);
    QString brailleTie(Note* note);
    QString brailleTimeSig(TimeSig* timeSig);
    QString brailleTremolo(Chord* chord);
    QString brailleTuplet(Tuplet* tuplet, DurationElement* el);
    QString brailleVolta(Measure* measure, Volta* volta, int staffCount);

    QString brailleHairpinBefore(ChordRest* chordRest, const std::vector<Hairpin*>& hairpin);
    QString brailleHairpinAfter(ChordRest* chordRest, const std::vector<Hairpin*>& hairpin);
    QString brailleSlurBefore(ChordRest* chordRest, const std::vector<Slur*>& slur);
    QString brailleSlurAfter(ChordRest* chordRest, const std::vector<Slur*>& slur);
};
}

#endif // MU_BRAILLE_BRAILLE_H
