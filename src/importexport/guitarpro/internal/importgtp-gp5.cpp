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

#include "importgtp.h"

#include "libmscore/factory.h"
#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/bend.h"
#include "libmscore/box.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/chordline.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/excerpt.h"
#include "libmscore/fingering.h"
#include "libmscore/glissando.h"
#include "libmscore/harmony.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurebase.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/part.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftype.h"
#include "libmscore/stringdata.h"
#include "types/symid.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/tie.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tremolobar.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"

#include "log.h"

using namespace mu::io;
using namespace mu::engraving;

namespace mu::engraving {
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
    QString copyright = readDelphiString();
    if (!copyright.isEmpty()) {
        score->setMetaTag(u"copyright", copyright);
    }

    transcriber  = readDelphiString();
    instructions = readDelphiString();
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

    uchar fxBits1 = readUChar();
    uchar fxBits2 = readUChar();
    if (fxBits1 & BEAT_FADE) {
        effects = 4;      // fade in
    }
    if (fxBits1 & BEAT_EFFECT) {
        int k = readUChar();
        effects = k + effects * 100;    // &effects;
    }
    if (fxBits1 & BEAT_VIBRATO_TREMOLO) {
        effects = 7 + effects * 100;
    }
    if (fxBits2 & BEAT_TREMOLO) {
        readTremoloBar(track, segment);           // readBend();
    }
    if (fxBits2 & 0x01) {   // Rasgueado effect
        StaffText* st = new StaffText(score->dummy()->segment());
        st->setXmlText(u"rasg.");
        st->setParent(segment);
        st->setTrack(track);
        score->addElement(st);
    }
    if (fxBits1 & BEAT_ARPEGGIO) {
        int strokeup = readUChar();                // up stroke length
        int strokedown = readUChar();                // down stroke length

        Arpeggio* a = Factory::createArpeggio(score->dummy()->chord());
        // representation is different in guitar pro 5 - the up/down order below is correct
        if (strokeup > 0) {
            a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
        } else if (strokedown > 0) {
            a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
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
    uchar beatBits = readUChar();
    bool dotted    = beatBits & BEAT_DOTTED;

    slide = -1;
    int track = staffIdx * VOICES + voice;
    if (slides.contains(track)) {
        slide = slides.take(track);
    }

    int pause = -1;
    if (beatBits & BEAT_PAUSE) {
        pause = readUChar();
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
        QString name;
        {
            readUChar();
            char c[21];
            f->read((uint8_t*)(c), 21);
            c[20] = 0;
            name = c;
        }
        skip(4);
        // no header to be read in the GP5 format - default to true.
        readChord(segment, staffIdx * VOICES, static_cast<int>(numStrings), name, true);
        skip(32);
    }
    Lyrics* lyrics = 0;
    QString free_text;
    if (beatBits & BEAT_LYRICS) {
        //free_text = readDelphiString();
        QString qs = readDelphiString();
        std::string txt = qs.toUtf8().constData();
        txt.erase(std::remove_if(txt.begin(), txt.end(), [](char c) { return c == '_'; }), txt.end());
//		auto pos = txt.find('-');
        auto buffer = txt;
        txt.resize(0);
        const char* c = buffer.c_str();
        while (*c) {
            if (*c == ' ') {
                while (*c == ' ') {
                    ++c;
                }
                if (*c == '-') {
                    txt += '-';
                    ++c;
                    while (*c == ' ') {
                        ++c;
                    }
                } else if (*c) {
                    txt += '-';
                }
            } else {
                txt += *(c++);
            }
        }
        if (gpLyrics.lyrics.size() == 0 || (gpLyrics.lyrics.size() == 1 && gpLyrics.lyrics[0].isEmpty())) {
//			gpLyrics.lyrics.resize(0);
            gpLyrics.lyrics.clear();
            gpLyrics.fromBeat = _beat_counter;
            gpLyrics.lyricTrack = track;
        }
        while (txt.size() && txt[txt.size() - 1] == '-') {
            txt.resize(txt.size() - 1);
        }
//		  gpLyrics.lyrics.append(txt);
        gpLyrics.lyrics.append(QString::fromUtf8(txt.data(), int(txt.size())));
        gpLyrics.segments.push_back(segment);
    }
    int beatEffects = 0;
    if (beatBits & BEAT_EFFECTS) {
        beatEffects = readBeatEffects(track, segment);
    }
    last_segment = segment;
    if (beatBits & BEAT_MIX_CHANGE) {
        readMixChange(measure);
    }

    int strings = readUChar();     // used strings mask

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
            Tuplet* tuplet = tuplets[staffIdx * 2 + voice];
            if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                tuplet = Factory::createTuplet(measure);
                tuplet->setTick(tick);
                // int track = staffIdx * 2 + voice;
                tuplets[staffIdx * 2 + voice] = tuplet;
                tuplet->setTrack(cr->track());
                setTuplet(tuplet, tuple);
                tuplet->setParent(measure);
            }
            tuplet->setTrack(cr->track());
            tuplet->setBaseLen(l);
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
        bool hasSlur = false;
        Note* _note{ nullptr };
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

                hasSlur = (readNote(6 - i, note) || hasSlur);
                if (slideList.size() && slideList.back() == nullptr) {
                    slideList.back() = note;
                    hasSlur = true;
                }
                if (note->fret() == -20) {
                    delnote.push_back(note);
                } else {
                    note->setTpcFromPitch();
                }
            }
        }
        if (delnote.size()) {
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
        if (free_text.length() && _note) {
            addTextToNote(free_text, _note);
        }
    }
    int rr = readChar();
    if (cr && cr->isChord()) {
        Chord* chord = toChord(cr);
        do {
            applyBeatEffects(chord, beatEffects % 100);
        } while (beatEffects /= 100);
        if (rr == ARPEGGIO_DOWN) {
            chord->setStemDirection(DirectionV::DOWN);
        } else if (rr == ARPEGGIO_UP) {
            chord->setStemDirection(DirectionV::UP);
        }
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
    //LOGD("read reverb: %d", reverb);
    if (phase >= 0) {
        readChar();
    }
    if (tremolo >= 0) {
        readChar();
    }
    if (temp >= 0) {
        if (last_segment) {
            score->setTempo(last_segment->tick(), double(temp) / 60.0);
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
    for (int i = 0; i < staves; ++i) {
        int tuning[GP_MAX_STRING_NUMBER];
        Staff* staff = score->staff(i);
        Part* part = staff->part();

        uchar c = readUChar();       // simulations bitmask
        if (c & 0x2) {               // 12 stringed guitar
        }
        if (c & 0x4) {               // banjo track
        }
        if (i == 0 || version == 500) {
            skip(1);
        }
        QString name = readPascalString(40);

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
        StringData stringData(frets, strings, &tuning2[0]);
        createTuningString(strings, &tuning2[0]);
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
            // instr->setUseDrumset(DrumsetKind::GUITAR_PRO);
            instr->setDrumset(gpDrumset);
            staff->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
        } else {
            clefId = defaultClef(patch);
        }
        Measure* measure = score->firstMeasure();
        Segment* segment = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
        Clef* clef = Factory::createClef(segment);
        clef->setClefType(clefId);
        clef->setTrack(i * VOICES);
        segment->add(clef);

        if (capo > 0) {
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            StaffText* st = new StaffText(s);
            st->setPlainText(QString("Capo. fret ") + QString::number(capo));
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

        //LOGD("default2: %d", channelDefaults[i].reverb);
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
    for (int bar = 0; bar < measures; ++bar, measure = measure->nextMeasure()) {
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
        for (int track = 0; track < staves * 2; ++track) {
            tuplets[track] = 0;
        }

        for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            _beat_counter = 0;
            readMeasure(measure, staffIdx, &tuplets[0], mixChange);
            if (!(((bar == (measures - 1)) && (staffIdx == (staves - 1))))) {
                /*int a = */
                readChar();
                // LOGD("    ======skip %02x", a);
            }
        }
        if (bar == 1 && !mixChange) {
            setTempo(tempo, score->firstMeasure());
        }
    }

    if (!gpLyrics.segments.empty()) {
        auto size = std::min(int(gpLyrics.segments.size()), int(gpLyrics.lyrics.size()));
        for (int i = 0; i < size; ++i) {
            std::string str = gpLyrics.lyrics[i].toUtf8().constData();
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
                        lyr->setPlainText(QString::fromUtf8(text.data(), int(text.size())));
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
    } else {
        int counter = 0;
//            int index = 0;
//TODO-ws ???		gpLyrics.lyricTrack -= 1;
        auto mes = score->firstMeasure();
        auto beg = mes->first();

        do {
            if (beg->isChordRestType() && beg->cr(gpLyrics.lyricTrack)) {
                ChordRest* cr = beg->cr(gpLyrics.lyricTrack);
                ++counter;
                if (!cr->isChord()) {
                    continue;
                }
                bool is_tied = false;
                Chord* chord = toChord(cr);
                for (auto n : chord->notes()) {
                    if (n->tiedNotes().size() > 1 && n->tiedNotes()[0] != n) {
                        is_tied = true;
                        break;
                    }
                }
                if (is_tied) {
                    continue;
                }
            }
        } while ((beg = beg->next()) || ((mes = toMeasure(mes->next())) && (beg = mes->first())));
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro5::read(IODevice* io)
{
    f = io;

    readInfo();
    readLyrics();
    readPageSetup();

    previousDynamic = -1;
    previousTempo = -1;
    //previousDynamic = new int [staves * VOICES];
    // initialise the dynamics to 0
    //for (int i = 0; i < staves * VOICES; i++)
    //      previousDynamic[i] = 0;

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
            x = readUChar();
            r = x;
            x = readUChar();
            r += x << 8;

            articulations[i] = r;
        }
    }

    //skip(42);
    skip(4);

    measures = readInt();
    staves  = readInt();

    for (int str = 0; str < 7; ++str) {
        for (int staff = 0; staff < staves; ++staff) {
            dead_end[{ staff, str }] = true;
        }
    }

    slurs = new Slur*[staves];
    for (int i = 0; i < staves; ++i) {
        slurs[i] = 0;
    }

    int tnumerator   = 4;
    int tdenominator = 4;
    for (int i = 0; i < measures; ++i) {
        if (i > 0) {
            skip(1);
        }
        GpBar bar;
        uchar barBits = readUChar();
        if (barBits & SCORE_TIMESIG_NUMERATOR) {
            tnumerator = readUChar();
        }
        if (barBits & SCORE_TIMESIG_DENOMINATOR) {
            tdenominator = readUChar();
        }
        if (barBits & SCORE_REPEAT_START) {
            bar.repeatFlags = bar.repeatFlags | Repeat::START;
        }
        if (barBits & SCORE_REPEAT_END) {                    // number of repeats
            bar.repeatFlags = bar.repeatFlags | Repeat::END;
            bar.repeats = readUChar();
        }
        if (barBits & SCORE_MARKER) {
            bar.marker = readDelphiString();           // new section?
            /*int color =*/ readInt();          // color?
        }
        if (barBits & SCORE_VOLTA) {                          // a volta
            uchar voltaNumber = readUChar();
            while (voltaNumber > 0) {
                // voltas are represented as flags
                bar.volta.voltaType = GP_VOLTA_FLAGS;
                bar.volta.voltaInfo.append(voltaNumber & 1);
                voltaNumber >>= 1;
            }
        }
        if (barBits & SCORE_KEYSIG) {
            int currentKey = readUChar();
            /* key signatures are specified as
             * 1# = 1, 2# = 2, ..., 7# = 7
             * 1b = 255, 2b = 254, ... 7b = 249 */
            bar.keysig = currentKey <= 7 ? currentKey : -256 + currentKey;
            readUChar();              // specified major/minor mode
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
        bars.append(bar);
    }

    //
    // create a part for every staff
    //
    for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
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
//		Note* next = nullptr;
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
                    "fine", "Da Capo", "D.C. al Coda", "D.C. al Double Coda",
                    "D.C. al Fine", "Da Segno", "D.S. al Coda", "D.S. al Double Coda",
                    "D.S. al Fine", "Da Segno Segno", "D.S.S. al Coda", "D.S.S. al Double Coda",
                    "D.S.S. al Fine", "Da Coda", "Da Double Coda"
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

    return true;
}

//---------------------------------------------------------
//   readNoteEffects
//---------------------------------------------------------

bool GuitarPro5::readNoteEffects(Note* note)
{
    uchar modMask1 = readUChar();
    uchar modMask2 = readUChar();
    bool slur = false;

    if (modMask1 & EFFECT_BEND) {
        readBend(note);
    }
    if (modMask1 & EFFECT_HAMMER) {
        slur = true;
    }
    if (modMask1 & EFFECT_LET_RING) {
        addLetRing(note);
    }

    if (modMask1 & EFFECT_GRACE) {
        int fret = readUChar();                // grace fret
        /*int dynamic =*/ readUChar();                // grace dynamic
        int transition = readUChar();                // grace transition
        /*int duration =*/ readUChar();                // grace duration
        int gflags = readUChar();

        NoteType note_type = NoteType::ACCIACCATURA;

        if (gflags & NOTE_APPOGIATURA) {   //on beat
            note_type = NoteType::APPOGGIATURA;
        }

        int grace_pitch = note->staff()->part()->instrument()->stringData()->getPitch(note->string(), fret, nullptr);
        auto gnote = score->setGraceNote(note->chord(), grace_pitch, note_type, Constants::division / 2);
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
        addPalmMute(note);
    }
    //note->setPalmMute(true);

    if (modMask2 & EFFECT_TREMOLO) {      // tremolo picking length
        int tremoloDivision = readUChar();
        Chord* chord = note->chord();
        Tremolo* t = Factory::createTremolo(chord);
        if (tremoloDivision == 1) {
            t->setTremoloType(TremoloType::R8);
            chord->add(t);
        } else if (tremoloDivision == 2) {
            t->setTremoloType(TremoloType::R16);
            chord->add(t);
        } else if (tremoloDivision == 3) {
            t->setTremoloType(TremoloType::R32);
            chord->add(t);
        } else {
            LOGD("Unknown tremolo value");
        }
    }
    if (modMask2 & EFFECT_SLIDE) {
        int slideKind = readUChar();
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
            auto convertSlideType = [](ChordLineType slideType) -> Note::SlideType {
                if (slideType == ChordLineType::FALL) {
                    return Note::SlideType::Fall;
                } else if (slideType == ChordLineType::DOIT) {
                    return Note::SlideType::Doit;
                } else if (slideType == ChordLineType::SCOOP) {
                    return Note::SlideType::Lift;
                } else if (slideType == ChordLineType::PLOP) {
                    return Note::SlideType::Plop;
                } else {
                    LOGE() << "wrong slide type";
                    return Note::SlideType::Undefined;
                }
            };

            sld->setChordLineType(slideType);
            sld->setStraight(true);
            note->chord()->add(sld);
            sld->setNote(note);
            Note::Slide sl{ convertSlideType(slideType), nullptr };
            note->attachSlide(sl);
        }

        if (false && !slideList.empty() && slideList.back()->chord()->segment() != note->chord()->segment()) {
            Note* start = slideList.front();
            slideList.pop_front();
            bool skip = false;
            for (auto e : start->el()) {
                if (e->isChordLine()) {
                    skip = true;
                }
            }
            if (!skip) {
                Glissando* s = new Glissando(start);
                s->setAnchor(Spanner::Anchor::NOTE);
                s->setStartElement(start);
                s->setTick(start->chord()->segment()->tick());
                s->setTrack(start->staffIdx());
                s->setParent(start);
                s->setGlissandoType(GlissandoType::STRAIGHT);
                s->setEndElement(note);
                s->setTick2(note->chord()->segment()->tick());
                s->setTrack2(note->staffIdx());
                score->addElement(s);
            }
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
        if (type == 1 || type == 3) {
            int fret = type == 3 ? (readChar() - note->fret()) : note->fret();

            note->setHarmonic(true);
            float harmonicFret = naturalHarmonicFromFret(fret);
            int harmonicOvertone = GuitarPro::harmonicOvertone(note, harmonicFret, type);
            note->setDisplayFret(Note::DisplayFretOption::NaturalHarmonic);
            note->setHarmonicFret(harmonicFret);
            auto staff = note->staff();
            int pitch = staff->part()->instrument()->stringData()->getPitch(note->string(), harmonicOvertone, staff);

            note->setPitch(std::clamp(pitch, 0, 127));
            note->setTpcFromPitch();
        } else if (type == 2) {
            int fret = note->fret();
            float midi = note->pitch() + fret;
            int currentOctave = floor(midi / 12);
            auto harmNote = readChar();
            readChar();
            auto octave = currentOctave + readChar();

            Note* harmonicNote = Factory::createNote(note->chord());

            harmonicNote->setHarmonic(true);
            harmonicNote->setDisplayFret(Note::DisplayFretOption::ArtificialHarmonic);
            note->setDisplayFret(Note::DisplayFretOption::Hide);

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

            int overtoneFret = GuitarPro::harmonicOvertone(note, harmonicFret, type);
            harmonicNote->setString(note->string());
            harmonicNote->setFret(note->fret());
            harmonicNote->setHarmonicFret(harmonicFret + fret);

            int pitch = staff->part()->instrument()->stringData()->getPitch(note->string(), overtoneFret + note->part()->capoFret(), staff);

            harmonicNote->setPitch(std::clamp(pitch, 0, 127));
            harmonicNote->setTpcFromPitch();
            note->chord()->add(harmonicNote);
            addTextToNote("A.H.", harmonicNote);
        }
    }

    if (modMask2 & 0x40) {
        addVibrato(note);
    }

    if (modMask2 & EFFECT_TRILL) {
        readUChar();          // trill fret

        int period = readUChar();          // trill length

        // add the trill articulation to the note
        Articulation* art = Factory::createArticulation(note->score()->dummy()->chord());
        art->setSymId(SymId::ornamentTrill);
        if (!note->score()->toggleArticulation(note, art)) {
            delete art;
        }

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
    return slur;
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

bool GuitarPro5::readNote(int string, Note* note)
{
    uchar noteBits = readUChar();
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
        uchar noteType = readUChar();
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

    if (noteBits & NOTE_DYNAMIC) {            // velocity
        int d = readChar();
        if (previousDynamic != d) {
            previousDynamic = d;
            addDynamic(note, d);
        }
    }

    int fretNumber = 0;
    if (noteBits & NOTE_FRET) {
        fretNumber = readChar();
    }

    if (noteBits & NOTE_FINGERING) {
        int leftFinger = readUChar();
        int rightFinger = readUChar();
        Fingering* fi = Factory::createFingering(note);
        QString finger;
        // if there is a valid left hand fingering
        if (leftFinger < 5) {
            if (leftFinger == 0) {
                finger = "T";
            } else if (leftFinger == 1) {
                finger = "1";
            } else if (leftFinger == 2) {
                finger = "2";
            } else if (leftFinger == 3) {
                finger = "3";
            } else if (leftFinger == 4) {
                finger = "4";
            }
        } else {
            if (rightFinger == 0) {
                finger = "T";
            } else if (rightFinger == 1) {
                finger = "I";
            } else if (rightFinger == 2) {
                finger = "M";
            } else if (rightFinger == 3) {
                finger = "A";
            } else if (rightFinger == 4) {
                finger = "O";
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

    readUChar();   //skip

    Staff* staff = note->staff();
    if (fretNumber == 255 || fretNumber < 0) {
        fretNumber = 0;
        note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
        note->setGhost(true);
    }
    int pitch = staff->part()->instrument()->stringData()->getPitch(string, fretNumber + note->part()->capoFret(), nullptr);
    note->setFret(fretNumber);
    note->setString(string);
    note->setPitch(pitch);

    // This function uses string and fret number, so it should be set before this
    bool slur = false;
    if (noteBits & NOTE_SLUR) {
        slur = readNoteEffects(note);
    }

    if (tieNote) {
        auto staffIdx = note->staffIdx();
        if (dead_end[{ static_cast<int>(staffIdx), string }]) {
            note->setFret(-20);
            return false;
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
            if (e) {
                if (e->isChord()) {
                    Chord* chord2 = toChord(e);
                    foreach (Note* note2, chord2->notes()) {
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
                } else {
                    if (e) {
                        chords.push_back(toChordRest(e));
                    }
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
                    auto rest = toRest(cr);
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
            note->setFret(-20);
            dead_end[{ static_cast<int>(staffIdx), string }] = true;
            LOGD("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
            return false;
        }
    }
    dead_end[{ note->staffIdx(), string }] = false;
    return slur;
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

//---------------------------------------------------------
//   readArtificialHarmonic
//---------------------------------------------------------

int GuitarPro5::readArtificialHarmonic()
{
    int type = readChar();
    switch (type) {
    case 1:                   // natural
        break;
    case 2:                   // artificial
        //skip(3);
        break;
    case 3:                   // tapped
        skip(1);
        break;
    case 4:                   // pinch
        break;
    case 5:                   // semi
        break;
    }
    return type;
}
}
