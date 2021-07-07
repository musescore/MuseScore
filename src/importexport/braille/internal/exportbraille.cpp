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

#include <QDebug>
#include <QRegularExpression>

#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/dynamic.h"
#include "libmscore/barline.h"
#include "libmscore/timesig.h"
#include "libmscore/pedal.h"
#include "libmscore/text.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/arpeggio.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/tempotext.h"
#include "libmscore/symid.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/utils.h"
#include "libmscore/articulation.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/element.h"
#include "libmscore/glissando.h"
#include "libmscore/navigate.h"
#include "libmscore/spanner.h"
#include "libmscore/mscore.h"
#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/chordline.h"
#include "libmscore/figuredbass.h"
#include "libmscore/stringdata.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/fret.h"
#include "libmscore/tie.h"
#include "libmscore/undo.h"
#include "libmscore/fermata.h"
#include "libmscore/durationtype.h"
#include "libmscore/types.h"
#include "libmscore/fingering.h"
#include "libmscore/mmrest.h"

// Table 1. Page 2. Music Braille Code 2015.
#define BRAILLE_EQUALS_METRONOME        QString("7")
#define BRAILLE_MUSICAL_HYPEN           QString("\"")
#define BRAILLE_BARLINE                 QString(" ")
#define BRAILLE_BARLINE_BROKEN          QString("K")
#define BRAILLE_BARLINE_DOUBLE          QString("<K'")
#define BRAILLE_BARLINE_END             QString("<K")
#define BRAILLE_BARLINE_DOTTED          QString("l")

#define BRAILLE_ACC_NATURAL             QString("*")
#define BRAILLE_ACC_SHARP               QString("%")
#define BRAILLE_ACC_FLAT                QString("<")
#define BRAILLE_ACC_QUARTER_STEP        QString("@")
#define BRAILLE_ACC_3QUARTER_STEP       QString("_")
#define BRAILLE_ACC_5QUARTER_STEP       QString(".") //extrapollated from octave marks. not sure.

// Table 2. Page 4. Music Braille Code 2015
//8th and 128th notes have the same representation in Braille
#define BRAILLE_C_8TH_128TH             'D'
#define BRAILLE_D_8TH_128TH             'E'
#define BRAILLE_E_8TH_128TH             'F'
#define BRAILLE_F_8TH_128TH             'G'
#define BRAILLE_G_8TH_128TH             'H'
#define BRAILLE_A_8TH_128TH             'I'
#define BRAILLE_B_8TH_128TH             'J'
//64th and quarter notes have the same representation in Braille
#define BRAILLE_C_64TH_QUARTER          '?'
#define BRAILLE_D_64TH_QUARTER          ':'
#define BRAILLE_E_64TH_QUARTER          '$'
#define BRAILLE_F_64TH_QUARTER          ']'
#define BRAILLE_G_64TH_QUARTER          '\\'
#define BRAILLE_A_64TH_QUARTER          '['
#define BRAILLE_B_64TH_QUARTER          'W'
//32nd and half notes have the same representation in Braille
#define BRAILLE_C_32ND_HALF             'N'
#define BRAILLE_D_32ND_HALF             'o'
#define BRAILLE_E_32ND_HALF             'p'
#define BRAILLE_F_32ND_HALF             'q'
#define BRAILLE_G_32ND_HALF             'r'
#define BRAILLE_A_32ND_HALF             's'
#define BRAILLE_B_32ND_HALF             't'
//16th and whole notes have the same representation in Braille.
// Breve has the same representation, but with an extra suffix;
// 256th has the same represantation, but with an extra prefix;
#define BRAILLE_C_16TH_WHOLE            'Y'
#define BRAILLE_D_16TH_WHOLE            'Z'
#define BRAILLE_E_16TH_WHOLE            '&'
#define BRAILLE_F_16TH_WHOLE            '='
#define BRAILLE_G_16TH_WHOLE            '('
#define BRAILLE_A_16TH_WHOLE            '!'
#define BRAILLE_B_16TH_WHOLE            ')'

#define BRAILLE_BREVE_SUFFIX            QString("k")
#define BRAILLE_256TH_PREFIX            QString(";<1")
#define BRAILLE_DURATION_DOT            QString("'")

// Table 3. Page 4. Music Braille Code 2015.
#define BRAILLE_BELOW_FIRST_OCTAVE      QString("@@")
#define BRAILLE_1ST_OCTAVE              QString("@")
#define BRAILLE_2ND_OCTAVE              QString("^")
#define BRAILLE_3RD_OCTAVE              QString("_")
#define BRAILLE_4TH_OCTAVE              QString("\"")
#define BRAILLE_5TH_OCTAVE              QString(".")
#define BRAILLE_6TH_OCTAVE              QString(";")
#define BRAILLE_7TH_OCTAVE              QString(",")
#define BRAILLE_ABOVE_7TH_OCTAVE        QString(",,")

// Table 4. Page 4. Music Braille Code 2015.
#define BRAILLE_TREBLE_CLEF             QString(">/l")
#define BRAILLE_TREBLE_CLEF_15MB        QString(">/l#15")
#define BRAILLE_TREBLE_CLEF_8VB         QString(">/l#8")
#define BRAILLE_TREBLE_CLEF_8VA         QString(">/l#h")
#define BRAILLE_TREBLE_CLEF_15MA        QString(">/l#ae")
#define BRAILLE_D_TREBLE_CLEF_8VB_O     QString(">/l/l#8")
#define BRAILLE_TREBLE_CLEF_8VB_P       QString(">/l#787")
#define BRAILLE_FRENCH_VIOLINE_CLEF     QString(">/@l")
#define BRAILLE_SOPRANO_CLEF            QString(">+@l")
#define BRAILLE_MEZZO_SOPRANO_CLEF      QString(">+^l")
#define BRAILLE_ALTO_CLEF               QString(">+l")
#define BRAILLE_TENOR_CLEF              QString(">+\"l")
#define BRAILLE_BARITONE_C_CLEF         QString(">+.l")
#define BRAILLE_BASS_CLEF               QString(">#l")
#define BRAILLE_BASS_CLEF_15MB          QString(">#l#15")
#define BRAILLE_BASS_CLEF_8VB           QString(">#l#8")
#define BRAILLE_BASS_CLEF_8VA           QString(">#l#h")
#define BRAILLE_BASS_CLEF_15MA          QString(">#l#ae")
#define BRAILLE_BARITONE_F_CLEF         QString(">#_l")
#define BRAILLE_SUBBASS_CLEF            QString(">#.l")

// Table 5. Page 5. Music Braille Code 2015.
#define BRAILLE_REST_LONG               QString("m^c^cm")
#define BRAILLE_REST_BREVE              QString("mk")
#define BRAILLE_REST_WHOLE              QString("m")
#define BRAILLE_REST_HALF               QString("u")
#define BRAILLE_REST_QUARTER            QString("v")
#define BRAILLE_REST_EIGHTH             QString("x")
#define BRAILLE_REST_16TH               QString("m")
#define BRAILLE_REST_32ND               QString("u")
#define BRAILLE_REST_64TH               QString("v")
#define BRAILLE_REST_128TH              QString("x")
#define BRAILLE_REST_256TH              QString(";<1m")
#define BRAILLE_REST_MEASURE            QString("m")

// Table 9. Page 7. Music Braille Code 2015.
#define BRAILLE_OCTAVE                  QString("-")
#define BRAILLE_SECOND                  QString("/")
#define BRAILLE_THIRD                   QString("+")
#define BRAILLE_FOURTH                  QString("#")
#define BRAILLE_FIFTH                   QString("9")
#define BRAILLE_SIXTH                   QString("0")
#define BRAILLE_SEVENTH                 QString("3")

// Table 10. Page 7. Music Braille Code 2015
#define BRAILLE_TIE                     QString("@c")
#define BRAILLE_TIE_CHORD               QString(".c")

// Table 11. Page 8. Music Braille Code 2015
#define BRAILLE_FULL_MEASURE_IN_ACORD   QString("<>")
#define BRAILLE_PART_MEASURE_IN_ACORD   QString("\"1")
#define BRAILLE_MEASURE_DIVISION        QString(".k")

// Table 13. Page 9. Music Braille Code 2015.
#define BRAILLE_SLUR_SHORT              QString("c")
#define BRAILLE_SLUR_SHORT_CONVERGENCE  QString(",c")
#define BRAILLE_SLUR_BRACKET_START      QString(";b")
#define BRAILLE_SLUR_BRACKET_END        QString("^2")

// Table 14. Page 10. Music Braille Code 2015
#define BRAILLE_TREMOLO_8THS            QString("^b")
#define BRAILLE_TREMOLO_16THS           QString("^l")
#define BRAILLE_TREMOLO_32NDS           QString("^1")
#define BRAILLE_TREMOLO_64THS           QString("^k")
#define BRAILLE_TREMOLO_128THS          QString("^'")
#define BRAILLE_TREMOLO_8THS_ALT        QString(".b")
#define BRAILLE_TREMOLO_16THS_ALT       QString(".l")
#define BRAILLE_TREMOLO_32NDS_ALT       QString(".1")
#define BRAILLE_TREMOLO_64THS_ALT       QString(".k")
#define BRAILLE_TREMOLO_128THS_ALT      QString(".'")

// Table 15. Page 11. Music Braille Code 2015.
#define BRAILLE_FINGERING_OPEN          QString("K")
#define BRAILLE_FINGERING_1ST           QString("a")
#define BRAILLE_FINGERING_2ND           QString("b")
#define BRAILLE_FINGERING_3RD           QString("l")
#define BRAILLE_FINGERING_4TH           QString("1")
#define BRAILLE_FINGERING_5TH           QString("k")
#define BRAILLE_FINGERING_CHANGE        QString("c")

// Table 16.A. Page 12. Music Braille Code 2015.
#define BRAILLE_GRACE_LONG              QString("\"5")
#define BRAILLE_GRACE_SHORT             QString("5")

// Table 16.B. Page 12. Music Braille Code 2015.
#define BRAILLE_TRILL                   QString("6")
#define BRAILLE_TRILL_FLAT              QString("<6")
#define BRAILLE_TRILL_SHARP             QString("%6")
#define BRAILLE_TURN_BTWN_NOTES         QString("4")
#define BRAILLE_TURN_NOTE               QString(",4")
#define BRAILLE_INVERTED_TURN_BTWN_NOTES QString("4l")
#define BRAILLE_INVERTED_TURN_NOTE      QString(",4l")

// Table 16.C. Page 13. Music Braille Code 2015
#define BRAILLE_SHORT_TRILL             QString("\"6")
#define BRAILLE_TREBLEMENT              QString(";6")
#define BRAILLE_MORDENT                 QString("\"6l")
#define BRAILLE_PRALL_MORDENT           QString(";6l")

// Table 16.D. Page 13. Music Braille Code 2015
#define BRAILLE_GLISSANDO               QString("@a")

// Table 16.F. Page 14. Music Braille Code 2015
#define BRAILLE_MORDENT_WITH_UPPER_FIX  QString("4;6")
#define BRAILLE_UP_PRALL                QString("4l;6")
#define BRAILLE_UP_MORDENT              QString("4l;6l") //extrapollated. not sure
#define BRAILLE_DOWN_MORDENT            QString("4;6")  //extrapollated. not sure
#define BRAILLE_PRALL_DOWN              QString(";64l")  //extrapollated. not sure
#define BRAILLE_PRALL_UP                QString(";64")   //extrapollated. not sure
#define BRAILLE_LINE_PRALL              QString(";6l4")  //extrapollated. not sure

// Table 17. Page 15.x Music Braille Code 2015
#define BRAILLE_BARLINE_START_REPEAT    QString("<7")
#define BRAILLE_BARLINE_END_REPEAT      QString("<2")

// Table 18. Page 15. Music Braille Code 2015
#define BRAILLE_MEASURE_REPEAT          QString("7")

// Table 20. Page 16. Music Braille Code 2015
#define BRAILLE_SEGNO                   QString("+ ")
#define BRAILLE_CODA                    QString("+l ")
#define BRAILLE_FINE                    QString(">fine> ")
#define BRAILLE_TOCODA                  QString(">to coda> ")
#define BRAILLE_DA_CAPO                 QString(">d'c'> ")
#define BRAILLE_DA_CAPO_AL_FINE         QString(">d'c' al fine> ")
#define BRAILLE_DA_CAPO_AL_CODA         QString(">d'c' al coda> ")
#define BRAILLE_DAL_SEGNO_AL_CODA       QString(">d's' al coda> ")
#define BRAILLE_DAL_SEGNO_AL_FINE       QString(">d's' al fine> ")
#define BRAILLE_DAL_SEGNO               QString(">d's'> ")

