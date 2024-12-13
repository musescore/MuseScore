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

#include "containers.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/arpeggio.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/box.h"
#include "engraving/dom/bracket.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordline.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/excerpt.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/glissando.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/measurebase.h"
#include "engraving/dom/note.h"
#include "engraving/dom/notedot.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/stafftype.h"
#include "engraving/dom/stringdata.h"
#include "types/symid.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/fretcircle.h"

#include "utils.h"

#include "log.h"

using namespace muse::io;
using namespace mu::engraving;

namespace mu::iex::guitarpro {
static TremoloType tremoloType(int division)
{
    static const std::map<int, TremoloType> types {
        { 1, TremoloType::R8 },
        { 2, TremoloType::R16 },
        { 3, TremoloType::R32 }
    };

    auto it = types.find(division);
    if (it != types.end()) {
        return it->second;
    }

    LOGE() << "wrong tremolo type";
    return TremoloType::INVALID_TREMOLO;
}

//---------------------------------------------------------
//   readInfo
//---------------------------------------------------------

void GuitarPro5::readInfo()
{
    title        = readDelphiString();
    subtitle     = readDelphiString();
    artist       = readDelphiString();
    album        = readDelphiString();
    composer     = readDelphiString();
    readDelphiString();
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
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro5::readBeatEffects(int track, Segment* segment)
{
    int effects = 0;

    uint8_t fxBits1 = readUInt8();
    uint8_t fxBits2 = readUInt8();
    if (fxBits1 & BEAT_FADE) {
        effects = 4;      // fade in
    }
    if (fxBits1 & BEAT_EFFECT) {
        int k = readUInt8();
        effects = k + effects * 100;    // &effects;
    }
    if (fxBits1 & BEAT_VIBRATO_TREMOLO) {
        effects = 7 + effects * 100;
    }
    if (fxBits2 & BEAT_TREMOLO) {
        readTremoloBar(track, segment);           // readBend();
    }
    if (fxBits2 & BEAT_RASGUEADO) {
        m_currentBeatHasRasgueado = true;
    }
    if (fxBits1 & BEAT_ARPEGGIO) {
        int strokeup = readUInt8();                // up stroke length
        int strokedown = readUInt8();                // down stroke length

        Arpeggio* a = Factory::createArpeggio(score->dummy()->chord());
        // representation is different in guitar pro 5 - the up/down order below is correct
        if (strokeup > 0) {
            a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
            if (strokeup < 7) {
                a->setStretch(1.0 / std::pow(2, 6 - strokeup));
            }
        } else if (strokedown > 0) {
            a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
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
        effects = readChar();                // stroke pick direction
        effects += 4;     //1 or 2 for effects becomes 4 or 5
    }
    return effects;
}

//---------------------------------------------------------
//   readPageSetup
//---------------------------------------------------------

void GuitarPro5::readPageSetup()
{
    skip(version > 500 ? 49 : 30);
    for (int i = 0; i < 11; ++i) {
        skip(4);
        readBytePascalString();
    }
}

//---------------------------------------------------------
//   readBeat
//---------------------------------------------------------

Fraction GuitarPro5::readBeat(const Fraction& tick, int voice, Measure* measure, int staffIdx, Tuplet** tuplets,
                              bool /*mixChange*/)
{
    uint8_t beatBits = readUInt8();
    bool dotted    = beatBits & BEAT_DOTTED;
    bool hasSlur = false;
    bool hasLetRing = false;
    bool hasPalmMute = false;
    bool hasTrill = false;
    bool hasVibratoLeftHand = false;
    bool hasVibratoWTremBar = false;
    bool hasHarmonicArtificial = false;
    bool hasHarmonicPinch = false;
    bool hasHarmonicTap = false;
    bool hasHarmonicSemi = false;

    m_currentBeatHasRasgueado = false;

    slide = -1;
    int track = staffIdx * VOICES + voice;
    if (muse::contains(slides, track)) {
        slide = muse::take(slides, track);
    }

    int pause = -1;
    if (beatBits & BEAT_PAUSE) {
        pause = readUInt8();
    }

    // readDuration
    int len   = readChar();
    int tuple = 0;
    if (beatBits & BEAT_TUPLET) {
        tuple = readInt();
    }

    Segment* segment = measure->getSegment(SegmentType::ChordRest, tick);
    if (beatBits & BEAT_CHORD) {
        size_t numStrings = score->staff(staffIdx)->part()->instrument()->stringData()->strings();
        skip(17);
        String name;
        {
            uint8_t charsCount = readUInt8();
            char c[21];
            f->read((uint8_t*)(c), 21);
            // Just in case
            if (charsCount <= 20) {
                c[charsCount] = '\0';
            } else {
                c[20] = '\0';
            }
            name = String::fromUtf8(c);
        }
        skip(4);
        // no header to be read in the GP5 format - default to true.
        readChord(segment, staffIdx * VOICES, static_cast<int>(numStrings), name, true);
        skip(32);
    }
    Lyrics* lyrics = 0;
    String free_text;
    if (beatBits & BEAT_LYRICS) {
        /// it is not beat lirics, it is free text
        free_text = readDelphiString();
    }

    int beatEffects = 0;
    if (beatBits & BEAT_EFFECTS) {
        beatEffects = readBeatEffects(track, segment);
    }

    last_segment = segment;
    if (beatBits & BEAT_MIX_CHANGE) {
        readMixChange(measure);
    }

    int strings = readUInt8();     // used strings mask

    Fraction l  = len2fraction(len);

    // Some beat effects could add a Chord before this
    ChordRest* cr = segment->cr(track);
    if (voice != 0 && pause == 0 && strings == 0) {
        cr = 0;
    } else {
        if (strings == 0) {
            if (cr) {
                segment->remove(cr);
                delete cr;
                cr = 0;
            }
            cr = Factory::createRest(score->dummy()->segment());
        } else {
            if (!cr) {
                cr = Factory::createChord(score->dummy()->segment());
            }
        }
        cr->setParent(segment);
        cr->setTrack(track);

        TDuration d(l);
        d.setDots(dotted ? 1 : 0);

        if (dotted) {
            l = l + (l * Fraction(1, 2));
        }

        if (tuple) {
            int track2 = staffIdx * 2 + voice;
            Tuplet* tuplet = tuplets[track2];
            if ((tuplet == nullptr) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                tuplet = Factory::createTuplet(measure);
                tuplets[track2] = tuplet;
                setTuplet(tuplet, tuple);
                tuplet->setParent(measure);
            }
            tuplet->setTrack(cr->track());
            tuplet->setBaseLen(l);
            tuplet->setTick(tick);
            tuplet->setTicks(l * tuplet->ratio().denominator());
            cr->setTuplet(tuplet);
            tuplet->add(cr);
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

        Staff* staff = cr->staff();
        size_t numStrings = staff->part()->instrument()->stringData()->strings();

        Note* _note = nullptr;
        std::vector<Note*> delnote;
        for (int i = 6; i >= 0; --i) {
            if (strings & (1 << i) && ((6 - i) < static_cast<int>(numStrings))) {
                Note* note = Factory::createNote(toChord(cr));
                _note = note;
                if (dotted) {
                    // there is at most one dotted note in this guitar pro version
                    NoteDot* dot = Factory::createNoteDot(note);
                    dot->setParent(note);
                    dot->setTrack(track);            // needed to know the staff it belongs to (and detect tablature)
                    dot->setVisible(true);
                    note->add(dot);
                }
                toChord(cr)->add(note);

                ReadNoteResult readResult = readNote(6 - i, note);
                hasSlur = readResult.slur;
                hasLetRing = readResult.letRing || hasLetRing;
                hasPalmMute = readResult.palmMute || hasPalmMute;
                hasTrill = readResult.trill || hasTrill;
                hasVibratoLeftHand = readResult.vibrato || hasVibratoLeftHand;
                hasHarmonicArtificial = readResult.harmonicArtificial || hasHarmonicArtificial;
                hasHarmonicPinch = readResult.harmonicPinch || hasHarmonicPinch;
                hasHarmonicTap = readResult.harmonicTap || hasHarmonicTap;
                hasHarmonicSemi = readResult.harmonicSemi || hasHarmonicSemi;
                if (!slideList.empty() && slideList.back() == nullptr) {
                    slideList.back() = note;
                    hasSlur = true;
                }
                if (note->fret() == -20) {
                    delnote.push_back(note);
                } else {
                    note->setTpcFromPitch(Prefer::SHARPS);
                }
            }
        }
        if (!delnote.empty()) {
            Chord* chord = toChord(cr);
            for (auto n : delnote) {
                chord->remove(n);
                delete n;
            }
            if (chord->notes().empty()) {
                if (chord->tuplet()) {
                    chord->tuplet()->remove(chord);
                }
                chord->segment()->remove(chord);
                delete chord;
                cr = nullptr;
            }
            delnote.clear();
        }
        createSlur(hasSlur, staffIdx, cr);
        if (lyrics) {
            cr->add(lyrics);
        }
        if (free_text.size() && _note) {
            addTextToNote(free_text, _note);
        }
    }

    int rr = readChar();
    if (cr && cr->isChord()) {
        Chord* chord = toChord(cr);

        if (engravingConfiguration()->enableExperimentalFretCircle()) {
            FretCircle* c = Factory::createFretCircle(chord);
            chord->add(c);
        }

        bool hasVibratoLeftHandOnBeat = false;
        bool hasVibratoWTremBarOnBeat = false;

        do {
            applyBeatEffects(chord, beatEffects % 100, hasVibratoLeftHandOnBeat, hasVibratoWTremBarOnBeat);
            hasVibratoLeftHand = hasVibratoLeftHand || hasVibratoLeftHandOnBeat;
            hasVibratoWTremBar = hasVibratoWTremBar || hasVibratoWTremBarOnBeat;
        } while (beatEffects /= 100);
        if (rr == ARPEGGIO_DOWN) {
            chord->setStemDirection(DirectionV::DOWN);
        } else if (rr == ARPEGGIO_UP) {
            chord->setStemDirection(DirectionV::UP);
        }
    }

    if (cr) {
        //  fixing gp5 bug with not storying let ring for tied notes
        if (hasLetRing) {
            m_letRingForChords.insert(cr);
        }

        addLetRing(cr, hasLetRing);
        addPalmMute(cr, hasPalmMute);
        addTrill(cr, hasTrill);
        addRasgueado(cr, m_currentBeatHasRasgueado);
        addVibratoLeftHand(cr, hasVibratoLeftHand);
        addVibratoWTremBar(cr, hasVibratoWTremBar);
        addHarmonicMarks(cr, hasHarmonicArtificial, hasHarmonicPinch, hasHarmonicTap, hasHarmonicSemi);
    }

    int r = readChar();
    if (r & 0x8) {
        int rrr = readChar();
        LOGD("  3beat read 0x%02x", rrr);
    }
    if (cr && cr->isChord()) {
        if (toChord(cr)->notes().size() == 0) {
            segment->remove(cr);
            delete cr;
            cr = 0;
        } else if (slide > 0) {
            createSlide(slide, cr, staffIdx);
        }
    }
    restsForEmptyBeats(segment, measure, cr, l, track, tick);
    return cr ? cr->actualTicks() : measure->ticks();
}

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

void GuitarPro5::readMeasure(Measure* measure, int staffIdx, Tuplet** tuplets, bool mixChange)
{
    for (int voice = 0; voice < 2; ++voice) {
        Fraction measureLen = { 0, 1 };
        Fraction tick       = measure->tick();
        int beats           = readInt();
        if (beats > 100) {
            return;
        }
        for (int beat = 0; beat < beats; ++beat) {
            Fraction ticks = readBeat(tick, voice, measure, staffIdx, tuplets, mixChange);
            ++_beat_counter;
            tick += ticks;
            measureLen += ticks;
        }
        if (measureLen < measure->ticks()) {
            score->setRest(tick, staffIdx * VOICES + voice, measure->ticks() - measureLen, false, nullptr, false);
            m_continiousElementsBuilder->notifyUncompletedMeasure();
        }
    }
}

//---------------------------------------------------------
//   readMixChange
//---------------------------------------------------------

bool GuitarPro5::readMixChange(Measure* measure)
{
    /*char patch   =*/
    readChar();
    skip(16);
    signed char volume  = readChar();
    signed char pan     = readChar();
    signed char chorus  = readChar();
    signed char reverb  = readChar();
    signed char phase   = readChar();
    signed char tremolo = readChar();
    readDelphiString();                   // tempo name

    int temp = readInt();
    bool editedTempo = false;
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
            score->setTempo(last_segment->tick(), BeatsPerSecond::fromBPM(temp));
            last_segment = nullptr;
        }
        if (temp != previousTempo) {
            previousTempo = temp;
            setTempo(temp, measure);
            editedTempo = true;
        }
        readChar();
        if (version > 500) {
            readChar();
        }
    }
    readChar();
    readChar();
    if (version > 500) {
        readDelphiString();
        readDelphiString();
    }
    return editedTempo;
}

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

bool GuitarPro5::readTracks()
{
    for (size_t i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];
        Staff* staff = score->staff(i);
        Part* part = staff->part();

        uint8_t c = readUInt8();       // simulations bitmask
        if (c & 0x2) {               // 12 stringed guitar
        }
        if (c & 0x4) {               // banjo track
        }
        if (i == 0 || version == 500) {
            skip(1);
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
        /*int midiChannel2 =*/ readInt();       // -1

        int frets        = readInt();
        int capo         = readInt();
        part->setCapoFret(capo);
        /*int color        =*/ readInt();

        skip(version > 500 ? 49 : 44);
        if (version > 500) {
            //  british stack clean / amp tone
            readDelphiString();
            readDelphiString();
        }
        std::vector<int> tuning2(strings);
        //int tuning2[strings];
        for (int k = 0; k < strings; ++k) {
            tuning2[strings - k - 1] = tuning[k];
        }
        bool useFlats = createTuningString(strings, &tuning2[0]);
        StringData stringData(frets, strings, &tuning2[0], useFlats);
        Instrument* instr = part->instrument();
        instr->setStringData(stringData);
        instr->setSingleNoteDynamics(false);
        part->setPartName(name);
        part->setPlainLongName(name);

        //
        // determine clef
        //
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
            st->setPlainText(u"Capo. fret " + String::number(capo));
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
    skip(version == 500 ? 2 : 1);

    return true;
}

//---------------------------------------------------------
//   readMeasures
//---------------------------------------------------------

void GuitarPro5::readMeasures(int /*startingTempo*/)
{
    Measure* measure = score->firstMeasure();
    bool mixChange = false;
    for (size_t bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
        const GpBar& gpbar = bars[bar];

        if (!gpbar.marker.isEmpty()) {
            Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
            RehearsalMark* s = Factory::createRehearsalMark(segment);
            s->setType(RehearsalMark::Type::Additional);
            s->setPlainText(gpbar.marker.trimmed());
            s->setTrack(0);
            segment->add(s);
        }

        std::vector<Tuplet*> tuplets(staves * 2);
        //Tuplet* tuplets[staves * 2];     // two voices
        for (size_t track = 0; track < staves * 2; ++track) {
            tuplets[track] = 0;
        }

        for (size_t staffIdx = 0; staffIdx < staves; ++staffIdx) {
            _beat_counter = 0;
            readMeasure(measure, static_cast<int>(staffIdx), &tuplets[0], mixChange);
            if (!(((bar == (measures - 1)) && (staffIdx == (staves - 1))))) {
                /*int a = */
                readChar();
            }
        }
        if (bar == 1 && !mixChange) {
            setTempo(tempo, score->firstMeasure());
        }
    }

    if (!gpLyrics.segments.empty()) {
        size_t size = std::min(gpLyrics.segments.size(), gpLyrics.lyrics.size());
        for (size_t i = 0; i < size; ++i) {
            std::string str = gpLyrics.lyrics[i].toStdString();
            auto seg = gpLyrics.segments[i];
            auto mes = seg->measure();
            while (str.size() && seg && seg->segmentType() == SegmentType::ChordRest) {
                auto cr = seg->cr(gpLyrics.lyricTrack);
                if (cr) {
                    if (str[0] != '-') {
                        Lyrics* lyr = Factory::createLyrics(score->dummy()->chord());

                        std::string text;
                        auto pos = str.find('-');
                        auto pos2 = str.find('\n');
                        if (pos2 < pos) {
                            pos = pos2;
                        }
                        if (pos != std::string::npos) {
                            const char* c = &str.c_str()[pos + 1];
                            if (*c == 0) {
                                pos = std::string::npos;
                                text = str;
                            } else {
                                text = str.substr(0, pos);
                                str = str.substr(pos + 1);
                            }
                        } else {
                            text = str;
                        }
                        if (pos == std::string::npos) {
                            str.resize(0);
                        }
                        lyr->setPlainText(String::fromStdString(text));
                        cr->add(lyr);
                    } else {
                        str = str.substr(1);
                    }
                }
                seg = seg->next();
                if (!seg) {
                    mes = mes->nextMeasure();
                    if (!mes) {
                        break;
                    }
                    seg = mes->first();
                }
            }
        }
    } else if (gpLyrics.lyricTrack != 0) {
        size_t counter = 0;
        size_t index = 0;
        gpLyrics.lyricTrack -= 1;
        Measure* mes = score->firstMeasure();
        if (!mes) {
            return;
        }

        Segment* beg = mes->first();

        do {
            if (beg->segmentType() == SegmentType::ChordRest && beg->cr(gpLyrics.lyricTrack)) {
                ChordRest* cr = beg->cr(gpLyrics.lyricTrack);
                assert(cr);
                ++counter;
                if (cr->type() != ElementType::CHORD) {
                    continue;
                }

                bool isTied = false;
                Chord* chord = toChord(cr);
                for (Note* n : chord->notes()) {
                    const auto& tiedNotes = n->tiedNotes();

                    if (tiedNotes.size() > 1 && tiedNotes.front() != n) {
                        isTied = true;
                        break;
                    }
                }

                if (isTied) {
                    continue;
                }

                if (counter >= gpLyrics.fromBeat) {
                    if (gpLyrics.lyricPos.size() == index) {
                        gpLyrics.lyricPos.push_back(0);
                    }

                    String& lyricsInIndex = gpLyrics.lyrics[index];
                    if (lyricsInIndex[0] != u'-') {
                        Lyrics* lyr = mu::engraving::Factory::createLyrics(cr);

                        String text;
                        size_t pos = lyricsInIndex.indexOf(u'-', gpLyrics.lyricPos[index]);
                        size_t pos2 = lyricsInIndex.indexOf(u'\n', gpLyrics.lyricPos[index]);
                        if (pos2 < pos) {
                            pos = pos2;
                        }

                        if (pos != std::string::npos && (pos + 1 < lyricsInIndex.size())) {
                            const char16_t* c = &lyricsInIndex[pos + 1];
                            if (*c == 0) {
                                pos = std::string::npos;
                                text = lyricsInIndex;
                            } else {
                                text = lyricsInIndex.mid(gpLyrics.lyricPos[index], pos + 1 - gpLyrics.lyricPos[index]);
                                gpLyrics.lyricPos[index] = pos + 1;
                            }
                        } else {
                            text = lyricsInIndex.mid(gpLyrics.lyricPos[index]);
                        }

                        if (pos == std::string::npos) {
                            ++index;
                        }

                        lyr->setPlainText(text);
                        cr->add(lyr);
                        if (index >= gpLyrics.lyrics.size()) {
                            break;
                        }
                    } else if (lyricsInIndex.size() >= 2) {
                        lyricsInIndex = lyricsInIndex.mid(1);
                    }
                }
            }
        } while ((beg = beg->next())
                 || (mes->next() && mes->next()->type() == ElementType::MEASURE && (mes = toMeasure(mes->next())) && (beg = mes->first())));
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro5::read(IODevice* io)
{
    m_continiousElementsBuilder = std::make_unique<ContiniousElementsBuilder>(score);
    if (engravingConfiguration()->experimentalGuitarBendImport()) {
        m_guitarBendImporter = std::make_unique<GuitarBendImporter>(score);
    }

    f = io;

    readInfo();
    readLyrics();
    readPageSetup();

    previousTempo = -1;

    tempo = readInt();
    if (version > 500) {
        skip(1);
    }

    key    = readInt();
    /* int octave =*/ readChar();      // octave

    readChannels();

    std::vector<unsigned int> articulations;
    articulations.resize(19);
    {
        unsigned int r;
        unsigned char x;
        for (int i = 0; i < 19; ++i) {
            x = readUInt8();
            r = x;
            x = readUInt8();
            r += x << 8;

            articulations[i] = r;
        }
    }

    //skip(42);
    skip(4);

    measures = readInt();
    staves  = readInt();
    initDynamics(staves);

    for (size_t str = 0; str < 7; ++str) {
        for (size_t staff = 0; staff < staves; ++staff) {
            dead_end[{ static_cast<int>(staff), static_cast<int>(str) }] = true;
        }
    }

    slurs = new Slur*[staves];
    for (size_t i = 0; i < staves; ++i) {
        slurs[i] = 0;
    }

    int tnumerator   = 4;
    int tdenominator = 4;
    for (size_t i = 0; i < measures; ++i) {
        if (i > 0) {
            skip(1);
        }
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
        if (barBits & SCORE_MARKER) {
            bar.marker = readDelphiString();           // new section?
            /*int color =*/ readInt();          // color?
        }
        if (barBits & SCORE_VOLTA) {                          // a volta
            uint8_t voltaNumber = readUInt8();
            while (voltaNumber > 0) {
                // voltas are represented as flags
                bar.volta.voltaType = GP_VOLTA_FLAGS;
                bar.volta.voltaInfo.push_back(voltaNumber & 1);
                voltaNumber >>= 1;
            }
        }
        if (barBits & SCORE_KEYSIG) {
            int currentKey = readUInt8();
            /* key signatures are specified as
             * 1# = 1, 2# = 2, ..., 7# = 7
             * 1b = 255, 2b = 254, ... 7b = 249 */
            bar.keysig = currentKey <= 7 ? currentKey : -256 + currentKey;
            readUInt8();              // specified major/minor mode
        }
        if (barBits & SCORE_DOUBLE_BAR) {
            bar.barLine = BarLineType::DOUBLE;
        }
        if (barBits & 0x3) {
            skip(4);
        }
        if ((barBits & 0x10) == 0) {
            skip(1);
        }

        readChar();                 // triple feel  (none, 8, 16)
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
    if (!readTracks()) {
        return false;
    }
    readMeasures(tempo);
    for (auto n : slideList) {
        auto segment = n->chord()->segment();
        auto measure = segment->measure();
        while ((segment = segment->next1(SegmentType::ChordRest)) || ((measure = measure->nextMeasure()) && (segment = measure->first()))) {
            if (segment->segmentType() != SegmentType::ChordRest) {
                continue;
            }
            bool br = false;
            ChordRest* cr = toChordRest(segment->cr(n->track()));
            if (cr && cr->isChord()) {
                Chord* c = toChord(cr);
                for (auto nt : c->notes()) {
                    if (nt->string() == n->string()) {
                        for (auto e : nt->el()) {
                            if (e->isChordLine()) {
                                ChordLine* cl = toChordLine(e);
                                if (cl->chordLineType() == ChordLineType::PLOP || cl->chordLineType() == ChordLineType::SCOOP) {
                                    br = true;
                                    break;
                                }
                            }
                        }
                        if (br) {
                            break;
                        }
                        Glissando* s = mu::engraving::Factory::createGlissando(n);
                        s->setAnchor(Spanner::Anchor::NOTE);
                        s->setStartElement(n);
                        s->setTick(n->chord()->segment()->tick());
                        s->setTrack(n->track());
                        s->setParent(n);
                        s->setGlissandoType(GlissandoType::STRAIGHT);
                        s->setGlissandoShift(true);
                        s->setEndElement(nt);
                        s->setTick2(nt->chord()->segment()->tick());
                        s->setTrack2(n->track());

                        for (Spanner* spanner : n->chord()->startingSpanners()) {
                            if (spanner && spanner->isSlur()) {
                                Slur* slur = toSlur(spanner);
                                if (slur->endElement() == nt->chord()) {
                                    slur->setConnectedElement(mu::engraving::Slur::ConnectedElement::GLISSANDO);
                                    s->setGlissandoShift(false);
                                    break;
                                }
                            }
                        }

                        score->addElement(s);
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

    std::map<int, int> counter;
    for (int i = 0; i < 19; ++i) {
        if (articulations[i] != 0xFFFF) {
            Measure* measure = toMeasure(score->measure(articulations[i] - 1));
            if (i < 4) {
                Segment* segment = measure->getSegment(SegmentType::BarLine, measure->tick());
                Symbol* sym = new Symbol(measure);
                if (i == 0) {
                    sym->setSym(SymId::coda);
                } else if (i == 1) {
                    sym->setSym(SymId::codaSquare);
                } else if (i == 2) {
                    sym->setSym(SymId::segno);
                } else {
                    sym->setSym(SymId::segnoSerpent2);
                }

                sym->setParent(measure);
                sym->setTrack(0);
                segment->add(sym);
                auto iter = counter.find(articulations[i]);
                if (iter == counter.end()) {
                    counter[articulations[i]] = 1;
                } else {
                    counter[articulations[i]] += 1;
                }
            } else {
                Segment* s = measure->getSegment(SegmentType::KeySig, measure->tick());
                StaffText* st = new StaffText(s);
                static constexpr char text[][22] = {
                    "fine", "Da Capo", "D.C. al Coda", "D.C. al Doppia Coda",
                    "D.C. al Fine", "Da Segno", "D.S. al Coda", "D.S. al Doppia Coda",
                    "D.S. al Fine", "Da Doppio Segno", "D.D.S. al Coda", "D.D.S. al Doppia Coda",
                    "D.D.S. al Fine", "Da Coda", "Da Doppia Coda"
                };
                st->setPlainText(String::fromAscii(text[i - 4]));
                st->setParent(s);
                st->setTrack(0);
                auto iter = counter.find(articulations[i]);
                if (iter == counter.end()) {
                    counter[articulations[i]] = 1;
                } else {
                    counter[articulations[i]] += 1;
                }
                score->addElement(st);
            }
        }
    }

    m_continiousElementsBuilder->addElementsToScore();
    if (engravingConfiguration()->experimentalGuitarBendImport()) {
        m_guitarBendImporter->applyBendsToChords();
    }

    return true;
}

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

GuitarPro::ReadNoteResult GuitarPro5::readNoteEffects(Note* note)
{
    ReadNoteResult result;
    uint8_t modMask1 = readUInt8();
    uint8_t modMask2 = readUInt8();
    std::vector<PitchValue> bendData;
    Note* bendParent = nullptr;

    if (modMask1 & EFFECT_BEND) {
        bendData = readBendDataFromFile();
        bendParent = note;
    }
    if (modMask1 & EFFECT_HAMMER) {
        result.slur = true;
    }
    if (modMask1 & EFFECT_LET_RING) {
        result.letRing = true;
    }

    if (modMask1 & EFFECT_GRACE) {
        int fret = readUInt8();                // grace fret
        /*int dynamic =*/ readUInt8();                // grace dynamic
        int transition = readUInt8();                // grace transition
        int duration = readUInt8();                // grace duration
        int gflags = readUInt8();

        int grace_len = Constants::DIVISION / 8;
        if (duration == 2) {
            grace_len = Constants::DIVISION / 6;       //24th
        } else if (duration == 3) {
            grace_len = Constants::DIVISION / 4;       //16th
        }
        NoteType note_type = NoteType::ACCIACCATURA;

        if (gflags & NOTE_APPOGIATURA) {   //on beat
            note_type = NoteType::APPOGGIATURA;
        }

        int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(note->string(), fret, nullptr);

        auto gnote = score->setGraceNote(note->chord(), grace_pitch, note_type, grace_len);

        // gp5 not supports more than one grace note,
        // so it's always should be shown as eight note
        gnote->chord()->setDurationType(Fraction { 1, 8 });

        gnote->setString(note->string());
        auto sd = note->part()->instrument()->stringData();
        gnote->setFret(grace_pitch - sd->stringList().at(sd->stringList().size() - note->string() - 1).pitch);
        if (transition == 0) {
            // no transition
        } else if (transition == 1) {
        } else if (transition == 3) {
            Slur* slur1 = Factory::createSlur(score->dummy());
            slur1->setAnchor(Spanner::Anchor::CHORD);
            slur1->setStartElement(gnote->chord());
            slur1->setEndElement(note->chord());
            slur1->setParent(0);
            slur1->setTrack(note->staffIdx());
            slur1->setTrack2(note->staffIdx());
            slur1->setTick(gnote->chord()->tick());
            slur1->setTick2(note->chord()->tick());
            score->addElement(slur1);
            //TODO: Add a 'slide' guitar effect when implemented
            //note->setSlideNote(gn);
        }
    }
    if (modMask2 & EFFECT_STACCATO) {
        Chord* chord = note->chord();
        Articulation* a = Factory::createArticulation(chord);
        a->setSymId(SymId::articStaccatoAbove);
        bool add = true;
        for (auto a1 : chord->articulations()) {
            if (a1->symId() == SymId::articStaccatoAbove) {
                add = false;
                break;
            }
        }
        if (add) {
            chord->add(a);
        }
    }
    if (modMask2 & EFFECT_PALM_MUTE) {
        result.palmMute = true;
    }

    if (modMask2 & EFFECT_TREMOLO) {      // tremolo picking length
        int tremoloDivision = readUInt8();
        if (tremoloDivision >= 1 && tremoloDivision <= 3) {
            TremoloType type = tremoloType(tremoloDivision);
            DO_ASSERT(!isTremoloTwoChord(type));
            Chord* chord = note->chord();
            TremoloSingleChord* t = Factory::createTremoloSingleChord(chord);
            t->setTremoloType(type);
            chord->add(t);
            m_tremolosInChords[chord] = type;
        } else {
            LOGD("Unknown tremolo value");
        }
    }
    if (modMask2 & EFFECT_SLIDE) {
        int slideKind = readUInt8();
        ChordLine* sld = nullptr;
        ChordLineType slideType = ChordLineType::NOTYPE;

        if (slideKind & SLIDE_OUT_DOWN) {
            slideKind &= ~SLIDE_OUT_DOWN;
            sld = Factory::createChordLine(score->dummy()->chord());
            slideType = ChordLineType::FALL;
        }
        // slide out upwards (doit)
        if (slideKind & SLIDE_OUT_UP) {
            slideKind &= ~SLIDE_OUT_UP;
            sld = Factory::createChordLine(score->dummy()->chord());
            slideType = ChordLineType::DOIT;
        }
        // slide in from below (plop)
        if (slideKind & SLIDE_IN_BELOW) {
            slideKind &= ~SLIDE_IN_BELOW;
            sld = Factory::createChordLine(score->dummy()->chord());
            slideType = ChordLineType::PLOP;
        }
        // slide in from above (scoop)
        if (slideKind & SLIDE_IN_ABOVE) {
            slideKind &= ~SLIDE_IN_ABOVE;
            sld = Factory::createChordLine(score->dummy()->chord());
            slideType = ChordLineType::SCOOP;
        }

        if (sld) {
            sld->setChordLineType(slideType);
            sld->setStraight(true);
            note->chord()->add(sld);
            sld->setNote(note);
        }

        if (slideKind & LEGATO_SLIDE) {
            slideKind &= ~LEGATO_SLIDE;
            slideList.push_back(nullptr);
            createSlur(true, note->staffIdx(), note->chord());
        }

        if (slideKind & SHIFT_SLIDE) {
            slideKind &= ~SHIFT_SLIDE;
            slideList.push_back(note);
        }
    }

    if (modMask2 & EFFECT_ARTIFICIAL_HARMONIC) {
        int type = readChar();
        if (type == HARMONIC_MARK_NATURAL) {
            int fret = note->fret();

            note->setHarmonic(true);
            float harmonicFret = naturalHarmonicFromFret(fret);
            int harmonicOvertone = utils::harmonicOvertone(note, harmonicFret, type);
            note->setDisplayFret(Note::DisplayFretOption::NaturalHarmonic);
            note->setHarmonicFret(harmonicFret);
            auto staff = note->staff();
            int pitch = staff->part()->instrument()->stringData()->getPitch(note->string(), harmonicOvertone, staff);

            note->setPitch(std::clamp(pitch, 0, 127));
            note->setTpcFromPitch(Prefer::SHARPS);
        } else if (type >= HARMONIC_MARK_ARTIFICIAL && type <= HARMONIC_MARK_SEMI) {
            int fret = (type == HARMONIC_MARK_TAP ? (readChar() - note->fret()) : note->fret());
            float midi = note->pitch() + fret;
            int currentOctave = floor(midi / 12);
            auto harmNote = (type == HARMONIC_MARK_ARTIFICIAL ? readChar() : 0);
            if (type == HARMONIC_MARK_ARTIFICIAL) {
                readChar();
            }

            auto octave = currentOctave + (type == HARMONIC_MARK_ARTIFICIAL ? readChar() : 0);

            Note* harmonicNote = Factory::createNote(note->chord());

            harmonicNote->setHarmonic(true);
            harmonicNote->setPlay(true);
            note->setPlay(false);
            /// @note option to show or not additional harmonic fret in "<>" to be implemented
            ///harmonicNote->setDisplayFret(Note::DisplayFretOption::ArtificialHarmonic);
            ///note->setDisplayFret(Note::DisplayFretOption::Hide);
            harmonicNote->setDisplayFret(Note::DisplayFretOption::Hide);
            m_harmonicNotes[note] = harmonicNote;

            Staff* staff = note->staff();
            float harmonicFret = 0;
            switch (static_cast<int>(((octave * 12) + harmNote) - midi)) {
            case 4:
                harmonicFret = 12;
                break;
            case 11:
                harmonicFret = 7;
                break;
            case 16:
                harmonicFret = 5;
                break;
            case 20:
                harmonicFret = 4;
                break;
            case 23:
                harmonicFret = 3.2f;
                break;
            case 25:
                harmonicFret = 2.7f;
                break;
            case 28:
                harmonicFret = 2.4f;
                break;
            default:
                harmonicFret = 12;
                break;
            }

            int overtoneFret = utils::harmonicOvertone(note, harmonicFret, type);
            harmonicNote->setString(note->string());
            harmonicNote->setFret(note->fret());
            harmonicNote->setHarmonicFret(harmonicFret + fret);

            int pitch = staff->part()->instrument()->stringData()->getPitch(note->string(), overtoneFret + note->part()->capoFret(), staff);

            harmonicNote->setPitch(std::clamp(pitch, 0, 127));
            harmonicNote->setTpcFromPitch(Prefer::SHARPS);
            note->chord()->add(harmonicNote);

            switch (type) {
            case HARMONIC_MARK_ARTIFICIAL:
                result.harmonicArtificial = true;
                break;
            case HARMONIC_MARK_PINCH:
                result.harmonicPinch = true;
                break;
            case HARMONIC_MARK_TAP:
                result.harmonicTap = true;
                break;
            case HARMONIC_MARK_SEMI:
                result.harmonicSemi = true;
                break;
            }

            if (!bendData.empty()) {
                bendParent = harmonicNote;
            }
        }
    }

    if (bendParent) {
        createBend(bendParent, bendData);
    }

    if (modMask2 & EFFECT_VIBRATO) {
        result.vibrato = true;
    }

    if (modMask2 & EFFECT_TRILL) {
        readUInt8();          // trill fret

        int period = readUInt8();          // trill length
        result.trill = true;

        switch (period) {
        case 1:                     // 16
            break;
        case 2:                     // 32
            break;
        case 3:                     // 64
            break;
        default:
            LOGD("unknown trill period %d", period);
            break;
        }
    }
    return result;
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

GuitarPro::ReadNoteResult GuitarPro5::readNote(int string, Note* note)
{
    uint8_t noteBits = readUInt8();
    //
    // noteBits:
    //    7 - Right hand or left hand fingering;
    //    6 - Accentuated note
    //    5 - Note type (rest, empty note, normal note);
    //    4 - note dynamic;
    //    3 - Presence of effects linked to the note;
    //    2 - Ghost note;
    //    1 - Dotted note;  ?
    //    0 - Time-independent duration

    if (noteBits & NOTE_GHOST) {
        note->setGhost(true);
    }

    bool tieNote = false;

    if (noteBits & NOTE_DEAD) {
        uint8_t noteType = readUInt8();
        if (noteType == 1) {
        }                         //standard note
        else if (noteType == 2) {
            tieNote = true;
        } else if (noteType == 3) {                   // dead notes
            note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
            note->setDeadNote(true);
        } else {
            LOGD("unknown note type: %d", noteType);
        }
    }

    int& previousDynamic = previousDynamicByTrack[note->track()];

    if (noteBits & NOTE_DYNAMIC) {            // velocity
        int d = readChar();
        if (previousDynamic != d) {
            previousDynamic = d;
            addDynamic(note, d);
        }
    } else if (previousDynamic != DEFAULT_DYNAMIC) {
        previousDynamic = DEFAULT_DYNAMIC;
        addDynamic(note, previousDynamic);
    }

    int fretNumber = 0;
    if (noteBits & NOTE_FRET) {
        fretNumber = readChar();
    }

    if (noteBits & NOTE_FINGERING) {
        int leftFinger = readUInt8();
        int rightFinger = readUInt8();
        Fingering* fi = Factory::createFingering(note);
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

    if (noteBits & 0x1) {     // Time independent duration
        skip(8);
    }

    // check if a note is supposed to be accented, and give it the marcato type
    if (noteBits & NOTE_MARCATO) {
        Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
        art->setSymId(SymId::articMarcatoAbove);
        if (!note->score()->toggleArticulation(note, art)) {
            delete art;
        }
    }
    // check if a note is supposed to be accented, and give it the sforzato type
    else if (noteBits & NOTE_SFORZATO) {
        Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
        art->setSymId(SymId::articAccentAbove);
        note->add(art);
        if (!note->score()->toggleArticulation(note, art)) {
            delete art;
        }
    }

    readUInt8();   //skip

    Staff* staff = note->staff();
    if (fretNumber == 255 || fretNumber < 0) {
        fretNumber = 0;
        note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        note->setDeadNote(true);
    }
    int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber + note->part()->capoFret(), nullptr);
    note->setFret(fretNumber);
    note->setString(string);
    note->setPitch(pitch);

    // This function uses string and fret number, so it should be set before this
    ReadNoteResult result;
    if (noteBits & NOTE_SLUR) {
        result = readNoteEffects(note);
    }

    if (tieNote) {
        auto staffIdx = note->staffIdx();
        if (dead_end[{ static_cast<int>(staffIdx), string }]) {
            note->setFret(INVALID_FRET_INDEX);
            result.slur = false;
            return result;
        }
        if (slurs[staffIdx]) {
            score->removeSpanner(slurs[staffIdx]);
            delete slurs[staffIdx];
            slurs[staffIdx] = 0;
        }
        bool found = false;
        Chord* chord     = note->chord();
        Segment* segment = chord->segment()->prev1(SegmentType::ChordRest);
        track_idx_t track = note->track();
        std::vector<ChordRest*> chords;
        Note* true_note = nullptr;
        while (segment) {
            EngravingItem* e = segment->element(track);
            if (e && e->isChord()) {
                Chord* chord2 = toChord(e);
                for (Note* note2 : chord2->notes()) {
                    if (note2->string() == string && chords.empty()) {
                        Tie* tie = Factory::createTie(note2);

                        //  fixing gp5 bug with not storying let ring for tied notes
                        if (m_letRingForChords.find(chord2) != m_letRingForChords.end()) {
                            result.letRing = true;
                            muse::remove(m_letRingForChords, chord2);
                        }

                        tie->setEndNote(note);
                        note2->add(tie);
                        if (m_harmonicNotes.find(note) != m_harmonicNotes.end() && m_harmonicNotes.find(note2) != m_harmonicNotes.end()) {
                            Note* startHarmonicNote = m_harmonicNotes.at(note2);
                            Note* endHarmonicNote = m_harmonicNotes.at(note);

                            Tie* tieHarmonic = Factory::createTie(startHarmonicNote);
                            tieHarmonic->setEndNote(endHarmonicNote);
                            startHarmonicNote->add(tieHarmonic);

                            muse::remove(m_harmonicNotes, startHarmonicNote);
                            muse::remove(m_harmonicNotes, endHarmonicNote);
                        }

                        note->setFret(note2->fret());
                        note->setPitch(note2->pitch());
                        true_note = note2;
                        if (m_tremolosInChords.find(chord2) != m_tremolosInChords.end()) {
                            TremoloType type = m_tremolosInChords.at(chord2);
                            DO_ASSERT(!isTremoloTwoChord(type));
                            TremoloSingleChord* t = Factory::createTremoloSingleChord(score->dummy()->chord());
                            t->setTremoloType(type);
                            chord->add(t);
                            muse::remove(m_tremolosInChords, chord2);
                            m_tremolosInChords[chord] = type;
                        }

                        found = true;
                        break;
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

        if (true_note && !chords.empty()) {
            Note* end_note = note;
            for (unsigned int i = 0; i < chords.size(); ++i) {
                Chord* chord1 = nullptr;
                ChordRest* cr = chords.at(i);
                if (cr->isChord()) {
                    chord1 = toChord(cr);
                } else {
                    Rest* rest = toRest(cr);
                    Fraction dur = rest->ticks();
                    TDuration dut = rest->durationType();
                    Segment* seg = rest->segment();
                    seg->remove(rest);
                    Tuplet* tuplet = rest->tuplet();
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
                note2->setTpcFromPitch(Prefer::SHARPS);
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
            note->setFret(INVALID_FRET_INDEX);
            dead_end[{ static_cast<int>(staffIdx), string }] = true;
            LOGD("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            result.slur = false;
            return result;
        }
    }
    dead_end[{ static_cast<int>(note->staffIdx()), string }] = false;
    return result;
}

float GuitarPro5::naturalHarmonicFromFret(int fret)
{
    switch (fret) {
    case 2:
        return 2.4f;
    case 3:
        return 3.2f;
    case 4:
    case 5:
    case 7:
    case 9:
    case 12:
    case 16:
    case 17:
    case 19:
    case 24:
        return fret;
    case 8:
        return 8.2f;
    case 10:
        return 9.6f;
    case 14:
        return 14.7f;
    case 15:
        return 14.7f;
    case 21:
        return 21.7f;
    case 22:
        return 21.7f;
    default:
        return 12.0f;
    }
}
} // namespace mu::iex::guitarpro
