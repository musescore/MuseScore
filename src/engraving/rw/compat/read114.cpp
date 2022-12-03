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

#include "read114.h"

#include <cmath>

#include "compat/pageformat.h"

#include "infrastructure/htmlparser.h"

#include "rw/xml.h"
#include "rw/compat/compatutils.h"

#include "style/defaultstyle.h"
#include "style/style.h"

#include "types/typesconv.h"

#include "libmscore/accidental.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/box.h"
#include "libmscore/bracketItem.h"
#include "libmscore/breath.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/dynamic.h"
#include "libmscore/excerpt.h"
#include "libmscore/factory.h"
#include "libmscore/fingering.h"
#include "libmscore/harmony.h"
#include "libmscore/image.h"
#include "libmscore/jump.h"
#include "libmscore/keysig.h"
#include "libmscore/linkedobjects.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/mmrest.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/rest.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/spacer.h"
#include "libmscore/staff.h"
#include "libmscore/stafftext.h"
#include "libmscore/stafftype.h"
#include "libmscore/stringdata.h"
#include "libmscore/tempo.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/textframe.h"
#include "libmscore/textline.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"
#include "libmscore/utils.h"
#include "libmscore/volta.h"

#include "readchordlisthook.h"
#include "readstyle.h"
#include "read206.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rw;
using namespace mu::engraving::compat;

static int g_guitarStrings[] = { 40, 45, 50, 55, 59, 64 };
static int g_bassStrings[]   = { 28, 33, 38, 43 };
static int g_violinStrings[] = { 55, 62, 69, 76 };
static int g_violaStrings[]  = { 48, 55, 62, 69 };
static int g_celloStrings[]  = { 36, 43, 50, 57 };

#define MM(x) ((x) / INCH)

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
    const char* name;
    double w, h;              // size in inch
    PaperSize(const char* n, double wi, double hi)
        : name(n), w(wi), h(hi) {}
};

static const PaperSize paperSizes114[] = {
    PaperSize("Custom",    MM(1),    MM(1)),
    PaperSize("A4",        MM(210),  MM(297)),
    PaperSize("B5",        MM(176),  MM(250)),
    PaperSize("Letter",    8.5,      11),
    PaperSize("Legal",     8.5,      14),
    PaperSize("Executive", 7.5,      10),
    PaperSize("A0",        MM(841),  MM(1189)),
    PaperSize("A1",        MM(594),  MM(841)),
    PaperSize("A2",        MM(420),  MM(594)),
    PaperSize("A3",        MM(297),  MM(420)),
    PaperSize("A5",        MM(148),  MM(210)),
    PaperSize("A6",        MM(105),  MM(148)),
    PaperSize("A7",        MM(74),   MM(105)),
    PaperSize("A8",        MM(52),   MM(74)),
    PaperSize("A9",        MM(37),   MM(52)),
    PaperSize("A10",       MM(26),   MM(37)),
    PaperSize("B0",        MM(1000), MM(1414)),
    PaperSize("B1",        MM(707),  MM(1000)),
    PaperSize("B2",        MM(500),  MM(707)),
    PaperSize("B3",        MM(353),  MM(500)),
    PaperSize("B4",        MM(250),  MM(353)),
    PaperSize("B6",        MM(125),  MM(176)),
    PaperSize("B7",        MM(88),   MM(125)),
    PaperSize("B8",        MM(62),   MM(88)),
    PaperSize("B9",        MM(44),   MM(62)),
    PaperSize("B10",       MM(31),   MM(44)),
    PaperSize("Comm10E",   MM(105),  MM(241)),
    PaperSize("DLE",       MM(110),  MM(220)),
    PaperSize("Folio",     MM(210),  MM(330)),
    PaperSize("Ledger",    MM(432),  MM(279)),
    PaperSize("Tabloid",   MM(279),  MM(432)),
    PaperSize(0,           MM(1),    MM(1))     // mark end of list
};

//---------------------------------------------------------
//   getPaperSize
//---------------------------------------------------------

static const PaperSize* getPaperSize114(const String& name)
{
    for (int i = 0; paperSizes114[i].name; ++i) {
        if (name == paperSizes114[i].name) {
            return &paperSizes114[i];
        }
    }
    LOGD("unknown paper size");
    return &paperSizes114[0];
}

//---------------------------------------------------------
//   convertFromHtml
//---------------------------------------------------------

static String convertFromHtml(const String& in_html)
{
    if (in_html.isEmpty()) {
        return in_html;
    }

    std::string html = in_html.toStdString();

    //! NOTE Get body
    auto body_b = html.find("<body");
    auto body_e = html.find_last_of("</body>");
    if (body_b == std::string::npos || body_e == std::string::npos) {
        return in_html;
    }
    std::string body = html.substr(body_b, body_e - body_b);

    std::vector<std::string> blocks;

    //! NOTE Split blocks
    std::string::size_type p_b = 0;
    std::string::size_type p_e = 0;
    while (true) {
        p_b = body.find("<p", p_e);
        p_e = body.find("/p>", p_b);
        if (p_b == std::string::npos || p_e == std::string::npos) {
            break;
        }

        std::string block = body.substr(p_b, p_e - p_b);
        blocks.push_back(block);
    }

    if (blocks.empty()) {
        blocks.push_back(body);
    }

    //! NOTE Format blocks
    auto extractText = [](const std::string& block) {
        std::string text;
        bool isTag = false;
        for (const char& c : block) {
            if (c == '<') {
                isTag = true;
            } else if (c == '>') {
                isTag = false;
            } else if (!isTag) {
                text += c;
            }
        }
        return text;
    };

    auto extractFont = [](const std::string& block) {
        auto fontsize_b = block.find("font-size");
        if (fontsize_b != std::string::npos) {
            std::string fontSize;
            bool started = false;
            for (auto i = fontsize_b; i < block.size(); ++i) {
                const char& c = block.at(i);
                if (strchr(".0123456789", c) != nullptr) {
                    started = true;
                    fontSize += c;
                } else if (started) {
                    break;
                }
            }

            return std::string("<font size=\"") + fontSize + std::string("\"/>");
        }
        return std::string();
    };

    auto formatRichText = [extractText, extractFont](const std::string& block) {
        std::string text = extractText(block);
        std::string font = extractFont(block);
        if (!font.empty()) {
            text = font + text;
        }
        return text;
    };

    //! NOTE Format rich text from blocks
    std::string text;
    for (const std::string& block : blocks) {
        if (!text.empty()) {
            text += "\n";
        }

        text += formatRichText(block);
    }

    auto replaceSym = [](std::string& str, int cc, const char* sym) {
        std::string code;
        code.resize(3);
        code[2] = static_cast<char>(cc);
        code[1] = static_cast<char>(cc >> 8);
        code[0] = static_cast<char>(cc >> 16);

        auto pos = str.find(code);
        if (pos != std::string::npos) {
            str.replace(pos, 3, sym);
        }
    };

    //! NOTE replace utf8 code /*utf16 code*/ on sym
    replaceSym(text, 0xee848e /*0xe10e*/, "<sym>accidentalNatural</sym>");        //natural
    replaceSym(text, 0xee848c /*0xe10c*/, "<sym>accidentalSharp</sym>");          // sharp
    replaceSym(text, 0xee848d /*0xe10d*/, "<sym>accidentalFlat</sym>");           // flat
    replaceSym(text, 0xee8484 /*0xe104*/, "<sym>metNoteHalfUp</sym>");            // note2_Sym
    replaceSym(text, 0xee8485 /*0xe105*/, "<sym>metNoteQuarterUp</sym>");         // note4_Sym
    replaceSym(text, 0xee8486 /*0xe106*/, "<sym>metNote8thUp</sym>");             // note8_Sym
    replaceSym(text, 0xee8487 /*0xe107*/, "<sym>metNote16thUp</sym>");            // note16_Sym
    replaceSym(text, 0xee8488 /*0xe108*/, "<sym>metNote32ndUp</sym>");            // note32_Sym
    replaceSym(text, 0xee8489 /*0xe109*/, "<sym>metNote64thUp</sym>");            // note64_Sym
    replaceSym(text, 0xee848a /*0xe10a*/, "<sym>metAugmentationDot</sym>");       // dot
    replaceSym(text, 0xee848b /*0xe10b*/, "<sym>metAugmentationDot</sym><sym>space</sym><sym>metAugmentationDot</sym>");          // dotdot
    replaceSym(text, 0xee85a7 /*0xe167*/, "<sym>segno</sym>");                    // segno
    replaceSym(text, 0xee85a8 /*0xe168*/, "<sym>coda</sym>");                     // coda
    replaceSym(text, 0xee85a9 /*0xe169*/, "<sym>codaSquare</sym>");               // varcoda

    return String::fromStdString(text);
}

//---------------------------------------------------------
//   readTextProperties
//---------------------------------------------------------

static bool readTextProperties(XmlReader& e, TextBase* t, EngravingItem*)
{
    const AsciiStringView tag(e.name());
    if (tag == "style") {
        int i = e.readInt();
        TextStyleType ss = TextStyleType::DEFAULT;
        switch (i) {
        case 2:  ss = TextStyleType::TITLE;
            break;
        case 3:  ss = TextStyleType::SUBTITLE;
            break;
        case 4:  ss = TextStyleType::COMPOSER;
            break;
        case 5:  ss = TextStyleType::POET;
            break;

        case 6:  ss = TextStyleType::LYRICS_ODD;
            break;
        case 7:  ss = TextStyleType::LYRICS_EVEN;
            break;

        case 8:  ss = TextStyleType::FINGERING;
            break;
        case 9:  ss = TextStyleType::INSTRUMENT_LONG;
            break;
        case 10: ss = TextStyleType::INSTRUMENT_SHORT;
            break;
        case 11: ss = TextStyleType::INSTRUMENT_EXCERPT;
            break;

        case 12: ss = TextStyleType::DYNAMICS;
            break;
        case 13: ss = TextStyleType::EXPRESSION;
            break;
        case 14: ss = TextStyleType::TEMPO;
            break;
        case 15: ss = TextStyleType::METRONOME;
            break;
        case 16: ss = TextStyleType::FOOTER;
            break;                                      // TextStyleType::COPYRIGHT
        case 17: ss = TextStyleType::MEASURE_NUMBER;
            break;
        case 18: ss = TextStyleType::FOOTER;
            break;                                     // TextStyleType::PAGE_NUMBER_ODD
        case 19: ss = TextStyleType::FOOTER;
            break;                                     // TextStyleType::PAGE_NUMBER_EVEN
        case 20: ss = TextStyleType::TRANSLATOR;
            break;
        case 21: ss = TextStyleType::TUPLET;
            break;

        case 22: ss = TextStyleType::SYSTEM;
            break;
        case 23: ss = TextStyleType::STAFF;
            break;
        case 24: ss = TextStyleType::HARMONY_A;
            break;
        case 25: ss = TextStyleType::REHEARSAL_MARK;
            break;
        case 26: ss = TextStyleType::REPEAT_LEFT;
            break;
        case 27: ss = TextStyleType::VOLTA;
            break;
        case 28: ss = TextStyleType::FRAME;
            break;
        case 29: ss = TextStyleType::TEXTLINE;
            break;
        case 30: ss = TextStyleType::GLISSANDO;
            break;
        case 31: ss = TextStyleType::STRING_NUMBER;
            break;

        case 32: ss = TextStyleType::OTTAVA;
            break;
//??                  case 33: ss = TextStyleName::BENCH;   break;
        case 34: ss = TextStyleType::HEADER;
            break;
        case 35: ss = TextStyleType::FOOTER;
            break;
        case 0:
        default:
            LOGD("style %d invalid", i);
            ss = TextStyleType::DEFAULT;
            break;
        }
        t->initTextStyleType(ss);
    } else if (tag == "subtype") {
        e.skipCurrentElement();
    } else if (tag == "html-data") {
        String ss = e.readXml();
        String s  = convertFromHtml(ss);
// LOGD("html-data <%s>", muPrintable(s));
        t->setXmlText(s);
    } else if (tag == "foregroundColor") { // same as "color" ?
        e.skipCurrentElement();
    } else if (tag == "frame") {
        t->setFrameType(e.readBool() ? FrameType::SQUARE : FrameType::NO_FRAME);
        t->setPropertyFlags(Pid::FRAME_TYPE, PropertyFlags::UNSTYLED);
    } else if (tag == "halign") {
        Align align = t->align();
        align.horizontal = TConv::fromXml(e.readAsciiText(), AlignH::LEFT);
        t->setAlign(align);
        t->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
    } else if (tag == "valign") {
        Align align = t->align();
        align.vertical = TConv::fromXml(e.readAsciiText(), AlignV::TOP);
        t->setAlign(align);
        t->setPropertyFlags(Pid::ALIGN, PropertyFlags::UNSTYLED);
    } else if (tag == "rxoffset") {
        e.readText();
    } else if (tag == "ryoffset") {
        e.readText();
    } else if (tag == "yoffset") {
        e.readText();
    } else if (tag == "systemFlag") {
        e.readText();
    } else if (!t->readProperties(e)) {
        return false;
    }
    t->setOffset(PointF());       // ignore user offsets
    t->setAutoplace(true);
    return true;
}

