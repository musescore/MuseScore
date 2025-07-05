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

#include "importgtp.h"

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/types/symid.h"

#include "guitarprodrumset.h"

#include "log.h"

using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::guitarpro {
//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro4::readMixChange(Measure* measure)
{
    /*char patch   =*/
    readChar();
    signed char volume  = readChar();
    signed char pan     = readChar();
    signed char chorus  = readChar();
    signed char reverb  = readChar();
    signed char phase   = readChar();
    signed char tremolo = readChar();
    int temp    = readInt();

    bool tempoEdited = false;

    if (volume >= 0) {
        readChar();
    }
    if (pan >= 0) {
        readChar();
    }
    if (chorus >= 0) {
        readChar();
    }
    if (reverb >= 0) {
        readChar();
    }
    if (phase >= 0) {
        readChar();
    }
    if (tremolo >= 0) {
        readChar();
    }
    if (temp >= 0) {
        if (last_segment) {
            score->setTempo(last_segment->tick(), double(temp) / 60.0f);
            last_segment = nullptr;
        }
        if (temp != previousTempo) {
            previousTempo = temp;
            setTempo(temp, measure);
        }
        readChar();
        tempoEdited = true;
    }

    readChar();         // bitmask: what should be applied to all tracks
    return tempoEdited;
}

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro4::readBeatEffects(int track, Segment* segment)
{
    int effects = 0;
    uint8_t fxBits1 = readUInt8();
    uint8_t fxBits2 = readUInt8();
    if (fxBits1 & BEAT_FADE) {
        effects = 4;     // fade in
    }
    if (fxBits1 & BEAT_EFFECT) {
        effects = readUInt8();          // effect 1-tapping, 2-slapping, 3-popping
    }
    if (fxBits2 & BEAT_TREMOLO) {
        readTremoloBar(track, segment);
    }
    if (fxBits1 & BEAT_ARPEGGIO) {
        int strokeup = readUInt8();                // up stroke length
        int strokedown = readUInt8();                // down stroke length

        Arpeggio* a = Factory::createArpeggio(score->dummy()->chord());
        if (strokeup > 0) {
            a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
            if (strokeup < 7) {
                a->setStretch(1.0 / std::pow(2, 6 - strokeup));
            }
        } else if (strokedown > 0) {
            a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
            if (strokedown < 7) {
                a->setStretch(1.0 / std::pow(2, 6 - strokedown));
            }
        } else {
            delete a;
            a = 0;
        }

        if (a) {
            ChordRest* cr = Factory::createChord(segment);
            cr->setTrack(track);
            cr->add(a);
            segment->add(cr);
        }
    }
    if (fxBits2 & BEAT_STROKE_DIR) {
        effects = readUInt8();                // stroke pick direction
        effects += 4;        //1 or 2 for effects becomes 4 or 5
    }
    if (fxBits1 & 0x01) {           // GP3 column-wide vibrato
    }
    if (fxBits1 & 0x2) {            // GP3 column-wide wide vibrato (="tremolo" in GP3)
    }
    return effects;
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

GuitarPro::ReadNoteResult GuitarPro4::readNote(int string, int staffIdx, Note* note)
{
    uint8_t noteBits = readUInt8();

    //
    // noteBits:
    //   80 - Right hand or left hand fingering;
    //   40 - Accentuated note
    //   20 - Note type (rest, empty note, normal note);
    //   10 - note dynamic;
    //    8 - Presence of effects linked to the note;
    //    4 - Ghost note;
    //    2 - Dotted note;  ?
    //    1 - Time-independent duration

    if (noteBits & BEAT_TREMOLO) {
        //note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        note->setGhost(true);
    }

    bool tieNote = false;
    uint8_t variant = 1;
    if (noteBits & BEAT_EFFECT) {         // 0x20
        variant = readUInt8();
        if (variant == 1) {         // normal note
        } else if (variant == 2) {
            /* guitar pro 4 bundles tied notes with slides in the representation
             * we take note when we see ties and do not create slides for these notes. */
            slides[staffIdx * VOICES] = -2;
            tieNote = true;
        } else if (variant == 3) {                   // dead notes = ghost note
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setDeadNote(true);
        } else {
            LOGD("unknown note variant: %d", variant);
        }
    }

    if (noteBits & 0x1) {                 // note != beat
        int a = readUInt8();              // length
        int b = readUInt8();              // t
        LOGD("          Time independent note len, len %d t %d", a, b);
    }
    if (noteBits & 0x2) {                 // note is dotted
        //readUInt8();
    }

    int& previousDynamic = previousDynamicByTrack[note->track()];

    // set dynamic information on note if different from previous note
    if (noteBits & NOTE_DYNAMIC) {              // 0x10
        int d = readChar();
        if (previousDynamic != d) {
            previousDynamic = d;
            addDynamic(note, d);
        }
    } else if (previousDynamic != DEFAULT_DYNAMIC) {
        previousDynamic = DEFAULT_DYNAMIC;
        addDynamic(note, previousDynamic);
    }

    int fretNumber = -1;
    if (noteBits & NOTE_FRET) {                   // 0x20
        // TODO: special case if note is tied
        fretNumber = readUInt8();
    }

    // check if a note is supposed to be accented, and give it the sforzato type
    if (noteBits & NOTE_SFORZATO) {             // 0x40
        Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
        art->setSymId(SymId::articAccentAbove);
        if (!note->score()->toggleArticulation(note, art)) {
            delete art;
        }
    }

    if (noteBits & NOTE_FINGERING) {            // 0x80
        int leftFinger  = readUInt8();
        int rightFinger = readUInt8();
        Fingering* fi   = Factory::createFingering(note);
        String finger;
        // if there is a valid left hand fingering
        if (leftFinger < 5) {
            if (leftFinger == 0) {
                finger = u"T";
            } else if (leftFinger == 1) {
                finger = u"1";
            } else if (leftFinger == 2) {
                finger = u"2";
            } else if (leftFinger == 3) {
                finger = u"3";
            } else if (leftFinger == 4) {
                finger = u"4";
            }
        } else {
            if (rightFinger == 0) {
                finger = u"T";
            } else if (rightFinger == 1) {
                finger = u"I";
            } else if (rightFinger == 2) {
                finger = u"M";
            } else if (rightFinger == 3) {
                finger = u"A";
            } else if (rightFinger == 4) {
                finger = u"O";
            }
        }
        fi->setPlainText(finger);
        note->add(fi);
        fi->reset();
    }

    ReadNoteResult readResult;
    uint8_t modMask2 = 0;
    if (noteBits & BEAT_EFFECTS) {
        uint8_t modMask1 = readUInt8();
        modMask2 = readUInt8();
        if (modMask1 & EFFECT_BEND) {
            readBend(note);
        }
        if (modMask1 & EFFECT_HAMMER) {
            readResult.hammerOnPullOff = true;
        }
        if (modMask1 & EFFECT_LET_RING) {
            readResult.letRing = true;
        }
        if (modMask2 & EFFECT_VIBRATO) {
            readResult.vibrato = true;
        }
        if (modMask1 & EFFECT_GRACE) {
            int fret = readUInt8();                   // grace fret
            int dynamic = readUInt8();                // grace dynamic
            int transition = readUInt8();             // grace transition
            int duration = readUInt8();               // grace duration

            int grace_len = Constants::DIVISION / 8;
            if (duration == 1) {
                grace_len = Constants::DIVISION / 8;       //32nd
            } else if (duration == 2) {
                grace_len = Constants::DIVISION / 6;       //24th
            } else if (duration == 3) {
                grace_len = Constants::DIVISION / 4;       //16th
            }
            Note* gn = Factory::createNote(score->dummy()->chord());

            if (fret == 255) {
                gn->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                gn->setGhost(true);
            }
            if (fret == 255) {
                fret = 0;
            }
            gn->setFret(fret);
            gn->setString(string);
            int grace_pitch = note->part()->instrument()->stringData()->getPitch(string, fret, nullptr);
            gn->setPitch(grace_pitch);
            gn->setTpcFromPitch();

            Chord* gc = Factory::createChord(score->dummy()->segment());
            gc->setTrack(note->chord()->track());
            gc->add(gn);
            gc->setParent(note->chord());

            TDuration d;
            d.setVal(grace_len);
            if (grace_len == Constants::DIVISION / 6) {
                d.setDots(1);
            }
            gc->setDurationType(d);
            gc->setTicks(d.fraction());
            gc->setNoteType(NoteType::ACCIACCATURA);
            gc->mutldata()->setMag(note->chord()->staff()->staffMag(Fraction(0, 1)) * score->style().styleD(Sid::graceNoteMag));
            note->chord()->add(gc);
            addDynamic(gn, dynamic);

            if (transition == 0) {
                // no transition
            } else if (transition == 1) {
                //TODO: Add a 'slide' guitar effect when implemented
                //TODO-ws				note->setSlideNote(gn);
            } else if (transition == 2 && fretNumber >= 0 && fretNumber <= 255 && fretNumber != gn->fret()) {
            } else if (transition == 3) {
                // TODO:
                //     major: replace with a 'hammer-on' guitar effect when implemented
                //     minor: make slurs for parts

                ChordRest* cr1 = toChord(gc);
                ChordRest* cr2 = toChord(note->chord());

                Slur* slur1 = Factory::createSlur(score->dummy());
                slur1->setAnchor(Spanner::Anchor::CHORD);
                slur1->setStartElement(cr1);
                slur1->setEndElement(cr2);
                slur1->setTick(cr1->tick());
                slur1->setTick2(cr2->tick());
                slur1->setTrack(cr1->track());
                slur1->setTrack2(cr2->track());
                // this case specifies only two-note slurs, don't set a parent
                score->undoAddElement(slur1);
            }
        }
        if (modMask2 & EFFECT_STACCATO) {       // staccato
            Chord* chord = note->chord();
            Articulation* a = Factory::createArticulation(chord);
            a->setSymId(SymId::articStaccatoAbove);
            chord->add(a);
        }
        if (modMask2 & EFFECT_PALM_MUTE) {
            readResult.palmMute = true;
        }

        if (modMask2 & EFFECT_TREMOLO) {        // tremolo picking length
            int tremoloDivision = readUInt8();
            Chord* chord = note->chord();
            TremoloType type = TremoloType::INVALID_TREMOLO;
            if (tremoloDivision == 1) {
                type = TremoloType::R8;
            } else if (tremoloDivision == 2) {
                type = TremoloType::R16;
            } else if (tremoloDivision == 3) {
                type = TremoloType::R32;
            } else {
                LOGD("Unknown tremolo value");
            }

            if (type != TremoloType::INVALID_TREMOLO) {
                TremoloSingleChord* t = Factory::createTremoloSingleChord(chord);
                t->setTremoloType(type);
                chord->add(t);
            }
        }
        if (modMask2 & EFFECT_SLIDE) {
            int slideKind = readUInt8();
            // if slide >= 4 then we are not dealing with legato slide nor shift slide
            if (slideKind >= 3 || slideKind == 254 || slideKind == 255) {
                slide = slideKind;
            } else {
                //slides[note->chord()->track()] = slideKind;
                slideList.push_back(note);
            }
            if (slideKind == 2) {
                readResult.slur = true;
            }
        }

        if (modMask2 & EFFECT_TRILL) {
            readUInt8();
            /*int period =*/ readUInt8();            // trill length
            readResult.trill = true;
        }
    }
    if (fretNumber == -1) {
        LOGD("Note: no fret number, tie %d", tieNote);
    }
    Staff* staff = note->staff();
    if (fretNumber == 255) {
        fretNumber = 0;
        note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        note->setGhost(true);
    }
    // dead note represented as high numbers - fix to zero
    if (fretNumber > 99 || fretNumber == -1) {
        fretNumber = 0;
    }
    int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber, nullptr);
    note->setFret(fretNumber);
    note->setString(string);
    note->setPitch(std::min(pitch, 127));

    if (modMask2 & 0x10) {
        int type = readUInt8();          // harmonic kind
        if (type == 1) {   //Natural
            note->setHarmonic(false);
        } else if (type == 3) { // Tapped
            readResult.harmonicTap = true;
        } else if (type == 4) { //Pinch
            readResult.harmonicPinch = true;
        } else if (type == 5) { //semi
            readResult.harmonicSemi = true;
        } else {   //Artificial
            readResult.harmonicArtificial = true;
            int harmonicFret = note->fret();
            harmonicFret += type - 10;
            Note* harmonicNote = Factory::createNote(note->chord());
            note->chord()->add(harmonicNote);
            harmonicNote->setFret(harmonicFret);
            harmonicNote->setString(note->string());
            harmonicNote->setPitch(note->staff()->part()->instrument()->stringData()->getPitch(note->string(), harmonicFret, nullptr));
            harmonicNote->setTpcFromPitch();
        }
    }

    if (tieNote) {
        staff_idx_t si = note->staffIdx();
        if (slurs[si]) {
            score->removeSpanner(slurs[si]);
            delete slurs[si];
            slurs[si] = 0;
        }
        bool found = false;
        Chord* chord = note->chord();
        Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
        track_idx_t track = note->track();
        std::vector<ChordRest*> chords;
        Note* true_note = 0;
        while (segment) {
            EngravingItem* e = segment->element(track);
            if (e) {
                if (e->isChord()) {
                    Chord* chord2 = toChord(e);
                    for (Note* note2 : chord2->notes()) {
                        if (note2->string() == string) {
                            if (chords.empty()) {
                                Tie* tie = Factory::createTie(note2);
                                tie->setEndNote(note);
                                note2->add(tie);
                            }
                            note->setFret(note2->fret());
                            note->setPitch(note2->pitch());
                            true_note = note2;
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    break;
                } else if (e) {
                    chords.push_back(toChordRest(e));
                }
            }
            segment = segment->prev1(SegmentType::ChordRest);
        }
        if (true_note && chords.size()) {
            Note* end_note = note;
            for (unsigned int i = 0; i < chords.size(); ++i) {
                Chord* chord1 = nullptr;
                auto cr = chords.at(i);
                if (cr->isChord()) {
                    chord1 = toChord(cr);
                } else {
                    Rest* rest = toRest(cr);
                    auto dur = rest->ticks();
                    auto dut = rest->durationType();
                    auto seg = rest->segment();
                    seg->remove(rest);
                    auto tuplet = rest->tuplet();
                    if (tuplet) {
                        tuplet->remove(rest);
                    }
                    delete rest;
                    chord1 = Factory::createChord(seg);
                    chord1->setTrack(note->track());
                    chord1->setTicks(dur);
                    chord1->setDurationType(dut);
                    seg->add(chord1);
                    if (tuplet) {
                        tuplet->add(chord1);
                    }
                }

                Note* note2 = Factory::createNote(chord1);
                note2->setString(true_note->string());
                note2->setFret(true_note->fret());
                note2->setPitch(true_note->pitch());
                note2->setTpcFromPitch();
                chord1->setNoteType(true_note->noteType());
                chord1->add(note2);
                Tie* tie = Factory::createTie(note2);
                tie->setEndNote(end_note);
                end_note->setHarmonic(true_note->harmonic());
                end_note = note2;
                note2->add(tie);
            }
            Tie* tie = Factory::createTie(true_note);
            tie->setEndNote(end_note);
            end_note->setHarmonic(true_note->harmonic());
            true_note->add(tie);
        }
        if (!found) {
            LOGD("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            readResult.slur = false;
            return readResult;
        }
    }

    return readResult;
}

//---------------------------------------------------------
//   readInfo
//---------------------------------------------------------

void GuitarPro4::readInfo()
{
    title        = readDelphiString();
    subtitle     = readDelphiString();
    artist       = readDelphiString();
    album        = readDelphiString();
    composer     = readDelphiString();
    String copyright = readDelphiString();
    if (!copyright.isEmpty()) {
        score->setMetaTag(u"copyright", copyright);
    }

    readDelphiString(); // transcriber
    readDelphiString(); // instructions
    int n = readInt();
    for (int i = 0; i < n; ++i) {
        comments.append(readDelphiString());
    }
}

//---------------------------------------------------------
//   convertGP4SlideNum
//---------------------------------------------------------

int GuitarPro4::convertGP4SlideNum(int sl)
{
    switch (sl) {
    case 1:
        return SHIFT_SLIDE;
    case 2:
        return LEGATO_SLIDE;
    case 3:             // slide out downwards
        return SLIDE_OUT_DOWN;
    case 4:             // slide out upwards
        return SLIDE_OUT_UP;
    case 254:           // slide in from above
        return SLIDE_IN_ABOVE;
    case 255:           // slide in from below
        return SLIDE_IN_BELOW;
    }
    return sl;
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro4::read(IODevice* io)
{
    m_continiousElementsBuilder = std::make_unique<ContiniousElementsBuilder>(score);
    m_guitarBendImporter = std::make_unique<GuitarBendImporter>(score);

    f      = io;
    curPos = 30;

    readInfo();
    readUInt8();        // triplet feeling
    readLyrics();

    int temp   = readInt();
    key        = readInt();
    /*int octave =*/ readUInt8();      // octave

    previousTempo = -1;

    readChannels();
    measures = readInt();
    staves   = readInt();

    initDynamics(staves);

    curDynam.resize(staves * VOICES);
    for (auto& i : curDynam) {
        i = -1;
    }

    int tnumerator   = 4;
    int tdenominator = 4;
    for (size_t i = 0; i < measures; ++i) {
        GpBar bar;
        uint8_t barBits = readUInt8();
        if (barBits & SCORE_TIMESIG_NUMERATOR) {
            tnumerator = readUInt8();
        }
        if (barBits & SCORE_TIMESIG_DENOMINATOR) {
            tdenominator = readUInt8();
        }
        if (barBits & SCORE_REPEAT_START) {
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::START;
        }
        if (barBits & SCORE_REPEAT_END) {                    // number of repeats
            bar.repeatFlags = bar.repeatFlags | mu::engraving::Repeat::END;
            bar.repeats = readUInt8() + 1;
        }
        if (barBits & SCORE_VOLTA) {                          // a volta
            uint8_t voltaNumber = readUInt8();
            while (voltaNumber > 0) {
                // volta information is represented as a binary number
                bar.volta.voltaType = GP_VOLTA_BINARY;
                bar.volta.voltaInfo.push_back(voltaNumber & 1);
                voltaNumber >>= 1;
            }
        }
        if (barBits & SCORE_MARKER) {
            bar.marker = readDelphiString();           // new section?
            /*int color = */ readInt();          // color?
        }
        if (barBits & SCORE_KEYSIG) {
            int currentKey = readUInt8();
            /* key signatures are specified as
             * 1# = 1, 2# = 2, ..., 7# = 7
             * 1b = 255, 2b = 254, ... 7b = 249 */
            bar.keysig = currentKey <= 7 ? currentKey : -256 + currentKey;
            readUInt8();              // specifies major/minor mode
        }
        if (barBits & SCORE_DOUBLE_BAR) {
            bar.barLine = BarLineType::DOUBLE;
        }
        bar.timesig = Fraction(tnumerator, tdenominator);
        bars.push_back(bar);
    }

    //
    // create a part for every staff
    //
    for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);

        score->appendStaff(s);
        score->appendPart(part);
    }

    createMeasures();

    setTempo(temp, score->firstMeasure());

    for (size_t i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];

        uint8_t c = readUInt8();       // simulations bitmask
        if (c & 0x2) {                    // 12 stringed guitar
        }
        if (c & 0x4) {                    // banjo track
        }
        String name = readPascalString(40);
        int strings  = readInt();
        if (strings <= 0 || strings > GP_MAX_STRING_NUMBER) {
            return false;
        }
        for (int j = 0; j < strings; ++j) {
            tuning[j] = readInt();
        }
        for (int j = strings; j < GP_MAX_STRING_NUMBER; ++j) {
            readInt();
        }
        int midiPort     = readInt() - 1;
        int midiChannel  = readInt() - 1;
        /*int midiChannel2 =*/ readInt();     // - 1;
        int frets        = readInt();

        int capo         = readInt();
        /*int color        =*/ readInt();

        std::vector<int> tuning2(strings);
        //int tuning2[strings];
        for (int k = 0; k < strings; ++k) {
            tuning2[strings - k - 1] = tuning[k];
        }
        bool useFlats = createTuningString(strings, &tuning2[0]);
        StringData stringData(frets, strings, &tuning2[0], useFlats);
        Part* part = score->staff(i)->part();
        Instrument* instr = part->instrument();
        instr->setStringData(stringData);
        instr->setSingleNoteDynamics(false);
        part->setPartName(name);
        part->setPlainLongName(name);

        //
        // determine clef
        //
        Staff* staff = score->staff(i);
        int patch = channelDefaults[midiChannel].patch;
        ClefType clefId = ClefType::G;
        if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
            clefId = ClefType::PERC;
            StaffTypes type = StaffTypes::PERC_DEFAULT;
            if (auto it = drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.find(name.toStdString());
                it != drumset::PERC_STAFF_LINES_FROM_INSTRUMENT.end()) {
                drumset::initGuitarProPercussionSet(it->second);
                drumset::setInstrumentDrumset(instr, it->second);
                switch (it->second.numLines) {
                case 1:
                    type = StaffTypes::PERC_1LINE;
                    break;
                case 2:
                    type = StaffTypes::PERC_2LINE;
                    break;
                case 3:
                    type = StaffTypes::PERC_3LINE;
                    break;
                default:
                    type = StaffTypes::PERC_DEFAULT;
                    break;
                }
            } else {
                drumset::initGuitarProDrumset();
                instr->setDrumset(drumset::gpDrumset);
            }
            staff->setStaffType(Fraction(0, 1), *StaffType::preset(type));
        } else {
            clefId = defaultClef(patch);
        }

        Measure* measure = score->firstMeasure();
        if (measure->tick() != Fraction(0, 1)) {
            Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));

            Clef* clef = Factory::createClef(segment);
            clef->setClefType(clefId);
            clef->setTrack(i * VOICES);
            segment->add(clef);
        } else {
            staff->setDefaultClefType(ClefTypeList(clefId, clefId));
        }

        if (capo > 0 && !engravingConfiguration()->guitarProImportExperimental()) {
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            StaffText* st = new StaffText(s);
            st->setPlainText(String(u"Capo. fret ") + String::number(capo));
            st->setTrack(i * VOICES);
            s->add(st);
        }

        InstrChannel* ch = instr->channel(0);
        if (midiChannel == GP_DEFAULT_PERCUSSION_CHANNEL) {
            ch->setProgram(0);
            ch->setBank(128);
        } else {
            ch->setProgram(patch);
            ch->setBank(0);
        }
        ch->setVolume(channelDefaults[midiChannel].volume);
        ch->setPan(channelDefaults[midiChannel].pan);
        ch->setChorus(channelDefaults[midiChannel].chorus);
        ch->setReverb(channelDefaults[midiChannel].reverb);
        staff->part()->setMidiChannel(midiChannel, midiPort);
        // missing: phase, tremolo
    }

    slurs.resize(staves, nullptr);
    tupleKind.resize(staves);
    for (auto& i : tupleKind) {
        i = 0;
    }

    Measure* measure = score->firstMeasure();
    bool mixChange = false;
    bool lastSlurAdd = false;
    for (size_t bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
        if (!f->isReadable()) {
            break;
        }
        const GpBar& gpbar = bars[bar];
        if (!gpbar.marker.isEmpty()) {
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            RehearsalMark* s = new RehearsalMark(segment);
            s->setPlainText(gpbar.marker.trimmed());
            s->setTrack(0);
            segment->add(s);
        }

        std::vector<Tuplet*> tuplets(staves);
        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            tuplets[staffIdx] = 0;
        }

        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            Fraction measureLen = { 0, 1 };
            Fraction tick  = measure->tick();
            int beats = readInt();
            track_idx_t track = staffIdx * VOICES;

            if (!f->isReadable()) {
                break;
            }
            for (int beat = 0; beat < beats; ++beat) {
                slide = -1;
                if (muse::contains(slides, static_cast<int>(track))) {
                    slide = muse::take(slides, static_cast<int>(track));
                }

                uint8_t beatBits = readUInt8();
                bool dotted = beatBits & 0x1;
                int pause = -1;
                if (beatBits & BEAT_PAUSE) {
                    pause = readUInt8();
                }
                int len = readChar();
                int tuple = 0;
                if (beatBits & BEAT_TUPLET) {
                    tuple = readInt();
                }
                Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
                if (beatBits & BEAT_CHORD) {
                    size_t numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
                    int header = readUInt8();
                    String name;
                    if ((header & 1) == 0) {
                        name = readDelphiString();
                        readChord(segment, static_cast<int>(staffIdx * VOICES), static_cast<int>(numStrings), name, false);
                    } else {
                        skip(16);
                        name = readPascalString(21);
                        skip(4);
                        readChord(segment, static_cast<int>(staffIdx * VOICES), static_cast<int>(numStrings), name, true);
                        skip(32);
                    }
                }

                Lyrics* lyrics = 0;
                if (beatBits & BEAT_LYRICS) {
                    lyrics = Factory::createLyrics(score->dummy()->chord());
                    auto str = readDelphiString();
                    //TODO-ws					str.erase(std::remove_if(str.begin(), str.end(), [](char c){return c == '_'; }), str.end());
                    lyrics->setPlainText(str);
                }
                gpLyrics.beatCounter++;
                if (gpLyrics.beatCounter >= gpLyrics.fromBeat && static_cast<size_t>(gpLyrics.lyricTrack) == staffIdx + 1) {
                    size_t index = gpLyrics.beatCounter - gpLyrics.fromBeat;
                    if (index < gpLyrics.lyrics.size()) {
                        lyrics = Factory::createLyrics(score->dummy()->chord());
                        lyrics->setPlainText(gpLyrics.lyrics[index]);
                    }
                }
                int beatEffects = 0;
                if (beatBits & BEAT_EFFECTS) {
                    beatEffects = readBeatEffects(static_cast<int>(track), segment);
                }
                last_segment = segment;
                if (beatBits & BEAT_MIX_CHANGE) {
                    readMixChange(measure);
                    mixChange = true;
                }
                int strings = readUInt8();           // used strings mask
                Fraction l  = len2fraction(len);

                // Some beat effects could add a Chord before this
                ChordRest* cr = segment->cr(track);

                if (strings == 0) {
                    if (segment->cr(track)) {
                        segment->remove(segment->cr(track));
                        delete cr;
                        cr = 0;
                    }
                    cr = Factory::createRest(score->dummy()->segment());
                } else {
                    if (!segment->cr(track)) {
                        cr = Factory::createChord(score->dummy()->segment());
                    }
                }
                cr->setParent(segment);
                cr->setTrack(track);
                if (lyrics) {
                    cr->add(lyrics);
                }

                TDuration d(l);
                d.setDots(dotted ? 1 : 0);

                if (dotted) {
                    l = l + (l * Fraction(1, 2));
                }
                if (tuple) {
                    Tuplet* tuplet = tuplets[staffIdx];
                    if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                        tuplet = Factory::createTuplet(measure);
                        tuplet->setTick(tick);
                        tuplet->setTrack(cr->track());
                        tuplets[staffIdx] = tuplet;
                        setTuplet(tuplet, tuple);
                        tuplet->setParent(measure);
                    }
                    tuplet->setTrack(track);
                    tuplet->setBaseLen(l);
                    tuplet->setTicks(l * tuplet->ratio().denominator());
                    cr->setTuplet(tuplet);
                    tuplet->add(cr);
                } else {
                    tuplets[staffIdx] = 0;            // needed?
                }
                cr->setTicks(l);
                if (cr->isRest() && (pause == 0 || l >= measure->ticks())) {
                    cr->setDurationType(DurationType::V_MEASURE);
                    cr->setTicks(measure->ticks());
                } else {
                    cr->setDurationType(d);
                }
                if (!segment->cr(track)) {
                    segment->add(cr);
                }
                Staff* staff   = cr->staff();
                size_t numStrings = staff->part()->instrument()->stringData()->strings();
                bool hasSlur   = false;
                bool hasHammerOnPullOff = false;
                bool hasLetRing = false;
                bool hasPalmMute = false;
                bool hasTrill = false;
                bool hasVibratoLeftHand = false;
                bool hasVibratoWTremBar = false;
                bool hasHarmonicArtificial = false;
                bool hasHarmonicPinch = false;
                bool hasHarmonicTap = false;
                bool hasHarmonicSemi = false;

                int dynam      = -1;
                if (cr && cr->isChord()) {
                    for (int i = 6; i >= 0; --i) {
                        if (strings & (1 << i) && ((6 - i) < static_cast<int>(numStrings))) {
                            Note* note = Factory::createNote(toChord(cr));
                            // apply dotted notes to the note
                            if (dotted) {
                                // there is at most one dotted note in this guitar pro version
                                NoteDot* dot = Factory::createNoteDot(note);
                                dot->setParent(note);
                                dot->setTrack(track);                  // needed to know the staff it belongs to (and detect tablature)
                                dot->setVisible(true);
                                note->add(dot);
                            }
                            toChord(cr)->add(note);

                            ReadNoteResult readResult = readNote(6 - i, static_cast<int>(staffIdx), note);
                            hasSlur = readResult.slur || hasSlur;
                            hasHammerOnPullOff = readResult.hammerOnPullOff || hasHammerOnPullOff;
                            hasLetRing = readResult.letRing || hasLetRing;
                            hasPalmMute = readResult.palmMute || hasPalmMute;
                            hasTrill = readResult.trill || hasTrill;
                            hasVibratoLeftHand = readResult.vibrato || hasVibratoLeftHand;
                            hasHarmonicArtificial = readResult.harmonicArtificial || hasHarmonicArtificial;
                            hasHarmonicPinch = readResult.harmonicPinch || hasHarmonicPinch;
                            hasHarmonicTap = readResult.harmonicTap || hasHarmonicTap;
                            hasHarmonicSemi = readResult.harmonicSemi || hasHarmonicSemi;
                            dynam = std::max(dynam, previousDynamicByTrack[track]);
                            note->setTpcFromPitch();
                        }
                    }

                    bool hasVibratoLeftHandOnBeat = false;
                    bool hasVibratoWTremBarOnBeat = false;
                    applyBeatEffects(toChord(cr), beatEffects, hasVibratoLeftHandOnBeat, hasVibratoWTremBarOnBeat);
                    hasVibratoLeftHand = hasVibratoLeftHand || hasVibratoLeftHandOnBeat;
                    hasVibratoWTremBar = hasVibratoWTremBar || hasVibratoWTremBarOnBeat;
                    if (dynam != curDynam[track]) {
                        curDynam[track] = dynam;
                        addDynamic(toChord(cr)->notes().front(), dynam);
                    }
                }

                if (cr) {
                    addLetRing(cr, hasLetRing);
                    addPalmMute(cr, hasPalmMute);
                    addTrill(cr, hasTrill);
                    addHammerOnPullOff(cr, hasHammerOnPullOff);
                    addVibratoLeftHand(cr, hasVibratoLeftHand);
                    addVibratoWTremBar(cr, hasVibratoWTremBar);
                    addHarmonicMarks(cr, hasHarmonicArtificial, hasHarmonicPinch, hasHarmonicTap, hasHarmonicSemi);
                }

                // if we see that a tied note has been constructed do not create the tie
                if (slides[static_cast<int>(track)] == -2) {
                    slide = 0;
                    slides[static_cast<int>(track)] = -1;
                }
                bool slurSwap = true;
                if (slide != 2) {
                    if (hasSlur && (slurs[staffIdx] == 0)) {
                        Slur* slur = Factory::createSlur(score->dummy());
                        slur->setParent(0);
                        slur->setTrack(track);
                        slur->setTrack2(track);
                        slur->setTick(cr->tick());
                        slur->setTick2(cr->tick());
                        slur->setStartElement(cr);
                        slurs[staffIdx] = slur;
                        score->addElement(slur);
                    } else if (slurs[staffIdx] && !hasSlur) {
                        // TODO: check slur
                        Slur* s = slurs[staffIdx];
                        slurs[staffIdx] = 0;
                        s->setTick2(cr->tick());
                        s->setTrack2(cr->track());
                        s->setEndElement(cr);
                        if (cr->isChord()) {
                            lastSlurAdd = true;
                            slurSwap = false;
                        }
                        //TODO-ws                           cr->has_slur = true;
                    } else if (slurs[staffIdx] && hasSlur) {
                    }
                }
                if (cr && (cr->isChord()) && slide > 0) {
                    auto chord = toChord(cr);
                    auto effect = convertGP4SlideNum(slide);
                    if (slide > 2) {
                        createSlide(convertGP4SlideNum(slide), cr, static_cast<int>(staffIdx));
                    }
                    if (slide < 3 || effect == SLIDE_OUT_UP) {
                        Note* last = chord->upNote();
                        auto seg = chord->segment();
                        Measure* mes = seg->measure();
                        while ((seg = seg->prev()) || (mes = mes->prevMeasure())) {
                            if (!seg) {
                                break;                //seg = mes->last();
                            }
                            if (seg->segmentType() == SegmentType::ChordRest
                                && seg->cr(chord->track()) && seg->cr(chord->track())->isChord()) {
                                bool br = false;
                                Chord* cr1 = toChord(seg->cr(chord->track()));
                                if (cr1) {
                                    for (auto n : cr1->notes()) {
                                        if (n->string() == last->string()) {
                                            Glissando* s = mu::engraving::Factory::createGlissando(n);
                                            s->setAnchor(Spanner::Anchor::NOTE);
                                            s->setStartElement(n);
                                            s->setTick(seg->tick());
                                            s->setTrack(chord->track());
                                            s->setParent(n);
                                            s->setGlissandoType(GlissandoType::STRAIGHT);
                                            s->setEndElement(last);
                                            s->setTick2(chord->segment()->tick());
                                            s->setTrack2(chord->track());
                                            score->addElement(s);
                                            if (slide == 2 || effect == SLIDE_OUT_UP) {
                                                if (!lastSlurAdd) {
                                                    createSlur(true, chord->staffIdx(), cr1);
                                                    createSlur(false, chord->staffIdx(), chord);
                                                }
                                            }
                                            br = true;
                                            break;
                                        }
                                    }
                                }
                                if (br) {
                                    break;
                                }
                            }
                        }
                    }
                }
                if (slurSwap) {
                    lastSlurAdd = false;
                }
                restsForEmptyBeats(segment, measure, cr, l, static_cast<int>(track), tick);
                createSlur(hasSlur, staffIdx, cr);
                tick += cr->actualTicks();
                measureLen += cr->actualTicks();
            }
            if (measureLen < measure->ticks()) {
                score->setRest(tick, track, measure->ticks() - measureLen, false, nullptr, false);
                m_continiousElementsBuilder->notifyUncompletedMeasure();
            }
        }

        if (bar == 1 && !mixChange) {
            setTempo(temp, score->firstMeasure());
        }
    }

    for (auto n : slideList) {
        Segment* segment = n->chord()->segment();
        Measure* measure1 = segment->measure();
        int segment_counter = 0;
        while ((segment = segment->next1(SegmentType::ChordRest))
               || ((measure1 = measure1->nextMeasure()) && (segment = measure1->first()))) {
            if (!segment->isChordRestType()) {
                continue;
            }
            bool br = false;
            ChordRest* cr = segment->cr(n->track());
            if (cr && cr->isChord()) {
                Chord* c = toChord(cr);
                ++segment_counter;
                if (segment_counter > 2) {
                    break;
                }
                for (Note* nt : c->notes()) {
                    if (nt->string() == n->string()) {
                        for (auto e : nt->el()) {
                            if (e->isChordLine()) {
                                auto cl = toChordLine(e);
                                if (cl->chordLineType() == ChordLineType::PLOP || cl->chordLineType() == ChordLineType::SCOOP) {
                                    br = true;
                                    break;
                                }
                            }
                        }
                        if (br) {
                            break;
                        }
                        Glissando* s = new Glissando(n);
                        s->setAnchor(Spanner::Anchor::NOTE);
                        s->setStartElement(n);
                        s->setTick(n->chord()->segment()->tick());
                        s->setTrack(n->track());
                        s->setParent(n);
                        s->setGlissandoType(GlissandoType::STRAIGHT);
                        s->setEndElement(nt);
                        s->setTick2(nt->chord()->segment()->tick());
                        s->setTrack2(n->track());
                        score->addElement(s);
                        br = true;
                        break;
                    }
                }
                if (br) {
                    break;
                }
            }
        }
    }

    m_continiousElementsBuilder->addElementsToScore();
    m_guitarBendImporter->applyBendsToChords();

    return true;
}
} // namespace mu::iex::guitarpro
