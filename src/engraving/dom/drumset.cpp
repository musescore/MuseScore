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

#include "drumset.h"

#include "translation.h"

#include "types/typesconv.h"
#include "types/symnames.h"

#include "rw/xmlwriter.h"
#include "rw/xmlreader.h"

#include "articulation.h"
#include "tremolo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
Drumset* smDrumset = nullptr;           // standard midi drumset

String Drumset::translatedName(int pitch) const
{
    return mtrc("engraving/drumset", name(pitch));
}

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(XmlWriter& xml) const
{
    for (int i = 0; i < 128; ++i) {
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
        _drum[pitch].notehead = TConv::fromXml(e.readAsciiText(), NoteHeadGroup::HEAD_NORMAL);
    } else if (tag == "noteheads") {
        _drum[pitch].notehead = NoteHeadGroup::HEAD_CUSTOM;
        while (e.readNextStartElement()) {
            const AsciiStringView nhTag(e.name());
            int noteType = int(TConv::fromXml(nhTag, NoteHeadType::HEAD_AUTO));
            if (noteType > int(NoteHeadType::HEAD_TYPES) - 1 || noteType < 0) {
                return false;
            }

            _drum[pitch].noteheads[noteType] = SymNames::symIdByName(e.readAsciiText());
        }
    } else if (tag == "line") {
        _drum[pitch].line = e.readInt();
    } else if (tag == "voice") {
        _drum[pitch].voice = e.readInt();
    } else if (tag == "name") {
        _drum[pitch].name = e.readText();
    } else if (tag == "stem") {
        _drum[pitch].stemDirection = DirectionV(e.readInt());
    } else if (tag == "shortcut") {
        bool isNum;
        AsciiStringView val = e.readAsciiText();
        int i = val.toInt(&isNum);
        _drum[pitch].shortcut = isNum ? i : val.at(0).toUpper();
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
                _drum[pitch].addVariant(div);
            }
        }
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
    if (pitch < 0 || pitch > 127) {
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
    for (int i = 0; i < 128; ++i) {
        _drum[i].name = u"";
        _drum[i].notehead = NoteHeadGroup::HEAD_INVALID;
        _drum[i].shortcut = 0;
        _drum[i].variants.clear();
    }
}

//---------------------------------------------------------
//   nextPitch
//---------------------------------------------------------