//---------------------------------------------------------
//   readText114
//---------------------------------------------------------

static void readText114(XmlReader& e, TextBase* t, EngravingItem* be)
{
    while (e.readNextStartElement()) {
        if (!readTextProperties(e, t, be)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readAccidental
//---------------------------------------------------------

static void readAccidental(Accidental* a, XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "bracket") {
            int i = e.readInt();
            if (i == 0 || i == 1) {
                a->setBracket(AccidentalBracket(i));
            }
        } else if (tag == "subtype") {
            String text(e.readText());
            bool isInt;
            int i = text.toInt(&isInt);
            if (isInt) {
                a->setBracket(i & 0x8000 ? AccidentalBracket::PARENTHESIS : AccidentalBracket::BRACKET);
                i &= ~0x8000;
                AccidentalType at;
                switch (i) {
                case 0:
                    at = AccidentalType::NONE;
                    break;
                case 6:
                    a->setBracket(AccidentalBracket::PARENTHESIS);
                // fall through
                case 1:
                case 11:
                    at = AccidentalType::SHARP;
                    break;
                case 7:
                    a->setBracket(AccidentalBracket::PARENTHESIS);
                // fall through
                case 2:
                case 12:
                    at = AccidentalType::FLAT;
                    break;
                case 8:
                    a->setBracket(AccidentalBracket::PARENTHESIS);
                // fall through
                case 3:
                case 13:
                    at = AccidentalType::SHARP2;
                    break;
                case 9:
                    a->setBracket(AccidentalBracket::PARENTHESIS);
                // fall through
                case 4:
                case 14:
                    at = AccidentalType::FLAT2;
                    break;
                case 10:
                    a->setBracket(AccidentalBracket::PARENTHESIS);
                // fall through
                case 5:
                case 15:
                    at = AccidentalType::NATURAL;
                    break;
                case 16:
                    at = AccidentalType::FLAT_SLASH;
                    break;
                case 17:
                    at = AccidentalType::FLAT_SLASH2;
                    break;
                case 18:
                    at = AccidentalType::MIRRORED_FLAT2;
                    break;
                case 19:
                    at = AccidentalType::MIRRORED_FLAT;
                    break;
                case 20:
                    at = AccidentalType::NONE;                //AccidentalType::MIRRORED_FLAT_SLASH;
                    break;
                case 21:
                    at = AccidentalType::NONE;                //AccidentalType::FLAT_FLAT_SLASH;
                    break;
                case 22:
                    at = AccidentalType::SHARP_SLASH;
                    break;
                case 23:
                    at = AccidentalType::SHARP_SLASH2;
                    break;
                case 24:
                    at = AccidentalType::SHARP_SLASH3;
                    break;
                case 25:
                    at = AccidentalType::SHARP_SLASH4;
                    break;
                case 26:
                    at = AccidentalType::SHARP_ARROW_UP;
                    break;
                case 27:
                    at = AccidentalType::SHARP_ARROW_DOWN;
                    break;
                case 28:
                    at = AccidentalType::NONE;                //AccidentalType::SHARP_ARROW_BOTH;
                    break;
                case 29:
                    at = AccidentalType::FLAT_ARROW_UP;
                    break;
                case 30:
                    at = AccidentalType::FLAT_ARROW_DOWN;
                    break;
                case 31:
                    at = AccidentalType::NONE;                //AccidentalType::FLAT_ARROW_BOTH;
                    break;
                case 32:
                    at = AccidentalType::NATURAL_ARROW_UP;
                    break;
                case 33:
                    at = AccidentalType::NATURAL_ARROW_DOWN;
                    break;
                case 34:
                    at = AccidentalType::NONE;                //AccidentalType::NATURAL_ARROW_BOTH;
                    break;
                default:
                    at = AccidentalType::NONE;
                    break;
                }
                a->setAccidentalType(at);
            } else {
                const static std::map<String, AccidentalType> accMap = {
                    { u"none", AccidentalType::NONE }, { u"sharp", AccidentalType::SHARP },
                    { u"flat", AccidentalType::FLAT }, { u"natural", AccidentalType::NATURAL },
                    { u"double sharp", AccidentalType::SHARP2 }, { u"double flat", AccidentalType::FLAT2 },
                    { u"flat-slash", AccidentalType::FLAT_SLASH }, { u"flat-slash2", AccidentalType::FLAT_SLASH2 },
                    { u"mirrored-flat2", AccidentalType::MIRRORED_FLAT2 }, { u"mirrored-flat", AccidentalType::MIRRORED_FLAT },
                    { u"sharp-slash", AccidentalType::SHARP_SLASH }, { u"sharp-slash2", AccidentalType::SHARP_SLASH2 },
                    { u"sharp-slash3", AccidentalType::SHARP_SLASH3 }, { u"sharp-slash4", AccidentalType::SHARP_SLASH4 },
                    { u"sharp arrow up", AccidentalType::SHARP_ARROW_UP }, { u"sharp arrow down", AccidentalType::SHARP_ARROW_DOWN },
                    { u"flat arrow up", AccidentalType::FLAT_ARROW_UP }, { u"flat arrow down", AccidentalType::FLAT_ARROW_DOWN },
                    { u"natural arrow up", AccidentalType::NATURAL_ARROW_UP },
                    { u"natural arrow down", AccidentalType::NATURAL_ARROW_DOWN },
                    { u"sori", AccidentalType::SORI }, { u"koron", AccidentalType::KORON }
                };
                auto it = accMap.find(text);
                if (it == accMap.end()) {
                    LOGD("invalid type %s", muPrintable(text));
                    a->setAccidentalType(AccidentalType::NONE);
                } else {
                    a->setAccidentalType(it->second);
                }
            }
        } else if (tag == "role") {
            AccidentalRole r = AccidentalRole(e.readInt());
            if (r == AccidentalRole::AUTO || r == AccidentalRole::USER) {
                a->setRole(r);
            }
        } else if (tag == "small") {
            a->setSmall(e.readInt());
        } else if (tag == "offset") {
            e.skipCurrentElement();       // ignore manual layout in older scores
        } else if (a->EngravingItem::readProperties(e)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readFingering114
//---------------------------------------------------------

static void readFingering114(XmlReader& e, Fingering* fing)
{
    bool isStringNumber = false;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "html-data") {
            auto htmlDdata = HtmlParser::parse(e.readXml());
            htmlDdata.replace(u" ", u"");
            fing->setPlainText(htmlDdata);
        } else if (tag == "subtype") {
            auto subtype = e.readText();
            if (subtype == "StringNumber") {
                isStringNumber = true;
                fing->setProperty(Pid::TEXT_STYLE, TextStyleType::STRING_NUMBER);
                fing->setPropertyFlags(Pid::TEXT_STYLE, PropertyFlags::UNSTYLED);
            }
        } else if (tag == "frame") {
            auto frame = e.readInt();
            if (frame) {
                if (isStringNumber) {       //default value is circle for stringnumber, square is set in tag circle
                    fing->setFrameType(FrameType::CIRCLE);
                } else {     //default value is square for stringnumber, circle is set in tag circle
                    fing->setFrameType(FrameType::SQUARE);
                }
            } else {
                fing->setFrameType(FrameType::NO_FRAME);
            }
        } else if (tag == "circle") {
            auto circle = e.readInt();
            if (circle) {
                fing->setFrameType(FrameType::CIRCLE);
            } else {
                fing->setFrameType(FrameType::SQUARE);
            }
        } else {
            e.skipCurrentElement();
        }
    }
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

static void readNote(Note* note, XmlReader& e, ReadContext& ctx)
{
    ctx.hasAccidental = false;                       // used for userAccidental backward compatibility

    note->setTpc1(Tpc::TPC_INVALID);
    note->setTpc2(Tpc::TPC_INVALID);

    if (e.hasAttribute("pitch")) {                   // obsolete
        note->setPitch(e.intAttribute("pitch"));
    }
    if (e.hasAttribute("tpc")) {                     // obsolete
        note->setTpc1(e.intAttribute("tpc"));
    }

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Accidental") {
            // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
            // if a userAccidental has some other property set (like for instance offset)
            Accidental* a;
            if (ctx.hasAccidental) {                // if the other tag has already been read,
                a = note->accidental();                // re-use the accidental it constructed
            } else {
                a = Factory::createAccidental(note);
            }
            // the accidental needs to know the properties of the
            // track it belongs to (??)
            a->setTrack(note->track());
            readAccidental(a, e);
            if (!ctx.hasAccidental) {              // only the new accidental, if it has been added previously
                note->add(a);
            }
            ctx.hasAccidental = true;         // we now have an accidental
        } else if (tag == "Text") {
            Fingering* f = Factory::createFingering(note);
            readFingering114(e, f);
            note->add(f);
        } else if (tag == "onTimeType") {
            if (e.readText() == "offset") {
                note->setOnTimeType(2);
            } else {
                note->setOnTimeType(1);
            }
        } else if (tag == "offTimeType") {
            if (e.readText() == "offset") {
                note->setOffTimeType(2);
            } else {
                note->setOffTimeType(1);
            }
        } else if (tag == "onTimeOffset") {
            if (note->onTimeType() == 1) {
                note->setOnTimeOffset(e.readInt() * 1000 / note->chord()->actualTicks().ticks());
            } else {
                note->setOnTimeOffset(e.readInt() * 10);
            }
        } else if (tag == "offTimeOffset") {
            if (note->offTimeType() == 1) {
                note->setOffTimeOffset(1000 + (e.readInt() * 1000 / note->chord()->actualTicks().ticks()));
            } else {
                note->setOffTimeOffset(1000 + (e.readInt() * 10));
            }
        } else if (tag == "userAccidental") {
            String val(e.readText());
            bool ok;
            int k = val.toInt(&ok);
            if (ok) {
                // on older scores, a note could have both a <userAccidental> tag and an <Accidental> tag
                // if a userAccidental has some other property set (like for instance offset)
                // only construct a new accidental, if the other tag has not been read yet
                // (<userAccidental> tag is only used in older scores: no need to check the score mscVersion)
                if (!ctx.hasAccidental) {
                    Accidental* a = Factory::createAccidental(note);
                    note->add(a);
                }
                // TODO: for backward compatibility
                bool bracket = k & 0x8000;
                k &= 0xfff;
                AccidentalType at = AccidentalType::NONE;
                switch (k) {
                case 0: at = AccidentalType::NONE;
                    break;
                case 1: at = AccidentalType::SHARP;
                    break;
                case 2: at = AccidentalType::FLAT;
                    break;
                case 3: at = AccidentalType::SHARP2;
                    break;
                case 4: at = AccidentalType::FLAT2;
                    break;
                case 5: at = AccidentalType::NATURAL;
                    break;

                case 6: at = AccidentalType::FLAT_SLASH;
                    break;
                case 7: at = AccidentalType::FLAT_SLASH2;
                    break;
                case 8: at = AccidentalType::MIRRORED_FLAT2;
                    break;
                case 9: at = AccidentalType::MIRRORED_FLAT;
                    break;
                case 10: at = AccidentalType::NONE;
                    break;                                               // AccidentalType::MIRRORED_FLAT_SLASH
                case 11: at = AccidentalType::NONE;
                    break;                                               // AccidentalType::FLAT_FLAT_SLASH

                case 12: at = AccidentalType::SHARP_SLASH;
                    break;
                case 13: at = AccidentalType::SHARP_SLASH2;
                    break;
                case 14: at = AccidentalType::SHARP_SLASH3;
                    break;
                case 15: at = AccidentalType::SHARP_SLASH4;
                    break;

                case 16: at = AccidentalType::SHARP_ARROW_UP;
                    break;
                case 17: at = AccidentalType::SHARP_ARROW_DOWN;
                    break;
                case 18: at = AccidentalType::NONE;
                    break;                                               // AccidentalType::SHARP_ARROW_BOTH
                case 19: at = AccidentalType::FLAT_ARROW_UP;
                    break;
                case 20: at = AccidentalType::FLAT_ARROW_DOWN;
                    break;
                case 21: at = AccidentalType::NONE;
                    break;                                               // AccidentalType::FLAT_ARROW_BOTH
                case 22: at = AccidentalType::NATURAL_ARROW_UP;
                    break;
                case 23: at = AccidentalType::NATURAL_ARROW_DOWN;
                    break;
                case 24: at = AccidentalType::NONE;
                    break;                                               // AccidentalType::NATURAL_ARROW_BOTH
                case 25: at = AccidentalType::SORI;
                    break;
                case 26: at = AccidentalType::KORON;
                    break;
                }
                note->accidental()->setAccidentalType(at);

                note->accidental()->setBracket(AccidentalBracket(bracket));

                note->accidental()->setRole(AccidentalRole::USER);
                ctx.hasAccidental = true;           // we now have an accidental
            }
        } else if (tag == "offset") {
            e.skipCurrentElement();       // ignore manual layout in older scores
        } else if (tag == "move") {
            note->chord()->setStaffMove(e.readInt());
        } else if (tag == "head") {
            int i = e.readInt();
            NoteHeadGroup val = Read206::convertHeadGroup(i);
            note->setHeadGroup(val);
        } else if (tag == "headType") {
            int i = e.readInt();
            NoteHeadType val;
            switch (i) {
            case 1:
                val = NoteHeadType::HEAD_WHOLE;
                break;
            case 2:
                val = NoteHeadType::HEAD_HALF;
                break;
            case 3:
                val = NoteHeadType::HEAD_QUARTER;
                break;
            case 4:
                val = NoteHeadType::HEAD_BREVIS;
                break;
            default:
                val = NoteHeadType::HEAD_AUTO;
            }
            note->setHeadType(val);
        } else if (Read206::readNoteProperties206(note, e, ctx)) {
        } else {
            e.unknown();
        }
    }
    // ensure sane values:
    note->setPitch(std::clamp(note->pitch(), 0, 127));

    if (note->concertPitch()) {
        note->setTpc2(Tpc::TPC_INVALID);
    } else {
        note->setPitch(note->pitch() + note->transposition());
        note->setTpc2(note->tpc1());
        note->setTpc1(Tpc::TPC_INVALID);
    }
    if (!tpcIsValid(note->tpc1()) && !tpcIsValid(note->tpc2())) {
        Key key = (note->staff() && note->chord()) ? note->staff()->key(note->chord()->tick()) : Key::C;
        int tpc = pitch2tpc(note->pitch(), key, Prefer::NEAREST);
        if (note->concertPitch()) {
            note->setTpc1(tpc);
        } else {
            note->setTpc2(tpc);
        }
    }
    if (!(tpcIsValid(note->tpc1()) && tpcIsValid(note->tpc2()))) {
        Fraction tick = note->chord() ? note->chord()->tick() : Fraction(-1, 1);
        Interval v = note->staff() ? note->part()->instrument(tick)->transpose() : Interval();
        if (tpcIsValid(note->tpc1())) {
            v.flip();
            if (v.isZero()) {
                note->setTpc2(note->tpc1());
            } else {
                note->setTpc2(mu::engraving::transposeTpc(note->tpc1(), v, true));
            }
        } else {
            if (v.isZero()) {
                note->setTpc1(note->tpc2());
            } else {
                note->setTpc1(mu::engraving::transposeTpc(note->tpc2(), v, true));
            }
        }
    }

    // check consistency of pitch, tpc1, tpc2, and transposition
    // see note in InstrumentChange::read() about a known case of tpc corruption produced in 2.0.x
    // but since there are other causes of tpc corruption (eg, https://musescore.org/en/node/74746)
    // including perhaps some we don't know about yet,
    // we will attempt to fix some problems here regardless of version

    if (!ctx.pasteMode() && !MScore::testMode) {
        int tpc1Pitch = (tpc2pitch(note->tpc1()) + 12) % 12;
        int tpc2Pitch = (tpc2pitch(note->tpc2()) + 12) % 12;
        int concertPitch = note->pitch() % 12;
        if (tpc1Pitch != concertPitch) {
            LOGD("bad tpc1 - concertPitch = %d, tpc1 = %d", concertPitch, tpc1Pitch);
            note->setPitch(note->pitch() + tpc1Pitch - concertPitch);
        }
        Interval v = note->staff()->part()->instrument(ctx.tick())->transpose();
        int transposedPitch = (note->pitch() - v.chromatic) % 12;
        if (tpc2Pitch != transposedPitch) {
            LOGD("bad tpc2 - transposedPitch = %d, tpc2 = %d", transposedPitch, tpc2Pitch);
            // just in case the staff transposition info is not reliable here,
            v.flip();
            note->setTpc2(mu::engraving::transposeTpc(note->tpc1(), v, true));
        }
    }
}

//---------------------------------------------------------
//   readClefType
//---------------------------------------------------------

static ClefType readClefType(int i)
{
    ClefType ct = ClefType::G;
    // convert obsolete old coding
    switch (i) {
    default:
    case  0: ct = ClefType::G;
        break;
    case  1: ct = ClefType::G8_VA;
        break;
    case  2: ct = ClefType::G15_MA;
        break;
    case  3: ct = ClefType::G8_VB;
        break;
    case  4: ct = ClefType::F;
        break;
    case  5: ct = ClefType::F8_VB;
        break;
    case  6: ct = ClefType::F15_MB;
        break;
    case  7: ct = ClefType::F_B;
        break;
    case  8: ct = ClefType::F_C;
        break;
    case  9: ct = ClefType::C1;
        break;
    case 10: ct = ClefType::C2;
        break;
    case 11: ct = ClefType::C3;
        break;
    case 12: ct = ClefType::C4;
        break;
    case 13: ct = ClefType::TAB;
        break;
    case 14: ct = ClefType::PERC;
        break;
    case 15: ct = ClefType::C5;
        break;
    case 16: ct = ClefType::G_1;
        break;
    case 17: ct = ClefType::F_8VA;
        break;
    case 18: ct = ClefType::F_15MA;
        break;
    case 19: ct = ClefType::PERC;
        break;                                              // PERC2 no longer supported
    case 20: ct = ClefType::TAB_SERIF;
        break;
    }

    return ct;
}

//---------------------------------------------------------
//   readClef
//---------------------------------------------------------

static void readClef(Clef* clef, XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            clef->setClefType(readClefType(e.readInt()));
        } else if (!clef->readProperties(e)) {
            e.unknown();
        }
    }
    if (clef->clefType() == ClefType::INVALID) {
        clef->setClefType(ClefType::G);
    }
}

