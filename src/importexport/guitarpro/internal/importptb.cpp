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
#include "importptb.h"
#include "assert.h"

#include <cmath>

#include "engraving/dom/articulation.h"
#include "engraving/dom/bend.h"
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/palmmute.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rehearsalmark.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"

#include "types/symid.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::guitarpro {
bool PowerTab::readBoolean()
{
    return readUInt8() != 0;
}

uint8_t PowerTab::readUInt8()
{
    uint8_t byte;
    _file->read((uint8_t*)&byte, 1);
    return byte;
}

unsigned short PowerTab::readShort()
{
    uint16_t val;
    _file->read((uint8_t*)&val, 2);
    return val;
}

char PowerTab::readChar()
{
    char byte;
    _file->read((uint8_t*)&byte, 1);
    return byte;
}

int PowerTab::readInt()
{
    int32_t val;
    _file->read((uint8_t*)&val, 4);
    return val;
}

std::string PowerTab::readString(int length)
{
    if (length == 0) {
        LOGE() << "reading string of length zero";
        return std::string();
    }

    if (length == -1) {
        length = readUInt8();
        if (length == 0xFF) {
            length = readShort();
        }
    }
    char* p = new char[length];
    _file->read((uint8_t*)p, length);

    std::string s(p, length);
    delete[] p;
    return s;
}

bool PowerTab::readVersion()
{
    std::string version = readString(4);
    version += std::string("-") + std::to_string(readShort());
    return version == "ptab-4";
}

void PowerTab::skip(int len /* = 1 */)
{
    for (int i = 0; i < len; ++i) {
        readChar();
    }
}

void PowerTab::readSongInfo(ptSongInfo& info)
{
    auto classification = readChar();
    info.classification = classification;
    if (classification == 0) {
        skip(1);
        info.name = readString();
        info.interpret = readString();

        auto releaseType = readChar();

        switch (releaseType) {
        case 0: {
            /*auto albumType =*/
            readChar();
            info.album = readString();
            info.year = readShort();
            info.liverecording = readChar() != 0;
            break;
        }
        case 1: {
            info.album = readString();
            info.liverecording = readChar() != 0;
            break;
        }
        case 2: {
            info.album  = readString();
            info.day    = readShort();
            info.month  = readShort();
            info.year   = readShort();
            break;
        }
        case 3: {
            break;
        }
        default:
            assert(0);
        }

        if (readChar() == 0) {
            info.author = readString();
            info.lyricist = readString();
        }

        info.arranger           = readString();
        info.guitarTranscriber  = readString();
        info.bassTranscriber    = readString();
        info.copyright          = readString();

        info.lyrics             = readString();

        info.guitarInstructions = readString();
        info.bassInstructions   = readString();
    } else if (classification == 1) {
        info.name         = readString();
        info.album        = readString();
        info.style        = readShort();
        info.level        = readUInt8();
        info.author       = readString();
        info.instructions = readString();
        info.copyright    = readString();
    }
}

int PowerTab::readHeaderItems()
{
    int itemCount = readShort();
    if (itemCount != 0) {
        int header = readShort();
        if (header == 0xFFFF) {
            if (readShort() != 1) {
                return -1;
            }
            auto str = readString(readShort());       //section title
        }
    }
    return itemCount;
}

void PowerTab::readTrackInfo(ptTrack& info)
{
    TrackInfo tr;

    tr.number = readUInt8();
    tr.name = readString();
    tr.instrument = readUInt8();
    tr.volume = readUInt8();
    tr.balance = readUInt8();
    tr.reverb = readUInt8();
    tr.chorus = readUInt8();
    tr.tremolo = readUInt8();
    tr.phaser = readUInt8();
    tr.capo = readUInt8();
    tr.tuningName = readString();
    tr.offset = readUInt8();

    int ln = readUInt8();
    for (int i = 0; i < ln; ++i) {
        tr.strings.push_back(readUInt8());
    }
    info.infos.push_back(tr);
}

