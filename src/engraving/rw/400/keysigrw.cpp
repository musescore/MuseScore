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
#include "keysigrw.h"

#include "../../types/symnames.h"

#include "../../libmscore/keysig.h"
#include "../../libmscore/score.h"

#include "../xmlreader.h"

#include "engravingitemrw.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

//---------------------------------------------------------
//    for import of 1.3 scores
//---------------------------------------------------------

static SymId convertFromOldId(int val)
{
    SymId symId = SymId::noSym;
    switch (val) {
    case 32: symId = SymId::accidentalSharp;
        break;
    case 33: symId = SymId::accidentalThreeQuarterTonesSharpArrowUp;
        break;
    case 34: symId = SymId::accidentalQuarterToneSharpArrowDown;
        break;
    // case 35: // "sharp arrow both" missing in SMuFL
    case 36: symId = SymId::accidentalQuarterToneSharpStein;
        break;
    case 37: symId = SymId::accidentalBuyukMucennebSharp;
        break;
    case 38: symId = SymId::accidentalKomaSharp;
        break;
    case 39: symId = SymId::accidentalThreeQuarterTonesSharpStein;
        break;
    case 40: symId = SymId::accidentalNatural;
        break;
    case 41: symId = SymId::accidentalQuarterToneSharpNaturalArrowUp;
        break;
    case 42: symId = SymId::accidentalQuarterToneFlatNaturalArrowDown;
        break;
    // case 43: // "natural arrow both" missing in SMuFL
    case 44: symId = SymId::accidentalFlat;
        break;
    case 45: symId = SymId::accidentalQuarterToneFlatArrowUp;
        break;
    case 46: symId = SymId::accidentalThreeQuarterTonesFlatArrowDown;
        break;
    // case 47: // "flat arrow both" missing in SMuFL
    case 48: symId = SymId::accidentalBakiyeFlat;
        break;
    case 49: symId = SymId::accidentalBuyukMucennebFlat;
        break;
    case 50: symId = SymId::accidentalThreeQuarterTonesFlatZimmermann;
        break;
    case 51: symId = SymId::accidentalQuarterToneFlatStein;
        break;
    // case 52: // "mirrored flat slash" missing in SMuFL
    case 53: symId = SymId::accidentalDoubleFlat;
        break;
    // case 54: // "flat flat slash" missing in SMuFL
    case 55: symId = SymId::accidentalDoubleSharp;
        break;
    case 56: symId = SymId::accidentalSori;
        break;
    case 57: symId = SymId::accidentalKoron;
        break;
    default:
        LOGD("MuseScore 1.3 symbol id corresponding to <%d> not found", val);
        symId = SymId::noSym;
        break;
    }
    return symId;
}

void KeySigRW::read(KeySig* s, XmlReader& e, ReadContext& ctx)
{
    KeySigEvent sig;
    int subtype = 0;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "CustDef" || tag == "KeySym") {
            CustDef cd;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "sym") {
                    AsciiStringView val(e.readAsciiText());
                    bool valid;
                    SymId id = SymId(val.toInt(&valid));
                    if (!valid) {
                        id = SymNames::symIdByName(val);
                    }
                    if (s->score()->mscVersion() <= 114) {
                        if (valid) {
                            id = convertFromOldId(val.toInt(&valid));
                        }
                        if (!valid) {
                            id = SymNames::symIdByOldName(val);
                        }
                    }
                    cd.sym = id;
                } else if (t == "def") {
                    cd.degree = e.intAttribute("degree", 0);
                    cd.octAlt = e.intAttribute("octAlt", 0);
                    cd.xAlt = e.doubleAttribute("xAlt", 0.0);
                    e.readNext();
                } else if (t == "pos") { // for older files
                    double prevx = 0;
                    double accidentalGap = s->score()->styleS(Sid::keysigAccidentalDistance).val();
                    double _spatium = s->spatium();
                    // count default x position
                    for (CustDef& cd : sig.customKeyDefs()) {
                        prevx += s->symWidth(cd.sym) / _spatium + accidentalGap + cd.xAlt;
                    }
                    bool flat = std::string(SymNames::nameForSymId(cd.sym).ascii()).find("Flat") != std::string::npos;
                    // if x not there, use default step
                    cd.xAlt = e.doubleAttribute("x", prevx) - prevx;
                    // if y not there, use middle line
                    int line = static_cast<int>(e.doubleAttribute("y", 2) * 2);
                    cd.degree = (3 - line) % 7;
                    cd.degree += (cd.degree < 0) ? 7 : 0;
                    line += flat ? -1 : 1; // top point in treble for # is -1 (gis), and for b is 1 (es)
                    cd.octAlt = static_cast<int>((line - (line >= 0 ? 0 : 6)) / 7);
                    e.readNext();
                } else {
                    e.unknown();
                }
            }
            sig.customKeyDefs().push_back(cd);
        } else if (tag == "showCourtesySig") {
            s->setShowCourtesy(e.readInt());
        } else if (tag == "showNaturals") {           // obsolete
            e.readInt();
        } else if (tag == "accidental") {
            sig.setKey(Key(e.readInt()));
        } else if (tag == "natural") {                // obsolete
            e.readInt();
        } else if (tag == "custom") {
            e.readInt();
            sig.setCustom(true);
        } else if (tag == "mode") {
            String m(e.readText());
            if (m == "none") {
                sig.setMode(KeyMode::NONE);
            } else if (m == "major") {
                sig.setMode(KeyMode::MAJOR);
            } else if (m == "minor") {
                sig.setMode(KeyMode::MINOR);
            } else if (m == "dorian") {
                sig.setMode(KeyMode::DORIAN);
            } else if (m == "phrygian") {
                sig.setMode(KeyMode::PHRYGIAN);
            } else if (m == "lydian") {
                sig.setMode(KeyMode::LYDIAN);
            } else if (m == "mixolydian") {
                sig.setMode(KeyMode::MIXOLYDIAN);
            } else if (m == "aeolian") {
                sig.setMode(KeyMode::AEOLIAN);
            } else if (m == "ionian") {
                sig.setMode(KeyMode::IONIAN);
            } else if (m == "locrian") {
                sig.setMode(KeyMode::LOCRIAN);
            } else {
                sig.setMode(KeyMode::UNKNOWN);
            }
        } else if (tag == "subtype") {
            subtype = e.readInt();
        } else if (tag == "forInstrumentChange") {
            s->setForInstrumentChange(e.readBool());
        } else if (!EngravingItemRW::readProperties(s, e, ctx)) {
            e.unknown();
        }
    }
    // for backward compatibility
    if (!sig.isValid()) {
        sig.initFromSubtype(subtype);
    }
    if (sig.custom() && sig.customKeyDefs().empty()) {
        sig.setMode(KeyMode::NONE);
    }

    s->setKeySigEvent(sig);
}