//---------------------------------------------------------
//   readTuplet
//---------------------------------------------------------

static void readTuplet(Tuplet* tuplet, XmlReader& e, const ReadContext& ctx)
{
    int bl = -1;
    tuplet->setId(e.intAttribute("id", 0));

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {      // obsolete
            e.skipCurrentElement();
        } else if (tag == "hasNumber") {  // obsolete even in 1.3
            tuplet->setNumberType(e.readInt() ? TupletNumberType::SHOW_NUMBER : TupletNumberType::NO_TEXT);
        } else if (tag == "hasLine") {    // obsolete even in 1.3
            tuplet->setHasBracket(e.readInt());
            tuplet->setBracketType(TupletBracketType::AUTO_BRACKET);
        } else if (tag == "baseLen") {    // obsolete even in 1.3
            bl = e.readInt();
        } else if (tag == "tick") {
            tuplet->setTick(Fraction::fromTicks(e.readInt()));
        } else if (!Read206::readTupletProperties206(e, ctx, tuplet)) {
            e.unknown();
        }
    }
    Fraction r = (tuplet->ratio() == Fraction(1, 1)) ? tuplet->ratio() : tuplet->ratio().reduced();
    // this may be wrong, but at this stage it is kept for compatibility. It will be corrected afterwards
    // during "sanitize" step
    Fraction f(r.denominator(), tuplet->baseLen().fraction().denominator());
    tuplet->setTicks(f.reduced());
    if (bl != -1) {           // obsolete, even in 1.3
        TDuration d;
        d.setVal(bl);
        tuplet->setBaseLen(d);
        d.setVal(bl * tuplet->ratio().denominator());
        tuplet->setTicks(d.fraction());
    }
}

//---------------------------------------------------------
//   readTremolo
//---------------------------------------------------------

static void readTremolo(Tremolo* tremolo, XmlReader& e)
{
    enum class OldTremoloType : char {
        OLD_R8 = 0,
        OLD_R16,
        OLD_R32,
        OLD_C8,
        OLD_C16,
        OLD_C32
    };
    while (e.readNextStartElement()) {
        if (e.name() == "subtype") {
            OldTremoloType sti = OldTremoloType(e.readText().toInt());
            TremoloType st;
            switch (sti) {
            default:
            case OldTremoloType::OLD_R8:  st = TremoloType::R8;
                break;
            case OldTremoloType::OLD_R16: st = TremoloType::R16;
                break;
            case OldTremoloType::OLD_R32: st = TremoloType::R32;
                break;
            case OldTremoloType::OLD_C8:  st = TremoloType::C8;
                break;
            case OldTremoloType::OLD_C16: st = TremoloType::C16;
                break;
            case OldTremoloType::OLD_C32: st = TremoloType::C32;
                break;
            }
            tremolo->setTremoloType(st);
        } else if (!tremolo->EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

static void readChord(Measure* m, Chord* chord, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Note") {
            Note* note = Factory::createNote(chord);
            // the note needs to know the properties of the track it belongs to
            note->setTrack(chord->track());
            note->setParent(chord);
            readNote(note, e, ctx);
            chord->add(note);
        } else if (tag == "Attribute" || tag == "Articulation") {
            EngravingItem* el = Read206::readArticulation(chord, e, ctx);
            if (el->isFermata()) {
                if (!chord->segment()) {
                    chord->setParent(m->getSegment(SegmentType::ChordRest, ctx.tick()));
                }
                chord->segment()->add(el);
            } else {
                chord->add(el);
            }
        } else if (tag == "Tremolo") {
            Tremolo* tremolo = Factory::createTremolo(chord);
            tremolo->setDurationType(chord->durationType());
            chord->setTremolo(tremolo);
            tremolo->setTrack(chord->track());
            readTremolo(tremolo, e);
            tremolo->setParent(chord);
        } else if (Read206::readChordProperties206(e, ctx, chord)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

static void readRest(Measure* m, Rest* rest, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Attribute" || tag == "Articulation") {
            EngravingItem* el = Read206::readArticulation(rest, e, ctx);
            if (el->isFermata()) {
                if (!rest->segment()) {
                    rest->setParent(m->getSegment(SegmentType::ChordRest, ctx.tick()));
                }
                rest->segment()->add(el);
            } else {
                rest->add(el);
            }
        } else if (Read206::readChordRestProperties206(e, ctx, rest)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readTempoText
//---------------------------------------------------------

void readTempoText(TempoText* t, XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "tempo") {
            t->setTempo(e.readDouble());
        } else if (!readTextProperties(e, t, t)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readStaffText
//---------------------------------------------------------

void readStaffText(StaffText* t, XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readTextProperties(e, t, t)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readLineSegment114
//---------------------------------------------------------

static void readLineSegment114(XmlReader& e, LineSegment* ls)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "off1") {
            ls->setOffset(e.readPoint() * ls->spatium());
        } else {
            ls->readProperties(e);
        }
    }
}

//---------------------------------------------------------
//   readTextLineProperties114
//---------------------------------------------------------

static bool readTextLineProperties114(XmlReader& e, const ReadContext& ctx, TextLineBase* tl)
{
    const AsciiStringView tag(e.name());

    if (tag == "beginText") {
        Text* text = Factory::createText(tl, TextStyleType::DEFAULT, false);
        readText114(e, text, tl);
        tl->setBeginText(text->xmlText());
        tl->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
        delete text;
    } else if (tag == "continueText") {
        Text* text = Factory::createText(tl, TextStyleType::DEFAULT, false);
        readText114(e, text, tl);
        tl->setContinueText(text->xmlText());
        tl->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::UNSTYLED);
        delete text;
    } else if (tag == "endText") {
        Text* text = Factory::createText(tl, TextStyleType::DEFAULT, false);
        readText114(e, text, tl);
        tl->setEndText(text->xmlText());
        tl->setPropertyFlags(Pid::END_TEXT, PropertyFlags::UNSTYLED);
        delete text;
    } else if (tag == "beginHook") {
        tl->setBeginHookType(e.readBool() ? HookType::HOOK_90 : HookType::NONE);
    } else if (tag == "endHook") {
        tl->setEndHookType(e.readBool() ? HookType::HOOK_90 : HookType::NONE);
    } else if (tag == "beginHookType") {
        tl->setBeginHookType(e.readInt() == 0 ? HookType::HOOK_90 : HookType::HOOK_45);
    } else if (tag == "endHookType") {
        tl->setEndHookType(e.readInt() == 0 ? HookType::HOOK_90 : HookType::HOOK_45);
    } else if (tag == "Segment") {
        LineSegment* ls = tl->createLineSegment(ctx.dummy()->system());
        ls->setTrack(tl->track());     // needed in read to get the right staff mag
        readLineSegment114(e, ls);
        // in v1.x "visible" is a property of the segment only;
        // we must ensure that it propagates also to the parent element.
        // That's why the visibility is set after adding the segment
        // to the corresponding spanner
        ls->setVisible(ls->visible());
        ls->setOffset(PointF());            // ignore offsets
        ls->setAutoplace(true);
        tl->add(ls);
    } else if (tl->readProperties(e)) {
        return true;
    }
    return true;
}

//---------------------------------------------------------
//   readVolta114
//---------------------------------------------------------

static void readVolta114(XmlReader& e, const ReadContext& ctx, Volta* volta)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "endings") {
            String s = e.readText();
            StringList sl = s.split(u',', mu::SkipEmptyParts);
            volta->endings().clear();
            for (const String& l : sl) {
                int i = l.simplified().toInt();
                volta->endings().push_back(i);
            }
        } else if (tag == "subtype") {
            e.readInt();
        } else if (tag == "lineWidth") {
            volta->setLineWidth(Millimetre(e.readDouble() * volta->spatium()));
            volta->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
        } else if (!readTextLineProperties114(e, ctx, volta)) {
            e.unknown();
        }
    }
    volta->setOffset(PointF());          // ignore offsets
    volta->setAutoplace(true);
}