void PowerTab::readChord(ptTrack& info)
{
    ptChord ch;
    ch.key = readShort();   //chord key
    ch.formula = readUInt8();
    ch.modification = readShort();   //Chord Modification
    ch.extra = readUInt8();
    ch.top_fret = readUInt8();
    auto stringCount = readUInt8();
    for (int i = 0; i < stringCount; ++i) {
        ch.frets.push_back(readUInt8());     // fret
    }
    if (info.diagramMap.find({ { ch.key, ch.formula, ch.modification } }) == info.diagramMap.end()) {
        info.diagramMap[ { { ch.key, ch.formula, ch.modification } }] = ch;
    } else {
        auto a1 = info.diagramMap[ { { ch.key, ch.formula, ch.modification } }];
        auto a2 = ch;

//??            int k = 1;
    }
}

void PowerTab::readFontSettings()
{
    readString();   //font name
    readInt();   // point size
    readInt();   // weight
    readBoolean();   //italic
    readBoolean();   //underline
    readBoolean();   //strikeout
    readInt();   //color
}

void PowerTab::readFloatingText()
{
    readString();   //text
    //rect :
    readInt();   // left
    readInt();   // top
    readInt();   // right
    readInt();   // bottom

    readUInt8();
    readFontSettings();
}

void PowerTab::readDynamic()
{
    readShort();
    readUInt8();
    readUInt8();
    readShort();
}

void PowerTab::readKeySignature()
{
    readUInt8();
}

void PowerTab::readRehearsalSign(ptSection& sec)
{
    auto c = readChar();
    auto str = readString();
    if (str.size()) {
        sec.partName = str;
        sec.partMarker = c;
    }
}

void PowerTab::readChordText(ptSection& sec)
{
    ptChordText cht;
    cht.position = readUInt8();
    cht.key = readShort();
    cht.formula = readUInt8();
    cht.formula_mod = readShort();
    cht.extra = readUInt8();
    //sec.chordTextMap.push_back(cht);
    sec.chordTextMap.insert({ cht.position, cht });
}

void PowerTab::readRhytmSlash(ptSection& sec)
{
    stRhytmSlash rs;
    rs.position = readUInt8();
    auto beaming = readUInt8();
    auto data = readInt();
    auto duration = (data & 0xE00000) >> 21;
    switch (duration) {
    case 0:
        rs.duration = 1;
        break;
    case 1:
        rs.duration = 2;
        break;
    case 2:
        rs.duration = 4;
        break;
    case 3:
        rs.duration = 8;
        break;
    case 4:
        rs.duration = 16;
    }

    rs.triplet = beaming & (0x20 | 0x40);
    rs.tripletend = beaming & 0x80;

    rs.dotted = data & 0x01;
    rs.doubleDotted = data & 0x02;

    rs.is_rest = data & 0x04;

    sec.rhythm.emplace_back(rs);
}

void PowerTab::readGuitarIn(ptTrack& info)
{
    ptGuitarIn gin;
    gin.section = readShort();
    gin.staff = readUInt8() + staffInc;
    gin.position = readUInt8();
    gin.rhythmSlash = readUInt8();
    gin.trackinfo = readUInt8();
    info.guitar_ins.push_back(gin);
}

void PowerTab::readTempoMarker(ptTrack& info)
{
    auto section = readShort();
    /*auto position =*/ readUInt8();
    auto tempo = readShort();
    /*auto data =*/ readShort();
    readString();   //description

    if (tempo > 0) {
        info.getSection(section).tempo = tempo;
    }
}

void PowerTab::readSectionSymbol(ptTrack& info)
{
    int section = readShort();
    int position = readUInt8();
    int data = readInt();

    int endNumber = data >> 16;
    info.getSection(section).getPosition(position).addComponent(new ptSymbol(endNumber));
}

void PowerTab::readTimeSignature(ptBar* bar)
{
    skip(3);
    int data = readUInt8();
    readUInt8();   // measure pulses

    bar->numerator = ((data - (data % 8)) / 8) + 1;
    bar->denominator = pow(2, data % 8);
}

void PowerTab::readBarLine(ptSection& sec)
{
    auto bar = new ptBar();
    /*int position =*/ readUInt8();
    auto b_type = readUInt8();

    bar->repeatStart = (b_type >> 5) == 3;
    bar->repeatClose = (b_type >> 5) == 4 ? b_type - 128 : 0;

    bar->measureNo = sec.number;

    readKeySignature();
    readTimeSignature(bar);
    readRehearsalSign(sec);
    //sec.getPosition(position).addComponent(bar);
    sec.bars.push_back(std::shared_ptr<ptBar>(bar));
    /*if (bars.empty() || ( bars.back()->measureNo < bar->measureNo ))
          bars.push_back(bar);*/
}