// Table 21. Page 16. Music Braille Code 2015
#define BRAILLE_MUSIC_PARENTHESES       QString(",'")

// Table 22. Page 17. Music Braille Code 2015.
#define BRAILLE_ARPEGGIO_UPWARD         QString(">k")
#define BRAILLE_ARPEGGIO_DOWNWARD       QString(">kk")
#define BRAILLE_ARPEGGIO_MULTISTAVES    QString("\"")

// Table 22.A. Page 17. Music Braille Code 2015.
#define BRAILLE_ARTIC_STACCATO          QString("8")
#define BRAILLE_ARTIC_STACCATISSIMO     QString(",8")
#define BRAILLE_ARTIC_TENUTO_STACCATO   QString("\"8")
#define BRAILLE_ARTIC_TENUTO            QString("_8")
#define BRAILLE_ARTIC_ACCENT            QString(".8")
#define BRAILLE_ARTIC_MARCATO           QString(";8")
#define BRAILLE_ARTIC_REVERSED_ACCENT   QString("@8")
#define BRAILLE_ARTIC_EXPRESSIVE_ACCENT QString("^8")
#define BRAILLE_ARTIC_SWELL             QString("*'")

// Table 22.2.B. Page 18. Music Braille Code 2015.
#define BRAILLE_FERMATA                 QString("<l")
#define BRAILLE_SHORT_FERMATA           QString("^<l")
#define BRAILLE_LONG_FERMATA            QString(";<l")
#define BRAILLE_BREATH                  QString(">1")
#define BRAILLE_CAESURA                 QString(",/")

// Table 22.C
#define BRAILLE_HAIRPIN_DIV_START       QString(">c")
#define BRAILLE_HAIRPIN_DIV_END         QString(">3")
#define BRAILLE_HAIRPIN_CONV_START      QString(">d")
#define BRAILLE_HAIRPIN_CONV_END        QString(">4")
#define BRAILLE_CRESC                   QString(">cr'")
#define BRAILLE_DESCRESC                QString(">decr'")
#define BRAILLE_DIM                     QString(">dim'")
#define BRAILLE_LV                      QString(">lvb'")
#define BRAILLE_LINE_CONT_START_1       QString("''")
#define BRAILLE_LINE_CONT_END_1         QString(">'")
#define BRAILLE_LINE_CONT_START_2       QString("--")
#define BRAILLE_LINE_CONT_END_2         QString(">-")

// TABLE 24.B. Page 22. Music Braille Code 2015
#define BRAILLE_DOWN_BOW                QString("<b")
#define BRAILLE_UP_BOW                  QString("<'")
#define BRAILLE_LEFT_HAND_PIZZICATO     QString("_>")

namespace Ms {
//This class currently supports just a limited coversion from text to braille
//TODO: enhance it to have full support from text to UEB, including contractions
//http://www.brailleauthority.org/learn/braillebasic.pdf
//https://www.teachingvisuallyimpaired.com/uploads/1/4/1/2/14122361/ueb_braille_chart.pdf
class TextToUEBBraille
{
    const int MAX_CHARS_PER_LINE = 40;
    const QString ASCII_PREFIX_CAPITAL_LETTER = QString(",");
    const QString ASCII_PREFIX_CAPITAL_WORD = QString(",,");
    const QString ASCII_PREFIX_NUMBER = QString("#");
    const QString ASCII_END_OF_CAPITALISATION = QString(",");
    const QString ASCII_END_OF_NUMBER = QString(";");

    QMap<QString, QString> textToBrailleASCII;
public:
    TextToUEBBraille()
    {
        textToBrailleASCII["a"] = 'a';
        textToBrailleASCII["b"] = 'b';
        textToBrailleASCII["c"] = 'c';
        textToBrailleASCII["d"] = 'd';
        textToBrailleASCII["e"] = 'e';
        textToBrailleASCII["f"] = 'f';
        textToBrailleASCII["g"] = 'g';
        textToBrailleASCII["h"] = 'h';
        textToBrailleASCII["i"] = 'i';
        textToBrailleASCII["j"] = 'j';
        textToBrailleASCII["k"] = 'k';
        textToBrailleASCII["l"] = 'l';
        textToBrailleASCII["m"] = 'm';
        textToBrailleASCII["n"] = 'n';
        textToBrailleASCII["o"] = 'o';
        textToBrailleASCII["p"] = 'p';
        textToBrailleASCII["q"] = 'q';
        textToBrailleASCII["r"] = 'r';
        textToBrailleASCII["s"] = 's';
        textToBrailleASCII["t"] = 't';
        textToBrailleASCII["u"] = 'u';
        textToBrailleASCII["v"] = 'v';
        textToBrailleASCII["w"] = 'w';
        textToBrailleASCII["x"] = 'x';
        textToBrailleASCII["y"] = 'y';
        textToBrailleASCII["z"] = 'z';

        textToBrailleASCII["A"] = 'a';
        textToBrailleASCII["B"] = 'b';
        textToBrailleASCII["C"] = 'c';
        textToBrailleASCII["D"] = 'd';
        textToBrailleASCII["E"] = 'e';
        textToBrailleASCII["F"] = 'f';
        textToBrailleASCII["G"] = 'g';
        textToBrailleASCII["H"] = 'h';
        textToBrailleASCII["I"] = 'i';
        textToBrailleASCII["J"] = 'j';
        textToBrailleASCII["K"] = 'k';
        textToBrailleASCII["L"] = 'l';
        textToBrailleASCII["M"] = 'm';
        textToBrailleASCII["N"] = 'n';
        textToBrailleASCII["O"] = 'o';
        textToBrailleASCII["P"] = 'p';
        textToBrailleASCII["Q"] = 'q';
        textToBrailleASCII["R"] = 'r';
        textToBrailleASCII["S"] = 's';
        textToBrailleASCII["T"] = 't';
        textToBrailleASCII["U"] = 'u';
        textToBrailleASCII["V"] = 'v';
        textToBrailleASCII["W"] = 'w';
        textToBrailleASCII["X"] = 'x';
        textToBrailleASCII["Y"] = 'y';
        textToBrailleASCII["Z"] = 'z';

        textToBrailleASCII["1"] = 'a';
        textToBrailleASCII["2"] = 'b';
        textToBrailleASCII["3"] = 'c';
        textToBrailleASCII["4"] = 'd';
        textToBrailleASCII["5"] = 'e';
        textToBrailleASCII["6"] = 'f';
        textToBrailleASCII["7"] = 'g';
        textToBrailleASCII["8"] = 'h';
        textToBrailleASCII["9"] = 'i';
        textToBrailleASCII["0"] = 'j';

        textToBrailleASCII[" "] = ' ';
        textToBrailleASCII[","] = '1';
        textToBrailleASCII[";"] = '2';
        textToBrailleASCII[":"] = '3';
        textToBrailleASCII["."] = '4';
        textToBrailleASCII["!"] = '6';
        textToBrailleASCII["("] = '7';
        textToBrailleASCII[")"] = '7';
        textToBrailleASCII["?"] = '8';
        textToBrailleASCII["="] = "\"7";
        textToBrailleASCII["\n"] = '\n';
        textToBrailleASCII["©"] = "~c";
        textToBrailleASCII["-"] = "-";
        textToBrailleASCII["*"] = "99";

        // Appendix B: Foreign language signs. Page 31.
        // https://www.ukaaf.org/wp-content/uploads/B017-Braille-Music-and-Unified-English-Braille.pdf
        textToBrailleASCII["ç"] = "&";
        textToBrailleASCII["é"] = "=";
        textToBrailleASCII["à"] = "(";
        textToBrailleASCII["è"] = "!";
        textToBrailleASCII["ù"] = ")";
        textToBrailleASCII["â"] = "*";
        textToBrailleASCII["ê"] = "<";
        textToBrailleASCII["î"] = "%";
        textToBrailleASCII["ô"] = "?";
        textToBrailleASCII["û"] = ":";
        textToBrailleASCII["ë"] = "$";
        textToBrailleASCII["ï"] = "]";
        textToBrailleASCII["ü"] = "\\";
        textToBrailleASCII["œ"] = "[";
        textToBrailleASCII["ß"] = "!";
        textToBrailleASCII["ö"] = "[";
        textToBrailleASCII["ä"] = ">";
        textToBrailleASCII["ì"] = "/";
        textToBrailleASCII["ò"] = "+";
        textToBrailleASCII["á"] = "(";
        textToBrailleASCII["ú"] = ")";
        textToBrailleASCII["ñ"] = "]";
        textToBrailleASCII["í"] = "/";
        textToBrailleASCII["ó"] = "+";

        // dynamics symbols
        textToBrailleASCII[QChar(0xE520)] = "p";
        textToBrailleASCII[QChar(0xE521)] = "m";
        textToBrailleASCII[QChar(0xE522)] = "f";
        textToBrailleASCII[QChar(0xE523)] = "r";
        textToBrailleASCII[QChar(0xE524)] = "s";
        textToBrailleASCII[QChar(0xE525)] = "z";
        textToBrailleASCII[QChar(0xE526)] = "n";
    }

    QString braille(QChar c);
    QString braille(QString text);
};

// Braille export is implemented according to Music Braille Code 2015
// published by the Braille Authority of North America
// http://www.brailleauthority.org/music/Music_Braille_Code_2015.pdf
// This class is not thread safe.
class ExportBraille
{
    const int MAX_CHARS_PER_LINE = 40;

    Score* score;

    /* ---------- Context ---------------- */
    std::vector<Note*> previousNote;
    std::vector<ClefType> currentCleffType;
    std::vector<Key> currentKey;
    /* ----------------------------------- */
    void resetOctaves();
    void resetOctave(int stave);

    void credits(QIODevice* dev);
    void instruments(QIODevice* dev);

    //utils. should we move these in libmscore?
    int computeInterval(Note* rootNote, Note* note, bool ignoreOctaves);
    std::vector<Slur*> slurs(ChordRest* chordRest);
    std::vector<Hairpin*> hairpins(ChordRest* chordRest);
    int notesInSlur(Slur* slur);
    bool isShortSlur(Slur* slur);
    bool isLongSlur(Slur* slur);
    bool isLongLongSlurConvergence(std::vector<Slur*>* slurs);
    bool isShortShortSlurConvergence(std::vector<Slur*>* slurs);
    bool hasTies(ChordRest* chordRest);
    bool ascendingChords(ClefType clefType);
    BarLine* lastBarline(Measure* measure, int track);
    BarLine* firstBarline(Measure* measure, int track);

    QString brailleMeasure(Measure* measure, int staffCount);
    QString brailleClef(Clef* clef);
    QString brailleTimeSig(TimeSig* timeSig);
    QString brailleMMRest(MMRest* mmRest);
    QString brailleRest(Rest* rest);
    QString brailleBreath(Breath* breath);
    QString brailleOctave(int octave);
    QString brailleChord(Chord* chord);
    QString brailleChordRootNote(Chord* chord, Note* rootNote);
    QString brailleNote(QString pitchName, TDuration::DurationType durationType, int dots);
    QString brailleChordInterval(Note* rootNote, QList<Note*>* notes, Note* note);
    QString brailleGraceNoteMarking(Chord* chord);
    QString brailleAccidentalType(AccidentalType accidental);
    QString brailleBarline(BarLine* barline);
    QString brailleDynamic(Dynamic* dynamic);
    QString brailleTempoText(TempoText* tempoText, int staffIdx);
    QString brailleKeySig(KeySig* keySig);
    QString brailleFermata(Fermata* fermata);
    QString brailleTuplet(Tuplet* tuplet, DurationElement* el);
    QString brailleJump(Jump* jump);
    QString brailleMarker(Marker* marker);
    QString brailleTie(Chord* chord);
    QString brailleTie(Note* note);
    QString brailleArpeggio(Arpeggio* arpeggio);
    QString brailleTremolo(Chord* chord);
    QString brailleArticulation(Articulation* articulation);
    QString brailleFingeringAfter(Fingering* fingering);
    QString brailleGlissando(Note* note);
    QString brailleMeasureRepeat(MeasureRepeat* measureRepeat);
    QString brailleVolta(Measure* measure, Volta* volta, int staffCount);

    QString brailleSlurBefore(ChordRest* chordRest, std::vector<Slur*>* slur);
    QString brailleSlurAfter(ChordRest* chordRest, std::vector<Slur*>* slur);
    QString brailleHairpinBefore(ChordRest* chordRest, std::vector<Hairpin*>* hairpin);
    QString brailleHairpinAfter(ChordRest* chordRest, std::vector<Hairpin*>* hairpin);

public:
    ExportBraille(Score* s)
    {
        score = s;
        for (int i = 0; i < score->staves().size(); ++i) {
            previousNote.push_back(nullptr);
            currentCleffType.push_back(ClefType::INVALID);
            currentKey.push_back(Key::INVALID);
        }
    }