//---------------------------------------------------------
//   readOttava114
//---------------------------------------------------------

static void readOttava114(XmlReader& e, const ReadContext& ctx, Ottava* ottava)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            String s = e.readText();
            bool ok;
            int idx = s.toInt(&ok);
            if (!ok) {
                idx = int(OttavaType::OTTAVA_8VA);
                for (unsigned i = 0; i < sizeof(ottavaDefault) / sizeof(*ottavaDefault); ++i) {
                    if (s == ottavaDefault[i].name) {
                        idx = i;
                        break;
                    }
                }
            }
            //subtype are now in a different order...
            if (idx == 1) {
                idx = 2;
            } else if (idx == 2) {
                idx = 1;
            }
            ottava->setOttavaType(OttavaType(idx));
        } else if (tag == "numbersOnly") {
            ottava->setNumbersOnly(e.readInt());
        } else if (tag == "lineWidth") {
            ottava->readProperty(e, Pid::LINE_WIDTH);
        } else if (tag == "lineStyle") {
            ottava->readProperty(e, Pid::LINE_STYLE);
        } else if (tag == "beginSymbol") {                        // obsolete
        } else if (tag == "continueSymbol") {                     // obsolete
        } else if (!readTextLineProperties114(e, ctx, ottava)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   resolveSymCompatibility
//---------------------------------------------------------

static String resolveSymCompatibility(SymId i, String programVersion)
{
    if (!programVersion.isEmpty() && programVersion < u"1.1") {
        i = SymId(int(i) + 5);
    }
    switch (int(i)) {
    case 197:
        return u"keyboardPedalPed";
    case 191:
        return u"keyboardPedalUp";
    case 193:
        return u"noSym";           //SymId(pedaldotSym);
    case 192:
        return u"noSym";           //SymId(pedaldashSym);
    case 139:
        return u"ornamentTrill";
    default:
        return u"noSym";
    }
}

//---------------------------------------------------------
//   readTextLine114
//---------------------------------------------------------

static void readTextLine114(XmlReader& e, const ReadContext& ctx, TextLine* textLine)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "lineVisible") {
            textLine->setLineVisible(e.readBool());
        } else if (tag == "beginHookHeight") {
            textLine->setBeginHookHeight(Spatium(e.readDouble()));
        } else if (tag == "endHookHeight" || tag == "hookHeight") {   // hookHeight is obsolete
            textLine->setEndHookHeight(Spatium(e.readDouble()));
            textLine->setPropertyFlags(Pid::END_HOOK_HEIGHT, PropertyFlags::UNSTYLED);
        } else if (tag == "hookUp") { // obsolete
            textLine->setEndHookHeight(Spatium(double(-1.0)));
        } else if (tag == "beginSymbol" || tag == "symbol") {   // "symbol" is obsolete
            String text(e.readText());
            textLine->setBeginText(String(u"<sym>%1</sym>").arg(
                                       text.at(0).isDigit()
                                       ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                       : text));
        } else if (tag == "continueSymbol") {
            String text(e.readText());
            textLine->setContinueText(String(u"<sym>%1</sym>").arg(
                                          text.at(0).isDigit()
                                          ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                          : text));
        } else if (tag == "endSymbol") {
            String text(e.readText());
            textLine->setEndText(String(u"<sym>%1</sym>").arg(
                                     text.at(0).isDigit()
                                     ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                     : text));
        } else if (tag == "beginSymbolOffset") { // obsolete
            e.readPoint();
        } else if (tag == "continueSymbolOffset") { // obsolete
            e.readPoint();
        } else if (tag == "endSymbolOffset") { // obsolete
            e.readPoint();
        } else if (tag == "beginTextPlace") {
            textLine->setBeginTextPlace(TConv::fromXml(e.readAsciiText(), TextPlace::AUTO));
        } else if (tag == "continueTextPlace") {
            textLine->setContinueTextPlace(TConv::fromXml(e.readAsciiText(), TextPlace::AUTO));
        } else if (tag == "endTextPlace") {
            textLine->setEndTextPlace(TConv::fromXml(e.readAsciiText(), TextPlace::AUTO));
        } else if (!readTextLineProperties114(e, ctx, textLine)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readPedal114
//---------------------------------------------------------

static void readPedal114(XmlReader& e, const ReadContext& ctx, Pedal* pedal)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            e.skipCurrentElement();
        } else if (tag == "endHookHeight" || tag == "hookHeight") {   // hookHeight is obsolete
            pedal->setEndHookHeight(Spatium(e.readDouble()));
            pedal->setPropertyFlags(Pid::END_HOOK_HEIGHT, PropertyFlags::UNSTYLED);
        } else if (tag == "lineWidth") {
            pedal->setLineWidth(Millimetre(e.readDouble()));
            pedal->setPropertyFlags(Pid::LINE_WIDTH, PropertyFlags::UNSTYLED);
        } else if (tag == "lineStyle") {
            pedal->readProperty(e, Pid::LINE_STYLE);
            pedal->setPropertyFlags(Pid::LINE_STYLE, PropertyFlags::UNSTYLED);
        } else if (tag == "beginSymbol" || tag == "symbol") {   // "symbol" is obsolete
            String text(e.readText());
            pedal->setBeginText(String(u"<sym>%1</sym>").arg(
                                    text.at(0).isDigit()
                                    ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                    : text));
        } else if (tag == "continueSymbol") {
            String text(e.readText());
            pedal->setContinueText(String(u"<sym>%1</sym>").arg(
                                       text.at(0).isDigit()
                                       ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                       : text));
        } else if (tag == "endSymbol") {
            String text(e.readText());
            pedal->setEndText(String(u"<sym>%1</sym>").arg(
                                  text.at(0).isDigit()
                                  ? resolveSymCompatibility(SymId(text.toInt()), ctx.mscoreVersion())
                                  : text));
        } else if (tag == "beginSymbolOffset") { // obsolete
            e.readPoint();
        } else if (tag == "continueSymbolOffset") { // obsolete
            e.readPoint();
        } else if (tag == "endSymbolOffset") { // obsolete
            e.readPoint();
        } else if (!readTextLineProperties114(e, ctx, pedal)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readHarmony114
//---------------------------------------------------------

static void readHarmony114(XmlReader& e, const ReadContext& ctx, Harmony* h)
{
    // convert table to tpc values
    static const int table[] = {
        14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
    };

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "base") {
            if (ctx.mscVersion() >= 106) {
                h->setBaseTpc(e.readInt());
            } else {
                h->setBaseTpc(table[e.readInt() - 1]);
            }
        } else if (tag == "baseCase") {
            h->setBaseCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "extension") {
            h->setId(e.readInt());
        } else if (tag == "name") {
            h->setTextName(e.readText());
        } else if (tag == "root") {
            if (ctx.mscVersion() >= 106) {
                h->setRootTpc(e.readInt());
            } else {
                h->setRootTpc(table[e.readInt() - 1]);
            }
        } else if (tag == "rootCase") {
            h->setRootCase(static_cast<NoteCaseType>(e.readInt()));
        } else if (tag == "degree") {
            int degreeValue = 0;
            int degreeAlter = 0;
            String degreeType;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "degree-value") {
                    degreeValue = e.readInt();
                } else if (t == "degree-alter") {
                    degreeAlter = e.readInt();
                } else if (t == "degree-type") {
                    degreeType = e.readText();
                } else {
                    e.unknown();
                }
            }
            if (degreeValue <= 0 || degreeValue > 13
                || degreeAlter < -2 || degreeAlter > 2
                || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                LOGD("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                     degreeValue, degreeAlter, muPrintable(degreeType));
            } else {
                if (degreeType == "add") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                } else if (degreeType == "alter") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                } else if (degreeType == "subtract") {
                    h->addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                }
            }
        } else if (tag == "leftParen") {
            h->setLeftParen(true);
            e.readNext();
        } else if (tag == "rightParen") {
            h->setRightParen(true);
            e.readNext();
        } else if (!readTextProperties(e, h, h)) {
            e.unknown();
        }
    }

    // TODO: now that we can render arbitrary chords,
    // we could try to construct a full representation from a degree list.
    // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
    // or constructed in the Chord Symbol Properties dialog.

    if (h->rootTpc() != Tpc::TPC_INVALID) {
        if (h->id() > 0) {
            // positive id will happen only for scores that were created with explicit chord lists
            // lookup id in chord list and generate new description if necessary
            h->getDescription();
        } else {
            // default case: look up by name
            // description will be found for any chord already read in this score
            // and we will generate a new one if necessary
            h->getDescription(h->hTextName());
        }
    } else if (h->hTextName() == "") {
        // unrecognized chords prior to 2.0 were stored as text with markup
        // we need to strip away the markup
        // this removes any user-applied formatting,
        // but we no longer support user-applied formatting for chord symbols anyhow
        // with any luck, the resulting text will be parseable now, so give it a shot
//            h->createLayout();
        String s = h->plainText();
        if (!s.isEmpty()) {
            h->setHarmony(s);
            return;
        }
        // empty text could also indicate a root-less slash chord ("/E")
        // we'll fall through and render it normally
    }

    // render chord from description (or _textName)
    h->render();
}

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