void PowerTab::readStaff(int staff, ptSection& sec)
{
    skip(5);
    for (int voice = 0; voice < 2; ++voice) {
        int itemCount = readHeaderItems();
        for (int i = 0; i < itemCount; ++i) {
            readPosition(staff, voice, sec);
            if (i < itemCount - 1) {
                readShort();
            }
        }
    }
}

void PowerTab::readNote(ptBeat* beat)
{
    ptNote note;
    auto position = readUInt8();
    auto simpleData = readShort();
    auto symbolCount = readUInt8();
    for (int i = 0; i < symbolCount; ++i) {
        skip(2);
        auto data3 = readUInt8();
        auto data4 = readUInt8();
        note.bend = data4 == 101 ? data3 / 16 + 1 : 0;
        note.slide = data4 == 100 ? data3 + 1 : 0;
    }
    note.value = position & 0x1F;
    note.str = ((position & 0xE0) >> 5);
    note.tied = simpleData & 0x01;
    note.dead = simpleData & 0x02;
    note.hammer = simpleData & 0x08;
    beat->notes.emplace_back(note);
}

void PowerTab::readPosition(int staff, int voice, ptSection& sec)
{
    auto position = readUInt8();

    ptBeat beat = ptBeat();
    beat.position = position;
    beat.voice = voice;
    bool add = false;
    if (voice == 0 || static_cast<size_t>(staff) >= sec.beats.size() || sec.beats[staff].empty()) {
        beat = ptBeat(staff, voice);
        beat.position = position;
        add = true;
    }
    auto beaming = readUInt8();
    beaming = (((int)beaming - 128 < 0) ? beaming : beaming - 128);

    readUInt8();

    auto data1 = readUInt8();
    auto data2 = readUInt8();   //32 - palm mute, 4 - accent, 2 - staccato
    auto data3 = readUInt8();
    auto durationValue = readUInt8();

    int multiBarRest = 1;
    auto complexCount = readUInt8();
    for (int i = 0; i < complexCount; ++i) {
        auto count = readShort();
        readUInt8();
        auto type = readUInt8();
        if (type & 0x08) {
            multiBarRest = count;
        }
    }

    auto itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readNote(&beat);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    curTrack->infos[staff].notes_count += itemCount;
    beat.mmrest = (itemCount == 0) ? multiBarRest : 1;
    beat.vibrato = (((data1 & 0x08) != 0) || ((data1 & 0x10) != 0));
    beat.grace = (data3 & 0x01) != 0;
    beat.tuplet = (data3 & 0x20) != 0;

    if (beat.duration != 0) {
        durationValue = std::max((int)durationValue, beat.duration);
    }

    beat.duration = durationValue;
    beat.dotted = (data1 & 0x01);
    beat.doubleDotted = (data1 & 0x02);
    beat.arpeggioUp = (data1 & 0x20);
    beat.arpeggioDown = (data1 & 0x40);
    beat.enters = ((beaming - (beaming % 8)) / 8) + 1;
    beat.times = (beaming % 8) + 1;
    beat.isRest = (data1 & 0x04);
    beat.palmMute = data2 & 0x20;
    beat.accent = data2 & 0x04;
    beat.staccato = data2 & 0x02;

    for (int i = int(sec.beats.size()); i < staff + 1; ++i) {
        sec.beats.push_back({});
    }
    if (add) {
        sec.beats[staff].push_back(beat);
    }
    //sec.getPosition(position).addComponent(beat);
}

