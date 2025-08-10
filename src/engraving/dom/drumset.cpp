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

#include "drumset.h"

#include "translation.h"

#include "types/typesconv.h"
#include "types/symnames.h"

#include "rw/xmlwriter.h"
#include "rw/xmlreader.h"

#include "articulation.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
Drumset* smDrumset = nullptr;           // standard midi drumset

bool Drumset::isValid(int pitch) const
{
    return pitch >= 0 && pitch < DRUM_INSTRUMENTS && !m_drums[pitch].name.empty();
}

String Drumset::translatedName(int pitch) const
{
    return muse::mtrc("engraving/drumset", name(pitch));
}

int Drumset::pitchForShortcut(const String& shortcut) const
{
    if (shortcut.isEmpty()) {
        return -1;
    }

    for (int pitch = 0; pitch < DRUM_INSTRUMENTS; ++pitch) {
        if (!isValid(pitch)) {
            continue;
        }

        if (drum(pitch).shortcut != shortcut) {
            continue;
        }

        return pitch;
    }

    return -1;
}

int Drumset::defaultPitchForLine(int val) const
{
    int firstValidPitch = -1;
    for (int pitch = 0; pitch < DRUM_INSTRUMENTS; ++pitch) {
        if (!isValid(pitch) || line(pitch) != val) {
            continue;
        }

        if (firstValidPitch < 0) {
            firstValidPitch = pitch;
        }

        const NoteHeadGroup headGroup = noteHead(pitch);
        if (headGroup == NoteHeadGroup::HEAD_NORMAL) {
            return pitch;
        }
    }
    return firstValidPitch;
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(XmlWriter& xml) const
{
    xml.tag("percussionPanelColumns", m_percussionPanelColumns);
    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        if (!isValid(i)) {
            continue;
        }
        xml.startElement("Drum", { { "pitch", i } });
        const NoteHeadGroup nh = noteHead(i);
        //write custom as Normal notehead group + noteheads tag to keep compatibility with 2.X versions
        const NoteHeadGroup saveNHValue = (nh == NoteHeadGroup::HEAD_CUSTOM) ? NoteHeadGroup::HEAD_NORMAL : nh;
        xml.tag("head", TConv::toXml(saveNHValue));
        if (nh == NoteHeadGroup::HEAD_CUSTOM) {
            xml.startElement("noteheads");
            for (int j = 0; j < int(NoteHeadType::HEAD_TYPES); j++) {
                xml.tag(TConv::toXml(NoteHeadType(j)), SymNames::nameForSymId(noteHeads(i, NoteHeadType(j))));
            }
            xml.endElement();
        }
        xml.tag("line", line(i));
        xml.tag("voice", voice(i));
        xml.tag("name", name(i));
        xml.tag("stem", int(stemDirection(i)));
        if (!shortcut(i).empty()) {
            xml.tag("shortcut", shortcut(i));
        }
        std::list<DrumInstrumentVariant> vs = variants(i);
        if (!vs.empty()) {
            xml.startElement("variants");
            for (const auto& v : vs) {
                xml.startElement("variant", { { "pitch", v.pitch } });
                if (!v.articulationName.empty()) {
                    xml.tag("articulation", v.articulationName);
                }
                if (v.tremolo != TremoloType::INVALID_TREMOLO) {
                    xml.tag("tremolo", TConv::toXml(v.tremolo));
                }
                xml.endElement();
            }
            xml.endElement();
        }
        if (panelRow(i) > -1 && panelColumn(i) > -1) {
            xml.tag("panelRow", panelRow(i));
            xml.tag("panelColumn", panelColumn(i));
        }
        xml.endElement();
    }
}