static void readMeasure(Measure* m, int staffIdx, XmlReader& e, ReadContext& ctx)
{
    Segment* segment = 0;
    double _spatium = m->spatium();

    std::vector<Chord*> graceNotes;

    //sort tuplet elements. needed for nested tuplets #22537
    for (auto& p : ctx.tuplets()) {
        Tuplet* t = p.second;
        t->sortElements();
    }
    ctx.tuplets().clear();
    ctx.setTrack(staffIdx * VOICES);

    m->createStaves(staffIdx);

    // tick is obsolete
    if (e.hasAttribute("tick")) {
        ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.intAttribute("tick"))));
    }

    if (e.hasAttribute("len")) {
        StringList sl = e.attribute("len").split('/');
        if (sl.size() == 2) {
            m->setTicks(Fraction(sl[0].toInt(), sl[1].toInt()));
        } else {
            LOGD("illegal measure size <%s>", muPrintable(e.attribute("len")));
        }
        ctx.sigmap()->add(m->tick().ticks(), SigEvent(m->ticks(), m->timesig()));
        ctx.sigmap()->add(m->endTick().ticks(), SigEvent(m->timesig()));
    }

    Staff* staff = ctx.staff(staffIdx);
    Fraction timeStretch(staff->timeStretch(m->tick()));

    // keep track of tick of previous element
    // this allows markings that need to apply to previous element to do so
    // even though we may have already advanced to next tick position
    Fraction lastTick = ctx.tick();

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "move") {
            ctx.setTick(e.readFraction() + m->tick());
        } else if (tag == "tick") {
            ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.readInt())));
            lastTick = ctx.tick();
        } else if (tag == "BarLine") {
            BarLine* barLine = Factory::createBarLine(ctx.dummy()->segment());
            barLine->setTrack(ctx.track());
            barLine->resetProperty(Pid::BARLINE_SPAN);
            barLine->resetProperty(Pid::BARLINE_SPAN_FROM);
            barLine->resetProperty(Pid::BARLINE_SPAN_TO);

            while (e.readNextStartElement()) {
                const AsciiStringView tg(e.name());
                if (tg == "subtype") {
                    BarLineType t = BarLineType::NORMAL;
                    switch (e.readInt()) {
                    default:
                    case 0:
                        t = BarLineType::NORMAL;
                        break;
                    case 1:
                        t = BarLineType::DOUBLE;
                        break;
                    case 2:
                        t = BarLineType::START_REPEAT;
                        break;
                    case 3:
                        t = BarLineType::END_REPEAT;
                        break;
                    case 4:
                        t = BarLineType::BROKEN;
                        break;
                    case 5:
                        t = BarLineType::END;
                        break;
                    case 6:
                        t = BarLineType::END_START_REPEAT;
                        break;
                    }
                    barLine->setBarLineType(t);
                } else if (!barLine->EngravingItem::readProperties(e)) {
                    e.unknown();
                }
            }

            //
            //  StartRepeatBarLine: always at the beginning tick of a measure, always BarLineType::START_REPEAT
            //  BarLine:            in the middle of a measure, has no semantic
            //  EndBarLine:         at the end tick of a measure
            //  BeginBarLine:       first segment of a measure

            SegmentType st;
            if ((ctx.tick() != m->tick()) && (ctx.tick() != m->endTick())) {
                st = SegmentType::BarLine;
            } else if (barLine->barLineType() == BarLineType::START_REPEAT && ctx.tick() == m->tick()) {
                st = SegmentType::StartRepeatBarLine;
            } else if (ctx.tick() == m->tick() && segment == 0) {
                st = SegmentType::BeginBarLine;
            } else {
                st = SegmentType::EndBarLine;
            }
            segment = m->getSegment(st, ctx.tick());
            segment->add(barLine);
        } else if (tag == "Chord") {
            Chord* chord = Factory::createChord(m->getSegment(SegmentType::ChordRest, ctx.tick()));
            chord->setTrack(ctx.track());
            readChord(m, chord, e, ctx);
            if (!chord->segment()) {
                chord->setParent(m->getSegment(SegmentType::ChordRest, ctx.tick()));
            }
            segment = chord->segment();
            if (chord->noteType() != NoteType::NORMAL) {
                graceNotes.push_back(chord);
            } else {
                segment->add(chord);
                assert(segment->segmentType() == SegmentType::ChordRest);

                for (size_t i = 0; i < graceNotes.size(); ++i) {
                    Chord* gc = graceNotes[i];
                    gc->setGraceIndex(static_cast<int>(i));
                    chord->add(gc);
                }
                graceNotes.clear();
                Fraction crticks = chord->actualTicks();

                if (chord->tremolo()) {
                    Tremolo* tremolo = chord->tremolo();
                    if (tremolo->twoNotes()) {
                        track_idx_t track = chord->track();
                        Segment* ss = 0;
                        for (Segment* ps = m->first(SegmentType::ChordRest); ps; ps = ps->next(SegmentType::ChordRest)) {
                            if (ps->tick() >= ctx.tick()) {
                                break;
                            }
                            if (ps->element(track)) {
                                ss = ps;
                            }
                        }
                        Chord* pch = 0;                   // previous chord
                        if (ss) {
                            ChordRest* cr = toChordRest(ss->element(track));
                            if (cr && cr->type() == ElementType::CHORD) {
                                pch = toChord(cr);
                            }
                        }
                        if (pch) {
                            tremolo->setParent(pch);
                            pch->setTremolo(tremolo);
                            chord->setTremolo(0);
                            // force duration to half
                            Fraction pts(timeStretch * pch->globalTicks());
                            pch->setTicks(pts * Fraction(1, 2));
                            chord->setTicks(crticks * Fraction(1, 2));
                        } else {
                            LOGD("tremolo: first note not found");
                        }
                        crticks = crticks * Fraction(1, 2);
                    } else {
                        tremolo->setParent(chord);
                    }
                }
                lastTick = ctx.tick();
                ctx.incTick(crticks);
            }
        } else if (tag == "Rest") {
            if (m->isMMRest()) {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                MMRest* mmr = Factory::createMMRest(segment);
                mmr->setParent(segment);
                mmr->setTrack(ctx.track());
                mmr->read(e);
                segment->add(mmr);
                lastTick = ctx.tick();
                ctx.incTick(mmr->actualTicks());
            } else {
                Segment* segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                Rest* rest = Factory::createRest(segment);
                rest->setDurationType(DurationType::V_MEASURE);
                rest->setTicks(m->timesig() / timeStretch);
                rest->setTrack(ctx.track());
                readRest(m, rest, e, ctx);
                if (!rest->segment()) {
                    rest->setParent(segment);
                }
                segment = rest->segment();
                segment->add(rest);

                if (!rest->ticks().isValid()) {    // hack
                    rest->setTicks(m->timesig() / timeStretch);
                }

                lastTick = ctx.tick();
                ctx.incTick(rest->actualTicks());
            }
        } else if (tag == "Breath") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            Breath* breath = Factory::createBreath(segment);
            breath->setTrack(ctx.track());
            Fraction tick = ctx.tick();
            breath->setPlacement(breath->track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
            breath->read(e);
            // older scores placed the breath segment right after the chord to which it applies
            // rather than before the next chordrest segment with an element for the staff
            // result would be layout too far left if there are other segments due to notes in other staves
            // we need to find tick of chord to which this applies, and add its duration
            Fraction prevTick;
            if (ctx.tick() < tick) {
                prevTick = ctx.tick();            // use our own tick if we explicitly reset to earlier position
            } else {
                prevTick = lastTick;            // otherwise use tick of previous tick/chord/rest tag
            }
            // find segment
            Segment* prev = m->findSegment(SegmentType::ChordRest, prevTick);
            if (prev) {
                // find chordrest
                ChordRest* lastCR = toChordRest(prev->element(ctx.track()));
                if (lastCR) {
                    tick = prevTick + lastCR->actualTicks();
                }
            }
            segment = m->getSegment(SegmentType::Breath, tick);
            segment->add(breath);
        } else if (tag == "endSpanner") {
            int id = e.attribute("id").toInt();
            Spanner* spanner = ctx.findSpanner(id);
            if (spanner) {
                spanner->setTicks(ctx.tick() - spanner->tick());
                // if (spanner->track2() == -1)
                // the absence of a track tag [?] means the
                // track is the same as the beginning of the slur
                if (spanner->track2() == mu::nidx) {
                    spanner->setTrack2(spanner->track() ? spanner->track() : ctx.track());
                }
            } else {
                // remember "endSpanner" values
                SpannerValues sv;
                sv.spannerId = id;
                sv.track2    = ctx.track();
                sv.tick2     = ctx.tick();
                ctx.addSpannerValues(sv);
            }
            e.readNext();
        } else if (tag == "Slur") {
            Slur* sl = Factory::createSlur(m);
            sl->setTick(ctx.tick());
            Read206::readSlur206(e, ctx, sl);
            //
            // check if we already saw "endSpanner"
            //
            int id = ctx.spannerId(sl);
            const SpannerValues* sv = ctx.spannerValues(id);
            if (sv) {
                sl->setTick2(sv->tick2);
                sl->setTrack2(sv->track2);
            }
            ctx.addSpanner(sl);
        } else if (tag == "HairPin"
                   || tag == "Pedal"
                   || tag == "Ottava"
                   || tag == "Trill"
                   || tag == "TextLine"
                   || tag == "Volta") {
            Spanner* sp = toSpanner(Factory::createItemByName(tag, m));
            sp->setTrack(ctx.track());
            sp->setTick(ctx.tick());
            // ?? sp->setAnchor(Spanner::Anchor::SEGMENT);
            if (tag == "Volta") {
                readVolta114(e, ctx, toVolta(sp));
            } else {
                Read206::readTextLine206(e, ctx, toTextLineBase(sp));
            }
            ctx.addSpanner(sp);
            //
            // check if we already saw "endSpanner"
            //
            int id = ctx.spannerId(sp);
            const SpannerValues* sv = ctx.spannerValues(id);
            if (sv) {
                sp->setTicks(sv->tick2 - sp->tick());
                sp->setTrack2(sv->track2);
            }
        } else if (tag == "RepeatMeasure") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            MeasureRepeat* rm = Factory::createMeasureRepeat(segment);
            rm->setTrack(ctx.track());
            readRest(m, rm, e, ctx);
            rm->setNumMeasures(1);
            m->setMeasureRepeatCount(1, staffIdx);
            segment->add(rm);
            if (rm->actualTicks().isZero()) {     // might happen with 1.3 scores
                rm->setTicks(m->ticks());
            }
            lastTick = ctx.tick();
            ctx.incTick(m->ticks());
        } else if (tag == "Clef") {
            // there may be more than one clef segment for same tick position
            // the first clef may be missing and is added later in layout

            bool header;
            if (ctx.tick() != m->tick()) {
                header = false;
            } else if (!segment) {
                header = true;
            } else {
                header = true;
                for (Segment* s = m->segments().first(); s && s->rtick().isZero(); s = s->next()) {
                    if (s->isKeySigType() || s->isTimeSigType()) {
                        // hack: there may be other segment types which should
                        // generate a clef at current position
                        header = false;
                        break;
                    }
                }
            }
            segment = m->getSegment(header ? SegmentType::HeaderClef : SegmentType::Clef, ctx.tick());

            Clef* clef = Factory::createClef(segment);
            clef->setTrack(ctx.track());
            readClef(clef, e);
            if (ctx.mscVersion() < 113) {
                clef->setOffset(PointF());
            }
            clef->setGenerated(false);
            // MS3 doesn't support wrong clef for staff type: Default to G
            bool isDrumStaff = staff->isDrumStaff(ctx.tick());
            if (clef->clefType() == ClefType::TAB
                || (clef->clefType() == ClefType::PERC && !isDrumStaff)
                || (clef->clefType() != ClefType::PERC && isDrumStaff)) {
                clef->setClefType(ClefType::G);
                staff->clefList().erase(ctx.tick().ticks());
                staff->clefList().insert(std::pair<int, ClefType>(ctx.tick().ticks(), ClefType::G));
            }

            segment->add(clef);
        } else if (tag == "TimeSig") {
            // if time sig not at beginning of measure => courtesy time sig
            Fraction currTick = ctx.tick();
            bool courtesySig = (currTick > m->tick());
            if (courtesySig) {
                // if courtesy sig., just add it without map processing
                segment = m->getSegment(SegmentType::TimeSigAnnounce, currTick);
            } else {
                // if 'real' time sig., do full process
                segment = m->getSegment(SegmentType::TimeSig, currTick);
            }

            TimeSig* ts = Factory::createTimeSig(segment);
            ts->setTrack(ctx.track());
            ts->read(e);

            segment->add(ts);
            if (!courtesySig) {
                timeStretch = ts->stretch().reduced();
                m->setTimesig(ts->sig() / timeStretch);
            }
        } else if (tag == "KeySig") {
            KeySig* ks = Factory::createKeySig(ctx.dummy()->segment());
            ks->setTrack(ctx.track());
            ks->read(e);
            Fraction curTick = ctx.tick();
            if (!ks->isCustom() && !ks->isAtonal() && ks->key() == Key::C && curTick.isZero()) {
                // ignore empty key signature
                LOGD("remove keysig c at tick 0");
                if (ks->links()) {
                    if (ks->links()->size() == 1) {
                        mu::remove(e.context()->linkIds(), ks->links()->lid());
                    }
                }
            } else {
                // if key sig not at beginning of measure => courtesy key sig
                bool courtesySig = (curTick == m->endTick());
                segment = m->getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
                segment->add(ks);
                if (!courtesySig) {
                    staff->setKey(curTick, ks->keySigEvent());
                }
            }
        } else if (tag == "Lyrics") {
            Lyrics* l = Factory::createLyrics(ctx.dummy()->chord());
            l->setTrack(ctx.track());

            int iEndTick = 0;                 // used for backward compatibility
            Text* _verseNumber = 0;

            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "no") {
                    l->setNo(e.readInt());
                } else if (t == "syllabic") {
                    String val(e.readText());
                    if (val == "single") {
                        l->setSyllabic(Lyrics::Syllabic::SINGLE);
                    } else if (val == "begin") {
                        l->setSyllabic(Lyrics::Syllabic::BEGIN);
                    } else if (val == "end") {
                        l->setSyllabic(Lyrics::Syllabic::END);
                    } else if (val == "middle") {
                        l->setSyllabic(Lyrics::Syllabic::MIDDLE);
                    } else {
                        LOGD("bad syllabic property");
                    }
                } else if (t == "endTick") {                // obsolete
                    // store <endTick> tag value until a <ticks> tag has been read
                    // which positions this lyrics element in the score
                    iEndTick = e.readInt();
                } else if (t == "ticks") {
                    l->setTicks(Fraction::fromTicks(e.readInt()));
                } else if (t == "Number") {                                 // obsolete
                    _verseNumber = Factory::createText(l);
                    readText114(e, _verseNumber, l);
                } else if (!readTextProperties(e, l, l)) {
                    e.unknown();
                }
            }
            // if any endTick, make it relative to current tick
            if (iEndTick) {
                l->setTicks(Fraction::fromTicks(iEndTick - ctx.tick().ticks()));
                // LOGD("Lyrics::endTick: %d  ticks %d", iEndTick, _ticks);
            }
            if (_verseNumber) {
                // TODO: add text to main text
            }

            delete _verseNumber;

            segment       = m->getSegment(SegmentType::ChordRest, ctx.tick());
            ChordRest* cr = toChordRest(segment->element(l->track()));
            if (!cr) {
                cr = toChordRest(segment->element(ctx.track()));         // in case lyric itself has bad track info
            }
            if (!cr) {
                LOGD("Internal error: no chord/rest for lyrics");
            } else {
                cr->add(l);
            }
        } else if (tag == "Text") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            StaffText* t = Factory::createStaffText(segment);
            t->setTrack(ctx.track());
            readStaffText(t, e);
            if (t->empty()) {
                LOGD("reading empty text: deleted");
                delete t;
            } else {
                segment->add(t);
            }
        } else if (tag == "Dynamic") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            Dynamic* dyn = Factory::createDynamic(segment);
            dyn->setTrack(ctx.track());
            dyn->read(e); // for 114 scores, dynamics are frontloaded in the measure with <tick> attributes.
                          // so we need to reset its parent to the correct one after that element is read.
            segment = m->getSegment(SegmentType::ChordRest, e.context()->tick());
            dyn->setParent(segment);
            if (dyn->dynamicType() == DynamicType::OTHER && dyn->xmlText().isEmpty()) {
                // if we add this dynamic, it will be an unselectable invisible object that
                // messes with collision detection.
                delete dyn;
            } else {
                dyn->setDynamicType(dyn->xmlText());
                segment->add(dyn);
            }
        } else if (tag == "Tempo") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            TempoText* t = Factory::createTempoText(segment);
            t->setTrack(ctx.track());
            readTempoText(t, e);
            segment->add(t);
        } else if (tag == "StaffText") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            StaffText* t = Factory::createStaffText(segment);
            t->setTrack(ctx.track());
            readStaffText(t, e);
            segment->add(t);
        } else if (tag == "Harmony") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            Harmony* h = Factory::createHarmony(segment);
            h->setTrack(ctx.track());
            readHarmony114(e, ctx, h);
            segment->add(h);
        } else if (tag == "Harmony"
                   || tag == "FretDiagram"
                   || tag == "TremoloBar"
                   || tag == "Symbol"
                   || tag == "StaffText"
                   || tag == "RehearsalMark"
                   || tag == "InstrumentChange"
                   || tag == "StaffState"
                   ) {
            EngravingItem* el = Factory::createItemByName(tag, ctx.dummy());
            // hack - needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            if (el->type() == ElementType::SYMBOL) {
                el->setParent(m);            // this will get reset when adding to segment
            }
            el->setTrack(ctx.track());
            el->read(e);
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            segment->add(el);
        } else if (tag == "Jump") {
            Jump* j = Factory::createJump(m);
            j->setTrack(ctx.track());
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "jumpTo") {
                    j->setJumpTo(e.readText());
                } else if (t == "playUntil") {
                    j->setPlayUntil(e.readText());
                } else if (t == "continueAt") {
                    j->setContinueAt(e.readText());
                } else if (t == "playRepeats") {
                    j->setPlayRepeats(e.readBool());
                } else if (t == "subtype") {
                    e.readInt();
                } else if (!j->TextBase::readProperties(e)) {
                    e.unknown();
                }
            }
            m->add(j);
        } else if (tag == "Marker") {
            Marker* a = Factory::createMarker(m);
            a->setTrack(ctx.track());

            MarkerType mt = MarkerType::SEGNO;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "subtype") {
                    AsciiStringView s(e.readAsciiText());
                    a->setLabel(String::fromAscii(s.ascii()));
                    mt = TConv::fromXml(s, MarkerType::USER);
                } else if (!a->TextBase::readProperties(e)) {
                    e.unknown();
                }
            }
            a->setMarkerType(mt);

            if (a->markerType() == MarkerType::SEGNO || a->markerType() == MarkerType::CODA
                || a->markerType() == MarkerType::VARCODA || a->markerType() == MarkerType::CODETTA) {
                // force the marker type for correct display
                a->setXmlText(u"");
                a->setMarkerType(a->markerType());
            }
            m->add(a);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                EngravingItem* el = Factory::createItemByName(tag, segment);
                el->setTrack(ctx.track());
                el->read(e);
                segment->add(el);
            }
        } else if (tag == "stretch") {
            double val = e.readDouble();
            if (val < 0.0) {
                val = 0;
            }
            m->setUserStretch(val);
        } else if (tag == "noOffset") {
            m->setNoOffset(e.readInt());
        } else if (tag == "measureNumberMode") {
            m->setMeasureNumberMode(MeasureNumberMode(e.readInt()));
        } else if (tag == "irregular") {
            m->setIrregular(e.readBool());
        } else if (tag == "breakMultiMeasureRest") {
            m->setBreakMultiMeasureRest(e.readBool());
        } else if (tag == "sysInitBarLineType") {
            segment = m->getSegment(SegmentType::BeginBarLine, m->tick());
            BarLine* barLine = Factory::createBarLine(segment);
            barLine->setTrack(ctx.track());
            barLine->setBarLineType(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));
            segment->add(barLine);
        } else if (tag == "Tuplet") {
            Tuplet* tuplet = Factory::createTuplet(m);
            tuplet->setTrack(ctx.track());
            tuplet->setTick(ctx.tick());
            tuplet->setParent(m);
            readTuplet(tuplet, e, ctx);
            ctx.addTuplet(tuplet);
        } else if (tag == "startRepeat") {
            m->setRepeatStart(true);
            e.readNext();
        } else if (tag == "endRepeat") {
            m->setRepeatCount(e.readInt());
            m->setRepeatEnd(true);
        } else if (tag == "vspacer" || tag == "vspacerDown") {
            if (!m->vspacerDown(staffIdx)) {
                Spacer* spacer = Factory::createSpacer(m);
                spacer->setSpacerType(SpacerType::DOWN);
                spacer->setTrack(staffIdx * VOICES);
                m->add(spacer);
            }
            m->vspacerDown(staffIdx)->setGap(Millimetre(e.readDouble() * _spatium));
        } else if (tag == "vspacer" || tag == "vspacerUp") {
            if (!m->vspacerUp(staffIdx)) {
                Spacer* spacer = Factory::createSpacer(m);
                spacer->setSpacerType(SpacerType::UP);
                spacer->setTrack(staffIdx * VOICES);
                m->add(spacer);
            }
            m->vspacerUp(staffIdx)->setGap(Millimetre(e.readDouble() * _spatium));
        } else if (tag == "visible") {
            m->setStaffVisible(staffIdx, e.readInt());
        } else if (tag == "slashStyle") {
            m->setStaffStemless(staffIdx, e.readInt());
        } else if (tag == "Beam") {
            Beam* beam = Factory::createBeam(ctx.dummy()->system());
            beam->setTrack(ctx.track());
            beam->read(e);
            beam->resetExplicitParent();
            ctx.addBeam(beam);
        } else if (tag == "Segment") {
            if (segment) {
                segment->read(e);
            }
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "off1") {
                    double o = e.readDouble();
                    LOGD("TODO: off1 %f", o);
                } else {
                    e.unknown();
                }
            }
        } else if (tag == "MeasureNumber") {
            MeasureNumber* noText = new MeasureNumber(m);
            readText114(e, noText, m);
            noText->setTrack(ctx.track());
            noText->setParent(m);
            m->setNoText(noText->staffIdx(), noText);
        } else if (tag == "multiMeasureRest") {
            m->setMMRestCount(e.readInt());
            // set tick to previous measure
            m->setTick(ctx.lastMeasure()->tick());
            ctx.setTick(ctx.lastMeasure()->tick());
        } else if (m->MeasureBase::readProperties(e)) {
        } else {
            e.unknown();
        }
    }
    // For nested tuplets created with MuseScore 1.3 tuplet dialog (i.e. "Other..." dialog),
    // the parent tuplet was not set. Try to infer if the tuplet was actually a nested tuplet
    for (auto& p : ctx.tuplets()) {
        Tuplet* tuplet = p.second;
        Fraction tupletTick = tuplet->tick();
        Fraction tupletDuration = tuplet->actualTicks() - Fraction::fromTicks(1);
        std::vector<DurationElement*> tElements = tuplet->elements();
        for (auto& p : ctx.tuplets()) {
            Tuplet* tuplet2 = p.second;
            if ((tuplet2->tuplet()) || (tuplet2->voice() != tuplet->voice())) {     // already a nested tuplet or in a different voice
                continue;
            }
            // int possibleDuration = tuplet2->duration().ticks() * tuplet->ratio().denominator() / tuplet->ratio().numerator() - 1;
            Fraction possibleDuration = tuplet2->ticks() * Fraction(tuplet->ratio().denominator(), (tuplet->ratio().numerator() - 1));

            if ((tuplet2 != tuplet) && (tuplet2->tick() >= tupletTick) && (tuplet2->tick() < tupletTick + tupletDuration)
                && (tuplet2->tick() + possibleDuration < tupletTick + tupletDuration)) {
                bool found = false;
                for (DurationElement* de : tElements) {
                    if (de == tuplet2) {
                        found = true;
                    }
                }
                if (!found) {
                    LOGD("Adding tuplet %p as nested tuplet to tuplet %p", tuplet2, tuplet);
                    tuplet2->setTuplet(tuplet);
                    tuplet->add(tuplet2);
                }
            }
        }
    }
    ctx.checkTuplets();
    m->connectTremolo();
}