std::vector<int> PowerTab::getStaffMap(ptSection& sec)
{
    std::vector<int> result;
    std::vector<int> slash;
    if (!staffInc && curTrack->guitar_ins.size()) {
        auto first = curTrack->guitar_ins.front();
        while (first.section == sec.number) {
            if (first.trackinfo) {
                for (unsigned int i = 0; i < curTrack->infos.size(); ++i) {
                    if ((1 << i) & first.trackinfo) {
                        result.push_back(i);
                    }
                }
            }

            if (first.rhythmSlash) {
                for (unsigned int i = 0; i < curTrack->infos.size(); ++i) {
                    if ((i << 1) & first.rhythmSlash) {
                        slash.push_back(-1 - i);
                    }
                }
            }

            curTrack->guitar_ins.pop_front();
            if (curTrack->guitar_ins.empty()) {
                break;
            }
            first = curTrack->guitar_ins.front();
        }
    }

    if (result.empty() || int(result.size()) < sec.staves) {
        result.clear();
        if (int(lastStaffMap.size()) < sec.staves) {
            for (int i = 0; i < sec.staves; ++i) {
                result.push_back(i + staffInc);
            }
        } else {
            result = lastStaffMap;
        }
    }

    // negative numbers
//    for (auto i : slash) {
//        result.push_back(i);
//    }

    lastStaffMap = result;
    return result;
}

//---------------------------------------------------------
//   addPalmMate
//---------------------------------------------------------

void PowerTab::addPalmMute(Chord* chord)
{
    track_idx_t track = chord->track();
    while (_palmMutes.size() < track + 1) {
        _palmMutes.push_back(0);
    }

    if (_palmMutes[track]) {
        PalmMute* pm = _palmMutes[track];
        Chord* lastChord = toChord(pm->endCR());
        if (lastChord == chord) {
            return;
        }
        //
        // extend the current palm mute or start a new one
        //
        Fraction tick = chord->segment()->tick();
        if (pm->tick2() < tick) {
            _palmMutes[track] = 0;
        } else {
            pm->setTick2(chord->tick() + chord->actualTicks());
            pm->setEndElement(chord);
        }
    }
    if (!_palmMutes[track]) {
        PalmMute* pm = new PalmMute(score->dummy());
        _palmMutes[track] = pm;
        Segment* segment = chord->segment();
        Fraction tick = segment->tick();

        pm->setTick(tick);
        pm->setTick2(tick + chord->actualTicks());
        pm->setTrack(track);
        pm->setTrack2(track);
        pm->setStartElement(chord);
        pm->setEndElement(chord);
        score->addElement(pm);
    }
}