    void write(QIODevice* dev);
};

//---------------------------------------------------------
//   saveBraille
//    return false on error
//---------------------------------------------------------

/**
     Save Score as Braille (brf) file \a name.

     Return false on error.
     */

bool saveBraille(Score* score, QIODevice* device)
{
    ExportBraille eb(score);
    eb.write(device);
    return true;
}

bool saveBraille(Score* score, const QString& name)
{
    QFile f(name);
    if (!f.open(QIODevice::WriteOnly)) {
        return false;
    }

    bool res = saveBraille(score, &f) && (f.error() == QFile::NoError);
    f.close();
    return res;
}

void ExportBraille::resetOctave(int stave)
{
    previousNote[stave]  = nullptr;
}

void ExportBraille::resetOctaves()
{
    for (int i = 0; i < score->staves().size(); ++i) {
        resetOctave(i);
    }
}

void ExportBraille::credits(QIODevice* dev)
{
    QTextStream out(dev);
    // find the vboxes in every page and write their elements as credit-words
    for (const auto page : score->pages()) {
        for (const auto system : page->systems()) {
            for (const auto mb : system->measures()) {
                if (mb->isVBox()) {
                    for (const Element* element : mb->el()) {
                        if (element->isText()) {
                            const Text* text = toText(element);
                            out << TextToUEBBraille().braille(text->plainText()).toUtf8() << Qt::endl;
                        }
                    }
                }
            }
        }
    }

    QStringList creators;
    // the creator types commonly found in MusicXML
    creators << "arranger" << "composer" << "lyricist" << "poet" << "translator";
    for (QString type : creators) {
        QString creator = score->metaTag(type);
        if (!creator.isEmpty()) {
            out << TextToUEBBraille().braille(QString("%1 %2").arg(type).arg(creator)).toUtf8() << Qt::endl;
        }
    }
    if (!score->metaTag("copyright").isEmpty()) {
        out << TextToUEBBraille().braille(QString("© %2").arg(score->metaTag("copyright"))).toUtf8() << Qt::endl;
    }
    out << Qt::endl;
    out.flush();
}

void ExportBraille::instruments(QIODevice* dev)
{
    //Print staff number to instrument mapping.
    QTextStream out(dev);
    for (int i = 0; i < score->staves().size(); ++i) {
        out << TextToUEBBraille().braille(QString("%1 %2").arg(i + 1).arg(score->staves()[i]->part()->instrumentName())) << Qt::endl;
    }
    out << Qt::endl;
    out.flush();
}

void ExportBraille::write(QIODevice* dev)
{
    credits(dev);
    instruments(dev);
    int nrStaves = score->staves().size();
    std::vector<QString> measureBraille(nrStaves);
    std::vector<QString> line(nrStaves + 1);
    int currentLineLenght = 0;
    int currentMeasureMaxLength = 0;
    bool measureAboveMax = false;

    for (MeasureBase* mb = score->measures()->first(); mb != nullptr; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        currentMeasureMaxLength = 0;
        Measure* m = toMeasure(mb);
        // if we are at the beginning of the line
        // we write the measure number
        if (currentLineLenght == 0) {
            TextToUEBBraille textToBraille;
            QString measureNumber = textToBraille.braille(QString::number(m->no() + 1)).remove(0, 1) + " ";
            int measureNumberLen = measureNumber.size();
            line[0] += measureNumber;
            for (int i = 1; i < nrStaves; i++) {
                line[i] += QString("").leftJustified(measureNumberLen);
            }
            currentLineLenght += measureNumberLen;
        }

        if (m->hasMMRest() && score->styleB(Sid::createMultiMeasureRests)) {
            mb = m = m->mmRest();
        }

        for (int i = 0; i < nrStaves; ++i) {
            qDebug() << "Measure " << mb->no() + 1 << " Staff " << i;

            measureBraille[i] = brailleMeasure(m, i).toUtf8();

            if (measureBraille[i].size() > currentMeasureMaxLength) {
                currentMeasureMaxLength = measureBraille[i].size();
            }
        }

        qDebug() << "Current measure max len: " << currentMeasureMaxLength;
        // TODO handle better the case when the size of the current measure
        // by itself is larger than the MAX_CHARS_PER_LINE. The measure will
        // have to be split on multiple lines based on specific rules
        if ((currentMeasureMaxLength + currentLineLenght > MAX_CHARS_PER_LINE) && !measureAboveMax) {
            QTextStream out(dev);
            for (int i = 0; i < nrStaves; ++i) {
                out << line[i].toUtf8() << Qt::endl;
                line[i] = QString();
            }
            out.flush();
            currentLineLenght = 0;
            // We need to re-render the current measure
            // as it will be on a new line.
            mb = mb->prev();
            // 3.2.1. Page 53. Music Braille Code 2015.
            // The octave is always marked for the first note of a braille line
            resetOctaves();
            if (currentMeasureMaxLength >= (MAX_CHARS_PER_LINE - 4)) {
                measureAboveMax = true;
            }
            continue;
        }

        currentLineLenght += currentMeasureMaxLength;
        for (int i = 0; i < nrStaves; ++i) {
            line[i] += measureBraille[i].leftJustified(currentMeasureMaxLength);
            measureBraille[i] = QString();
        }

        if (measureAboveMax || m->sectionBreak()) {
            QTextStream out(dev);
            for (int i = 0; i < nrStaves; ++i) {
                out << line[i].toUtf8() << Qt::endl;
                line[i] = QString();
            }
            currentLineLenght = 0;
            // 3.2.1. Page 53. Music Braille Code 2015.
            // The octave is always marked for the first note of a braille line
            resetOctaves();
            measureAboveMax = false;
            if (m->sectionBreak()) {
                out << Qt::endl;
            }
            out.flush();
        }
    }

    // Write the last measures
    QTextStream out(dev);
    for (int i = 0; i < nrStaves; ++i) {
        out << line[i].toUtf8() << Qt::endl;
        line[i] = QString();
    }
    out.flush();
}

bool ExportBraille::ascendingChords(ClefType clefType)
{
    // 9.2. Direction of Intervals (in Chords). Page 75. Music Braille Code 2015
    // In Treble, Soprano, Alto clefs: Write the upper most note, then rest of notes as intervals downward
    // In Tenor, Baritone, Bass clefs: Write the lower most note, then rest of notes as intervals upward
    switch (clefType) {
    case ClefType::G:              //Treble clef
    case ClefType::G15_MB:         //Treble clef 15ma bassa
    case ClefType::G8_VB:          //Treble clef 8va bassa
    case ClefType::G8_VA:          //Treble clef 8va alta
    case ClefType::G15_MA:         //Treble clef 15ma alta
    case ClefType::G8_VB_O:        //Double treble clef 8va bassa on 2nd line
    case ClefType::G8_VB_P:        //Treble clef optional 8va bassa
    case ClefType::G_1:            //French violin clef
    case ClefType::C1:             //Soprano clef
    case ClefType::C1_F18C:        //Soprano clef (French, 18th century)
    case ClefType::C1_F20C:        //Soprano clef (French, 20th century)
    case ClefType::C2:             //Mezzo-soprano clef
    case ClefType::C3:             //Alto clef
    case ClefType::C3_F18C:        //Alto clef (French, 18th century)
    case ClefType::C3_F20C:        //Alto clef (French, 20th century)
    case ClefType::C_19C:          //C clef, H shape (19th century)
    case ClefType::TAB:            //TAB
    case ClefType::TAB4:           //TAB
    case ClefType::TAB_SERIF:      //TAB
    case ClefType::TAB4_SERIF:     //TAB
        return false;
    case ClefType::C4:             //Tenor clef
    case ClefType::C4_F18C:        //Tenor clef (French, 18th century)
    case ClefType::C4_F20C:        //Tenor clef (French, 20th century)
    case ClefType::C5:             //Baritone clef (C clef)
    case ClefType::F:              //Bass clef
    case ClefType::F_F18C:         //F clef (French, 18th century)
    case ClefType::F_19C:          //F clef (19th century)
    case ClefType::F15_MB:         //Bass clef 15ma bassa
    case ClefType::F8_VB:          //Bass clef 8va bassa
    case ClefType::F_8VA:          //Bass clef 8va alta
    case ClefType::F_15MA:         //Bass clef 15ma alta
    case ClefType::F_B:            //Baritone clef (F clef)
    case ClefType::F_C:            //Subbass clef
        return true;
    case ClefType::PERC:           //Percussion
    case ClefType::PERC2:          //Percussion
        return false;
    default:
        break;
    }
    return false;
}

QString ExportBraille::brailleClef(Clef* clef)
{
    //In Braille, the clef is printed only at it's first appearance.
    if (currentCleffType[clef->staffIdx()] == clef->clefType()) {
        return QString();
    }

    currentCleffType[clef->staffIdx()] = clef->clefType();
    resetOctave(clef->staffIdx());

    switch (clef->clefType()) {
    case ClefType::G:              //Treble clef
        return BRAILLE_TREBLE_CLEF;
    case ClefType::G15_MB:         //Treble clef 15ma bassa
        return BRAILLE_TREBLE_CLEF_15MB;
    case ClefType::G8_VB:          //Treble clef 8va bassa
        return BRAILLE_TREBLE_CLEF_8VB;
    case ClefType::G8_VA:          //Treble clef 8va alta
        return BRAILLE_TREBLE_CLEF_8VA;
    case ClefType::G15_MA:         //Treble clef 15ma alta
        return BRAILLE_TREBLE_CLEF_15MA;
    case ClefType::G8_VB_O:        //Double treble clef 8va bassa on 2nd line
        return BRAILLE_D_TREBLE_CLEF_8VB_O;
    case ClefType::G8_VB_P:        //Treble clef optional 8va bassa
        return BRAILLE_TREBLE_CLEF_8VB_P;
    case ClefType::G_1:            //French violin clef
        return BRAILLE_FRENCH_VIOLINE_CLEF;
    case ClefType::C1:             //Soprano clef
    case ClefType::C1_F18C:        //Soprano clef (French, 18th century)
    case ClefType::C1_F20C:        //Soprano clef (French, 20th century)
        return BRAILLE_SOPRANO_CLEF;
    case ClefType::C2:             //Mezzo-soprano clef
        return BRAILLE_MEZZO_SOPRANO_CLEF;
    case ClefType::C3:             //Alto clef
    case ClefType::C3_F18C:        //Alto clef (French, 18th century)
    case ClefType::C3_F20C:        //Alto clef (French, 20th century)
    case ClefType::C_19C:          //C clef, H shape (19th century)
        return BRAILLE_ALTO_CLEF;
    case ClefType::C4:             //Tenor clef
    case ClefType::C4_F18C:        //Tenor clef (French, 18th century)
    case ClefType::C4_F20C:        //Tenor clef (French, 20th century)
        return BRAILLE_TENOR_CLEF;
    case ClefType::C5:             //Baritone clef (C clef)
        return BRAILLE_BARITONE_C_CLEF;
    case ClefType::F:              //Bass clef
    case ClefType::F_F18C:         //F clef (French, 18th century)
    case ClefType::F_19C:          //F clef (19th century)
        return BRAILLE_BASS_CLEF;
    case ClefType::F15_MB:         //Bass clef 15ma bassa
        return BRAILLE_BASS_CLEF_15MB;
    case ClefType::F8_VB:          //Bass clef 8va bassa
        return BRAILLE_BASS_CLEF_8VB;
    case ClefType::F_8VA:          //Bass clef 8va alta
        return BRAILLE_BASS_CLEF_8VA;
    case ClefType::F_15MA:         //Bass clef 15ma alta
        return BRAILLE_BASS_CLEF_15MA;
    case ClefType::F_B:            //Baritone clef (F clef)
        return BRAILLE_BARITONE_F_CLEF;
    case ClefType::F_C:            //Subbass clef
        return BRAILLE_SUBBASS_CLEF;
    case ClefType::PERC:           //Percussion
    case ClefType::PERC2:          //Percussion
    case ClefType::TAB:            //TAB
    case ClefType::TAB4:           //TAB
    case ClefType::TAB_SERIF:      //TAB
    case ClefType::TAB4_SERIF:     //TAB
        return QString();
    default:
        return QString();
    }
}

QString ExportBraille::brailleTimeSig(TimeSig* timeSig)
{
    if (!timeSig || timeSig->segment()->isTimeSigAnnounceType()) {
        return QString();
    }
    resetOctave(timeSig->staffIdx());

    QString rez;
    QTextStream rezult(&rez);
    if (timeSig->timeSigType() == TimeSigType::FOUR_FOUR) {
        return QString(".c");
    } else if (timeSig->timeSigType() == TimeSigType::ALLA_BREVE) {
        return QString("_c");
    }
#if 0 //ToDo
    else if (timeSig->timeSigType() == TimeSigType::CUT_BACH) {
        return QString("_c"); // this is basically a Cut time, so ALLA_BREVE, but an alternatove glyph for it
    } else if (timeSig->timeSigType() == TimeSigType::CUT_TRIPLE) {
        return QString("#i8 "); // this is an 9/8 timesig with an alternative glyph so "#i8 "?
    }
#endif
    QMap<QString, QString> numeratorToBraille;
    numeratorToBraille["1"] = 'a';
    numeratorToBraille["2"] = 'b';
    numeratorToBraille["3"] = 'c';
    numeratorToBraille["4"] = 'd';
    numeratorToBraille["5"] = 'e';
    numeratorToBraille["6"] = 'f';
    numeratorToBraille["7"] = 'g';
    numeratorToBraille["8"] = 'h';
    numeratorToBraille["9"] = 'i';
    numeratorToBraille["0"] = 'j';

    QMap<QString, QString> denominatorToBraille;
    denominatorToBraille["1"] = '1';
    denominatorToBraille["2"] = '2';
    denominatorToBraille["3"] = '3';
    denominatorToBraille["4"] = '4';
    denominatorToBraille["5"] = '5';
    denominatorToBraille["6"] = '6';
    denominatorToBraille["7"] = '7';
    denominatorToBraille["8"] = '8';
    denominatorToBraille["9"] = '9';
    denominatorToBraille["0"] = '0';

    rezult << "#";

    for (QChar n : QString::number(timeSig->numerator())) {
        rezult << numeratorToBraille[n];
    }

    for (QChar d : QString::number(timeSig->denominator())) {
        rezult << denominatorToBraille[d];
    }

    rezult << " ";
    rezult.flush();
    return rez;
}

QString ExportBraille::brailleMMRest(MMRest* mmRest)
{
    if (!mmRest) {
        return QString();
    }
    // 5.3 Multiple-Measure Rests. Page 58. MBC2015.

    if (mmRest->measure()->mmRestCount() <= 3) {
        return BRAILLE_REST_MEASURE.repeated(mmRest->measure()->mmRestCount());
    }

    resetOctave(mmRest->staffIdx());
    return TextToUEBBraille().braille(QString::number(mmRest->measure()->mmRestCount())) + BRAILLE_REST_MEASURE;
}

QString ExportBraille::brailleRest(Rest* rest)
{
    QString restBraille;
    switch (rest->durationType().type()) {
    case TDuration::DurationType::V_LONG:      restBraille = BRAILLE_REST_LONG;
        break;
    case TDuration::DurationType::V_BREVE:     restBraille = BRAILLE_REST_BREVE;
        break;
    case TDuration::DurationType::V_WHOLE:     restBraille = BRAILLE_REST_WHOLE;
        break;
    case TDuration::DurationType::V_HALF:      restBraille = BRAILLE_REST_HALF;
        break;
    case TDuration::DurationType::V_QUARTER:   restBraille = BRAILLE_REST_QUARTER;
        break;
    case TDuration::DurationType::V_EIGHTH:    restBraille = BRAILLE_REST_EIGHTH;
        break;
    case TDuration::DurationType::V_16TH:      restBraille = BRAILLE_REST_16TH;
        break;
    case TDuration::DurationType::V_32ND:      restBraille = BRAILLE_REST_32ND;
        break;
    case TDuration::DurationType::V_64TH:      restBraille = BRAILLE_REST_64TH;
        break;
    case TDuration::DurationType::V_128TH:     restBraille = BRAILLE_REST_128TH;
        break;
    case TDuration::DurationType::V_256TH:     restBraille = BRAILLE_REST_256TH;
        break;
    case TDuration::DurationType::V_512TH:     restBraille = QString();
        break;                                                                                //not supported in braille
    case TDuration::DurationType::V_1024TH:    restBraille = QString();
        break;                                                                                //not supported in braille
    case TDuration::DurationType::V_ZERO:      restBraille = QString();
        break;                                                                                //not sure what this means
    case TDuration::DurationType::V_MEASURE:   restBraille = BRAILLE_REST_MEASURE;
        break;
    case TDuration::DurationType::V_INVALID:   restBraille = QString();
        break;
    }

    for (int i = 0; i < rest->dots(); ++i) {
        restBraille += BRAILLE_DURATION_DOT;
    }

    QString fermata = QString();
    for (Element* el : rest->segment()->annotations()) {
        if (el->isFermata()) {
            fermata = brailleFermata(toFermata(el));
        }
    }
    std::vector<Slur*> restSlurs = slurs(rest);
    QString slurBrailleBefore = brailleSlurBefore(rest, &restSlurs);
    QString slurBrailleAfter  = brailleSlurAfter(rest, &restSlurs);
    std::vector<Hairpin*> restHairpins = hairpins(rest);
    QString hairpinBrailleBefore = brailleHairpinBefore(rest, &restHairpins);
    QString hairpinBrailleAfter = brailleHairpinAfter(rest, &restHairpins);

    QString tupletBraille = brailleTuplet(rest->tuplet(), rest);

    QString result = QString();
    result += hairpinBrailleBefore;
    result += slurBrailleBefore;
    result += tupletBraille;
    result += restBraille;
    result += slurBrailleAfter;
    result += hairpinBrailleAfter;
    result += fermata;

    return result;
}

QString ExportBraille::brailleBreath(Breath* breath)
{
    if (!breath) {
        return QString();
    }

    switch (breath->symId()) {
    case SymId::breathMarkTick:
    case SymId::breathMarkComma:
    case SymId::breathMarkUpbow:
    case SymId::breathMarkSalzedo:
        return BRAILLE_BREATH;
    case SymId::caesura:
    case SymId::caesuraShort:
    case SymId::caesuraThick:
    case SymId::caesuraCurved:
        return BRAILLE_CAESURA;
    default:
        break;
    }
    return QString();
}

BarLine* ExportBraille::lastBarline(Measure* measure, int track)
{
    for (Segment* seg = measure->last(); seg; seg = seg->prev()) {
        Element* el = seg->element(track);
        if (el && el->isBarLine()) {
            return toBarLine(el);
        }
    }
    return nullptr;
}

BarLine* ExportBraille::firstBarline(Measure* measure, int track)
{
    for (Segment* seg = measure->first(); seg; seg = seg->next()) {
        Element* el = seg->element(track);
        if (el && el->isChordRest()) {
            return nullptr;
        }
        if (el && el->isBarLine()) {
            return toBarLine(el);
        }
    }
    return nullptr;
}

QString ExportBraille::brailleMeasure(Measure* measure, int staffCount)
{
    QString rez;
    QTextStream out(&rez);

    //Render all repeats and jumps that are on the left
    for (Element* el : measure->el()) {
        if (el->isMarker()) {
            Marker* marker = toMarker(el);
            if (marker->tid() == Tid::REPEAT_LEFT) {
                out << brailleMarker(toMarker(el));
            }
        }
        if (el->isJump()) {
            Jump* jump = toJump(el);
            if (jump->tid() == Tid::REPEAT_LEFT) {
                out << brailleJump(toJump(el));
            }
        }
    }

    auto spanners = score->spannerMap().findOverlapping(measure->tick().ticks(), measure->endTick().ticks());
    for (auto interval : spanners) {
        Spanner* s = interval.value;
        if (s && s->isVolta()) {
            out << brailleVolta(measure, toVolta(s), staffCount);
        }
    }

    //Render everything that is in Voice 1
    for (auto seg = measure->first(); seg; seg = seg->next()) {
        for (Element* annotation : seg->annotations()) {
            if (annotation->isTempoText()) {
                out << brailleTempoText(toTempoText(annotation), staffCount);
            }
            if (annotation->track() == staffCount * VOICES) {
                if (annotation->isDynamic()) {
                    out << brailleDynamic(toDynamic(annotation));
                }
            }
        }

        Element* el = seg->element(staffCount * VOICES);
        if (!el) {
            continue;
        }

        if (el->isClef()) {
            out << brailleClef(toClef(el));
        } else if (el->isKeySig()) {
            out << brailleKeySig(toKeySig(el));
        } else if (el->isTimeSig()) {
            out << brailleTimeSig(toTimeSig(el));
        } else if (el->isMMRest()) {
            out << brailleMMRest(toMMRest(el));
        } else if (el->isRest()) {
            out << brailleRest(toRest(el));
        } else if (el->isChord()) {
            out << brailleChord(toChord(el));
        } else if (el->isBreath()) {
            out << brailleBreath(toBreath(el));
        } else if (el->isBarLine() && toBarLine(el) != lastBarline(measure, el->track())) {
            out << brailleBarline(toBarLine(el));
        } else if (el->isMeasureRepeat()) {
            out << brailleMeasureRepeat(toMeasureRepeat(el));
        }
    }

    // Render the rest of the voices
    for (int i = 1; i < VOICES; ++i) {
        if (measure->hasVoice(staffCount * VOICES + i)) {
            // 11.1.1. Page 87. Music Braille Code 2015.
            // All voices must be complete when writting the other voices in Braille.
            // We exchange the voices to voice 0 and back for MuseScore to add the missing beats as rests
            // Then we undo the change, so we don't have an altered score.
            // TODO: Braille dot 5 should be put before the rests that appear in Braille, but are not originally in the score
            score->deselectAll();
            score->select(measure, SelectType::RANGE, staffCount);
            score->update();
            score->startCmd();
            score->cmdExchangeVoice(0, i);
            score->endCmd();
            score->startCmd();
            score->cmdExchangeVoice(0, i);
            score->endCmd();

            resetOctave(staffCount);
            out << BRAILLE_FULL_MEASURE_IN_ACORD;
            for (auto seg = measure->first(); seg; seg = seg->next()) {
                Element* el = seg->element(staffCount * VOICES + i);
                if (!el) {
                    continue;
                }

                if (el->isRest()) {
                    out << brailleRest(toRest(el));
                }
                if (el->isChord()) {
                    out << brailleChord(toChord(el));
                }
                if (el->isBreath()) {
                    out << brailleBreath(toBreath(el));
                }
            }
            resetOctave(staffCount);

            // Undo filling the missing beats with rests, so we don't have an altered score.
            score->undoRedo(true, nullptr);
            score->undoRedo(true, nullptr);
            score->deselectAll();
        }
    }

    //Render the barline
    out << brailleBarline(lastBarline(measure, staffCount * VOICES));

    //Render repeats and jumps that are on the right
    for (Element* el : measure->el()) {
        if (el->isMarker()) {
            Marker* marker = toMarker(el);
            if (marker->tid() == Tid::REPEAT_RIGHT) {
                out << brailleMarker(toMarker(el));
            }
        }
        if (el->isJump()) {
            Jump* jump = toJump(el);
            if (jump->tid() == Tid::REPEAT_RIGHT) {
                out << brailleJump(toJump(el));
            }
        }
    }
    out.flush();
    return rez;
}

QString ExportBraille::brailleOctave(int octave)
{
    //Table 9. Page 7. Music Braille Code 2015.
    if (octave < 1) {
        return BRAILLE_BELOW_FIRST_OCTAVE;
    }
    if (octave == 1) {
        return BRAILLE_1ST_OCTAVE;
    }
    if (octave == 2) {
        return BRAILLE_2ND_OCTAVE;
    }
    if (octave == 3) {
        return BRAILLE_3RD_OCTAVE;
    }
    if (octave == 4) {
        return BRAILLE_4TH_OCTAVE;
    }
    if (octave == 5) {
        return BRAILLE_5TH_OCTAVE;
    }
    if (octave == 6) {
        return BRAILLE_6TH_OCTAVE;
    }
    if (octave == 7) {
        return BRAILLE_7TH_OCTAVE;
    }
    if (octave > 7) {
        return BRAILLE_ABOVE_7TH_OCTAVE;
    }

    return QString();
}

QString ExportBraille::brailleAccidentalType(AccidentalType accidental)
{
    switch (accidental) {
    case AccidentalType::NONE:               return QString();
    case AccidentalType::FLAT:               return BRAILLE_ACC_FLAT;
    case AccidentalType::NATURAL:            return BRAILLE_ACC_NATURAL;
    case AccidentalType::SHARP:              return BRAILLE_ACC_SHARP;
    case AccidentalType::SHARP2:             return BRAILLE_ACC_SHARP + BRAILLE_ACC_SHARP;
    case AccidentalType::FLAT2:              return BRAILLE_ACC_FLAT + BRAILLE_ACC_FLAT;
    case AccidentalType::NATURAL_FLAT:       return BRAILLE_ACC_NATURAL + BRAILLE_ACC_FLAT;     //extrapollated
    case AccidentalType::NATURAL_SHARP:      return BRAILLE_ACC_NATURAL + BRAILLE_ACC_SHARP;     //extrapollated
    case AccidentalType::SHARP_SHARP:        return BRAILLE_ACC_SHARP + BRAILLE_ACC_SHARP;
    case AccidentalType::FLAT_ARROW_UP:      return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::FLAT_ARROW_DOWN:    return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::NATURAL_ARROW_UP:   return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::NATURAL_ARROW_DOWN: return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::SHARP_ARROW_UP:     return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::SHARP_ARROW_DOWN:   return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::SHARP2_ARROW_UP:    return BRAILLE_ACC_5QUARTER_STEP + BRAILLE_ACC_SHARP;     //extrapollated. not sure
    case AccidentalType::SHARP2_ARROW_DOWN:  return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::FLAT2_ARROW_UP:     return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::FLAT2_ARROW_DOWN:   return BRAILLE_ACC_5QUARTER_STEP + BRAILLE_ACC_FLAT;     //extrapollated. not sure
    case AccidentalType::MIRRORED_FLAT:      return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::MIRRORED_FLAT2:     return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_FLAT;
    case AccidentalType::SHARP_SLASH:        return BRAILLE_ACC_QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::SHARP_SLASH4:       return BRAILLE_ACC_3QUARTER_STEP + BRAILLE_ACC_SHARP;
    case AccidentalType::FLAT_SLASH2:        return BRAILLE_ACC_FLAT;
    case AccidentalType::FLAT_SLASH:         return BRAILLE_ACC_FLAT;
    case AccidentalType::SHARP_SLASH3:       return BRAILLE_ACC_SHARP;
    case AccidentalType::SHARP_SLASH2:       return BRAILLE_ACC_SHARP;
    //TODO the rest of the accidentals
    default:
        break;
    }
    return QString();
}

QString ExportBraille::brailleNote(QString pitchName, TDuration::DurationType durationType, int dots)
{
    QString noteBraille = QString();
    static QMap<TDuration::DurationType, QMap<QString, QString> > noteToBraille;
    if (noteToBraille.isEmpty()) {
        //8th and 128th notes have the same representation in Braille
        noteToBraille[TDuration::DurationType::V_128TH]["C"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["C"] = BRAILLE_C_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["D"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["D"] = BRAILLE_D_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["E"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["E"] = BRAILLE_E_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["F"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["F"] = BRAILLE_F_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["G"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["G"] = BRAILLE_G_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["A"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["A"] = BRAILLE_A_8TH_128TH;
        noteToBraille[TDuration::DurationType::V_128TH]["B"] = noteToBraille[TDuration::DurationType::V_EIGHTH]["B"] = BRAILLE_B_8TH_128TH;

        //64th and quarter notes have the same representation in Braille
        noteToBraille[TDuration::DurationType::V_64TH]["C"] = noteToBraille[TDuration::DurationType::V_QUARTER]["C"]
                                                                  = BRAILLE_C_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["D"] = noteToBraille[TDuration::DurationType::V_QUARTER]["D"]
                                                                  = BRAILLE_D_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["E"] = noteToBraille[TDuration::DurationType::V_QUARTER]["E"]
                                                                  = BRAILLE_E_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["F"] = noteToBraille[TDuration::DurationType::V_QUARTER]["F"]
                                                                  = BRAILLE_F_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["G"] = noteToBraille[TDuration::DurationType::V_QUARTER]["G"]
                                                                  = BRAILLE_G_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["A"] = noteToBraille[TDuration::DurationType::V_QUARTER]["A"]
                                                                  = BRAILLE_A_64TH_QUARTER;
        noteToBraille[TDuration::DurationType::V_64TH]["B"] = noteToBraille[TDuration::DurationType::V_QUARTER]["B"]
                                                                  = BRAILLE_B_64TH_QUARTER;

        //32nd and half notes have the same representation in Braille
        noteToBraille[TDuration::DurationType::V_32ND]["C"] = noteToBraille[TDuration::DurationType::V_HALF]["C"] = BRAILLE_C_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["D"] = noteToBraille[TDuration::DurationType::V_HALF]["D"] = BRAILLE_D_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["E"] = noteToBraille[TDuration::DurationType::V_HALF]["E"] = BRAILLE_E_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["F"] = noteToBraille[TDuration::DurationType::V_HALF]["F"] = BRAILLE_F_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["G"] = noteToBraille[TDuration::DurationType::V_HALF]["G"] = BRAILLE_G_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["A"] = noteToBraille[TDuration::DurationType::V_HALF]["A"] = BRAILLE_A_32ND_HALF;
        noteToBraille[TDuration::DurationType::V_32ND]["B"] = noteToBraille[TDuration::DurationType::V_HALF]["B"] = BRAILLE_B_32ND_HALF;

        //16th and whole notes have the same representation in Braille.
        // Breve has the same representation, but with an extra suffix;
        // 256th has the same represantation, but with an extra prefix;
        noteToBraille[TDuration::DurationType::V_256TH]["C"] = noteToBraille[TDuration::DurationType::V_16TH]["C"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["C"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["C"]
                                                                       = BRAILLE_C_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["D"] = noteToBraille[TDuration::DurationType::V_16TH]["D"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["D"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["D"]
                                                                       = BRAILLE_D_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["E"] = noteToBraille[TDuration::DurationType::V_16TH]["E"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["E"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["E"]
                                                                       = BRAILLE_E_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["F"] = noteToBraille[TDuration::DurationType::V_16TH]["F"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["F"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["F"]
                                                                       = BRAILLE_F_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["G"] = noteToBraille[TDuration::DurationType::V_16TH]["G"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["G"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["G"]
                                                                       = BRAILLE_G_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["A"] = noteToBraille[TDuration::DurationType::V_16TH]["A"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["A"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["A"]
                                                                       = BRAILLE_A_16TH_WHOLE;

        noteToBraille[TDuration::DurationType::V_256TH]["B"] = noteToBraille[TDuration::DurationType::V_16TH]["B"]
                                                                   =noteToBraille[TDuration::DurationType::V_WHOLE]["B"]
                                                                     = noteToBraille[TDuration::DurationType::V_BREVE]["B"]
                                                                       = BRAILLE_B_16TH_WHOLE;
    }

    switch (durationType) {
    case TDuration::DurationType::V_LONG:      break;     //TODO
    case TDuration::DurationType::V_BREVE:
        noteBraille = noteToBraille[TDuration::DurationType::V_BREVE][pitchName] + BRAILLE_BREVE_SUFFIX;
        break;
    case TDuration::DurationType::V_WHOLE:
    case TDuration::DurationType::V_HALF:
    case TDuration::DurationType::V_QUARTER:
    case TDuration::DurationType::V_EIGHTH:
    case TDuration::DurationType::V_16TH:
    case TDuration::DurationType::V_32ND:
    case TDuration::DurationType::V_64TH:
    case TDuration::DurationType::V_128TH:
        noteBraille = QString(noteToBraille[durationType][pitchName]);
        break;
    case TDuration::DurationType::V_256TH:
        noteBraille = BRAILLE_256TH_PREFIX + QString(noteToBraille[TDuration::DurationType::V_256TH][pitchName]);
        break;
    case TDuration::DurationType::V_512TH:     break;     //TODO not supported in braille?
    case TDuration::DurationType::V_1024TH:    break;     //TODO not supported in braille?
    case TDuration::DurationType::V_ZERO:      break;     //TODO
    case TDuration::DurationType::V_MEASURE:   break;     //TODO
    case TDuration::DurationType::V_INVALID:   break;
    }

    for (int i = 0; i < dots; ++i) {
        noteBraille += BRAILLE_DURATION_DOT;
    }

    return noteBraille;
}

QString ExportBraille::brailleChordRootNote(Chord* chord, Note* rootNote)
{
    QString pitchName;
    QString accidental; //TODO, do we need this for anything?
    QString octaveBraille = QString();
    tpc2name(rootNote->tpc(), NoteSpellingType::STANDARD, NoteCaseType::UPPER, pitchName, accidental);
    QString noteBraille = brailleNote(pitchName, chord->durationType().type(), chord->dots());

    int octave = rootNote->octave();
    // 3.2.1. Page 53. Music Braille Code 2015.
    // Octave signs at the beginning of the line or when the melodic line is interrupted by specific elements
    if (!this->previousNote[chord->staffIdx()]) {
        octaveBraille = brailleOctave(octave);
    }
    // 3.2.2. Octave signs in a melodic progression. Page 53. Music Braille Code 2015.
    // a. the octave is not marked for the second of two consecutive notes if the interval is less than a fourth
    // b. the octave is always marked in a skip greater than a fifth
    // c. the octave is only marked in a skip of a fourth or fifth when the second note is in a different octave from the first.
    else {
        int interval = computeInterval(previousNote[chord->staffIdx()], rootNote, false);
        if (interval > 5 || (interval > 3 && (previousNote[chord->staffIdx()]->octave() != rootNote->octave()))) {
            octaveBraille = brailleOctave(octave);
        }
    }
    previousNote[chord->staffIdx()] = rootNote;

    QString accidentalBraille = QString();
    if (rootNote->accidental()) {
        accidentalBraille = brailleAccidentalType(rootNote->accidental()->accidentalType());
    }

    QString noteTieBraille = brailleTie(rootNote);

    QString fingeringAfterBraille = QString();
    for (Element* el : rootNote->el()) {
        if (el->isFingering()) {
            fingeringAfterBraille += brailleFingeringAfter(toFingering(el));
        }
    }

    return
        accidentalBraille
        + octaveBraille
        + noteBraille
        + fingeringAfterBraille
        + noteTieBraille;
}

int ExportBraille::computeInterval(Note* note1, Note* note2, bool ignoreOctave)
{
    // TODO is it possible to not have pitch?
    // if yes, then this algorithm will fail in that case

    QList<QString> initNotes;
    if (note1->epitch() < note2->epitch()) {
        initNotes << "A" << "B" << "C" << "D" << "E" << "F" << "G";
    } else {
        initNotes << "G" << "F" << "E" << "D" << "C" << "B" << "A";
    }

    QString note1PitchName;
    QString note2PitchName;
    QString accidental; //We don't need this, but tpc2name requires it
    tpc2name(note1->tpc(), NoteSpellingType::STANDARD, NoteCaseType::UPPER, note1PitchName, accidental);
    tpc2name(note2->tpc(), NoteSpellingType::STANDARD, NoteCaseType::UPPER, note2PitchName, accidental);

    QList<QString> notes;
    int rootNodeIdx = initNotes.indexOf(note1PitchName);
    for (int i = rootNodeIdx; i < initNotes.size(); i++) {
        notes << initNotes.at(i);
    }
    for (int i = 0; i < rootNodeIdx; i++) {
        notes << initNotes.at(i);
    }

    int interval = notes.indexOf(note2PitchName) + 1;
    if (!ignoreOctave && abs(note1->epitch() - note2->epitch()) >= 12) {
        interval += (abs(note1->epitch() - note2->epitch()) / 12) * 8 - 1;
    }

    return interval;
}

//-----------------------------------------------------------------------------------
//   brailleChordInterval
//    rootNote: The root note of the chord.
//    notes: The notes in the chord in the order that they are brailled
//    note:  The note for which we are currently representing the braille interval
//    return the braille interval representation in the chord
//-----------------------------------------------------------------------------------
QString ExportBraille::brailleChordInterval(Note* rootNote, QList<Note*>* notes, Note* note)
{
    if (!rootNote || !note || notes->size() < 2) {
        return QString();
    }

    int interval = computeInterval(rootNote, note, true);
    QString intervalBraille;
    //Table 9. Intervals. Page 7. Music Braille Code 2015
    switch (interval) {
    case 1: intervalBraille = BRAILLE_OCTAVE;
        break;
    case 2: intervalBraille = BRAILLE_SECOND;
        break;
    case 3: intervalBraille = BRAILLE_THIRD;
        break;
    case 4: intervalBraille = BRAILLE_FOURTH;
        break;
    case 5: intervalBraille = BRAILLE_FIFTH;
        break;
    case 6: intervalBraille = BRAILLE_SIXTH;
        break;
    case 7: intervalBraille = BRAILLE_SEVENTH;
        break;
    }
    QString noteOctaveBraille = QString();
    QString noteAccidentalBraille = QString();
    if (note->accidental() != nullptr) {
        noteAccidentalBraille = brailleAccidentalType(note->accidental()->accidentalType());
    }

    // 9.1.1. Page 74. Music Braille Code 2015.
    // A note represented by an interval sign must be preceded by the appropriate octave mark when:
    // (a) it forms a unison with the written note (the octave-interval sign is used with the appropriate octave mark),
    if (interval == 1 && rootNote->octave() == note->octave()) {
        noteOctaveBraille = brailleOctave(note->octave());
    }
    int noteIdx = notes->indexOf(note);
    int intervalWithPreviousNoteInChord = computeInterval(notes->at(noteIdx - 1), note, false);
    // (b) it is the first or only interval and is more than an octave from the written note,
    if (noteIdx == 1 && intervalWithPreviousNoteInChord > 8) {
        noteOctaveBraille = brailleOctave(note->octave());
    }
    // (c) it is the second of two adjacent intervals that are an octave or more apart, or
    if (noteIdx != 1 && intervalWithPreviousNoteInChord >= 8) {
        noteOctaveBraille = brailleOctave(note->octave());
    }
    // (d) it forms a unison with another interval that has been given previously in the chord.
    if (noteIdx != 1 && intervalWithPreviousNoteInChord == 1) {
        noteOctaveBraille = brailleOctave(note->octave());
    }

    QString noteTieBraille = brailleTie(note);

    QString fingeringAfterBraille = QString();
    for (Element* el : rootNote->el()) {
        if (el->isFingering()) {
            fingeringAfterBraille += brailleFingeringAfter(toFingering(el));
        }
    }

    return
        noteAccidentalBraille
        + noteOctaveBraille
        + intervalBraille
        + fingeringAfterBraille
        + noteTieBraille;
}

QString ExportBraille::brailleChord(Chord* chord)
{
    // 9.3 Interval Doubling and 9.5 Moving notes methods are not implemented as they are optional methods

    if (!chord || chord->notes().empty()) {
        return QString();
    }

    QString graceNoteMarking = brailleGraceNoteMarking(chord);
    QString graceNotesBefore = QString();
    for (Chord* graceChord : chord->graceNotesBefore()) {
        graceNotesBefore += brailleChord(graceChord);
    }

    QString tupletBraille = brailleTuplet(chord->tuplet(), chord);

    std::vector<Hairpin*> chordHairpins = hairpins(chord);
    QString hairpinBrailleBefore = brailleHairpinBefore(chord, &chordHairpins);

    // 9.2. Direction of Intervals (in Chords). Page 75. Music Braille Code 2015
    // In Treble, Soprano, Alto clefs: Write the upper most note, then rest of notes as intervals downward
    // In Tenor, Baritone, Bass clefs: Write the lower most note, then rest of notes as intervals upward
    QList<Note*> notes;
    if (ascendingChords(currentCleffType[chord->staffIdx()])) {
        for (auto it = chord->notes().begin(); it != chord->notes().end(); ++it) {
            notes.push_back(*it);
        }
    } else {
        for (auto it = chord->notes().rbegin(); it != chord->notes().rend(); ++it) {
            notes.push_back(*it);
        }
    }

    Note* rootNote = notes.front();
    QString rootNoteBraille = brailleChordRootNote(chord, rootNote);

    QString intervals = QString();
    for (auto it = notes.begin() + 1; it != notes.end(); ++it) {
        intervals += brailleChordInterval(rootNote, &notes, *it);
    }

    QString graceNotesAfter = QString();
    for (Chord* graceChord : chord->graceNotesAfter()) {
        graceNotesAfter  += brailleChord(graceChord);
    }

    QString chordTieBraille = brailleTie(chord);

    QString fermata = QString();
    for (Element* el : chord->segment()->annotations()) {
        if (el->isFermata()) {
            fermata = brailleFermata(toFermata(el));
        }
    }

    QString articulationsBraille = QString();
    for (Articulation* artic : chord->articulations()) {
        articulationsBraille += brailleArticulation(artic);
    }

    std::vector<Slur*> chordSlurs = slurs(chord);
    QString slurBrailleBefore = brailleSlurBefore(chord, &chordSlurs);
    QString slurBrailleAfter  = brailleSlurAfter(chord, &chordSlurs);

    QString hairpinBrailleAfter = brailleHairpinAfter(chord, &chordHairpins);

    QString arpeggio = brailleArpeggio(chord->arpeggio());
    QString tremolloBraille = brailleTremolo(chord);
    QString glissandoLastNoteBraille = brailleGlissando(notes.back());

    // In Braille the order of elements is clearly defined
    QString result = QString();
    result += hairpinBrailleBefore;
    result += graceNotesBefore;
    result += graceNoteMarking;
    result += arpeggio;
    result += articulationsBraille;
    result += slurBrailleBefore;
    result += tupletBraille;
    result += rootNoteBraille;
    result += intervals;
    result += tremolloBraille;
    result += chordTieBraille;
    result += slurBrailleAfter;
    result += glissandoLastNoteBraille;
    /* not really sure where grace notes that follow the chord should be put
     * MBC 2015, doesn't mention them, so tentatively putting them here. */
    result += graceNotesAfter;
    result += hairpinBrailleAfter;
    result += fermata;

    return result;
}

QString ExportBraille::brailleGraceNoteMarking(Chord* chord)
{
    // 16.2.1 - 16.2.3. Page 111 - 112. Music Braille Code 2015.
    // TODO: doubling the grace note sign where there are more than 4 grace notes.
    //       the book is not very clear how this doubling is done and how the user
    //       understands where do grace notes end.
    if (!chord->isGrace() || !chord->parent()->isChord()) {
        return QString();
    }

    QString result = QString();
    if (chord->stemSlash()) {
        result = BRAILLE_GRACE_SHORT;
    } else {
        result = BRAILLE_GRACE_LONG;
    }
    return result;
}

QString ExportBraille::brailleBarline(BarLine* barline)
{
    if (!barline) {
        return QString();
    }

    if (barline->measure()->system()
        && barline->measure()->system()->firstMeasure()
        && barline->measure()->system()->firstMeasure()->segments().first() == barline->segment()
        && barline->barLineType() == BarLineType::NORMAL) {
        // This is the barline at the beginning of the system
        // We don't need to render it
        return QString();
    }

    bool spaceAfter = false;
    bool spaceBefore = false;
    QString barlineBraille = QString();

    switch (barline->barLineType()) {
    // Table 1. Page 2.  Music Braille Code 2015
    case BarLineType::NORMAL:
        barlineBraille = BRAILLE_BARLINE;
        break;
    case BarLineType::BROKEN:
        barlineBraille = BRAILLE_BARLINE_BROKEN;
        spaceBefore = true;
        spaceAfter = true;
        break;
    case BarLineType::DOUBLE:
        barlineBraille = BRAILLE_BARLINE_DOUBLE;
        resetOctave(barline->staffIdx());
        spaceAfter = true;
        break;
    case BarLineType::END:
        resetOctave(barline->staffIdx());
        barlineBraille = BRAILLE_BARLINE_END;
        spaceAfter = true;
        break;
    case BarLineType::DOTTED:
        barlineBraille = BRAILLE_BARLINE_DOTTED;
        spaceBefore = true;
        spaceAfter = true;
        break;
    // Start/End Repeat Table 17. Page 15. Music Braille Code 2015
    case BarLineType::START_REPEAT:
        resetOctave(barline->staffIdx());
        barlineBraille = BRAILLE_BARLINE_START_REPEAT;
        break;
    case BarLineType::END_REPEAT:
        resetOctave(barline->staffIdx());
        barlineBraille = BRAILLE_BARLINE_END_REPEAT;
        spaceAfter = true;
        break;
    //This sign was not present in the book. Extrapolated from the above two signs
    case BarLineType::END_START_REPEAT:
        barlineBraille = BRAILLE_BARLINE_START_REPEAT + " " + BRAILLE_BARLINE_END_REPEAT;
        break;
    case BarLineType::REVERSE_END:
    //case BarLineType::REVERSE_FINALE: // duplicate of the above, alternative name
    case BarLineType::HEAVY:
    case BarLineType::DOUBLE_HEAVY:
        break; // ToDo ?!
    }

    QString fermataBraille = QString();
    for (Element* el : barline->segment()->annotations()) {
        if (el->isFermata()) {
            fermataBraille = brailleFermata(toFermata(el));
        }
    }

    //This is a special case
    if (!fermataBraille.isEmpty() && barline->barLineType() == BarLineType::NORMAL) {
        barlineBraille = BRAILLE_BARLINE_DOTTED;
        spaceBefore = true;
        spaceAfter = true;
    }

    QString musicalHypenBrailleBefore = QString();
    QString musicalHypenBrailleAfter = QString();

    // 1.10.1. Page 43. Music Braille Code 2015.
    if ((barline->barLineType() == BarLineType::BROKEN || barline->barLineType() == BarLineType::DOTTED)
        && barline != lastBarline(barline->measure(), barline->track())) {
        musicalHypenBrailleBefore = BRAILLE_MUSICAL_HYPEN;
    }

    // 17.1. Page 120. Music Braille Code 2015.
    if ((barline->barLineType() == BarLineType::START_REPEAT || barline->barLineType() == BarLineType::END_START_REPEAT)
        && barline != firstBarline(barline->measure(), barline->track())) {
        spaceBefore = true;
        musicalHypenBrailleBefore = BRAILLE_MUSICAL_HYPEN;
    }

    // 17.1. Page 120. Music Braille Code 2015.
    if ((barline->barLineType() == BarLineType::END_REPEAT || barline->barLineType() == BarLineType::END_START_REPEAT)
        && barline != lastBarline(barline->measure(), barline->track())) {
        musicalHypenBrailleAfter = BRAILLE_MUSICAL_HYPEN;
    }

    return
        musicalHypenBrailleBefore
        + (spaceBefore ? " " : "")
        + barlineBraille
        + fermataBraille
        + (spaceAfter ? " " : "")
        + musicalHypenBrailleAfter;
}

QString ExportBraille::brailleDynamic(Dynamic* dynamic)
{
    if (!dynamic || dynamic->plainText().isEmpty()) {
        return QString();
    }

    resetOctave(dynamic->staffIdx());
    // Table 22C. Page 19. Music Braille Code 2015
    return ">" + TextToUEBBraille().braille(dynamic->plainText());
}

QString ExportBraille::brailleTempoText(TempoText* tempoText, int staffIdx)
{
    if (!tempoText) {
        return QString();
    }
    resetOctave(staffIdx);
    // Sections 1.8 and 1.8.1. Page 42. Music Braille Code 2015.
    TDuration t1;
    TDuration t2;
    int len1;
    int len2;
    QString text = tempoText->plainText();
    QString firstPart = text.split(" = ").first();
    QString secondPart = text.split(" = ").back();
    int x1 = TempoText::findTempoDuration(firstPart, len1, t1);
    int x2 = TempoText::findTempoDuration(secondPart, len2, t2);

    if (x1 != -1) {
        QString dots1;
        QString dots2;
        dots1 = brailleNote("C", t1.type(), t1.dots());
        if (x2 != -1) {
            dots2 = brailleNote("C", t2.type(), t2.dots());
            QString result = BRAILLE_MUSIC_PARENTHESES + dots1 + BRAILLE_EQUALS_METRONOME + dots2 + BRAILLE_MUSIC_PARENTHESES;
            result += " ";
            return result;
        } else {
            QString result = BRAILLE_MUSIC_PARENTHESES + dots1 + BRAILLE_EQUALS_METRONOME + TextToUEBBraille().braille(secondPart)
                             + BRAILLE_MUSIC_PARENTHESES;
            result += " ";
            return result;
        }
    } else {
        return ">" + TextToUEBBraille().braille(text.toLower()) + "'";
    }
}

QString ExportBraille::brailleKeySig(KeySig* keySig)
{
    //In Braille, the key signature is printed only at it's first appearance.
    //Paragraf 6.5. Page 61. Music Braille Code 2015.
    if (!keySig || keySig->segment()->isKeySigAnnounceType()
        || (currentKey[keySig->staffIdx()] == keySig->key() && !keySig->isCustom())) {
        return QString();
    }

    currentKey[keySig->staffIdx()] = keySig->key();
    resetOctave(keySig->staffIdx());

    QString brailleKeySig = QString();
    if (keySig->isCustom()) {
        //Sectioon 6.5.1. Page 62. Music Braille Code 2015.
        //TODO
    } else {
        //Section 6.5. Page 61. Music Braille Code 2015.
        //Table 6. Page 5. Music Braille Code 2015.
        switch (keySig->key()) {
        case Key::C_B: brailleKeySig = QString(TextToUEBBraille().braille("7") + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::G_B: brailleKeySig = QString(TextToUEBBraille().braille("6") + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::D_B: brailleKeySig = QString(TextToUEBBraille().braille("5") + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::A_B: brailleKeySig = QString(TextToUEBBraille().braille("4") + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::E_B: brailleKeySig = QString(brailleAccidentalType(AccidentalType::FLAT) + brailleAccidentalType(
                                                   AccidentalType::FLAT) + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::B_B: brailleKeySig = QString(brailleAccidentalType(AccidentalType::FLAT) + brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::F:   brailleKeySig = QString(brailleAccidentalType(AccidentalType::FLAT));
            break;
        case Key::C:   return QString();
        case Key::G:   brailleKeySig = QString(brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::D:   brailleKeySig = QString(brailleAccidentalType(AccidentalType::SHARP) + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::A:   brailleKeySig = QString(brailleAccidentalType(AccidentalType::SHARP) + brailleAccidentalType(
                                                   AccidentalType::SHARP) + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::E:   brailleKeySig = QString(TextToUEBBraille().braille("4") + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::B:   brailleKeySig = QString(TextToUEBBraille().braille("5") + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::F_S: brailleKeySig = QString(TextToUEBBraille().braille("6") + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::C_S: brailleKeySig = QString(TextToUEBBraille().braille("7") + brailleAccidentalType(AccidentalType::SHARP));
            break;
        case Key::INVALID: return QString();
        case Key::NUM_OF:  return QString();     //TODO What is this?
        case Key::DELTA_ENHARMONIC: return QString();     //TODO What is this?
        }
    }

    if (keySig->segment()->prev() && !keySig->prevSegmentElement()->isClef()) {
        brailleKeySig = " " + brailleKeySig;
    }

    if (!keySig->nextSegmentElement()->isTimeSig()) {
        brailleKeySig += " ";
    }

    return brailleKeySig;
}

QString ExportBraille::brailleTuplet(Tuplet* tuplet, DurationElement* el)
{
    if (tuplet == nullptr || *tuplet->elements().begin() != el) {
        return QString();
    }

    return QString("_") + QString::number(tuplet->ratio().numerator()) + QString("'");
}

QString ExportBraille::brailleTie(Chord* chord)
{
    // This is not a chord
    if (!chord || chord->notes().size() < 2) {
        return QString();
    }

    int nrTies = 0;
    Tie* firstTie = nullptr;
    for (Note* note : chord->notes()) {
        if (note->tieFor() != nullptr) {
            if (firstTie == nullptr) {
                firstTie = note->tieFor();
            }
            nrTies++;
        }
    }

    // There are no notes in this chord with ties
    // or there is just one note with tie
    if (nrTies < 2) {
        return QString();
    }

    // There is at least one tie in the chord
    Chord* nextChord = firstTie->endNote()->chord();
    for (Note* chord1Note : chord->notes()) {
        for (Note* chord2Note : nextChord->notes()) {
            if (chord1Note->epitch() == chord2Note->epitch()
                && (!chord1Note->tieFor()
                    || chord1Note->tieFor() != chord2Note->tieBack())) {
                // Not all notes with the same pitch are tied in the
                // two chords, so we cannot use the chord tie symbol;
                // Each note that is tied will receive individual note tie symbol.
                // 10.2-3. Page 83. Music Braille Code 2015.
                return QString();
            }
        }
    }

    return BRAILLE_TIE_CHORD;
}

bool ExportBraille::hasTies(ChordRest* chordRest)
{
    if (chordRest->isRest()) {
        return false;
    }

    Chord* chord = toChord(chordRest);
    for (Note* note : chord->notes()) {
        if (note->tieFor()) {
            return true;
        }
    }

    return false;
}

QString ExportBraille::brailleTie(Note* note)
{
    if (!note || !note->tieFor() || !brailleTie(note->chord()).isEmpty()) {
        return QString();
    }

    return BRAILLE_TIE;
}

QString ExportBraille::brailleArpeggio(Arpeggio* arpeggio)
{
    if (!arpeggio) {
        return QString();
    }

    // Table 22. Page 17. Music Braille Code 2015.
    QString result = QString();
    // fix me - this is a simple heuristic. can we do better?
    if (arpeggio->height() > 200) {
        result += BRAILLE_ARPEGGIO_MULTISTAVES;
    }

    if (arpeggio->arpeggioType() == ArpeggioType::NORMAL
        || arpeggio->arpeggioType() == ArpeggioType::UP
        || arpeggio->arpeggioType() == ArpeggioType::UP_STRAIGHT
        || arpeggio->arpeggioType() == ArpeggioType::BRACKET) {
        result += BRAILLE_ARPEGGIO_UPWARD;
    }

    if (arpeggio->arpeggioType() == ArpeggioType::DOWN
        || arpeggio->arpeggioType() == ArpeggioType::DOWN_STRAIGHT) {
        result += BRAILLE_ARPEGGIO_DOWNWARD;
    }

    return result;
}

QString ExportBraille::brailleTremolo(Chord* chord)
{
    if (!chord->tremolo() || chord != chord->tremolo()->chord1()) {
        return QString();
    }

    switch (chord->tremolo()->tremoloType()) {
    case TremoloType::R8:  return BRAILLE_TREMOLO_8THS;
    case TremoloType::R16: return BRAILLE_TREMOLO_16THS;
    case TremoloType::R32: return BRAILLE_TREMOLO_32NDS;
    case TremoloType::R64: return BRAILLE_TREMOLO_64THS;
    case TremoloType::C8:  return BRAILLE_TREMOLO_8THS_ALT;
    case TremoloType::C16: return BRAILLE_TREMOLO_16THS_ALT;
    case TremoloType::C32: return BRAILLE_TREMOLO_32NDS_ALT;
    case TremoloType::C64: return BRAILLE_TREMOLO_64THS_ALT;
    case TremoloType::BUZZ_ROLL: return QString();     //TODO
    case TremoloType::INVALID_TREMOLO: return QString();
    }
    return QString();
}

QString ExportBraille::brailleArticulation(Articulation* artic)
{
    switch (artic->symId()) {
    case SymId::articAccentAbove:
    case SymId::articAccentBelow:
        return BRAILLE_ARTIC_ACCENT;
    case SymId::articStaccatoAbove:
    case SymId::articStaccatoBelow:
        return BRAILLE_ARTIC_STACCATO;
    case SymId::articStaccatissimoAbove:
    case SymId::articStaccatissimoBelow:
    case SymId::articStaccatissimoStrokeAbove:     //extrapollated
    case SymId::articStaccatissimoStrokeBelow:     //extrapollated
    case SymId::articStaccatissimoWedgeAbove:      //extrapollated
    case SymId::articStaccatissimoWedgeBelow:      //extrapollated
        return BRAILLE_ARTIC_STACCATISSIMO;
    case SymId::articTenutoAbove:
    case SymId::articTenutoBelow:
        return BRAILLE_ARTIC_TENUTO;
    case SymId::articTenutoStaccatoAbove:
    case SymId::articTenutoStaccatoBelow:
        return BRAILLE_ARTIC_TENUTO_STACCATO;
    case SymId::articMarcatoAbove:
    case SymId::articMarcatoBelow:
        return BRAILLE_ARTIC_MARCATO;
    case SymId::articAccentStaccatoAbove:
    case SymId::articAccentStaccatoBelow:
        return BRAILLE_ARTIC_STACCATO + BRAILLE_ARTIC_ACCENT;     //extrapollated
    case SymId::articMarcatoStaccatoAbove:
    case SymId::articMarcatoStaccatoBelow:
        return BRAILLE_ARTIC_STACCATO + BRAILLE_ARTIC_MARCATO;     //extrapollated
    case SymId::articMarcatoTenutoAbove:
    case SymId::articMarcatoTenutoBelow:
        return BRAILLE_ARTIC_MARCATO + BRAILLE_ARTIC_TENUTO;       //extrapollated
    case SymId::articStressAbove:
    case SymId::articStressBelow:
        return QString();     //TODO
    case SymId::articTenutoAccentAbove:
    case SymId::articTenutoAccentBelow:
        return BRAILLE_ARTIC_TENUTO + BRAILLE_ARTIC_ACCENT;     //extrapollated
    case SymId::articUnstressAbove:
    case SymId::articUnstressBelow:
        return QString();     //TODO
    case SymId::articSoftAccentAbove:
    case SymId::articSoftAccentBelow:
        return QString();     //TODO
    case SymId::articSoftAccentStaccatoAbove:
    case SymId::articSoftAccentStaccatoBelow:
        return QString();     //TODO
    case SymId::articSoftAccentTenutoAbove:
    case SymId::articSoftAccentTenutoBelow:
        return QString();     //TODO
    case SymId::articSoftAccentTenutoStaccatoAbove:
    case SymId::articSoftAccentTenutoStaccatoBelow:
        return QString();     //TODO
    case SymId::guitarFadeIn:
        return BRAILLE_ARTIC_REVERSED_ACCENT;     // not sure
    case SymId::guitarFadeOut:
        return BRAILLE_ARTIC_EXPRESSIVE_ACCENT;     //not sure
    case SymId::guitarVolumeSwell:
        return BRAILLE_ARTIC_SWELL;
    case SymId::stringsDownBow:
        return BRAILLE_DOWN_BOW;
    case SymId::stringsUpBow:
        return BRAILLE_UP_BOW;
    case SymId::pluckedLeftHandPizzicato:
    case SymId::pluckedBuzzPizzicato:     // extrapollated. not sure
    case SymId::pluckedSnapPizzicatoAbove:     // extrapollated. not sure
    case SymId::pluckedSnapPizzicatoBelow:     // extrapollated. not sure
        return BRAILLE_LEFT_HAND_PIZZICATO;
    case SymId::ornamentTurn:
        return BRAILLE_TURN_NOTE;
    case SymId::ornamentTurnInverted:
        return BRAILLE_INVERTED_TURN_NOTE;
    case SymId::ornamentTrill:
        return BRAILLE_TRILL;
    case SymId::ornamentMordent:
        return BRAILLE_MORDENT;
    case SymId::ornamentShortTrill:
        return BRAILLE_SHORT_TRILL;
    case SymId::ornamentTremblement:
        return BRAILLE_TREBLEMENT;
    case SymId::ornamentPrallMordent:
        return BRAILLE_PRALL_MORDENT;
    case SymId::ornamentUpPrall:
        return BRAILLE_UP_PRALL;
    case SymId::ornamentPrecompMordentUpperPrefix:
        return BRAILLE_MORDENT_WITH_UPPER_FIX;
    case SymId::ornamentUpMordent:
        return BRAILLE_UP_MORDENT;
    case SymId::ornamentDownMordent:
        return BRAILLE_DOWN_MORDENT;
    case SymId::ornamentPrallDown:
        return BRAILLE_PRALL_DOWN;
    case SymId::ornamentPrallUp:
        return BRAILLE_PRALL_UP;
    case SymId::ornamentLinePrall:
        return BRAILLE_LINE_PRALL;
    //TODO. there are still more articulations
    default:
        break;
    }
    return QString();
}

QString ExportBraille::brailleFingeringAfter(Fingering* fingering)
{
    // 15.1 - 15.2. Page 108. Music Braille Code 2015.
    // TODO: 15.3 - 15.4. Page 108. Music Braille Code 2015.
    QString result = QString();
    if (fingering->tid() == Tid::FINGERING
        || fingering->tid() == Tid::LH_GUITAR_FINGERING) {
        for (QChar c : fingering->plainText()) {
            if (c == QChar('0')) {
                result += BRAILLE_FINGERING_OPEN;
            }
            if (c == QChar('1')) {
                result += BRAILLE_FINGERING_1ST;
            }
            if (c == QChar('2')) {
                result += BRAILLE_FINGERING_2ND;
            }
            if (c == QChar('3')) {
                result += BRAILLE_FINGERING_3RD;
            }
            if (c == QChar('4')) {
                result += BRAILLE_FINGERING_4TH;
            }
            if (c == QChar('5')) {
                result += BRAILLE_FINGERING_5TH;
            }
            if (c == QChar('-')) {
                result += BRAILLE_FINGERING_CHANGE;
            }
        }
    }

    return result;
}

QString ExportBraille::brailleGlissando(Note* note)
{
    if (!note) {
        return QString();
    }

    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->isGlissando()) {
            return BRAILLE_GLISSANDO;
        }
    }

    return QString();
}

QString ExportBraille::brailleMeasureRepeat(MeasureRepeat* measureRepeat)
{
    if (!measureRepeat) {
        return QString();
    }

    return BRAILLE_MEASURE_REPEAT;
}

QString ExportBraille::brailleVolta(Measure* measure, Volta* volta, int staffCount)
{
    if (!volta || volta->startMeasure() != measure || volta->text().isEmpty()) {
        return QString();
    }

    // 17.1.1. Page 121. Music Braille Code 2015.
    resetOctave(staffCount);
    QStringList voltaNumbers = volta->text().split(QRegularExpression("(,|\\.| )"));
    QString result = QString();
    for (QString voltaNumber : voltaNumbers) {
        if (voltaNumber.isEmpty()) {
            continue;
        }
        result += QString("#") + voltaNumber;
    }

    // Note: Normally the separator is only added in certain
    // conditions, but for simplicity we add it everytime
    result += "'";

    return result;
}

std::vector<Slur*> ExportBraille::slurs(ChordRest* chordRest)
{
    std::vector<Slur*> result;
    SpannerMap& smap = score->spannerMap();
    auto spanners = smap.findOverlapping(chordRest->tick().ticks(), chordRest->tick().ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (spanner && spanner->isSlur()
            && spanner->track() == chordRest->track()
            && spanner->effectiveTrack2() == chordRest->track()) {
            result.push_back(toSlur(spanner));
        }
    }

    return result;
}

std::vector<Hairpin*> ExportBraille::hairpins(ChordRest* chordRest)
{
    std::vector<Hairpin*> result;
    SpannerMap& smap = score->spannerMap();
    auto spanners = smap.findOverlapping(chordRest->tick().ticks(), chordRest->tick().ticks());
    for (auto interval : spanners) {
        Spanner* spanner = interval.value;
        if (spanner && spanner->isHairpin()
            && spanner->track() == chordRest->track()
            && spanner->effectiveTrack2() == chordRest->track()) {
            result.push_back(toHairpin(spanner));
        }
    }

    return result;
}

QString ExportBraille::brailleSlurBefore(ChordRest* chordRest, std::vector<Slur*>* slurs)
{
    if (!chordRest || !slurs || slurs->empty()) {
        return QString();
    }

    // 13.4 Page 98. Music Braille Code 2015.
    if (isLongLongSlurConvergence(slurs)) {
        return BRAILLE_SLUR_BRACKET_START + BRAILLE_SLUR_BRACKET_END;
    }

    QString longSlur = QString();
    for (Slur* slur : *slurs) {
        //13.3. Page 94. Braille Music Code 2015.
        if (isLongSlur(slur) && chordRest->segment() == slur->startSegment()) {
            longSlur += BRAILLE_SLUR_BRACKET_START;
        }
    }

    // TODO Implement 13.6 - 13.10.  Page 98. Music Braille Code 2015.

    return longSlur;
}

QString ExportBraille::brailleSlurAfter(ChordRest* chordRest, std::vector<Slur*>* crSlurs)
{
    if (!chordRest || !crSlurs || crSlurs->empty()) {
        return QString();
    }

    // 13.4 Page 98. Music Braille Code 2015.
    if (isLongLongSlurConvergence(crSlurs)) {
        return QString();
    }

    bool hasShortSlurSymbol = false;
    QString longSlurBraille = QString();
    QString shortSlurBraille = QString();

    for (Slur* slur : *crSlurs) {
        // 13.2. Page 94. Braille Music Code 2015.
        // 13.5. Page 98. Music Braille Code 2015. In Braille, slurs are redundant on notes with ties
        if (isShortSlur(slur) && chordRest->segment() != slur->endSegment() && !hasTies(chordRest)) {
            hasShortSlurSymbol = true;
        }

        //13.3. Page 94. Braille Music Code 2015.
        if (isLongSlur(slur) && chordRest->segment() == slur->endSegment() && !isLongLongSlurConvergence(crSlurs)) {
            longSlurBraille += BRAILLE_SLUR_BRACKET_END;
        }
    }

    if (hasShortSlurSymbol) {
        std::vector<Slur*> nextCRSlurs;
        if (chordRest->segment()->next1()) {
            ChordRest* nextCR = chordRest->segment()->next1()->nextChordRest(chordRest->track());
            nextCRSlurs = slurs(nextCR);
        }

        // 13.4 Page 98. Music Braille Code 2015.
        if (isShortShortSlurConvergence(&nextCRSlurs)) {
            shortSlurBraille = BRAILLE_SLUR_SHORT_CONVERGENCE;
        }
        // 13.2. Page 94. Braille Music Code 2015.
        else {
            shortSlurBraille = BRAILLE_SLUR_SHORT;
        }
    }
    // TODO Implement 13.6 - 13.10.  Page 98. Music Braille Code 2015.

    return shortSlurBraille + longSlurBraille;
}

bool ExportBraille::isShortShortSlurConvergence(std::vector<Slur*>* slurs)
{
    for (Slur* slur1 : *slurs) {
        for (Slur* slur2 : *slurs) {
            if (slur1 != slur2
                && isShortSlur(slur1)
                && isShortSlur(slur2)
                && (slur1->startCR() == slur2->endCR() || slur2->startCR() == slur1->endCR())) {
                return true;
            }
        }
    }
    return false;
}

bool ExportBraille::isLongLongSlurConvergence(std::vector<Slur*>* slurs)
{
    for (Slur* slur1 : *slurs) {
        for (Slur* slur2 : *slurs) {
            if (slur1 != slur2
                && isLongSlur(slur1)
                && isLongSlur(slur2)
                && (slur1->startCR() == slur2->endCR() || slur2->startCR() == slur1->endCR())) {
                return true;
            }
        }
    }
    return false;
}

int ExportBraille::notesInSlur(Slur* slur)
{
    int result = 0;
    for (Segment* segment = slur->startSegment(); segment; segment = segment->next1()) {
        if (!segment->isChordRestType()) {
            continue;
        }

        ChordRest* cr = toChordRest(segment->element(slur->track()));
        if (!cr) {
            continue;
        }

        // Notes in tie do not count in slur.
        // See Bar 5, Example 13.5.1.b, page 98. Music Braille Code 2015
        if (cr->isChord() && cr != slur->endCR() && segment->next1()) {
            Chord* chord = toChord(cr);
            bool skip = false;
            for (Note* note : chord->notes()) {
                if (note->tieFor() && note->tieFor()->endNote()->chord() == segment->next1()->nextChordRest(chord->track())) {
                    skip = true;
                    break;
                }
            }
            if (skip) {
                continue;
            }
        }

        result++;
        if (segment == slur->endSegment()) {
            break;
        }
    }
    return result;
}

bool ExportBraille::isShortSlur(Slur* slur)
{
    int notesNr = notesInSlur(slur);

    //13.2. Page 94. Braille Music Code 2015.
    if (notesNr <= 4) {
        return true;
    }
    return false;
}

bool ExportBraille::isLongSlur(Slur* slur)
{
    //13.3. Page 94. Braille Music Code 2015.
    return !isShortSlur(slur);
}

QString ExportBraille::brailleHairpinBefore(ChordRest* chordRest, std::vector<Hairpin*>* hairpins)
{
    if (!chordRest) {
        return QString();
    }
    //TODO we are supposed to use line continuation 1 and 2 if there are multiple lines overlaping
    QString result = QString();
    for (Hairpin* hairpin : *hairpins) {
        if (!hairpin || hairpin->startCR() != chordRest) {
            continue;
        }
        QString beginTextBraille = QString();
        if (!hairpin->beginText().isEmpty()) {
            //TODO handle symbols
            QString beginText = hairpin->beginText().toLower();
            if (beginText.endsWith(".")) {
                beginText = beginText.left(beginText.lastIndexOf("."));
            }
            if (beginText.endsWith("'")) {
                beginText = beginText.left(beginText.lastIndexOf("'"));
            }
            beginTextBraille = QString(">") + TextToUEBBraille().braille(beginText);
            resetOctave(hairpin->staffIdx());
        }

        switch (hairpin->hairpinType()) {
        case HairpinType::CRESC_HAIRPIN:
            result += beginTextBraille + BRAILLE_HAIRPIN_DIV_START;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::DECRESC_HAIRPIN:
            result += beginTextBraille + BRAILLE_HAIRPIN_CONV_START;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::CRESC_LINE:
        case HairpinType::DECRESC_LINE:
            result += beginTextBraille + BRAILLE_LINE_CONT_START_1;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::INVALID:
            ;
        }
    }

    return result;
}

QString ExportBraille::brailleHairpinAfter(ChordRest* chordRest, std::vector<Hairpin*>* hairpins)
{
    if (!chordRest) {
        return QString();
    }
    //TODO we are supposed to use line continuation 1 and 2 if there are multiple lines overlaping

    // 23.3.3.C says end of  *may* be omitted in some specific cases,
    // but since it is not mandatory, we don't do it for simplicity.
    // This needs to be accounted when using examples from MBC2015 for tests
    QString result = QString();
    for (Hairpin* hairpin : *hairpins) {
        if (!hairpin || hairpin->endCR() != chordRest) {
            continue;
        }
        switch (hairpin->hairpinType()) {
        case HairpinType::CRESC_HAIRPIN:
            result += BRAILLE_HAIRPIN_DIV_END;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::DECRESC_HAIRPIN:
            result += BRAILLE_HAIRPIN_CONV_END;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::CRESC_LINE:
        case HairpinType::DECRESC_LINE:
            result += BRAILLE_LINE_CONT_END_1;
            resetOctave(hairpin->staffIdx());
            break;
        case HairpinType::INVALID:
            ;
        }
    }
    return result;
}

QString ExportBraille::brailleFermata(Fermata* fermata)
{
    if (!fermata) {
        return QString();
    }

    // Table 22B. Page 18. Music Braille Code 2015.
    // TODO Could not find dedicated Braille symbols for Henze
    // or very long variants, so regular ones were used. Are there dedicated symbols?
    switch (fermata->symId()) {
    case SymId::fermataAbove:
    case SymId::fermataBelow:
        return BRAILLE_FERMATA;
    case SymId::fermataVeryShortAbove:
    case SymId::fermataVeryShortBelow:
    case SymId::fermataShortHenzeAbove:
    case SymId::fermataShortHenzeBelow:
    case SymId::fermataShortAbove:
    case SymId::fermataShortBelow:
        return BRAILLE_SHORT_FERMATA;
    case SymId::fermataVeryLongAbove:
    case SymId::fermataVeryLongBelow:
    case SymId::fermataLongHenzeAbove:
    case SymId::fermataLongHenzeBelow:
    case SymId::fermataLongAbove:
    case SymId::fermataLongBelow:
        return BRAILLE_LONG_FERMATA;
    default:
        break;
    }
    return QString();
}

QString ExportBraille::brailleMarker(Marker* marker)
{
    switch (marker->markerType()) {
    case Marker::Type::SEGNO:
    case Marker::Type::VARSEGNO:
        return BRAILLE_SEGNO;
    case Marker::Type::CODA:
    case Marker::Type::VARCODA:
    case Marker::Type::CODETTA:
        return BRAILLE_CODA;
    case Marker::Type::FINE:
        return BRAILLE_FINE;
    case Marker::Type::TOCODA:
    case Marker::Type::TOCODASYM:
        return BRAILLE_TOCODA;
    case Marker::Type::USER:
        return QString(">") + TextToUEBBraille().braille(marker->plainText().toLower()) + QString("> ");
    }
    return QString();
}

QString ExportBraille::brailleJump(Jump* jump)
{
    switch (jump->jumpType()) {
    case Jump::Type::DC:
        return BRAILLE_DA_CAPO;
    case Jump::Type::DC_AL_FINE:
        return BRAILLE_DA_CAPO_AL_FINE;
    case Jump::Type::DC_AL_CODA:
        return BRAILLE_DA_CAPO_AL_CODA;
    case Jump::Type::DS_AL_CODA:
        return BRAILLE_DAL_SEGNO_AL_CODA;
    case Jump::Type::DS_AL_FINE:
        return BRAILLE_DAL_SEGNO_AL_FINE;
    case Jump::Type::DS:
        return BRAILLE_DAL_SEGNO;
    case Jump::Type::USER:
        return QString(">") + TextToUEBBraille().braille(jump->plainText().toLower()) + QString("> ");
    }
    return QString();
}

QString TextToUEBBraille::braille(QChar c)
{
    if (textToBrailleASCII[c] != nullptr) {
        return QString(textToBrailleASCII[c]);
    }
    return QString(c);
}

QString TextToUEBBraille::braille(QString text)
{
    QString buffer, t, p;
    QTextStream rez(&buffer), tmp(&t);
    QChar prev;

    for (QChar c : text) {
        if (prev.isNumber() && c.isLetter()) {
            rez << ASCII_END_OF_NUMBER;
        }

        if (prev.isLetter() && prev.isUpper() && (!c.isLetter() || (c.isLetter() && c.isLower()))) {
            tmp.flush();
            rez << p << t;
            t.clear();
            p.clear();
        }

        if (c.isLetter()) {
            if (c.isUpper()) {
                if (prev.isUpper()) {
                    p = ASCII_PREFIX_CAPITAL_WORD;
                } else {
                    p = ASCII_PREFIX_CAPITAL_LETTER;
                }
                tmp << braille(c);
            } else {
                rez << braille(c);
            }
        } else if (c.isNumber()) {
            if (!prev.isNumber()) {
                rez << ASCII_PREFIX_NUMBER;
            }
            rez << braille(c);
        } else {
            rez << braille(c);
        }
        prev = c;
    }

    tmp.flush();
    rez << p << t;
    t.clear();
    p.clear();

    return rez.readAll();
}
}