//---------------------------------------------------------
//   readBoxProperties
//---------------------------------------------------------

static void readBox(XmlReader& e, Box* b);

static bool readBoxProperties(XmlReader& e, Box* b)
{
    const AsciiStringView tag(e.name());
    if (tag == "height") {
        b->setBoxHeight(Spatium(e.readDouble()));
    } else if (tag == "width") {
        b->setBoxWidth(Spatium(e.readDouble()));
    } else if (tag == "topGap") {
        b->readProperty(e, Pid::TOP_GAP);
    } else if (tag == "bottomGap") {
        b->readProperty(e, Pid::BOTTOM_GAP);
    } else if (tag == "leftMargin") {
        b->setLeftMargin(e.readDouble());
    } else if (tag == "rightMargin") {
        b->setRightMargin(e.readDouble());
    } else if (tag == "topMargin") {
        b->setTopMargin(e.readDouble());
    } else if (tag == "bottomMargin") {
        b->setBottomMargin(e.readDouble());
    } else if (tag == "offset") {
        b->setOffset(e.readPoint() * b->spatium());
    } else if (tag == "pos") {      // ignore
        e.readPoint();
    } else if (tag == "Text") {
        Text* t;
        if (b->isTBox()) {
            t = toTBox(b)->text();
            readText114(e, t, t);
        } else {
            t = Factory::createText(b);
            readText114(e, t, t);
            if (t->empty()) {
                LOGD("read empty text");
            } else {
                b->add(t);
            }
        }
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(b);
        s->read(e);
        b->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(b);
            image->setTrack(e.context()->track());
            image->read(e);
            b->add(image);
        }
    } else if (tag == "HBox") {
        HBox* hb = Factory::createHBox(b->system());
        readBox(e, hb);
        b->add(hb);
    } else if (tag == "VBox") {
        VBox* vb = Factory::createVBox(b->system());
        readBox(e, vb);
        b->add(vb);
    }
//      else if (MeasureBase::readProperties(e))
//            ;
    else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   readBox
//---------------------------------------------------------