void PowerTab::fillMeasure(tBeatList& elist, Measure* measure, int staff, std::vector<Note*>& tiedNotes)
{
    Tuplet* tuple = nullptr;
    int tupleBeatCounter{ 0 };
    Fraction tick = measure->tick();
    Fraction endtick = measure->endTick();
    Chord* hammer{ nullptr };

    while (elist.size() && tick < endtick) {
        auto& beat = elist.front();
        auto segment = measure->getSegment(SegmentType::ChordRest, tick);
        Fraction l(1, beat.duration);
        int dots = beat.dotted ? (beat.doubleDotted ? 2 : 1) : (beat.doubleDotted ? 2 : 0);
        switch (dots) {
        case 1:
            l = l + (l * Fraction(1, 2));
            break;
        case 2:
            l = l + (l * Fraction(3, 4));
            break;
        }

        TDuration d(l);
        d.setDots(dots);

        if (beat.tuplet || tupleBeatCounter) {
            Fraction nt = (l * Fraction(1, 3) * Fraction(2, 1));
            tick += nt;
        } else {
            tick += l;
        }
        ChordRest* cr;
        if (beat.notes.empty()) {
            auto rest = Factory::createRest(segment);
            cr = rest;
            rest->setTrack(staff * VOICES);
            rest->setTicks(l);
            rest->setDurationType(d);
            segment->add(rest);
        } else {
            Chord* chord = Factory::createChord(segment);
            cr = chord;
            chord->setTrack(staff * VOICES);
            chord->setTicks(l);
            chord->setDurationType(d);
            segment->add(chord);

            if (beat.palmMute) {
                addPalmMute(chord);
            }
            if (beat.accent) {
                auto accent = Factory::createArticulation(chord);
                accent->setSymId(SymId::articAccentAbove);
                chord->add(accent);
            }
            if (beat.staccato) {
                auto st = Factory::createArticulation(chord);
                st->setSymId(SymId::articStaccatoAbove);
                chord->add(st);
            }
            bool has_hammer = false;
            for (const auto& n : beat.notes) {
                auto note = Factory::createNote(chord);
                chord->add(note);
                if (n.dead) {
                    note->setHeadGroup(NoteHeadGroup::HEAD_CROSS);
                    note->setGhost(true);
                }

                if (n.hammer) {
                    has_hammer = true;
                }

                if (n.tied && tiedNotes[n.str]) {
                    Tie* tie = Factory::createTie(tiedNotes[n.str]);
                    tie->setEndNote(note);
                    tiedNotes[n.str]->add(tie);
                }

                if (n.bend) {
                    Bend* bend = Factory::createBend(note);
//TODO-ws                              bend->setNote(note);
                    bend->points().push_back(PitchValue(0, n.bend * 25 - 12));
                    bend->points().push_back(PitchValue(50, 0));
                    note->add(bend);
                }

                if (false && n.slide) {
                    Text* st = Factory::createText(chord->notes().front(), TextStyleType::HARMONY_A);
                    st->setXmlText(String(u"SLIDE %1").arg(n.slide));
                    st->setTrack(staff * VOICES);
                    chord->notes().front()->add(st);
                }

                tiedNotes[n.str] = note;
                note->setFret(n.value);
                note->setString(n.str);
                const StringData* sd = score->staff(staff)->part()->instrument()->stringData();
                int k     = std::max(int(curTrack->infos[staff].strings.size()) - n.str - 1, 0);
                int pitch = sd->stringList().at(k).pitch + n.value;         //getPitch(n.str, n.value, 0);
                note->setPitch(pitch);
                note->setTpcFromPitch();
            }

            if (hammer) {
                auto cr1 = hammer;
                auto cr2 = chord;
                hammer = nullptr;

                Slur* slur = Factory::createSlur(score->dummy());
                slur->setStartElement(cr1);
                slur->setEndElement(cr2);
                slur->setTick(cr1->tick());
                slur->setTick2(tick);
                slur->setTrack(staff * VOICES);
                slur->setTrack2(staff * VOICES);
                score->addElement(slur);

                Text* st = Factory::createText(chord->notes().front(), TextStyleType::HARMONY_A);
                st->setXmlText(u"H");
                st->setTrack(staff * VOICES);
                cr1->notes().front()->add(st);
            }
            if (has_hammer) {
                hammer = chord;
            }
        }

        if (tupleBeatCounter && tuple) {
            tupleBeatCounter--;
            cr->setTuplet(tuple);
            tuple->add(cr);
        }

        if (beat.tuplet && !tuple) {
            tuple = Factory::createTuplet(measure);
            tuple->setParent(measure);
            tuple->setTrack(cr->track());
            tuple->setBaseLen(l);
            tuple->setRatio(Fraction(3, 2));
            tuple->setTicks(l * tuple->ratio().denominator());
            cr->setTuplet(tuple);
            tuple->add(cr);
            tupleBeatCounter = 2;
        }
        elist.pop_front();
    }

    if (tick == measure->tick()) {
        Segment* seg = measure->getSegment(SegmentType::ChordRest, tick);
        auto rest = Factory::createRest(seg);
        rest->setTrack(staff * VOICES);
        auto ts = measure->timesig();
        rest->setTicks(ts);
        rest->setDurationType(TDuration(ts));
        seg->add(rest);
    }
}