bool Drumset::readDrumProperties(XmlReader& e, int pitch)
{
    if (pitch < 0 || pitch > DRUM_INSTRUMENTS - 1) {
        return false;
    }

    const AsciiStringView tag(e.name());
    if (tag == "head") {
        m_drums[pitch].notehead = TConv::fromXml(e.readAsciiText(), NoteHeadGroup::HEAD_NORMAL);
    } else if (tag == "noteheads") {
        m_drums[pitch].notehead = NoteHeadGroup::HEAD_CUSTOM;
        while (e.readNextStartElement()) {
            const AsciiStringView nhTag(e.name());
            int noteType = int(TConv::fromXml(nhTag, NoteHeadType::HEAD_AUTO));
            if (noteType > int(NoteHeadType::HEAD_TYPES) - 1 || noteType < 0) {
                return false;
            }

            m_drums[pitch].noteheads[noteType] = SymNames::symIdByName(e.readAsciiText());
        }
    } else if (tag == "line") {
        m_drums[pitch].line = e.readInt();
    } else if (tag == "voice") {
        m_drums[pitch].voice = e.readInt();
    } else if (tag == "name") {
        m_drums[pitch].name = e.readText();
    } else if (tag == "stem") {
        m_drums[pitch].stemDirection = DirectionV(e.readInt());
    } else if (tag == "shortcut") {
        m_drums[pitch].shortcut = e.readText();
    } else if (tag == "variants") {
        while (e.readNextStartElement()) {
            const AsciiStringView tagv(e.name());
            if (tagv == "variant") {
                DrumInstrumentVariant div;
                div.pitch = e.attribute("pitch").toInt();
                while (e.readNextStartElement()) {
                    const AsciiStringView taga(e.name());
                    if (taga == "articulation") {
                        div.articulationName = e.readText();
                    } else if (taga == "tremolo") {
                        div.tremolo = TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO);
                    }
                }
                m_drums[pitch].addVariant(div);
            }
        }
    } else if (tag == "panelRow") {
        m_drums[pitch].panelRow = e.readInt();
    } else if (tag == "panelColumn") {
        m_drums[pitch].panelColumn = e.readInt();
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::loadDrum(XmlReader& e)
{
    int pitch = e.intAttribute("pitch", -1);
    if (pitch < 0 || pitch > DRUM_INSTRUMENTS - 1) {
        LOGD("load drumset: invalid pitch %d", pitch);
        return;
    }
    while (e.readNextStartElement()) {
        if (readDrumProperties(e, pitch)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Drumset::clear()
{
    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        m_drums[i].name.clear();
        m_drums[i].notehead = NoteHeadGroup::HEAD_INVALID;
        m_drums[i].shortcut.clear();
        m_drums[i].variants.clear();
        m_drums[i].panelRow = -1;
        m_drums[i].panelColumn = -1;
    }
}

//---------------------------------------------------------
//   nextPitch
//---------------------------------------------------------

int Drumset::nextPitch(int ii) const
{
    for (int i = ii + 1; i < DRUM_INSTRUMENTS - 1; ++i) {
        if (isValid(i)) {
            return i;
        }
    }
    for (int i = 0; i <= ii; ++i) {
        if (isValid(i)) {
            return i;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   prevPitch
//---------------------------------------------------------

int Drumset::prevPitch(int ii) const
{
    for (int i = ii - 1; i >= 0; --i) {
        if (isValid(i)) {
            return i;
        }
    }
    for (int i = DRUM_INSTRUMENTS - 1; i >= ii; --i) {
        if (isValid(i)) {
            return i;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   findVariant
/// find a variant for the given pitch with matching chord articulation and tremolo
//---------------------------------------------------------

DrumInstrumentVariant Drumset::findVariant(int p, const std::vector<Articulation*>& articulations, TremoloType tremType) const
{
    DrumInstrumentVariant div;
    auto vs = variants(p);
    for (const auto& v : vs) {
        bool matchTremolo = ((tremType == TremoloType::INVALID_TREMOLO) && (v.tremolo == TremoloType::INVALID_TREMOLO))
                            || (v.tremolo == tremType);
        bool matchArticulation = v.articulationName.empty() && articulations.empty();
        for (auto a : articulations) {
            matchArticulation = a->articulationName() == v.articulationName;
            if (!matchArticulation) {
                break;
            }
        }
        if (matchArticulation && matchTremolo) {
            div = v;
            break;
        }
    }
    return div;
}

//---------------------------------------------------------
//   initDrumset
//    initialize standard midi drumset
//---------------------------------------------------------

void Drumset::initDrumset()
{
    smDrumset = new Drumset;
    for (int i = 0; i < DRUM_INSTRUMENTS; ++i) {
        smDrumset->drum(i).name.clear();
        smDrumset->drum(i).notehead = NoteHeadGroup::HEAD_INVALID;
        smDrumset->drum(i).line = 0;
        smDrumset->drum(i).shortcut.clear();
        smDrumset->drum(i).voice = 0;
        smDrumset->drum(i).stemDirection = DirectionV::UP;
        smDrumset->drum(i).panelRow = -1;
        smDrumset->drum(i).panelColumn = -1;
    }

    // Code generated by share/instruments/update_instruments_xml.py. To edit this code:
    // 1. Make your changes in the proper place:
    //     * To change instrument data, edit the online spreadsheet.
    //     * For other code changes, edit the Python script.
    // 2. Run the Python script to generate the changes here.
    // 3. Run Uncrustify and update the Python script if necessary to match Uncrustify's formatting.

    // BEGIN GENERATED CODE

    // Laser (High Q)
    smDrumset->drum(27) = DrumInstrument(
        TConv::userName(DrumNum(27)),
        NoteHeadGroup::HEAD_SLASH,
        /*line*/ 8,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Slap
    smDrumset->drum(28) = DrumInstrument(
        TConv::userName(DrumNum(28)),
        NoteHeadGroup::HEAD_CUSTOM,
        /*line*/ 4,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    smDrumset->drum(28).noteheads[static_cast<int>(NoteHeadType::HEAD_WHOLE)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(28).noteheads[static_cast<int>(NoteHeadType::HEAD_HALF)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(28).noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(28).noteheads[static_cast<int>(NoteHeadType::HEAD_BREVIS)] = SymNames::symIdByName("noteheadSlashX");

    // Scratch Push
    smDrumset->drum(29) = DrumInstrument(
        TConv::userName(DrumNum(29)),
        NoteHeadGroup::HEAD_SLASH,
        /*line*/ 6,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Scratch Pull
    smDrumset->drum(30) = DrumInstrument(
        TConv::userName(DrumNum(30)),
        NoteHeadGroup::HEAD_SLASH,
        /*line*/ 6,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Sticks
    smDrumset->drum(31) = DrumInstrument(
        TConv::userName(DrumNum(31)),
        NoteHeadGroup::HEAD_PLUS,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Square Click
    smDrumset->drum(32) = DrumInstrument(
        TConv::userName(DrumNum(32)),
        NoteHeadGroup::HEAD_PLUS,
        /*line*/ 10,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Metronome Click
    smDrumset->drum(33) = DrumInstrument(
        TConv::userName(DrumNum(33)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 10,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Metronome Bell
    smDrumset->drum(34) = DrumInstrument(
        TConv::userName(DrumNum(34)),
        NoteHeadGroup::HEAD_TRIANGLE_UP,
        /*line*/ 10,
        DirectionV::UP,
        /*panelRow*/ 0,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Acoustic Bass Drum
    smDrumset->drum(35) = DrumInstrument(
        TConv::userName(DrumNum(35)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 8,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 0,
        /*voice*/ 1,
        /*shortcut*/ String());

    // Electric Bass Drum
    smDrumset->drum(36) = DrumInstrument(
        TConv::userName(DrumNum(36)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 7,
        DirectionV::DOWN,
        /*panelRow*/ 1,
        /*panelColumn*/ 1,
        /*voice*/ 1,
        /*shortcut*/ String());

    // Side Stick
    smDrumset->drum(37) = DrumInstrument(
        TConv::userName(DrumNum(37)),
        NoteHeadGroup::HEAD_SLASHED1,
        /*line*/ 3,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Acoustic Snare
    smDrumset->drum(38) = DrumInstrument(
        TConv::userName(DrumNum(38)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 3,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Hand Clap
    smDrumset->drum(39) = DrumInstrument(
        TConv::userName(DrumNum(39)),
        NoteHeadGroup::HEAD_PLUS,
        /*line*/ -2,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Electric Snare
    smDrumset->drum(40) = DrumInstrument(
        TConv::userName(DrumNum(40)),
        NoteHeadGroup::HEAD_SLASH,
        /*line*/ 3,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Floor Tom
    smDrumset->drum(41) = DrumInstrument(
        TConv::userName(DrumNum(41)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 6,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Closed Hi-hat
    smDrumset->drum(42) = DrumInstrument(
        TConv::userName(DrumNum(42)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 1,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Floor Tom
    smDrumset->drum(43) = DrumInstrument(
        TConv::userName(DrumNum(43)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 5,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Pedal Hi-hat
    smDrumset->drum(44) = DrumInstrument(
        TConv::userName(DrumNum(44)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 9,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 1,
        /*voice*/ 1,
        /*shortcut*/ String());

    // Low Tom
    smDrumset->drum(45) = DrumInstrument(
        TConv::userName(DrumNum(45)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 4,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Open Hi-hat
    smDrumset->drum(46) = DrumInstrument(
        TConv::userName(DrumNum(46)),
        NoteHeadGroup::HEAD_XCIRCLE,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low-Mid Tom
    smDrumset->drum(47) = DrumInstrument(
        TConv::userName(DrumNum(47)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Hi-Mid Tom
    smDrumset->drum(48) = DrumInstrument(
        TConv::userName(DrumNum(48)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 1,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Crash Cymbal 1
    smDrumset->drum(49) = DrumInstrument(
        TConv::userName(DrumNum(49)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -2,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Tom
    smDrumset->drum(50) = DrumInstrument(
        TConv::userName(DrumNum(50)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 2,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Ride Cymbal 1
    smDrumset->drum(51) = DrumInstrument(
        TConv::userName(DrumNum(51)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // China Cymbal
    smDrumset->drum(52) = DrumInstrument(
        TConv::userName(DrumNum(52)),
        NoteHeadGroup::HEAD_CUSTOM,
        /*line*/ -3,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    smDrumset->drum(52).noteheads[static_cast<int>(NoteHeadType::HEAD_WHOLE)] = SymNames::symIdByName("noteheadHeavyXHat");
    smDrumset->drum(52).noteheads[static_cast<int>(NoteHeadType::HEAD_HALF)] = SymNames::symIdByName("noteheadHeavyXHat");
    smDrumset->drum(52).noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)] = SymNames::symIdByName("noteheadHeavyXHat");
    smDrumset->drum(52).noteheads[static_cast<int>(NoteHeadType::HEAD_BREVIS)] = SymNames::symIdByName("noteheadHeavyXHat");

    // Ride Bell
    smDrumset->drum(53) = DrumInstrument(
        TConv::userName(DrumNum(53)),
        NoteHeadGroup::HEAD_DIAMOND,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Tambourine
    smDrumset->drum(54) = DrumInstrument(
        TConv::userName(DrumNum(54)),
        NoteHeadGroup::HEAD_DIAMOND,
        /*line*/ 6,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Splash Cymbal
    smDrumset->drum(55) = DrumInstrument(
        TConv::userName(DrumNum(55)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -4,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Cowbell
    smDrumset->drum(56) = DrumInstrument(
        TConv::userName(DrumNum(56)),
        NoteHeadGroup::HEAD_TRIANGLE_DOWN,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Crash Cymbal 2
    smDrumset->drum(57) = DrumInstrument(
        TConv::userName(DrumNum(57)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -3,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Vibraslap
    smDrumset->drum(58) = DrumInstrument(
        TConv::userName(DrumNum(58)),
        NoteHeadGroup::HEAD_TI,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 3,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Ride Cymbal 2
    smDrumset->drum(59) = DrumInstrument(
        TConv::userName(DrumNum(59)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Bongo
    smDrumset->drum(60) = DrumInstrument(
        TConv::userName(DrumNum(60)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Bongo
    smDrumset->drum(61) = DrumInstrument(
        TConv::userName(DrumNum(61)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Mute High Conga
    smDrumset->drum(62) = DrumInstrument(
        TConv::userName(DrumNum(62)),
        NoteHeadGroup::HEAD_CUSTOM,
        /*line*/ 1,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    smDrumset->drum(62).noteheads[static_cast<int>(NoteHeadType::HEAD_WHOLE)] = SymNames::symIdByName("noteheadXOrnate");
    smDrumset->drum(62).noteheads[static_cast<int>(NoteHeadType::HEAD_HALF)] = SymNames::symIdByName("noteheadXOrnate");
    smDrumset->drum(62).noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)] = SymNames::symIdByName("noteheadXOrnate");
    smDrumset->drum(62).noteheads[static_cast<int>(NoteHeadType::HEAD_BREVIS)] = SymNames::symIdByName("noteheadXOrnate");

    // Open High Conga
    smDrumset->drum(63) = DrumInstrument(
        TConv::userName(DrumNum(63)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 1,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Conga
    smDrumset->drum(64) = DrumInstrument(
        TConv::userName(DrumNum(64)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Timbale
    smDrumset->drum(65) = DrumInstrument(
        TConv::userName(DrumNum(65)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 5,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Timbale
    smDrumset->drum(66) = DrumInstrument(
        TConv::userName(DrumNum(66)),
        NoteHeadGroup::HEAD_NORMAL,
        /*line*/ 7,
        DirectionV::UP,
        /*panelRow*/ 4,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Agogô
    smDrumset->drum(67) = DrumInstrument(
        TConv::userName(DrumNum(67)),
        NoteHeadGroup::HEAD_TRIANGLE_DOWN,
        /*line*/ -2,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Agogô
    smDrumset->drum(68) = DrumInstrument(
        TConv::userName(DrumNum(68)),
        NoteHeadGroup::HEAD_TRIANGLE_DOWN,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Cabasa
    smDrumset->drum(69) = DrumInstrument(
        TConv::userName(DrumNum(69)),
        NoteHeadGroup::HEAD_DIAMOND,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Maracas
    smDrumset->drum(70) = DrumInstrument(
        TConv::userName(DrumNum(70)),
        NoteHeadGroup::HEAD_DIAMOND,
        /*line*/ 4,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Short Whistle
    smDrumset->drum(71) = DrumInstrument(
        TConv::userName(DrumNum(71)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -3,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Long Whistle
    smDrumset->drum(72) = DrumInstrument(
        TConv::userName(DrumNum(72)),
        NoteHeadGroup::HEAD_TI,
        /*line*/ -3,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Short Güiro
    smDrumset->drum(73) = DrumInstrument(
        TConv::userName(DrumNum(73)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Long Güiro
    smDrumset->drum(74) = DrumInstrument(
        TConv::userName(DrumNum(74)),
        NoteHeadGroup::HEAD_SLASHED1,
        /*line*/ -1,
        DirectionV::UP,
        /*panelRow*/ 5,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Claves
    smDrumset->drum(75) = DrumInstrument(
        TConv::userName(DrumNum(75)),
        NoteHeadGroup::HEAD_LA,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // High Woodblock
    smDrumset->drum(76) = DrumInstrument(
        TConv::userName(DrumNum(76)),
        NoteHeadGroup::HEAD_LA,
        /*line*/ 5,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Low Woodblock
    smDrumset->drum(77) = DrumInstrument(
        TConv::userName(DrumNum(77)),
        NoteHeadGroup::HEAD_LA,
        /*line*/ 7,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Mute Cuica
    smDrumset->drum(78) = DrumInstrument(
        TConv::userName(DrumNum(78)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 8,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Open Cuica
    smDrumset->drum(79) = DrumInstrument(
        TConv::userName(DrumNum(79)),
        NoteHeadGroup::HEAD_SLASHED2,
        /*line*/ 8,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Mute Triangle
    smDrumset->drum(80) = DrumInstrument(
        TConv::userName(DrumNum(80)),
        NoteHeadGroup::HEAD_CROSS,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 5,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Open Triangle
    smDrumset->drum(81) = DrumInstrument(
        TConv::userName(DrumNum(81)),
        NoteHeadGroup::HEAD_TRIANGLE_UP,
        /*line*/ 0,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 6,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Shaker
    smDrumset->drum(82) = DrumInstrument(
        TConv::userName(DrumNum(82)),
        NoteHeadGroup::HEAD_DIAMOND,
        /*line*/ 5,
        DirectionV::UP,
        /*panelRow*/ 6,
        /*panelColumn*/ 7,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Jingle Bell
    smDrumset->drum(83) = DrumInstrument(
        TConv::userName(DrumNum(83)),
        NoteHeadGroup::HEAD_TRIANGLE_DOWN,
        /*line*/ 3,
        DirectionV::UP,
        /*panelRow*/ 7,
        /*panelColumn*/ 0,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Belltree
    smDrumset->drum(84) = DrumInstrument(
        TConv::userName(DrumNum(84)),
        NoteHeadGroup::HEAD_TI,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 7,
        /*panelColumn*/ 1,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Castanets
    smDrumset->drum(85) = DrumInstrument(
        TConv::userName(DrumNum(85)),
        NoteHeadGroup::HEAD_LA,
        /*line*/ 2,
        DirectionV::UP,
        /*panelRow*/ 7,
        /*panelColumn*/ 2,
        /*voice*/ 0,
        /*shortcut*/ String());

    // Mute Surdo
    smDrumset->drum(86) = DrumInstrument(
        TConv::userName(DrumNum(86)),
        NoteHeadGroup::HEAD_CUSTOM,
        /*line*/ 4,
        DirectionV::UP,
        /*panelRow*/ 7,
        /*panelColumn*/ 3,
        /*voice*/ 0,
        /*shortcut*/ String());

    smDrumset->drum(86).noteheads[static_cast<int>(NoteHeadType::HEAD_WHOLE)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(86).noteheads[static_cast<int>(NoteHeadType::HEAD_HALF)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(86).noteheads[static_cast<int>(NoteHeadType::HEAD_QUARTER)] = SymNames::symIdByName("noteheadSlashX");
    smDrumset->drum(86).noteheads[static_cast<int>(NoteHeadType::HEAD_BREVIS)] = SymNames::symIdByName("noteheadSlashX");

    // Open Surdo
    smDrumset->drum(87) = DrumInstrument(
        TConv::userName(DrumNum(87)),
        NoteHeadGroup::HEAD_SLASH,
        /*line*/ 4,
        DirectionV::UP,
        /*panelRow*/ 7,
        /*panelColumn*/ 4,
        /*voice*/ 0,
        /*shortcut*/ String());

    // END GENERATED CODE
}
}