static void readBox(XmlReader& e, Box* b)
{
    b->setLeftMargin(0.0);
    b->setRightMargin(0.0);
    b->setTopMargin(0.0);
    b->setBottomMargin(0.0);
    b->setBoxHeight(Spatium(0));       // override default set in constructor
    b->setBoxWidth(Spatium(0));
    b->setAutoSizeEnabled(false);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "HBox") {
            HBox* hb = Factory::createHBox(b->system());
            readBox(e, hb);
            b->add(hb);
        } else if (tag == "VBox") {
            VBox* vb = Factory::createVBox(b->system());
            readBox(e, vb);
            b->add(vb);
        } else if (!readBoxProperties(e, b)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readStaffContent
//---------------------------------------------------------

static void readStaffContent(Score* score, XmlReader& e, ReadContext& ctx)
{
    int staff = e.intAttribute("id", 1) - 1;
    ctx.setTick(Fraction(0, 1));
    ctx.setTrack(staff * VOICES);

    Measure* measure = score->firstMeasure();
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "Measure") {
            if (staff == 0) {
                measure = Factory::createMeasure(score->dummy()->system());
                measure->setTick(ctx.tick());
                const SigEvent& ev = score->sigmap()->timesig(measure->tick());
                measure->setTicks(ev.timesig());
                measure->setTimesig(ev.nominal());

                readMeasure(measure, staff, e, ctx);
                measure->checkMeasure(staff);

                if (!measure->isMMRest()) {
                    score->measures()->add(measure);
                    ctx.setLastMeasure(measure);
                    ctx.setTick(measure->tick() + measure->ticks());
                } else {
                    // this is a multi measure rest
                    // always preceded by the first measure it replaces
                    Measure* m = ctx.lastMeasure();

                    if (m) {
                        m->setMMRest(measure);
                        measure->setTick(m->tick());
                    }
                }
            } else {
                if (measure == 0) {
                    LOGD("Score::readStaff(): missing measure!");
                    measure = Factory::createMeasure(score->dummy()->system());
                    measure->setTick(ctx.tick());
                    score->measures()->add(measure);
                }
                ctx.setTick(measure->tick());

                readMeasure(measure, staff, e, ctx);
                measure->checkMeasure(staff);

                if (measure->isMMRest()) {
                    measure = ctx.lastMeasure()->nextMeasure();
                } else {
                    ctx.setLastMeasure(measure);
                    if (measure->mmRest()) {
                        measure = measure->mmRest();
                    } else {
                        measure = measure->nextMeasure();
                    }
                }
            }
        } else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
            Box* mb = toBox(Factory::createItemByName(tag, score->dummy()));
            readBox(e, mb);
            mb->setTick(ctx.tick());
            score->measures()->add(mb);
        } else if (tag == "tick") {
            ctx.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

static void readStaff(Staff* staff, XmlReader& e, const ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "lines") {
            int lines = e.readInt();
            staff->setLines(Fraction(0, 1), lines);
            if (lines != 5) {
                staff->setBarLineFrom(lines == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                staff->setBarLineTo(lines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (lines - 1) * 2);
            }
        } else if (tag == "small") {
            staff->staffType(Fraction(0, 1))->setSmall(e.readInt());
        } else if (tag == "invisible") {
            staff->setIsLinesInvisible(Fraction(0, 1), e.readInt());
        } else if (tag == "slashStyle") {
            e.skipCurrentElement();
        } else if (tag == "cleflist") {
            staff->clefList().clear();
            while (e.readNextStartElement()) {
                if (e.name() == "clef") {
                    int tick    = e.intAttribute("tick", 0);
                    ClefType ct = readClefType(e.intAttribute("idx", 0));
                    staff->clefList().insert(std::pair<int, ClefType>(ctx.fileDivision(tick), ct));
                    e.readNext();
                } else {
                    e.unknown();
                }
            }
            if (staff->clefList().empty()) {
                staff->clefList().insert(std::pair<int, ClefType>(0, ClefType::G));
            }
        } else if (tag == "keylist") {
            staff->keyList()->read(e, ctx);
        } else if (tag == "bracket") {
            size_t col = staff->brackets().size();
            staff->setBracketType(col, BracketType(e.intAttribute("type", -1)));
            staff->setBracketSpan(col, e.intAttribute("span", 0));
            e.readNext();
        } else if (tag == "barLineSpan") {
            staff->setBarLineSpan(e.readInt());
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readDrumset
//---------------------------------------------------------

static void readDrumset(Drumset* ds, XmlReader& e)
{
    int pitch = e.intAttribute("pitch", -1);
    if (pitch < 0 || pitch > 127) {
        LOGD("load drumset: invalid pitch %d", pitch);
        return;
    }
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "head") {
            ds->drum(pitch).notehead = Read206::convertHeadGroup(e.readInt());
        } else if (ds->readProperties(e, pitch)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

static void readInstrument(Instrument* i, Part* p, XmlReader& e)
{
    int program = -1;
    int bank    = 0;
    int chorus  = 30;
    int reverb  = 30;
    int volume  = 100;
    int pan     = 60;
    bool customDrumset = false;
    i->clearChannels();         // remove default channel
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "chorus") {
            chorus = e.readInt();
        } else if (tag == "reverb") {
            reverb = e.readInt();
        } else if (tag == "midiProgram") {
            program = e.readInt();
        } else if (tag == "volume") {
            volume = e.readInt();
        } else if (tag == "pan") {
            pan = e.readInt();
        } else if (tag == "midiChannel") {
            e.skipCurrentElement();
        } else if (tag == "Drum") {
            // if we see one of this tags, a custom drumset will
            // be created
            if (!i->drumset()) {
                i->setDrumset(new Drumset(*smDrumset));
            }
            if (!customDrumset) {
                i->drumset()->clear();
                customDrumset = true;
            }
            readDrumset(i->drumset(), e);
        } else if (i->readProperties(e, p, &customDrumset)) {
        } else {
            e.unknown();
        }
    }

    i->updateInstrumentId();

    if (program == -1) {
        program = i->recognizeMidiProgram();
    }

    if (i->channel().empty()) {        // for backward compatibility
        InstrChannel* a = new InstrChannel;
        a->setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
        a->setProgram(program);
        a->setBank(bank);
        a->setVolume(volume);
        a->setPan(pan);
        a->setReverb(reverb);
        a->setChorus(chorus);
        i->appendChannel(a);
    } else if (i->channel(0)->program() < 0) {
        i->channel(0)->setProgram(program);
    }
    if (i->useDrumset()) {
        if (i->channel()[0]->bank() == 0) {
            i->channel()[0]->setBank(128);
        }
    }

    // Fix user bank controller read
    for (InstrChannel* c : i->channel()) {
        if (c->bank() == 0) {
            c->setUserBankController(false);
        }
    }

    // Read single-note dynamics from template
    i->setSingleNoteDynamicsFromTemplate();
}

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

static void readPart(Part* part, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            Staff* staff = Factory::createStaff(part);
            staff->setStaffType(Fraction(0, 1), StaffType());       // will reset later if needed
            ctx.appendStaff(staff);
            readStaff(staff, e, ctx);
        } else if (tag == "Instrument") {
            Instrument* i = part->instrument();
            readInstrument(i, part, e);
            // add string data from MIDI program number, if possible
            if (i->stringData()->strings() == 0
                && i->channel().size() > 0
                && i->drumset() == nullptr) {
                int program = i->channel(0)->program();
                if (program >= 24 && program <= 30) {             // guitars
                    i->setStringData(StringData(19, 6, g_guitarStrings));
                } else if ((program >= 32 && program <= 39) || program == 43) {           // bass / double-bass
                    i->setStringData(StringData(24, 4, g_bassStrings));
                } else if (program == 40) {                       // violin and other treble string instr.
                    i->setStringData(StringData(24, 4, g_violinStrings));
                } else if (program == 41) {                       // viola and other alto string instr.
                    i->setStringData(StringData(24, 4, g_violaStrings));
                } else if (program == 42) {                       // cello and other bass string instr.
                    i->setStringData(StringData(24, 4, g_celloStrings));
                }
            }
            Drumset* d = i->drumset();
            Staff* st = part->staff(0);
            if (d && st && st->lines(Fraction(0, 1)) != 5) {
                int n = 0;
                if (st->lines(Fraction(0, 1)) == 1) {
                    n = 4;
                }
                for (int j = 0; j < DRUM_INSTRUMENTS; ++j) {
                    d->drum(j).line -= n;
                }
            }
        } else if (tag == "name") {
            Text* t = Factory::createText(ctx.dummy(), TextStyleType::DEFAULT, false);
            readText114(e, t, t);
            part->instrument()->setLongName(t->xmlText());
            delete t;
        } else if (tag == "shortName") {
            Text* t = Factory::createText(ctx.dummy(), TextStyleType::DEFAULT, false);
            readText114(e, t, t);
            part->instrument()->setShortName(t->xmlText());
            delete t;
        } else if (tag == "trackName") {
            part->setPartName(e.readText());
        } else if (tag == "show") {
            part->setShow(e.readInt());
        } else {
            e.unknown();
        }
    }
    if (part->partName().isEmpty()) {
        part->setPartName(part->instrument()->trackName());
    }

    if (part->instrument()->useDrumset()) {
        for (Staff* staff : part->staves()) {
            int lines = staff->lines(Fraction(0, 1));
            int bf    = staff->barLineFrom();
            int bt    = staff->barLineTo();
            staff->setStaffType(Fraction(0, 1), *StaffType::getDefaultPreset(StaffGroup::PERCUSSION));

            // this allows 2/3-line percussion staves to keep the double spacing they had in 1.3

            if (lines == 2 || lines == 3) {
                ((StaffType*)(staff->staffType(Fraction(0, 1))))->setLineDistance(Spatium(2.0));
            }

            staff->setLines(Fraction(0, 1), lines);             // this also sets stepOffset
            staff->setBarLineFrom(bf);
            staff->setBarLineTo(bt);
        }
    }
    //set default articulations
    std::vector<MidiArticulation> articulations;
    articulations.push_back(MidiArticulation(u"", u"", 100, 100));
    articulations.push_back(MidiArticulation(u"staccato", u"", 100, 50));
    articulations.push_back(MidiArticulation(u"tenuto", u"", 100, 100));
    articulations.push_back(MidiArticulation(u"sforzato", u"", 120, 100));
    part->instrument()->setArticulation(articulations);
}

//---------------------------------------------------------
//   readPageFormat
//---------------------------------------------------------

static void readPageFormat(PageFormat* pf, XmlReader& e)
{
    double _oddRightMargin  = 0.0;
    double _evenRightMargin = 0.0;
    bool landscape         = false;
    AsciiStringView type;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "landscape") {
            landscape = e.readInt();
        } else if (tag == "page-margins") {
            type = e.asciiAttribute("type", "both");
            double lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                double val = e.readDouble() * 0.5 / PPI;
                if (t == "left-margin") {
                    lm = val;
                } else if (t == "right-margin") {
                    rm = val;
                } else if (t == "top-margin") {
                    tm = val;
                } else if (t == "bottom-margin") {
                    bm = val;
                } else {
                    e.unknown();
                }
            }
            pf->setTwosided(type == "odd" || type == "even");
            if (type == "odd" || type == "both") {
                pf->setOddLeftMargin(lm);
                _oddRightMargin = rm;
                pf->setOddTopMargin(tm);
                pf->setOddBottomMargin(bm);
            }
            if (type == "even" || type == "both") {
                pf->setEvenLeftMargin(lm);
                _evenRightMargin = rm;
                pf->setEvenTopMargin(tm);
                pf->setEvenBottomMargin(bm);
            }
        } else if (tag == "page-height") {
            pf->setSize(SizeF(pf->size().width(), e.readDouble() * 0.5 / PPI));
        } else if (tag == "page-width") {
            pf->setSize(SizeF(e.readDouble() * 0.5 / PPI, pf->size().height()));
        } else if (tag == "pageFormat") {
            const PaperSize* s = getPaperSize114(e.readText());
            pf->setSize(SizeF(s->w, s->h));
        } else if (tag == "page-offset") {
            e.readInt();
        } else {
            e.unknown();
        }
    }
    if (landscape) {
        pf->setSize(pf->size().transposed());
    }
    double w1 = pf->size().width() - pf->oddLeftMargin() - _oddRightMargin;
    double w2 = pf->size().width() - pf->evenLeftMargin() - _evenRightMargin;
    pf->setPrintableWidth(std::min(w1, w2));       // silently adjust right margins
}

//---------------------------------------------------------
//   readStyle
//---------------------------------------------------------

static void readStyle(MStyle* style, XmlReader& e, ReadChordListHook& readChordListHook)
{
    while (e.readNextStartElement()) {
        String tag = String::fromAscii(e.name().ascii());

        if (tag == "lyricsDistance") {          // was renamed
            tag = u"lyricsPosBelow";
        }

        if (tag == "TextStyle") {
            e.skipCurrentElement();
        } else if (tag == "lyricsMinBottomDistance") {
            // no longer meaningful since it is now measured from skyline rather than staff
            //style->set(Sid::lyricsMinBottomDistance, PointF(0.0, y));
            e.skipCurrentElement();
        } else if (tag == "Spatium") {
            style->set(Sid::spatium, e.readDouble() * DPMM);
        } else if (tag == "page-layout") {
            readPageFormat206(style, e);
        } else if (tag == "displayInConcertPitch") {
            style->set(Sid::concertPitch, bool(e.readInt()));
        } else if (tag == "ChordList") {
            readChordListHook.read(e);
        } else if (tag == "pageFillLimit" || tag == "genTimesig" || tag == "FixMeasureNumbers" || tag == "FixMeasureWidth") {   // obsolete
            e.skipCurrentElement();
        } else if (tag == "systemDistance") {  // obsolete
            style->set(Sid::minSystemDistance, e.readDouble());
        } else if (tag == "stemDir") {
            int voice = e.intAttribute("voice", 1) - 1;
            switch (voice) {
            case 0: tag = u"StemDir1";
                break;
            case 1: tag = u"StemDir2";
                break;
            case 2: tag = u"StemDir3";
                break;
            case 3: tag = u"StemDir4";
                break;
            }
        }
        // for compatibility:
        else if (tag == "oddHeader" || tag == "evenHeader" || tag == "oddFooter" || tag == "evenFooter") {
            tag += u"C";
        } else {
            if (!ReadStyleHook::readStyleProperties(style, e)) {
                e.skipCurrentElement();
            }
        }
    }

    readChordListHook.validate();
}

//---------------------------------------------------------
//   read114
//    import old version <= 1.3 files
//---------------------------------------------------------