void PowerTab::addToScore(ptSection& sec)
{
    cur_section = &sec;
    Fraction tick = score->lastMeasure() ? score->lastMeasure()->endTick() : Fraction(0, 1);

    Fraction lastTS(-1, -1);
    bool firstMeasure = true;
    if (score->lastMeasure()) {
        lastTS = score->lastMeasure()->timesig();
        firstMeasure = false;
    }
    if (firstMeasure) {
        //int lastStaff = sec.staffMap.back() + 1;
        for (int i = 0; i < staves; ++i) {
            Part* part = new Part(score);
            Staff* s = Factory::createStaff(part);
            auto info = &curTrack->infos[i];
            std::string ss = info->name;
            String staffName;
            if (muse::UtfCodec::isValidUtf8(ss)) {
                staffName = muse::String::fromStdString(ss);
            } else {
                // invalid utf most likely windows-125x used
                std::string n = "part " + std::to_string(i + 1);
                staffName = muse::String::fromStdString(n);
            }
            part->setPartName(staffName);
            part->setPlainLongName(staffName);

            std::vector<int> reverseStr;
            for (auto it = info->strings.rbegin(); it != info->strings.rend(); ++it) {
                reverseStr.push_back(*it);
            }
            StringData stringData(32, int(info->strings.size()), reverseStr.data());
            part->instrument()->setStringData(stringData);

            part->setMidiProgram(info->instrument);
            part->setMidiChannel(info->number);

            score->appendStaff(s);
            score->appendPart(part);
        }
    }
    auto bar1 = sec.bars.front();
    while (bar1->denominator == 0) {
        if (sec.bars.size() == 1) {
            break;
        }
        sec.bars.pop_front();
        bar1 = sec.bars.front();
    }
    if (bar1->denominator == 0) {
        bar1->denominator = 4;
        bar1->numerator = 4;
    }
    auto measure = createMeasure(bar1.get(), tick);
    if (repeatCount) {
        measure->setRepeatEnd(true);
        measure->setRepeatCount(repeatCount);
    }
    repeatCount = bar1->repeatClose;
    if (bar1->repeatStart) {
        measure->setRepeatStart(true);
    }
    if (sec.bars.size() > 1) {
        sec.bars.pop_front();
    }
    if (sec.tempo) {
        Segment* segment = measure->getSegment(SegmentType::ChordRest, measure->tick());
        TempoText* tt = new TempoText(segment);
        tt->setTempo(double(sec.tempo) / 60.0f);
        tt->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(sec.tempo));
        tt->setTrack(0);
        segment->add(tt);
        score->setTempo(measure->tick(), tt->tempo());
    }
    if (!sec.partName.empty() && lastPart != sec.partMarker) {
        lastPart = sec.partMarker;
        auto seg = measure->getSegment(SegmentType::ChordRest, measure->tick());
        RehearsalMark* t = new RehearsalMark(seg);
        t->setBorderType(BorderType::SQUARE);
        t->setPlainText(String(Char::fromAscii(sec.partMarker)));
        t->setTrack(0);
        seg->add(t);

        t = new RehearsalMark(seg);
        t->setBorderType(BorderType::NO_BORDER);
        std::string valid;
        muse::UtfCodec::replaceInvalid(sec.partName, valid);
        t->setPlainText(String::fromStdString(valid));
        t->setOffset(PointF(10.0, 0.0));
        t->setTrack(0);
        seg->add(t);
    }
    if (firstMeasure) {
        for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            int keysig = staffIdx >= staffInc ? 0 : 1;       // Can be parsed int beat section
            Segment* s = measure->getSegment(SegmentType::KeySig, tick);
            KeySig* t = Factory::createKeySig(s);
            t->setKey(Key(keysig));
            t->setTrack(staffIdx * VOICES);
            s->add(t);

            ClefType clefId = staffIdx >= staffInc ? ClefType::F8_VB : ClefType::G15_MB;
            s = measure->getSegment(SegmentType::HeaderClef, Fraction(0, 1));
            auto clef = Factory::createClef(s);
            clef->setTrack(staffIdx * VOICES);
            clef->setClefType(clefId);
            s->add(clef);
        }
    }

    std::vector<std::vector<Note*> > tiedNotes(staves);
    for (auto& nvec : tiedNotes) {
        nvec.resize(10);
        memset(nvec.data(), 0, sizeof(Note*) * 10);
    }
    //std::vector<tBeatList>
    while (true) {
        bool empty = true;
        while (int(sec.beats.size()) < staves) {
            sec.beats.push_back({});
        }
        for (unsigned int i = 0; i < sec.beats.size(); ++i) {
            fillMeasure(sec.beats[i], measure, i, tiedNotes[i]);
            if (sec.beats[i].size() && i < unsigned(staffInc)) {
                empty = false;
            }
        }
        if (lastTS != measure->timesig()) {
            lastTS = measure->timesig();
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                Staff* staff = score->staff(staffIdx);
                auto staffType = staff->staffType(Fraction(0, 1));
                if (staffType->genTimesig()) {
                    Segment* s = measure->getSegment(SegmentType::TimeSig, measure->tick());
                    auto t = Factory::createTimeSig(s);
                    t->setTrack(staffIdx * VOICES);
                    t->setSig(lastTS);
                    s->add(t);
                }
            }
        }
        if (empty) {
            break;
        }
        auto bar = sec.bars.front();
        while (bar->denominator == 0) {
            if (sec.bars.size() == 1) {
                break;
            }
            sec.bars.pop_front();
            bar = sec.bars.front();
        }
        if (bar->denominator == 0) {
            bar->denominator = measure->timesig().denominator();
            bar->numerator = measure->timesig().numerator();
        }
        measure = createMeasure(bar.get(), measure->endTick());
        if (repeatCount) {
            measure->setRepeatEnd(true);
            measure->setRepeatCount(repeatCount);
        }
        repeatCount = bar->repeatClose;
        if (bar->repeatStart) {
            measure->setRepeatStart(true);
        }
        if (sec.bars.size() > 2) {
            sec.bars.pop_front();
        }
    }
    if (sec.bars.size()) {
        auto bar = sec.bars.back();
        if (bar->repeatStart) {
            measure->setRepeatStart(true);
        }
        if (bar->repeatClose) {
            measure->setRepeatEnd(true);
            measure->setRepeatCount(bar->repeatClose);
        }
        sec.bars.clear();
    }
}

