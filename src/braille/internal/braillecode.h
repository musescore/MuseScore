/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#ifndef MU_BRAILLE_BRAILLEDOTS_H
#define MU_BRAILLE_BRAILLEDOTS_H

#include <string>

namespace mu::engraving {
class braille_code
{
public:
    braille_code(std::string t, std::string c);
    ~braille_code();
    void print();
    std::string tag;
    std::string code;
    std::string braille;
    int num_cells;
};

//In the table below, things added by Haipeng are indicated as this:
//Short (one or two) list: "Added by Haipeng" is enclosed by two hash signs on both sides, then every entry is marked as a hash.
//Longer list: "Added by Haipeng" is enclosed by 10 hash signs on both sides. Comments within this section begins with two hashes, and normal entries begin with one hash. The end of section is marked by a line of 20 hashes.

//[BrailleIndicator]
//Braille indicators
extern braille_code Braille_CapIndicator;
extern braille_code Braille_NumIndicator;
extern braille_code Braille_TxtIndicator;

//[Letter]
extern braille_code Braille_a;
extern braille_code Braille_b;
extern braille_code Braille_c;
extern braille_code Braille_d;
extern braille_code Braille_e;
extern braille_code Braille_f;
extern braille_code Braille_g;
extern braille_code Braille_h;
extern braille_code Braille_i;
extern braille_code Braille_j;
extern braille_code Braille_k;
extern braille_code Braille_l;
extern braille_code Braille_m;
extern braille_code Braille_n;
extern braille_code Braille_o;
extern braille_code Braille_p;
extern braille_code Braille_q;
extern braille_code Braille_r;
extern braille_code Braille_s;
extern braille_code Braille_t;
extern braille_code Braille_u;
extern braille_code Braille_v;
extern braille_code Braille_w;
extern braille_code Braille_x;
extern braille_code Braille_y;
extern braille_code Braille_z;

extern braille_code* Braille_Letter[];
//[Number]
//upper and lower numbers
extern braille_code Braille_Upper0;
extern braille_code Braille_Upper1;
extern braille_code Braille_Upper2;
extern braille_code Braille_Upper3;
extern braille_code Braille_Upper4;
extern braille_code Braille_Upper5;
extern braille_code Braille_Upper6;
extern braille_code Braille_Upper7;
extern braille_code Braille_Upper8;
extern braille_code Braille_Upper9;

extern braille_code* Braille_UpperNumbers[];

//#lower numbers
extern braille_code Braille_Lower0;
extern braille_code Braille_Lower1;
extern braille_code Braille_Lower2;
extern braille_code Braille_Lower3;
extern braille_code Braille_Lower4;
extern braille_code Braille_Lower5;
extern braille_code Braille_Lower6;
extern braille_code Braille_Lower7;
extern braille_code Braille_Lower8;
extern braille_code Braille_Lower9;

extern braille_code* Braille_LowerNumbers[];

//[NoteShape]
//#Notes: Braille note shapes are same such as whole=16th=256, half=32nd=512 etc.
//#to deal with complicate back-translation and reading issues, each note should be defined, even it could easily handle by coding.
extern braille_code Braille_aWhole;
extern braille_code Braille_aHalf;
extern braille_code Braille_aQuarter;
extern braille_code Braille_a8th;
extern braille_code Braille_bWhole;
extern braille_code Braille_bHalf;
extern braille_code Braille_bQuarter;
extern braille_code Braille_b8th;
extern braille_code Braille_cWhole;
extern braille_code Braille_cHalf;
extern braille_code Braille_cQuarter;
extern braille_code Braille_c8th;
extern braille_code Braille_dWhole;
extern braille_code Braille_dHalf;
extern braille_code Braille_dQuarter;
extern braille_code Braille_d8th;
extern braille_code Braille_eWhole;
extern braille_code Braille_eHalf;
extern braille_code Braille_eQuarter;
extern braille_code Braille_e8th;
extern braille_code Braille_fWhole;
extern braille_code Braille_fHalf;
extern braille_code Braille_fQuarter;
extern braille_code Braille_f8th;
extern braille_code Braille_gWhole;
extern braille_code Braille_gHalf;
extern braille_code Braille_gQuarter;
extern braille_code Braille_g8th;
//#16th to 128th notes
extern braille_code Braille_a16th;
extern braille_code Braille_a32nd;
extern braille_code Braille_a64th;
extern braille_code Braille_a128th;
extern braille_code Braille_b16th;
extern braille_code Braille_b32nd;
extern braille_code Braille_b64th;
extern braille_code Braille_b128th;
extern braille_code Braille_c16th;
extern braille_code Braille_c32nd;
extern braille_code Braille_c64th;
extern braille_code Braille_c128th;
extern braille_code Braille_d16th;
extern braille_code Braille_d32nd;
extern braille_code Braille_d64th;
extern braille_code Braille_d128th;
extern braille_code Braille_e16th;
extern braille_code Braille_e32nd;
extern braille_code Braille_e64th;
extern braille_code Braille_e128th;
extern braille_code Braille_f16th;
extern braille_code Braille_f32nd;
extern braille_code Braille_f64th;
extern braille_code Braille_f128th;
extern braille_code Braille_g16th;
extern braille_code Braille_g32nd;
extern braille_code Braille_g64th;
extern braille_code Braille_g128th;
//#256th to 2048th notes
extern braille_code Braille_a256th;
extern braille_code Braille_a512th;
extern braille_code Braille_a1024th;
extern braille_code Braille_a2048th;
extern braille_code Braille_b256th;
extern braille_code Braille_b512th;
extern braille_code Braille_b1024th;
extern braille_code Braille_b2048th;
extern braille_code Braille_c256th;
extern braille_code Braille_c512th;
extern braille_code Braille_c1024th;
extern braille_code Braille_c2048th;
extern braille_code Braille_d256th;
extern braille_code Braille_d512th;
extern braille_code Braille_d1024th;
extern braille_code Braille_d2048th;
extern braille_code Braille_e256th;
extern braille_code Braille_e512th;
extern braille_code Braille_e1024th;
extern braille_code Braille_e2048th;
extern braille_code Braille_f256th;
extern braille_code Braille_f512th;
extern braille_code Braille_f1024th;
extern braille_code Braille_f2048th;
extern braille_code Braille_g256th;
extern braille_code Braille_g512th;
extern braille_code Braille_g1024th;
extern braille_code Braille_g2048th;
//#special notes: breve (double whole notes)
//#Advised to use the definitions of Haipeng as first option for breve notes right below this section.
extern braille_code Braille_aBreve;
extern braille_code Braille_bBreve;
extern braille_code Braille_cBreve;
extern braille_code Braille_dBreve;
extern braille_code Braille_eBreve;
extern braille_code Braille_fBreve;
extern braille_code Braille_gBreve;
//##########Added by Haipeng
//#Another kind of breve, mainly used other than the above one which should be used mainly in plain chant
extern braille_code Braille_aBreveAlt;
extern braille_code Braille_bBreveAlt;
extern braille_code Braille_cBreveAlt;
extern braille_code Braille_dBreveAlt;
extern braille_code Braille_eBreveAlt;
extern braille_code Braille_fBreveAlt;
extern braille_code Braille_gBreveAlt;
//##Other longer notes in ancient music, including Longa and Maxima (quadruple whole note)
extern braille_code Braille_aLonga;
extern braille_code Braille_bLonga;
extern braille_code Braille_cLonga;
extern braille_code Braille_dLonga;
extern braille_code Braille_eLonga;
extern braille_code Braille_fLonga;
extern braille_code Braille_gLonga;
extern braille_code Braille_aMaxima;
extern braille_code Braille_bMaxima;
extern braille_code Braille_cMaxima;
extern braille_code Braille_dMaxima;
extern braille_code Braille_eMaxima;
extern braille_code Braille_fMaxima;
extern braille_code Braille_gMaxima;

extern braille_code* Braille_aNotes[];
extern braille_code* Braille_bNotes[];
extern braille_code* Braille_cNotes[];
extern braille_code* Braille_dNotes[];
extern braille_code* Braille_eNotes[];
extern braille_code* Braille_fNotes[];
extern braille_code* Braille_gNotes[];

extern braille_code* Braille_wholeNotes[];
extern braille_code* Braille_halfNotes[];
extern braille_code* Braille_quarterNotes[];
extern braille_code* Braille_8thNotes[];
extern braille_code* Braille_16thNotes[];
extern braille_code* Braille_32ndNotes[];
extern braille_code* Braille_64thNotes[];
extern braille_code* Braille_128thNotes[];
extern braille_code* Braille_256thNotes[];
extern braille_code* Braille_512thNotes[];
extern braille_code* Braille_1024thNotes[];
extern braille_code* Braille_2048thNotes[];
//####################

//#Braille rests' shapes are same as notes above, whole rest=16th rest=256th etc.
extern braille_code Braille_RestWhole;
extern braille_code Braille_RestHalf;
extern braille_code Braille_RestQuarter;
extern braille_code Braille_Rest8th;
//#16th to 128th rests
extern braille_code Braille_Rest16th;
extern braille_code Braille_Rest32nd;
extern braille_code Braille_Rest64th;
extern braille_code Braille_Rest128th;
//#256th to 2048th rests
extern braille_code Braille_Rest256th;
extern braille_code Braille_Rest512th;
extern braille_code Braille_Rest1024th;
extern braille_code Braille_Rest2048th;
//#breve rest, use Haipeng's breve rest definition below as first option.
extern braille_code Braille_RestBreve;
//##########Added by Haipeng##########
//#More widely used Breve rest, swap the above with Alt
extern braille_code Braille_RestBreveAlt;
//#Other longer rests in ancient music
extern braille_code Braille_RestLonga;
extern braille_code Braille_RestMaxima;

extern braille_code* Braille_Rests[];
//####################

//[MusicPunctuation]
extern braille_code Braille_PlusSign;
extern braille_code Braille_MinusSign;
extern braille_code Braille_SlashSign;
//#note dot sign, to add value for the associated note and some other situations
extern braille_code Braille_Dot;
extern braille_code Braille_Parentheses;
extern braille_code Braille_OpenParentheses;
extern braille_code Braille_CloseParentheses;
extern braille_code Braille_SpecialParentheses;
extern braille_code Braille_PageIndicator;
extern braille_code Braille_LineIndicator;
extern braille_code Braille_Hyphen;
extern braille_code Braille_MusicComma;
extern braille_code Braille_MusicCommaEnd;
//##########Added by Haipeng##########
//#Prefix for cautionary accidentals and hidden rests
extern braille_code Braille_Cautionary;
//#Prefix for editorial elements such as dashed slurs, ties and hairpins
extern braille_code Braille_Editorial;
//####################
extern braille_code Braille_EqualSign;
extern braille_code Braille_UpperOpenBracket;
extern braille_code Braille_UpperCloseBracket;
extern braille_code Braille_UpperBrokenOpenBracket;
extern braille_code Braille_UpperBrokenCloseBracket;
extern braille_code Braille_UpperOpenEndedBracket;
extern braille_code Braille_UpperCloseEndedBracket;
extern braille_code Braille_LowerOpenBracket;
extern braille_code Braille_LowerCloseBracket;
extern braille_code Braille_LowerBrokenOpenBracket;
extern braille_code Braille_LowerBrokenCloseBracket;
extern braille_code Braille_LowerOpenEndedBracket;
extern braille_code Braille_LowerCloseEndedBracket;
extern braille_code Braille_OpenMusicCodeIndicator;
extern braille_code Braille_CloseMusicCodeIndicator;
extern braille_code Braille_AsteriskSign;
extern braille_code Braille_FootnoteSeparator;

//[Harmony]
extern braille_code Braille_Diminished;
extern braille_code Braille_HalfDiminished;
extern braille_code Braille_Triangle;
extern braille_code Braille_HalfTriangle;
//#No chord
extern braille_code Braille_NoHarmony;

//##########Newly added by Haipeng
//[FiguredBass]
//#A figure is prefixed by a number sign, so here only give the signs not available elsewhere
extern braille_code Braille_IsolatedSharp;
extern braille_code Braille_IsolatedDoubleSharp;
extern braille_code Braille_IsolatedFlat;
extern braille_code Braille_IsolatedDoubleFlat;
extern braille_code Braille_IsolatedNatural;
extern braille_code Braille_Cross;

extern braille_code Braille_FiguredBassIndicator;
extern braille_code Braille_FiguredBassSeparator;
extern braille_code Braille_PlusAccidental;
extern braille_code Braille_AccidentalIsolator;
extern braille_code Braille_BackslashFigure;
extern braille_code Braille_SlashFigure;
extern braille_code Braille_FigureExtension;
//####################

//[ValueIndicator]
//#note value indicators: to add before a note having same Braille shape but with different value like between  half and 32nd notes.
//#whole-8th notes range
extern braille_code Braille_FirstValueRange;
//#16th-128th notes range
extern braille_code Braille_SecondValueRange;
//#256th notes and and further range
extern braille_code Braille_ThirdValueRange;

extern braille_code* Braille_ValueRanges[];

//[Octave]
//#octave signs
//#based on Piano keyboard, octave 0 and 8 for lowest and highest notes out of the full octave
//#there are 7 full octaves from 1 to 7
extern braille_code Braille_Octave0;
extern braille_code Braille_Octave1;
extern braille_code Braille_Octave2;
extern braille_code Braille_Octave3;
extern braille_code Braille_Octave4;
extern braille_code Braille_Octave5;
extern braille_code Braille_Octave6;
extern braille_code Braille_Octave7;
extern braille_code Braille_Octave8;

extern braille_code* Braille_Octaves[];

//[Clef]
//#clef signs
extern braille_code Braille_ClefG;
extern braille_code Braille_ClefF;
extern braille_code Braille_ClefC;
//#Treble and bass clefs used in different hands
extern braille_code Braille_ClefGLeft;
extern braille_code Braille_ClefFRight;
//#special clefs
//#G/F/C clef on first line (French violin), second/third/fourth/fifth line etc
extern braille_code Braille_ClefGFirstLine;
extern braille_code Braille_ClefGThirdLine;
extern braille_code Braille_ClefGFourthLine;
extern braille_code Braille_ClefGFifthLine;
extern braille_code Braille_ClefFFirstLine;
extern braille_code Braille_ClefFSecondLine;
extern braille_code Braille_ClefFThirdLine;
//extern braille_code Braille_ClefFFourthLine = {"ClefFFourthLine", "345-3456-5-123", false, false};
extern braille_code Braille_ClefFFifthLine;
extern braille_code Braille_ClefCFirstLine;
extern braille_code Braille_ClefCSecondLine;
//extern braille_code Braille_ClefCThirdLine = {"ClefCThirdLine", "345-346-456-123", false, false};
extern braille_code Braille_ClefCFourthLine;
extern braille_code Braille_ClefCFifthLine;

//[HandSign]
//#hands
extern braille_code Braille_RightHand;
extern braille_code Braille_LeftHand;
//##########Added by Haipeng##########
//#Right and left hands with reversed interval directions in hand-changing passage. Not used for general interval direction changes such as piano left hand reading downwards in orchestral scores where all intervals are down.
extern braille_code Braille_RightHandUp;
extern braille_code Braille_LeftHandDown;
//####################
extern braille_code Braille_OrganPedal;
//##########Added by Haipeng##########
//#Chord and figured bass prefix
extern braille_code Braille_ChordPrefix;
//#Accordion prefix, detected by instrument definition
//AccordionBass 6-345
//#Outline prefix, when producing piano accompaniment with melody outline
extern braille_code Braille_Outline;
//####################

//[Accidental]
//#accidental signs: natural, sharp and flat.
extern braille_code Braille_NaturalAccidental;
//#sharps
extern braille_code Braille_SharpAccidental;
//#1/4 sharp
extern braille_code Braille_QuarterSharp;
//#3/4 sharp
extern braille_code Braille_ThreeQuarterSharp;
//#flats
extern braille_code Braille_FlatAccidental;
extern braille_code Braille_QuarterFlat;
extern braille_code Braille_ThreeQuarterFlat;

extern braille_code* Braille_Accidentals[];
//[TimeSignature]
//#Time signatures
extern braille_code Braille_CommonTime;
extern braille_code Braille_CutTime;
//# time signature by seconds
extern braille_code Braille_TimeInSecondSign;
extern braille_code Braille_TimeExtensionSign;
//[Tie]
//#ties
extern braille_code Braille_NoteTie;
extern braille_code Braille_ChordTie;
extern braille_code Braille_ChordTieDoubling;
extern braille_code Braille_Arpeggio;
extern braille_code Braille_TieLetRing;
extern braille_code Braille_TieNoStart;
//##########Added by Haipeng##########
extern braille_code Braille_TieCrossVoice;
extern braille_code Braille_TieCrossVoiceFrom;
extern braille_code Braille_TieCrossStaff;
extern braille_code Braille_TieCrossStaffFrom;
extern braille_code Braille_ChordTieCrossVoice;
extern braille_code Braille_ChordTieCrossVoiceFrom;
extern braille_code Braille_ChordTieCrossStaff;
extern braille_code Braille_ChordTieCrossStaffFrom;
//####################
//[Slur]
//#slurs
extern braille_code Braille_NoteSlur;
extern braille_code Braille_LongSlurOpenBracket;
extern braille_code Braille_LongSlurCloseBracket;
extern braille_code Braille_ConvergentSlur;
extern braille_code Braille_SameNoteSlur;

extern braille_code Braille_GraceSlur;
extern braille_code Braille_GraceSlurDoubling;
//##########Added by Haipeng##########
extern braille_code Braille_SlurCrossVoice;
extern braille_code Braille_SlurCrossVoiceFrom;
extern braille_code Braille_SlurCrossStaff;
extern braille_code Braille_SlurCrossStaffFrom;
//####################
//[Interval]
//#Intervals, Braille signs to write for chords
//#Braille sign for second interval has same dots for 9, 16, 23 (+7)
extern braille_code Braille_Interval2;
extern braille_code Braille_Interval3;
extern braille_code Braille_Interval4;
extern braille_code Braille_Interval5;
extern braille_code Braille_Interval6;
extern braille_code Braille_Interval7;
extern braille_code Braille_Interval8;

extern braille_code* Braille_Intervals[];
//[Tuplet]
//#tuplet: note grouping 2/3/5/6/XXX-notes grouping
extern braille_code Braille_Tuplet3;
extern braille_code Braille_TupletPrefix;
extern braille_code Braille_TupletSuffix;
//[Repetition]
//#repeats' signs in print score
extern braille_code Braille_RepetitionForward;
extern braille_code Braille_RepetitionBackward;
extern braille_code Braille_Coda;
extern braille_code Braille_Segno;
// Fermata
extern braille_code Braille_InvertedType;
extern braille_code Braille_FermataDefault;
extern braille_code Braille_FermataSquare;
extern braille_code Braille_FermataAngled;
extern braille_code Braille_FermataDoubleSquare;
extern braille_code Braille_FermataDoubleAngled;
extern braille_code Braille_FermataHalfCurve;
extern braille_code Braille_FermataDoubleDot;
//##Newly added by Haipeng##
extern braille_code Braille_FermataBarline;
extern braille_code Braille_FermataNormalBarline;
//#Braille measure or partial repetition sign
extern braille_code Braille_NotesRepeat;
//#Newly added by Haipeng##
//#This repeat should be used according to beamings in unmeasured passage. It should be used between beams, within a long beam, but can't cross beams in different places, thus break the musical meaning implied by the beaming.
extern braille_code Braille_StartingBeamRepeat;
//[Barline]
//#barlines
extern braille_code Braille_SingleBarline;
extern braille_code Braille_DashedBarline;
extern braille_code Braille_SpecialBarline;
extern braille_code Braille_SectionalDouble;
extern braille_code Braille_FinalDouble;
//[Finger]
//#fingering
extern braille_code Braille_Finger0;
extern braille_code Braille_Finger1;
extern braille_code Braille_Finger2;
extern braille_code Braille_Finger3;
extern braille_code Braille_Finger4;
extern braille_code Braille_Finger5;

extern braille_code Braille_FingerSlur;

extern braille_code* Braille_Fingers[];
//##Comment by Haipeng: The names above are not strict, but I believe they can be correctly mapped to Musicxml.##
//[Pluck]
//#plucks, for string instruments
extern braille_code Braille_PluckP;
extern braille_code Braille_PluckI;
extern braille_code Braille_PluckM;
extern braille_code Braille_PluckA;
extern braille_code Braille_PluckC;

extern braille_code* Braille_Plucks[];

extern braille_code Braille_Dot6PluckP;
extern braille_code Braille_Dot6PluckI;
extern braille_code Braille_Dot6PluckM;
extern braille_code Braille_Dot6PluckA;
extern braille_code Braille_Dot6PluckC;
//[String]
//#string
extern braille_code Braille_String1;
extern braille_code Braille_String2;
extern braille_code Braille_String3;
extern braille_code Braille_String4;
extern braille_code Braille_String5;
extern braille_code Braille_String6;
extern braille_code Braille_String7;

extern braille_code* Braille_Strings[];

extern braille_code Braille_String1_Doubling;
extern braille_code Braille_String2_Doubling;
extern braille_code Braille_String3_Doubling;
extern braille_code Braille_String4_Doubling;
extern braille_code Braille_String5_Doubling;
extern braille_code Braille_String6_Doubling;
extern braille_code Braille_String7_Doubling;

extern braille_code* Braille_Strings_Doubling[];
//[Fret]
//#frets
extern braille_code Braille_Fret1;
extern braille_code Braille_Fret2;
extern braille_code Braille_Fret3;
extern braille_code Braille_Fret4;
extern braille_code Braille_Fret5;
extern braille_code Braille_Fret6;
extern braille_code Braille_Fret7;
extern braille_code Braille_Fret8;
extern braille_code Braille_Fret9;
extern braille_code Braille_Fret10;
extern braille_code Braille_Fret11;
extern braille_code Braille_Fret12;
extern braille_code Braille_Fret13;

extern braille_code* Braille_Frets[];
//[VoiceAccord]
//#accord
extern braille_code Braille_FullMeasureAccord;
extern braille_code Braille_PartialMeasureAccord;
extern braille_code Braille_MeasureSeparator;
//[Pedal]
//#pedal signs
extern braille_code Braille_PedalDown;
extern braille_code Braille_PedalUp;
extern braille_code Braille_NotePedalUpDownAtNote;
extern braille_code Braille_HalfPedal;
extern braille_code Braille_AfterNotePedalDown;
extern braille_code Braille_AfterNotePedalUp;
extern braille_code Braille_NoPedal;
//[Ornament]
//#articulations, ornaments
extern braille_code Braille_Spiccato;
extern braille_code Braille_Staccato;
extern braille_code Braille_Staccatissimo;
extern braille_code Braille_DetachedLegato;
extern braille_code Braille_Tenuto;
extern braille_code Braille_Accent;
extern braille_code Braille_Stress;
extern braille_code Braille_Unstress;
extern braille_code Braille_StrongAccent;
//##########Added by Haipeng##########
extern braille_code Braille_SoftAccent;
extern braille_code Braille_Scoop;
extern braille_code Braille_Plop;
extern braille_code Braille_Doit;
extern braille_code Braille_Falloff;
extern braille_code Braille_UpBow;
extern braille_code Braille_DownBow;
extern braille_code Braille_HarmonicNatural;
extern braille_code Braille_HarmonicArtificial;
extern braille_code Braille_OpenString;
extern braille_code Braille_Stopped;
extern braille_code Braille_SnapPizzicato;
extern braille_code Braille_HammerOn;
extern braille_code Braille_PullOff;
//####################
extern braille_code Braille_ArpeggiateUp;
extern braille_code Braille_ArpeggiateDown;
extern braille_code Braille_Cue;
extern braille_code Braille_GraceShort;
extern braille_code Braille_GraceLong;
extern braille_code Braille_TrillMark;
extern braille_code Braille_Turn;
extern braille_code Braille_InvertedTurn;
extern braille_code Braille_DelayedTurn;
extern braille_code Braille_DelayedInvertedTurn;
extern braille_code Braille_MordentShort;
extern braille_code Braille_MordentLong;
extern braille_code Braille_InvertedMordentShort;
extern braille_code Braille_InvertedMordentLong;
//##Added by Haipeng##
extern braille_code Braille_Schleifer;
extern braille_code Braille_Shake;
extern braille_code Braille_BreathMark;
extern braille_code Braille_Caesura;

extern braille_code Braille_TremoloSingle1;
extern braille_code Braille_TremoloSingle2;
extern braille_code Braille_TremoloSingle3;
extern braille_code Braille_TremoloSingle4;
extern braille_code Braille_TremoloSingle5;

extern braille_code* Braille_TremoloSingles[];

extern braille_code Braille_TremoloDouble1;
extern braille_code Braille_TremoloDouble2;
extern braille_code Braille_TremoloDouble3;
extern braille_code Braille_TremoloDouble4;
extern braille_code Braille_TremoloDouble5;

extern braille_code* Braille_TremoloDoubles[];

extern braille_code Braille_Glissando;
extern braille_code Braille_LongGlissandoStart;
extern braille_code Braille_LongGlissandoStop;
extern braille_code Braille_Slide;
extern braille_code Braille_LongSlideStart;
extern braille_code Braille_LongSlideStop;
//##########Added by Haipeng##########
//[Noteheads]
extern braille_code Braille_blackHead;
extern braille_code Braille_XShape;
extern braille_code Braille_Diamond;
extern braille_code Braille_DiamondHarmonic;
extern braille_code Braille_Slashed;
//#Comment: If the diamond is in a chord, then use DiamondHarmonic, since it's an artificial harmonic.
extern braille_code Braille_StemOnly;
extern braille_code Braille_Circled;
//[Stems]
extern braille_code Braille_StemPrefix;
extern braille_code Braille_StemWhole;
extern braille_code Braille_StemHalf;
extern braille_code Braille_StemQuarter;
extern braille_code Braille_Stem8th;
extern braille_code Braille_Stem16th;
extern braille_code Braille_Stem32nd;
//[HarpPedalDiagram]
extern braille_code Braille_HarpPedalBegin;
extern braille_code Braille_HarpPedalEnd;
extern braille_code Braille_HarpPedalRaised;
extern braille_code Braille_HarpPedalCentered;
extern braille_code Braille_HarpPedalLowered;
extern braille_code Braille_HarpPedalDivider;
//[FetherBeams]
extern braille_code Braille_FanBeamAccelerando;
extern braille_code Braille_FanBeamRitardando;
extern braille_code Braille_FanBeamSteady;
extern braille_code Braille_FanBeamEnd;
// Placement
extern braille_code Braille_PlacementBelow;
// Hand interval up/down
extern braille_code Braille_LeftHandIntervalDown;
extern braille_code Braille_RightHandIntervalUp;
// Additional braille codes
extern braille_code Braille_NewLine;
// Slurs
extern braille_code Braille_SlurCrossVoiceDubling;
extern braille_code Braille_SlurCrossStaffDubling;
// Wavy
extern braille_code Braille_OrnamentWavyStart;
extern braille_code Braille_OrnamentWavyStop;
extern braille_code Braille_WavyRepetitionLine;
// Dubling
extern braille_code Braille_CueDubling;
extern braille_code Braille_GraceLongDubling;
// Noteheads dubling
extern braille_code Braille_blackHeadDubling;
extern braille_code Braille_XShapeDubling;
extern braille_code Braille_DiamondDubling;
extern braille_code Braille_DiamondHarmonicDubling;
extern braille_code Braille_SlashedDubling;
extern braille_code Braille_StemOnlyDubling;
extern braille_code Braille_CircledDubling;
// Tremolo dubling
extern braille_code Braille_TremoloSingle1Dubling;
extern braille_code Braille_TremoloSingle2Dubling;
extern braille_code Braille_TremoloSingle3Dubling;
extern braille_code Braille_TremoloSingle4Dubling;
extern braille_code Braille_TremoloSingle5Dubling;
extern braille_code Braille_TremoloDouble1Dubling;
extern braille_code Braille_TremoloDouble2Dubling;
extern braille_code Braille_TremoloDouble3Dubling;
extern braille_code Braille_TremoloDouble4Dubling;
extern braille_code Braille_TremoloDouble5Dubling;
// Dash sign
extern braille_code Braille_DashSign;
// Dashline
extern braille_code Braille_DashLineStart;
extern braille_code Braille_DashLineStop;
extern braille_code Braille_DashNestedLineStart;
extern braille_code Braille_DashNestedLineStop;
// Elision
extern braille_code Braille_Elision2;
extern braille_code Braille_Elision3;
// Empty
extern braille_code Braille_Empty;
// line over line format
extern braille_code Braille_HarmonyLineIndicator;
extern braille_code Braille_LyricLineIndicator;
extern braille_code Braille_MusicLineIndicator;
extern braille_code Braille_PluckLineIndicator;
extern braille_code Braille_SoloMelodyLineIndicator;
// wedge stop
extern braille_code Braille_WedgeCStop;
extern braille_code Braille_WedgeDStop;
// Repeats
extern braille_code Braille_PartialRepeat;
extern braille_code Braille_MeasureRepeat;
// Check dot
extern braille_code Braille_CheckDot;
// Check figure Bass Separator
extern braille_code Braille_CheckFiguredBassSeparator;
//
extern braille_code Braille_ThumbPosition;
// Accordion
extern braille_code Braille_AccordionMiddle2;
extern braille_code Braille_AccordionMiddle3;
// String intrument
extern braille_code Braille_LeftHandPizzicato;

std::string getBraillePattern(std::string dots);
std::string translate2Braille(std::string codes);
std::string intToBrailleUpperNumbers(std::string txt, bool indicator);
std::string intToBrailleLowerNumbers(std::string txt, bool indicator);
}
#endif // MU_BRAILLE_BRAILLEDOTS_H