int Drumset::nextPitch(int ii) const
{
    for (int i = ii + 1; i < 127; ++i) {
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
    for (int i = 127; i >= ii; --i) {
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

DrumInstrumentVariant Drumset::findVariant(int p, const std::vector<Articulation*> articulations, Tremolo* tremolo) const
{
    DrumInstrumentVariant div;
    auto vs = variants(p);
    for (const auto& v : vs) {
        bool matchTremolo = (!tremolo && v.tremolo == TremoloType::INVALID_TREMOLO) || (tremolo && v.tremolo == tremolo->tremoloType());
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

void initDrumset()
{
    smDrumset = new Drumset;
    for (int i = 0; i < 128; ++i) {
        smDrumset->drum(i).notehead = NoteHeadGroup::HEAD_INVALID;
        smDrumset->drum(i).line     = 0;
        smDrumset->drum(i).shortcut = 0;
        smDrumset->drum(i).voice    = 0;
        smDrumset->drum(i).stemDirection = DirectionV::UP;
    }
    smDrumset->drum(35) = DrumInstrument(TConv::userName(DrumNum(35)), NoteHeadGroup::HEAD_NORMAL,   7, DirectionV::DOWN, 1);
    smDrumset->drum(36) = DrumInstrument(TConv::userName(DrumNum(36)), NoteHeadGroup::HEAD_NORMAL,   7, DirectionV::DOWN, 1, Key_B);
    smDrumset->drum(37) = DrumInstrument(TConv::userName(DrumNum(37)), NoteHeadGroup::HEAD_CROSS,    3, DirectionV::UP);
    smDrumset->drum(38) = DrumInstrument(TConv::userName(DrumNum(38)), NoteHeadGroup::HEAD_NORMAL,   3, DirectionV::UP, 0, Key_A);

    smDrumset->drum(40) = DrumInstrument(TConv::userName(DrumNum(40)), NoteHeadGroup::HEAD_NORMAL,   3, DirectionV::UP);
    smDrumset->drum(41) = DrumInstrument(TConv::userName(DrumNum(41)), NoteHeadGroup::HEAD_NORMAL,   5, DirectionV::UP);
    smDrumset->drum(42) = DrumInstrument(TConv::userName(DrumNum(42)), NoteHeadGroup::HEAD_CROSS,   -1, DirectionV::UP, 0, Key_G);
    smDrumset->drum(43) = DrumInstrument(TConv::userName(DrumNum(43)), NoteHeadGroup::HEAD_NORMAL,   5, DirectionV::DOWN, 1);
    smDrumset->drum(44) = DrumInstrument(TConv::userName(DrumNum(44)), NoteHeadGroup::HEAD_CROSS,    9, DirectionV::DOWN, 1, Key_F);
    smDrumset->drum(45) = DrumInstrument(TConv::userName(DrumNum(45)), NoteHeadGroup::HEAD_NORMAL,   2, DirectionV::UP);
    smDrumset->drum(46) = DrumInstrument(TConv::userName(DrumNum(46)), NoteHeadGroup::HEAD_CROSS,    1, DirectionV::UP);
    smDrumset->drum(47) = DrumInstrument(TConv::userName(DrumNum(47)), NoteHeadGroup::HEAD_NORMAL,   1, DirectionV::UP);
    smDrumset->drum(48) = DrumInstrument(TConv::userName(DrumNum(48)), NoteHeadGroup::HEAD_NORMAL,   0, DirectionV::UP);
    smDrumset->drum(49) = DrumInstrument(TConv::userName(DrumNum(49)), NoteHeadGroup::HEAD_CROSS,   -2, DirectionV::UP, 0, Key_C);

    smDrumset->drum(50) = DrumInstrument(TConv::userName(DrumNum(50)), NoteHeadGroup::HEAD_NORMAL,   0, DirectionV::UP, 0, Key_E);
    smDrumset->drum(51) = DrumInstrument(TConv::userName(DrumNum(51)), NoteHeadGroup::HEAD_CROSS,    0, DirectionV::UP, 0, Key_D);
    smDrumset->drum(52) = DrumInstrument(TConv::userName(DrumNum(52)), NoteHeadGroup::HEAD_CROSS,   -3, DirectionV::UP);
    smDrumset->drum(53) = DrumInstrument(TConv::userName(DrumNum(53)), NoteHeadGroup::HEAD_DIAMOND,  0, DirectionV::UP);
    smDrumset->drum(54) = DrumInstrument(TConv::userName(DrumNum(54)), NoteHeadGroup::HEAD_DIAMOND,  2, DirectionV::UP);
    smDrumset->drum(55) = DrumInstrument(TConv::userName(DrumNum(55)), NoteHeadGroup::HEAD_CROSS,   -3, DirectionV::UP);
    smDrumset->drum(56) = DrumInstrument(TConv::userName(DrumNum(56)), NoteHeadGroup::HEAD_TRIANGLE_DOWN, 1, DirectionV::UP);
    smDrumset->drum(57) = DrumInstrument(TConv::userName(DrumNum(57)), NoteHeadGroup::HEAD_CROSS,   -3, DirectionV::UP);

    smDrumset->drum(59) = DrumInstrument(TConv::userName(DrumNum(59)), NoteHeadGroup::HEAD_CROSS,    2, DirectionV::UP);

    smDrumset->drum(63) = DrumInstrument(TConv::userName(DrumNum(63)), NoteHeadGroup::HEAD_CROSS,    4, DirectionV::UP);
    smDrumset->drum(64) = DrumInstrument(TConv::userName(DrumNum(64)), NoteHeadGroup::HEAD_CROSS,    6, DirectionV::UP);
}
}