void PowerTab::ptSection::copyTracks(ptTrack* track)
{
    //if not found GuitarIn in section or all tracks are read -> return
    if (staves == int(staffMap.size())) {
        return;
    }

    auto signature = chordTextMap.begin();
    for (unsigned index = 0; index < staffMap.size(); ++index) {
        auto staff = (staffMap[index] + 1) * -1;
        if (staff < 0) {
            continue;
        }

        for (const auto& rt : rhythm) {
            auto newSig = chordTextMap.find(rt.position);
            if (newSig != chordTextMap.end()) {
                signature = newSig;
            }

            ptBeat beat = ptBeat(staff, 0);
            beat.position = rt.position;
            beat.duration = rt.duration;
            beat.dotted = rt.dotted;
            beat.doubleDotted = rt.doubleDotted;
            beat.isRest = rt.is_rest;
            beat.tuplet = rt.triplet;
            if (!rt.is_rest) {
                auto diagram = track->diagramMap.find({ { signature->second.key, signature->second.formula,
                                                          signature->second.formula_mod } });

                if (diagram != track->diagramMap.end()) {
                    for (unsigned int string = 0; string < diagram->second.frets.size(); ++string) {
                        int fret = diagram->second.frets[string];
                        if (fret >= 0xFE) {
                            continue;
                        }
                        ptNote note;
                        note.value = fret;
                        note.str = string;
                        if (fret == 0xFE) {
                            note.dead = true;
                            note.value = 0;
                        }
                        beat.notes.emplace_back(note);
                    }
                } else {
                    LOGD() << "diagram wasn't found";
                }
            }
            while (int(beats.size()) <= staff) {
                beats.push_back({});
            }
            beats[staff].push_back(beat);
        }
    }
}

void PowerTab::readSection(ptSection& sec)
{
    //rect:
    readInt();   // left
    readInt();   // top
    readInt();   // right
    readInt();   // bottom

    auto lastBarData = readUInt8();

    skip(4);   //spacing from staff

    readBarLine(sec);

    int itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readDirection(sec);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //ChordText section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readChordText(sec);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //RhytmSlash
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readRhytmSlash(sec);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    // Staff
    sec.staves = readHeaderItems();
    sec.staffMap = getStaffMap(sec);
    for (int i = 0; i < sec.staves; ++i) {
        int staff = sec.staffMap[i];
        readStaff(staff, sec);
        if (i < sec.staves - 1) {
            readShort();
        }
    }
    sec.copyTracks(curTrack);
    // MusicBar section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readBarLine(sec);
        if (i < itemCount - 1) {
            readShort();
        }
    }

    auto bar = new ptBar();
    bar->repeatClose = (lastBarData >> 5 == 4) ? lastBarData - 128 : 0;
    sec.bars.push_back(std::shared_ptr<ptBar>(bar));
    //sec.getPosition(sec.getNextPositionNumber()).addComponent(bar);
}

void PowerTab::readDirection(ptSection& sec)
{
    int position = readUInt8();
    int symbolCount = readUInt8();
    for (int i = 0; i < symbolCount; ++i) {
        unsigned int data = readShort();
        sec.getPosition(position).addComponent(new ptDirection((data >> 8), ((data & 0xC0) >> 6), (data & 0x1F)));
    }
}

