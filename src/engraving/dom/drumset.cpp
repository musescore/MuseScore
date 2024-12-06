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

String Drumset::translatedName(int pitch) const
{
    return muse::mtrc("engraving/drumset", name(pitch));
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(XmlWriter& xml) const
{
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
        if (shortcut(i)) {
            switch (shortcut(i)) {
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'G':
            case 'A':
            case 'B':
            {
                char a[2];
                a[0] = shortcut(i);
                a[1] = 0;
                xml.tag("shortcut", a);
            }
            break;
            default:
                LOGD("illegal drum shortcut");
                break;
            }
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

bool Drumset::readProperties(XmlReader& e, int pitch)
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
        bool isNum;
        AsciiStringView val = e.readAsciiText();
        int i = val.toInt(&isNum);
        m_drums[pitch].shortcut = isNum ? i : val.at(0).toUpper();
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

void Drumset::load(XmlReader& e)
{
    int pitch = e.intAttribute("pitch", -1);
    if (pitch < 0 || pitch > DRUM_INSTRUMENTS - 1) {
        LOGD("load drumset: invalid pitch %d", pitch);
        return;
    }
    while (e.readNextStartElement()) {
        if (readProperties(e, pitch)) {
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
        m_drums[i].name = u"";
        m_drums[i].notehead = NoteHeadGroup::HEAD_INVALID;
        m_drums[i].shortcut = 0;
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
        smDrumset->drum(i).notehead = NoteHeadGroup::HEAD_INVALID;
        smDrumset->drum(i).line     = 0;
        smDrumset->drum(i).shortcut = 0;
        smDrumset->drum(i).voice    = 0;
        smDrumset->drum(i).stemDirection = DirectionV::UP;
        smDrumset->drum(i).panelRow     = -1;
        smDrumset->drum(i).panelColumn  = -1;
    }
    // Acoustic Bass Drum
    smDrumset->drum(35) = DrumInstrument(TConv::userName(DrumNum(35)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 8, DirectionV::DOWN,
                                         /*panelRow*/ 2, /*panelColumn*/ 1, /*voice*/ 1);

    // Bass Drum 1
    smDrumset->drum(36) = DrumInstrument(TConv::userName(DrumNum(36)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 7, DirectionV::DOWN,
                                         /*panelRow*/ 2, /*panelColumn*/ 0, /*voice*/ 1, Key_B);

    // Side Stick
    smDrumset->drum(37) = DrumInstrument(TConv::userName(DrumNum(37)), NoteHeadGroup::HEAD_SLASHED1, /*line*/ 3, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 1);

    // Acoustic Snare
    smDrumset->drum(38) = DrumInstrument(TConv::userName(DrumNum(38)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 3, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 0, /*voice*/ 0, Key_A);

    // Electric Snare
    smDrumset->drum(40) = DrumInstrument(TConv::userName(DrumNum(40)), NoteHeadGroup::HEAD_SLASH, /*line*/ 3, DirectionV::UP,
                                         /*panelRow*/ 2, /*panelColumn*/ 6);

    // Low Floor Tom
    smDrumset->drum(41) = DrumInstrument(TConv::userName(DrumNum(41)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 6, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 7);

    // Closed Hi-Hat
    smDrumset->drum(42) = DrumInstrument(TConv::userName(DrumNum(42)), NoteHeadGroup::HEAD_CROSS, /*line*/ -1, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 0, /*voice*/ 0, Key_G);

    // High Floor Tom
    smDrumset->drum(43) = DrumInstrument(TConv::userName(DrumNum(43)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 5, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 6);

    // Pedal Hi-Hat
    smDrumset->drum(44) = DrumInstrument(TConv::userName(DrumNum(44)), NoteHeadGroup::HEAD_CROSS, /*line*/ 9, DirectionV::DOWN,
                                         /*panelRow*/ 2, /*panelColumn*/ 2, /*voice*/ 1, Key_F);

    // Low Tom
    smDrumset->drum(45) = DrumInstrument(TConv::userName(DrumNum(45)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 4, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 5);

    // Open Hi-Hat
    smDrumset->drum(46) = DrumInstrument(TConv::userName(DrumNum(46)), NoteHeadGroup::HEAD_XCIRCLE, /*line*/ -1, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 1);

    // Low-Mid Tom
    smDrumset->drum(47) = DrumInstrument(TConv::userName(DrumNum(47)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 2, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 4);

    // Hi-Mid Tom
    smDrumset->drum(48) = DrumInstrument(TConv::userName(DrumNum(48)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 1, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 3);

    // Crash Cymbal 1
    smDrumset->drum(49) = DrumInstrument(TConv::userName(DrumNum(49)), NoteHeadGroup::HEAD_CROSS, /*line*/ -2, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 4, /*voice*/ 0, Key_C);

    // High Tom
    smDrumset->drum(50) = DrumInstrument(TConv::userName(DrumNum(50)), NoteHeadGroup::HEAD_NORMAL, /*line*/ 0, DirectionV::UP,
                                         /*panelRow*/ 1, /*panelColumn*/ 2, /*voice*/ 0, Key_E);

    // Ride Cymbal 1
    smDrumset->drum(51) = DrumInstrument(TConv::userName(DrumNum(51)), NoteHeadGroup::HEAD_CROSS, /*line*/ 0, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 2, /*voice*/ 0, Key_D);

    // Chinese Cymbal
    smDrumset->drum(52) = DrumInstrument(TConv::userName(DrumNum(52)), NoteHeadGroup::HEAD_CROSS, /*line*/ -4, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 6);

    // Ride Bell
    smDrumset->drum(53) = DrumInstrument(TConv::userName(DrumNum(53)), NoteHeadGroup::HEAD_DIAMOND, /*line*/ 0, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 3);

    // Tambourine
    smDrumset->drum(54) = DrumInstrument(TConv::userName(DrumNum(54)), NoteHeadGroup::HEAD_DIAMOND, /*line*/ 1, DirectionV::UP,
                                         /*panelRow*/ 2, /*panelColumn*/ 4);

    // Splash Cymbal
    smDrumset->drum(55) = DrumInstrument(TConv::userName(DrumNum(55)), NoteHeadGroup::HEAD_CROSS, /*line*/ -5, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 7);

    // Cowbell
    smDrumset->drum(56) = DrumInstrument(TConv::userName(DrumNum(56)), NoteHeadGroup::HEAD_TRIANGLE_DOWN, /*line*/ 1, DirectionV::UP,
                                         /*panelRow*/ 2, /*panelColumn*/ 3);

    // Crash Cymbal 2
    smDrumset->drum(57) = DrumInstrument(TConv::userName(DrumNum(57)), NoteHeadGroup::HEAD_CROSS, /*line*/ -3, DirectionV::UP,
                                         /*panelRow*/ 0, /*panelColumn*/ 5);

    // Ride Cymbal 2
    smDrumset->drum(59) = DrumInstrument(TConv::userName(DrumNum(59)), NoteHeadGroup::HEAD_CROSS, /*line*/ 2, DirectionV::UP,
                                         /*panelRow*/ 2, /*panelColumn*/ 5);
}
}