Err Read114::read114(MasterScore* masterScore, XmlReader& e, ReadContext& ctx)
{
    TempoMap tm;
    while (e.readNextStartElement()) {
        ctx.setTrack(mu::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            readStaffContent(masterScore, e, ctx);
        } else if (tag == "KeySig") {                 // not supported
            KeySig* ks = Factory::createKeySig(masterScore->dummy()->segment());
            ks->read(e);
            delete ks;
        } else if (tag == "siglist") {
            masterScore->_sigmap->read(e, ctx.fileDivision());
        } else if (tag == "programVersion") {
            masterScore->setMscoreVersion(e.readText());
        } else if (tag == "programRevision") {
            masterScore->setMscoreRevision(e.readInt());
        } else if (tag == "Mag"
                   || tag == "MagIdx"
                   || tag == "xoff"
                   || tag == "Symbols"
                   || tag == "cursorTrack"
                   || tag == "yoff") {
            e.skipCurrentElement();             // obsolete
        } else if (tag == "tempolist") {
            // store the tempo list to create invisible tempo text later
            double tempo = e.doubleAttribute("fix", 2.0);
            tm.setTempoMultiplier(tempo);
            while (e.readNextStartElement()) {
                if (e.name() == "tempo") {
                    int tick   = e.attribute("tick").toInt();
                    double tmp = e.readText().toDouble();
                    tick       = ctx.fileDivision(tick);
                    auto pos   = tm.find(tick);
                    if (pos != tm.end()) {
                        tm.erase(pos);
                    }
                    tm.setTempo(tick, tmp);
                } else if (e.name() == "relTempo") {
                    e.readText();
                } else {
                    e.unknown();
                }
            }
        } else if (tag == "playMode") {
            masterScore->setPlayMode(PlayMode(e.readInt()));
        } else if (tag == "SyntiSettings") {
            masterScore->_synthesizerState.read(e);
        } else if (tag == "Spatium") {
            masterScore->setSpatium(e.readDouble() * DPMM);
        } else if (tag == "Division") {
            masterScore->_fileDivision = e.readInt();
        } else if (tag == "showInvisible") {
            masterScore->setShowInvisible(e.readInt());
        } else if (tag == "showFrames") {
            masterScore->setShowFrames(e.readInt());
        } else if (tag == "showMargins") {
            masterScore->setShowPageborders(e.readInt());
        } else if (tag == "Style") {
            double sp = masterScore->spatium();
            compat::ReadChordListHook clhook(masterScore);
            readStyle(&masterScore->style(), e, clhook);
            //style()->load(e);
            // adjust this now so chords render properly on read
            // other style adjustments can wait until reading is finished
            if (masterScore->styleB(Sid::useGermanNoteNames)) {
                masterScore->style().set(Sid::useStandardNoteNames, false);
            }
            if (masterScore->layoutMode() == LayoutMode::FLOAT) {
                // style should not change spatium in
                // float mode
                masterScore->setSpatium(sp);
            }
        } else if (tag == "TextStyle") {
            e.skipCurrentElement();
        } else if (tag == "page-layout") {
            compat::PageFormat pf;
            readPageFormat(&pf, e);
        } else if (tag == "copyright" || tag == "rights") {
            Text* text = Factory::createText(masterScore->dummy(), TextStyleType::DEFAULT, false);
            readText114(e, text, text);
            masterScore->setMetaTag(u"copyright", text->plainText());
            delete text;
        } else if (tag == "movement-number") {
            masterScore->setMetaTag(u"movementNumber", e.readText());
        } else if (tag == "movement-title") {
            masterScore->setMetaTag(u"movementTitle", e.readText());
        } else if (tag == "work-number") {
            masterScore->setMetaTag(u"workNumber", e.readText());
        } else if (tag == "work-title") {
            masterScore->setMetaTag(u"workTitle", e.readText());
        } else if (tag == "source") {
            masterScore->setMetaTag(u"source", e.readText());
        } else if (tag == "metaTag") {
            String name = e.attribute("name");
            masterScore->setMetaTag(name, e.readText());
        } else if (tag == "Part") {
            Part* part = new Part(masterScore);
            readPart(part, e, ctx);
            masterScore->appendPart(part);
        } else if (tag == "Slur") {
            Slur* slur = Factory::createSlur(ctx.dummy());
            Read206::readSlur206(e, ctx, slur);
            ctx.addSpanner(slur);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, masterScore->dummy()));
            if (tag == "Volta") {
                readVolta114(e, ctx, toVolta(s));
            } else if (tag == "Ottava") {
                readOttava114(e, ctx, toOttava(s));
            } else if (tag == "TextLine") {
                readTextLine114(e, ctx, toTextLine(s));
            } else if (tag == "Pedal") {
                readPedal114(e, ctx, toPedal(s));
            } else if (tag == "Trill") {
                Read206::readTrill206(e, toTrill(s));
            } else {
                assert(tag == "HairPin");
                Read206::readHairpin206(e, ctx, toHairpin(s));
            }
            if (s->track() == mu::nidx) {
                s->setTrack(ctx.track());
            } else {
                ctx.setTrack(s->track());               // update current track
            }
            if (s->tick() == Fraction(-1, 1)) {
                s->setTick(ctx.tick());
            } else {
                ctx.setTick(s->tick());              // update current tick
            }
            if (s->track2() == mu::nidx) {
                s->setTrack2(s->track());
            }
            if (s->ticks().isZero()) {
                LOGD("zero spanner %s ticks: %d", s->typeName(), s->ticks().ticks());
                delete s;
            } else {
                masterScore->addSpanner(s);
            }
        } else if (tag == "Excerpt") {
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                Excerpt* ex = new Excerpt(masterScore);
                ex->read(e);
                masterScore->_excerpts.push_back(ex);
            }
        } else if (tag == "Beam") {
            Beam* beam = Factory::createBeam(masterScore->dummy()->system());
            beam->read(e);
            beam->resetExplicitParent();
            // _beams.append(beam);
            delete beam;
        } else {
            e.unknown();
        }
    }

    if (e.error() != XmlStreamReader::NoError) {
        LOGD("%lld %lld: %s ", e.lineNumber(), e.columnNumber(), muPrintable(e.errorString()));
        return Err::FileBadFormat;
    }

    for (Staff* s : masterScore->staves()) {
        size_t idx = s->idx();
        track_idx_t track = idx * VOICES;

        // check barLineSpan
        if (s->barLineSpan() > static_cast<int>(masterScore->nstaves() - idx)) {
            LOGD("read114: invalid barline span %d (max %zu)",
                 s->barLineSpan(), masterScore->nstaves() - idx);
            s->setBarLineSpan(static_cast<int>(masterScore->nstaves() - idx));
        }
        for (auto i : s->clefList()) {
            Fraction tick   = Fraction::fromTicks(i.first);
            ClefType clefId = i.second._concertClef;
            Measure* m      = masterScore->tick2measure(tick);
            if (!m) {
                continue;
            }
            SegmentType st = SegmentType::Clef;
            if (tick == m->tick()) {
                if (m->prevMeasure()) {
                    m = m->prevMeasure();
                } else {
                    st = SegmentType::HeaderClef;
                }
            }
            Segment* seg = m->getSegment(st, tick);
            if (seg->element(track)) {
                seg->element(track)->setGenerated(false);
            } else {
                Clef* clef = Factory::createClef(seg);
                clef->setClefType(clefId);
                clef->setTrack(track);
                clef->setGenerated(false);
                seg->add(clef);
            }
        }

        // create missing KeySig
        KeyList* km = s->keyList();
        for (auto i = km->begin(); i != km->end(); ++i) {
            Fraction tick = Fraction::fromTicks(i->first);
            if (tick < Fraction(0, 1)) {
                LOGD("read114: Key tick %d", tick.ticks());
                continue;
            }
            if (tick.isZero() && i->second.key() == Key::C) {
                continue;
            }
            Measure* m = masterScore->tick2measure(tick);
            if (!m) {               //empty score
                break;
            }
            Segment* seg = m->getSegment(SegmentType::KeySig, tick);
            if (seg->element(track)) {
                toKeySig(seg->element(track))->setGenerated(false);
            } else {
                KeySigEvent ke = i->second;
                KeySig* ks = Factory::createKeySig(seg);
                ks->setKeySigEvent(ke);
                ks->setParent(seg);
                ks->setTrack(track);
                ks->setGenerated(false);
                seg->add(ks);
            }
        }
    }

    for (std::pair<int, Spanner*> p : masterScore->spanner()) {
        Spanner* s = p.second;
        if (!s->isSlur()) {
            if (s->isVolta()) {
                Volta* volta = toVolta(s);
                volta->setAnchor(Spanner::Anchor::MEASURE);
            }
        }

        if (s->isOttava() || s->isPedal() || s->isTrill() || s->isTextLine()) {
            double yo = 0;
            if (s->isOttava()) {
                // fix ottava position
                yo = masterScore->styleValue(Pid::OFFSET, Sid::ottavaPosAbove).value<PointF>().y();
                if (s->placeBelow()) {
                    yo = -yo + s->staff()->height();
                }
            } else if (s->isPedal()) {
                yo = masterScore->styleValue(Pid::OFFSET, Sid::pedalPosBelow).value<PointF>().y();
            } else if (s->isTrill()) {
                yo = masterScore->styleValue(Pid::OFFSET, Sid::trillPosAbove).value<PointF>().y();
            } else if (s->isTextLine()) {
                yo = -5.0 * masterScore->spatium();
            }
            if (!s->spannerSegments().empty()) {
                for (SpannerSegment* seg : s->spannerSegments()) {
                    if (!seg->offset().isNull()) {
                        seg->ryoffset() = seg->offset().y() - yo;
                    }
                }
            } else {
                s->ryoffset() = -yo;
            }
        }
    }

    masterScore->connectTies();

    //
    // remove "middle beam" flags from first ChordRest in
    // measure
    //
    for (Measure* m = masterScore->firstMeasure(); m; m = m->nextMeasure()) {
        size_t tracks = masterScore->nstaves() * VOICES;
        bool first = true;
        for (size_t track = 0; track < tracks; ++track) {
            for (Segment* s = m->first(); s; s = s->next()) {
                if (s->segmentType() != SegmentType::ChordRest) {
                    continue;
                }
                ChordRest* cr = toChordRest(s->element(static_cast<int>(track)));
                if (cr) {
                    if (!first) {
                        switch (cr->beamMode()) {
                        case BeamMode::AUTO:
                        case BeamMode::BEGIN:
                        case BeamMode::END:
                        case BeamMode::NONE:
                            break;
                        case BeamMode::MID:
                        case BeamMode::BEGIN32:
                        case BeamMode::BEGIN64:
                            cr->setBeamMode(BeamMode::BEGIN);
                            break;
                        case BeamMode::INVALID:
                            if (cr->isChord()) {
                                cr->setBeamMode(BeamMode::AUTO);
                            } else {
                                cr->setBeamMode(BeamMode::NONE);
                            }
                            break;
                        }
                        first = false;
                    }
                }
            }
        }
    }
    for (MeasureBase* mb = masterScore->first(); mb; mb = mb->next()) {
        if (mb->isVBox()) {
            VBox* b  = toVBox(mb);
            Millimetre y = masterScore->styleMM(Sid::staffUpperBorder);
            b->setBottomGap(y);
        }
    }

    masterScore->_fileDivision = Constants::division;

    //
    //    sanity check for barLineSpan and update ottavas
    //
    for (Staff* staff : masterScore->staves()) {
        int barLineSpan = staff->barLineSpan();
        staff_idx_t idx = staff->idx();
        size_t n = masterScore->nstaves();
        if (idx + barLineSpan > n) {
            LOGD("bad span: idx %zu  span %d staves %zu", idx, barLineSpan, n);
            staff->setBarLineSpan(static_cast<int>(n - idx));
        }
        staff->updateOttava();
    }

    // adjust some styles
    if (masterScore->styleB(Sid::hideEmptyStaves)) {        // http://musescore.org/en/node/16228
        masterScore->style().set(Sid::dontHideStavesInFirstSystem, false);
    }
    if (masterScore->styleB(Sid::showPageNumberOne)) {      // http://musescore.org/en/node/21207
        masterScore->style().set(Sid::evenFooterL, String(u"$P"));
        masterScore->style().set(Sid::oddFooterR, String(u"$P"));
    }
    if (masterScore->styleI(Sid::minEmptyMeasures) == 0) {
        masterScore->style().set(Sid::minEmptyMeasures, 1);
    }
    masterScore->style().set(Sid::frameSystemDistance, masterScore->styleS(Sid::frameSystemDistance) + Spatium(6.0));
    masterScore->resetStyleValue(Sid::measureSpacing);

    // add invisible tempo text if necessary
    // some 1.3 scores have tempolist but no tempo text
    masterScore->setUpTempoMap();
    for (const auto& i : tm) {
        Fraction tick = Fraction::fromTicks(i.first);
        BeatsPerSecond tempo   = i.second.tempo;
        if (masterScore->tempomap()->tempo(tick.ticks()) != tempo) {
            TempoText* tt = Factory::createTempoText(masterScore->dummy()->segment());
            tt->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(std::round(tempo.toBPM().val)));
            tt->setTempo(tempo);
            tt->setTrack(0);
            tt->setVisible(false);
            Measure* m = masterScore->tick2measure(tick);
            if (m) {
                Segment* seg = m->getSegment(SegmentType::ChordRest, tick);
                seg->add(tt);
                masterScore->setTempo(tick, tempo);
            } else {
                delete tt;
            }
        }
    }

    // create excerpts

    std::vector<Excerpt*> readExcerpts;
    readExcerpts.swap(masterScore->_excerpts);
    for (Excerpt* excerpt : readExcerpts) {
        if (excerpt->parts().empty()) {             // ignore empty parts
            continue;
        }
        if (!excerpt->parts().empty()) {
            masterScore->_excerpts.push_back(excerpt);
            Score* nscore = masterScore->createScore();
            ReadStyleHook::setupDefaultStyle(nscore);
            excerpt->setExcerptScore(nscore);
            nscore->style().set(Sid::createMultiMeasureRests, true);
            Excerpt::createExcerpt(excerpt);
        }
    }

    // volta offsets in older scores are hardcoded to be relative to a voltaY of -2.0sp
    // we'll force this and live with it for the score
    // but we wait until now to do it so parts don't have this issue

    if (masterScore->styleV(Sid::voltaPosAbove) == DefaultStyle::baseStyle().value(Sid::voltaPosAbove)) {
        masterScore->style().set(Sid::voltaPosAbove, PointF(0.0, -2.0f));
    }

    masterScore->setUpTempoMap();

    for (Part* p : masterScore->parts()) {
        p->updateHarmonyChannels(false);
    }

    masterScore->rebuildMidiMapping();
    masterScore->updateChannel();

    for (Score* score : masterScore->scoreList()) {
        CompatUtils::replaceStaffTextWithPlayTechniqueAnnotation(score);
    }

    CompatUtils::assignInitialPartToExcerpts(masterScore->excerpts());

    return Err::NoError;
}