void PowerTab::readDataInstruments(ptTrack& info)
{
    curTrack = &info;
    //Guitar section
    auto itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readTrackInfo(info);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    // Chord Diagram Section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readChord(info);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //Floating Text section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readFloatingText();
        if (i < itemCount - 1) {
            readShort();
        }
    }
    // GuitarIn section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readGuitarIn(info);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    // Tempo marker
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readTempoMarker(info);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //Dynamic section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readDynamic();
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //Symbol section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readSectionSymbol(info);
        if (i < itemCount - 1) {
            readShort();
        }
    }
    //Section section
    itemCount = readHeaderItems();
    for (int i = 0; i < itemCount; ++i) {
        readSection(info.getSection(i));
        if (i < itemCount - 1) {
            readShort();
        }
    }
}

std::string crTS(int strings, int tuning[])
{
    const static char* tune[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    std::vector<int> pitch;
    for (int i = 0; i < strings; ++i) {
        pitch.push_back(tuning[i]);
    }
    std::string t;
    for (auto i : pitch) {
        t += tune[i % 12];
        t += " ";
    }
    return t;
}

Measure* PowerTab::createMeasure(ptBar* bar, const Fraction& tick)
{
    auto measure = Factory::createMeasure(score->dummy()->system());
    Fraction nts(bar->numerator, bar->denominator);

    measure->setTick(tick);
    measure->setTimesig(nts);
    measure->setTicks(nts);

    score->measures()->append(measure);

    return measure;
}

PowerTab::ptSection& PowerTab::ptTrack::getSection(int ind)
{
    for (int i = (int)sections.size(); i < ind + 1; ++i) {
        sections.push_back(ptSection(i));
    }
    return sections[ind];
}

PowerTab::ptSection::ptSection(int num)
{
    number = num;
}

PowerTab::ptPosition& PowerTab::ptSection::getPosition(int pos)
{
    unsigned int i = 0;
    while (i < positions.size()) {
        auto& p = positions[i];
        if (p.position == pos) {
            return p;
        }
        ++i;
    }
    ptPosition p;
    p.position = pos;
    positions.emplace_back(p);
    return positions.back();
}

int PowerTab::ptSection::getNextPositionNumber()
{
    int next = 0;
    unsigned int k = 0;
    while (k < positions.size()) {
        auto p = positions[k];
        next = std::max(next, p.position + 1);
        ++k;
    }
    return next;
}

void PowerTab::ptPosition::addComponent(ptComponent* c)
{
    components.push_back(std::shared_ptr<ptComponent>(c));
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

Err PowerTab::read()
{
    if (!readVersion()) {
        return Err::FileBadFormat;
    }
    ptSong song;

    readSongInfo(song.info);
    readDataInstruments(song.track1);
    staffInc = int(song.track1.infos.size());
    lastStaffMap.clear();
    readDataInstruments(song.track1);

    staves = int(song.track1.infos.size());

    std::vector<tBeatList> parts(staves);
    for (int i = staffInc; i < staves; ++i) {
        for (auto& sec : song.track1.sections) {
            while (int(sec.beats.size()) < staves) {
                sec.beats.push_back({});
            }
            parts[i].insert(parts[i].end(), sec.beats[i].begin(), sec.beats[i].end());
            sec.beats[i].clear();
        }
    }

    for (auto& sec : song.track1.sections) {
        for (int i = 0; i < staves; ++i) {
            if (parts[i].size()) {
                sec.beats[i].insert(sec.beats[i].begin(), parts[i].begin(), parts[i].end());
                parts[i].clear();
            }
        }
        addToScore(sec);
        for (int i = 0; i < staves; ++i) {
            parts[i] = sec.beats[i];
        }
    }

    MeasureBase* m;
    if (!score->measures()->first()) {
        m = Factory::createTitleVBox(score->dummy()->system());
        score->measures()->append(m);
    } else {
        m = score->measures()->first();
        if (!m->isVBox()) {
            MeasureBase* mb = Factory::createTitleVBox(score->dummy()->system());
            score->addMeasure(mb, m);
            m = mb;
        }
    }
    // create title
    std::string name = song.info.name;
    if (!name.empty()) {
        Text* s = Factory::createText(m, TextStyleType::TITLE);
        std::string valid;
        muse::UtfCodec::replaceInvalid(name, valid);
        s->setPlainText(String::fromStdString(valid));
        m->add(s);
    }
    return Err::NoError;
}
} // namespace mu::iex::guitarpro
