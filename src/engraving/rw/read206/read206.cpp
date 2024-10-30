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

#include "read206.h"

#include <cmath>

#include "global/defer.h"

#include "compat/pageformat.h"

#include "iengravingfont.h"

#include "rw/compat/compatutils.h"

#include "style/style.h"
#include "style/textstyle.h"

#include "types/symnames.h"
#include "types/typesconv.h"

#include "dom/accidental.h"
#include "dom/ambitus.h"
#include "dom/arpeggio.h"
#include "dom/articulation.h"
#include "dom/audio.h"
#include "dom/barline.h"
#include "dom/beam.h"
#include "dom/bend.h"
#include "dom/box.h"
#include "dom/breath.h"
#include "dom/chord.h"
#include "dom/chordline.h"
#include "dom/drumset.h"
#include "dom/dynamic.h"
#include "dom/excerpt.h"
#include "dom/factory.h"
#include "dom/fermata.h"
#include "dom/fingering.h"
#include "dom/glissando.h"
#include "dom/hairpin.h"
#include "dom/hook.h"
#include "dom/image.h"
#include "dom/keysig.h"
#include "dom/linkedobjects.h"
#include "dom/lyrics.h"
#include "dom/marker.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/measurenumber.h"
#include "dom/measurerepeat.h"
#include "dom/mmrest.h"
#include "dom/noteline.h"
#include "dom/ornament.h"
#include "dom/ottava.h"
#include "dom/page.h"
#include "dom/part.h"
#include "dom/pedal.h"
#include "dom/rehearsalmark.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/sig.h"
#include "dom/slur.h"
#include "dom/spacer.h"
#include "dom/staff.h"
#include "dom/staff.h"
#include "dom/stafftext.h"
#include "dom/stem.h"
#include "dom/stemslash.h"
#include "dom/systemdivider.h"
#include "dom/systemtext.h"
#include "dom/tempotext.h"
#include "dom/textline.h"
#include "dom/tie.h"
#include "dom/timesig.h"
#include "dom/trill.h"
#include "dom/tremolotwochord.h"
#include "dom/tremolosinglechord.h"
#include "dom/tuplet.h"
#include "dom/undo.h"
#include "dom/utils.h"
#include "dom/volta.h"

#include "../compat/readchordlisthook.h"
#include "../compat/readstyle.h"

#include "../read400/tread.h"

#include "log.h"

using namespace mu;
using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rw;
using namespace mu::engraving::read400;
using namespace mu::engraving::read206;
using namespace mu::engraving::compat;

static void readText206(XmlReader& e, ReadContext& ctx, TextBase* t, EngravingItem* be);

//---------------------------------------------------------
//   excessTextStyles206
//    The first map has the name of the style as the string
//    The second map has the mapping of each Sid that the style identifies
//    to the default value for that sid.
//---------------------------------------------------------

static std::map<String, std::map<Sid, PropertyValue> > excessTextStyles206;

void Read206::readTextStyle206(MStyle* style, XmlReader& e, ReadContext& ctx, std::map<String, std::map<Sid, PropertyValue> >& excessStyles)
{
    String family = u"FreeSerif";
    double size = 10;
    bool sizeIsSpatiumDependent = false;
    FontStyle fontStyle = FontStyle::Normal;
    Align align = { AlignH::LEFT, AlignV::TOP };
    PointF offset;
    OffsetType offsetType = OffsetType::SPATIUM;

    FrameType frameType = FrameType::NO_FRAME;
    Spatium paddingWidth(0.0);
    Spatium frameWidth(0.0);
    Color foregroundColor = Color::BLACK;
    Color backgroundColor = Color::transparent;

    PlacementV placement = PlacementV::ABOVE;
    bool placementValid = false;

    String name = e.attribute("name");
    Color frameColor = Color::BLACK;

    bool systemFlag = false;
    double lineWidth = -1.0;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "name") {
            name = e.readText();
        } else if (tag == "family") {
            family = e.readText();
        } else if (tag == "size") {
            size = e.readDouble();
        } else if (tag == "bold") {
            if (e.readInt()) {
                fontStyle = fontStyle + FontStyle::Bold;
            }
        } else if (tag == "italic") {
            if (e.readInt()) {
                fontStyle = fontStyle + FontStyle::Italic;
            }
        } else if (tag == "underline") {
            if (e.readInt()) {
                fontStyle = fontStyle + FontStyle::Underline;
            }
#if 0 // should not happen, but won't harm either
        } else if (tag == "strike") {
            if (e.readInt()) {
                fontStyle = fontStyle + FontStyle::Strike;
            }
#endif
        } else if (tag == "align") {      // obsolete
            e.skipCurrentElement();
        } else if (tag == "anchor") {     // obsolete
            e.skipCurrentElement();
        } else if (tag == "halign") {
            align.horizontal = TConv::fromXml(e.readAsciiText(), AlignH::LEFT);
        } else if (tag == "valign") {
            align.vertical = TConv::fromXml(e.readAsciiText(), AlignV::TOP);
        } else if (tag == "xoffset") {
            double xo = e.readDouble();
            if (offsetType == OffsetType::ABS) {
                xo /= INCH;
            }
            offset.setX(xo);
        } else if (tag == "yoffset") {
            double yo = e.readDouble();
            if (offsetType == OffsetType::ABS) {
                yo /= INCH;
            }
            offset.setY(yo);
        } else if (tag == "rxoffset" || tag == "ryoffset") {         // obsolete
            e.readDouble();
        } else if (tag == "offsetType") {
            const String& val(e.readText());
            OffsetType ot = OffsetType::ABS;
            if (val == "spatium" || val == "1") {
                ot = OffsetType::SPATIUM;
            }
            if (ot != offsetType) {
                offsetType = ot;
                if (ot == OffsetType::ABS) {
                    offset /= INCH;            // convert spatium -> inch
                } else {
                    offset *= INCH;            // convert inch -> spatium
                }
            }
        } else if (tag == "sizeIsSpatiumDependent" || tag == "spatiumSizeDependent") {
            sizeIsSpatiumDependent = e.readInt();
        } else if (tag == "frameWidth") {   // obsolete
            frameType = FrameType::SQUARE;
            /*frameWidthMM =*/ e.readDouble();
        } else if (tag == "frameWidthS") {
            frameType = FrameType::SQUARE;
            frameWidth = Spatium(e.readDouble());
        } else if (tag == "frame") {
            frameType = e.readInt() ? FrameType::SQUARE : FrameType::NO_FRAME;
        } else if (tag == "paddingWidth") {          // obsolete
            /*paddingWidthMM =*/
            e.readDouble();
        } else if (tag == "paddingWidthS") {
            paddingWidth = Spatium(e.readDouble());
        } else if (tag == "frameRound") {
            e.readInt();
        } else if (tag == "frameColor") {
            frameColor = e.readColor();
        } else if (tag == "foregroundColor") {
            foregroundColor = e.readColor();
        } else if (tag == "backgroundColor") {
            backgroundColor = e.readColor();
        } else if (tag == "circle") {
            frameType = e.readInt() ? FrameType::CIRCLE : FrameType::NO_FRAME;
        } else if (tag == "systemFlag") {
            systemFlag = e.readInt();
        } else if (tag == "placement") {
            String value(e.readText());
            if (value == "above") {
                placement = PlacementV::ABOVE;
            } else if (value == "below") {
                placement = PlacementV::BELOW;
            }
            placementValid = true;
        } else if (tag == "lineWidth") {
            lineWidth = e.readDouble();
        } else {
            e.unknown();
        }
    }
    if (family == "MuseJazz") {
        family = u"MuseJazz Text";
    }

    struct StyleTable {
        const char* name;
        TextStyleType ss;
    } styleTable[] = {
        { "",                        TextStyleType::DEFAULT },
        { "Title",                   TextStyleType::TITLE },
        { "Subtitle",                TextStyleType::SUBTITLE },
        { "Composer",                TextStyleType::COMPOSER },
        { "Lyricist",                TextStyleType::LYRICIST },
        { "Lyrics Odd Lines",        TextStyleType::LYRICS_ODD },
        { "Lyrics Even Lines",       TextStyleType::LYRICS_EVEN },
        { "Fingering",               TextStyleType::FINGERING },
        { "LH Guitar Fingering",     TextStyleType::LH_GUITAR_FINGERING },
        { "RH Guitar Fingering",     TextStyleType::RH_GUITAR_FINGERING },
        { "String Number",           TextStyleType::STRING_NUMBER },
        { "Instrument Name (Long)",  TextStyleType::INSTRUMENT_LONG },
        { "Instrument Name (Short)", TextStyleType::INSTRUMENT_SHORT },
        { "Instrument Name (Part)",  TextStyleType::INSTRUMENT_EXCERPT },
        { "Dynamics",                TextStyleType::DYNAMICS },
        { "Technique",               TextStyleType::IGNORED_TYPES },
        { "Tempo",                   TextStyleType::TEMPO },
        { "Metronome",               TextStyleType::METRONOME },
        { "Measure Number",          TextStyleType::MEASURE_NUMBER },
        { "Translator",              TextStyleType::TRANSLATOR },
        { "Tuplet",                  TextStyleType::TUPLET },
        { "System",                  TextStyleType::SYSTEM },
        { "Staff",                   TextStyleType::STAFF },
        { "Chord Symbol",            TextStyleType::HARMONY_A },
        { "Rehearsal Mark",          TextStyleType::REHEARSAL_MARK },
        { "Repeat Text Left",        TextStyleType::REPEAT_LEFT },
        { "Repeat Text Right",       TextStyleType::REPEAT_RIGHT },
        { "Frame",                   TextStyleType::FRAME },
        { "Text Line",               TextStyleType::TEXTLINE },
        { "Glissando",               TextStyleType::GLISSANDO },
        { "Ottava",                  TextStyleType::OTTAVA },
        { "Pedal",                   TextStyleType::PEDAL },
        { "Hairpin",                 TextStyleType::HAIRPIN },
        { "Bend",                    TextStyleType::BEND },
        { "Header",                  TextStyleType::HEADER },
        { "Footer",                  TextStyleType::FOOTER },
        { "Instrument Change",       TextStyleType::INSTRUMENT_CHANGE },
        { "Repeat Text",             TextStyleType::IGNORED_TYPES, },         // Repeat Text style no longer exists
        { "Figured Bass",            TextStyleType::IGNORED_TYPES, },         // F.B. data are in style properties
        { "Volta",                   TextStyleType::VOLTA },
    };
    TextStyleType ss = TextStyleType::TEXT_TYPES;
    for (const auto& i : styleTable) {
        if (name == i.name) {
            ss = i.ss;
            break;
        }
    }

    if (ss == TextStyleType::IGNORED_TYPES) {
        return;
    }

    if (ss == TextStyleType::DEFAULT) {
        // This could be false in older files but newer files must have this true
        sizeIsSpatiumDependent = true;
    }

    bool isExcessStyle = false;
    if (ss == TextStyleType::TEXT_TYPES) {
        ss = ctx.addUserTextStyle(name);
        if (ss == TextStyleType::TEXT_TYPES) {
            LOGD("unhandled substyle <%s>", muPrintable(name));
            isExcessStyle = true;
        } else {
            int idx = int(ss) - int(TextStyleType::USER1);
            if ((int(ss) < int(TextStyleType::USER1)) || (int(ss) > int(TextStyleType::USER12))) {
                LOGD("User style index %d outside of range.", idx);
                return;
            }
            Sid sid[] = { Sid::user1Name, Sid::user2Name, Sid::user3Name, Sid::user4Name, Sid::user5Name, Sid::user6Name,
                          Sid::user7Name, Sid::user8Name, Sid::user9Name, Sid::user10Name, Sid::user11Name, Sid::user12Name };
            style->set(sid[idx], name);
        }
    }

    std::map<Sid, PropertyValue> excessPairs;
    const TextStyle* ts;
    if (isExcessStyle) {
        ts = textStyle(TextStyleType::USER1);
    } else {
        ts = textStyle(ss);
    }
    for (const auto& i : *ts) {
        PropertyValue value;
        if (i.sid == Sid::NOSTYLE) {
            break;
        }
        switch (i.pid) {
        case Pid::TEXT_STYLE:
            value = int(ss);
            break;
        case Pid::BEGIN_FONT_FACE:
        case Pid::CONTINUE_FONT_FACE:
        case Pid::END_FONT_FACE:
        case Pid::FONT_FACE:
            value = family;
            break;
        case Pid::BEGIN_FONT_SIZE:
        case Pid::CONTINUE_FONT_SIZE:
        case Pid::END_FONT_SIZE:
        case Pid::FONT_SIZE:
            value = size;
            break;
        case Pid::BEGIN_FONT_STYLE:
        case Pid::CONTINUE_FONT_STYLE:
        case Pid::END_FONT_STYLE:
        case Pid::FONT_STYLE:
            value = int(fontStyle);
            break;
        case Pid::FRAME_TYPE:
            value = int(frameType);
            break;
        case Pid::FRAME_WIDTH:
            value = frameWidth;
            break;
        case Pid::FRAME_PADDING:
            value = paddingWidth;
            break;
        case Pid::FRAME_FG_COLOR:
            value = PropertyValue::fromValue(frameColor);
            break;
        case Pid::FRAME_BG_COLOR:
            value = PropertyValue::fromValue(backgroundColor);
            break;
        case Pid::SIZE_SPATIUM_DEPENDENT:
            value = sizeIsSpatiumDependent;
            break;
        case Pid::BEGIN_TEXT_ALIGN:
        case Pid::CONTINUE_TEXT_ALIGN:
        case Pid::END_TEXT_ALIGN:
        case Pid::ALIGN:
            value = PropertyValue::fromValue(align);
            break;
        case Pid::SYSTEM_FLAG:
            value = systemFlag;
            break;
        case Pid::BEGIN_HOOK_HEIGHT:
        case Pid::END_HOOK_HEIGHT:
            value = PropertyValue();
            break;
        case Pid::PLACEMENT:
            if (placementValid) {
                value = int(placement);
            }
            break;
        case Pid::LINE_WIDTH:
            if (lineWidth != -1.0) {
                value = Millimetre(lineWidth);
            }
            break;
        default:
//                        LOGD("unhandled property <%s>%d", propertyName(i.pid), int (i.pid));
            break;
        }
        if (value.isValid()) {
            if (isExcessStyle) {
                excessPairs[i.sid] = value;
            } else {
                style->set(i.sid, value);
            }
        }
//            else
//                  LOGD("invalid style value <%s> pid<%s>", MStyle::valueName(i.sid), propertyName(i.pid));
    }

    if (isExcessStyle && excessPairs.size() > 0) {
        excessStyles[name] = excessPairs;
    }
}

void Read206::readAccidental206(Accidental* a, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "bracket") {
            int i = e.readInt();
            if (i == 0 || i == 1) {
                a->setBracket(AccidentalBracket(i));
            }
        } else if (tag == "subtype") {
            String text = e.readText();
            static const std::map<String, AccidentalType> accMap = {
                { u"none",               AccidentalType::NONE },
                { u"sharp",              AccidentalType::SHARP },
                { u"flat",               AccidentalType::FLAT },
                { u"natural",            AccidentalType::NATURAL },
                { u"double sharp",       AccidentalType::SHARP2 },
                { u"double flat",        AccidentalType::FLAT2 },
                { u"flat-slash",         AccidentalType::FLAT_SLASH },
                { u"flat-slash2",        AccidentalType::FLAT_SLASH2 },
                { u"mirrored-flat2",     AccidentalType::MIRRORED_FLAT2 },
                { u"mirrored-flat",      AccidentalType::MIRRORED_FLAT },
                { u"sharp-slash",        AccidentalType::SHARP_SLASH },
                { u"sharp-slash2",       AccidentalType::SHARP_SLASH2 },
                { u"sharp-slash3",       AccidentalType::SHARP_SLASH3 },
                { u"sharp-slash4",       AccidentalType::SHARP_SLASH4 },
                { u"sharp arrow up",     AccidentalType::SHARP_ARROW_UP },
                { u"sharp arrow down",   AccidentalType::SHARP_ARROW_DOWN },
                { u"flat arrow up",      AccidentalType::FLAT_ARROW_UP },
                { u"flat arrow down",    AccidentalType::FLAT_ARROW_DOWN },
                { u"natural arrow up",   AccidentalType::NATURAL_ARROW_UP },
                { u"natural arrow down", AccidentalType::NATURAL_ARROW_DOWN },
                { u"sori",               AccidentalType::SORI },
                { u"koron",              AccidentalType::KORON }
            };
            auto it = accMap.find(text);
            if (it == accMap.end()) {
                LOGD("invalid type %s", muPrintable(text));
                a->setAccidentalType(AccidentalType::NONE);
            } else {
                a->setAccidentalType(it->second);
            }
        } else if (tag == "role") {
            AccidentalRole r = AccidentalRole(e.readInt());
            if (r == AccidentalRole::AUTO || r == AccidentalRole::USER) {
                a->setRole(r);
            }
        } else if (tag == "small") {
            a->setSmall(e.readInt());
        } else if (TRead::readItemProperties(a, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

NoteHeadGroup Read206::convertHeadGroup(int i)
{
    NoteHeadGroup val;
    switch (i) {
    case 1:
        val = NoteHeadGroup::HEAD_CROSS;
        break;
    case 2:
        val = NoteHeadGroup::HEAD_DIAMOND;
        break;
    case 3:
        val = NoteHeadGroup::HEAD_TRIANGLE_DOWN;
        break;
    case 4:
        val = NoteHeadGroup::HEAD_MI;
        break;
    case 5:
        val = NoteHeadGroup::HEAD_SLASH;
        break;
    case 6:
        val = NoteHeadGroup::HEAD_XCIRCLE;
        break;
    case 7:
        val = NoteHeadGroup::HEAD_DO;
        break;
    case 8:
        val = NoteHeadGroup::HEAD_RE;
        break;
    case 9:
        val = NoteHeadGroup::HEAD_FA;
        break;
    case 10:
        val = NoteHeadGroup::HEAD_LA;
        break;
    case 11:
        val = NoteHeadGroup::HEAD_TI;
        break;
    case 12:
        val = NoteHeadGroup::HEAD_SOL;
        break;
    case 13:
        val = NoteHeadGroup::HEAD_BREVIS_ALT;
        break;
    case 0:
    default:
        val = NoteHeadGroup::HEAD_NORMAL;
    }
    return val;
}

static NoteHeadType convertHeadType(int i)
{
    NoteHeadType val;
    switch (i) {
    case 0:
        val = NoteHeadType::HEAD_WHOLE;
        break;
    case 1:
        val = NoteHeadType::HEAD_HALF;
        break;
    case 2:
        val = NoteHeadType::HEAD_QUARTER;
        break;
    case 3:
        val = NoteHeadType::HEAD_BREVIS;
        break;
    default:
        val = NoteHeadType::HEAD_AUTO;
    }
    return val;
}

//---------------------------------------------------------
//   ArticulationNames
//---------------------------------------------------------

static struct ArticulationNames {
    SymId id;
    AsciiStringView name;
} articulationNames[] = {
    { SymId::fermataAbove,              "fermata",                   },
    { SymId::fermataShortAbove,         "shortfermata",              },
    { SymId::fermataLongAbove,          "longfermata",               },
    { SymId::fermataVeryLongAbove,      "verylongfermata",           },
    { SymId::articAccentAbove,          "sforzato",                  },
    { SymId::articStaccatoAbove,        "staccato",                  },
    { SymId::articStaccatissimoAbove,   "staccatissimo",             },
    { SymId::articTenutoAbove,          "tenuto",                    },
    { SymId::articTenutoStaccatoAbove,  "portato",                   },
    { SymId::articMarcatoAbove,         "marcato",                   },
    { SymId::guitarFadeIn,              "fadein",                    },
    { SymId::guitarFadeOut,             "fadeout",                   },
    { SymId::guitarVolumeSwell,         "volumeswell",               },
    { SymId::wiggleSawtooth,            "wigglesawtooth",            },
    { SymId::wiggleSawtoothWide,        "wigglesawtoothwide",        },
    { SymId::wiggleVibratoLargeFaster,  "wigglevibratolargefaster",  },
    { SymId::wiggleVibratoLargeSlowest, "wigglevibratolargeslowest", },
    { SymId::brassMuteOpen,             "ouvert",                    },
    { SymId::brassMuteClosed,           "plusstop",                  },
    { SymId::stringsUpBow,              "upbow",                     },
    { SymId::stringsDownBow,            "downbow",                   },
    { SymId::ornamentTurnInverted,      "reverseturn",               },
    { SymId::ornamentTurn,              "turn",                      },
    { SymId::ornamentTrill,             "trill",                     },
    { SymId::ornamentShortTrill,        "prall",                     },
    { SymId::ornamentMordent,           "mordent",                   },
    { SymId::ornamentTremblement,       "prallprall",                },
    { SymId::ornamentPrallMordent,      "prallmordent",              },
    { SymId::ornamentUpPrall,           "upprall",                   },
    { SymId::ornamentUpMordent,         "upmordent",                 },
    { SymId::ornamentDownMordent,       "downmordent",               },
    { SymId::ornamentPrallDown,         "pralldown",                 },
    { SymId::ornamentPrallUp,           "prallup",                   },
    { SymId::ornamentLinePrall,         "lineprall",                 },
    { SymId::ornamentPrecompSlide,      "schleifer",                 },
    { SymId::pluckedSnapPizzicatoAbove, "snappizzicato",             },
    { SymId::stringsThumbPosition,      "thumb",                     },
    { SymId::luteFingeringRHThumb,      "lutefingeringthumb",        },
    { SymId::luteFingeringRHFirst,      "lutefingering1st",          },
    { SymId::luteFingeringRHSecond,     "lutefingering2nd",          },
    { SymId::luteFingeringRHThird,      "lutefingering3rd",          },

    { SymId::ornamentPrecompMordentUpperPrefix, "downprall" },
    { SymId::ornamentPrecompMordentUpperPrefix, "ornamentDownPrall" },
};

SymId Read206::articulationNames2SymId206(const AsciiStringView& s)
{
    for (auto i : articulationNames) {
        if (i.name == s) {
            return i.id;
        }
    }
    return SymId::noSym;
}

//---------------------------------------------------------
//   readDrumset
//---------------------------------------------------------

static void readDrumset206(Drumset* ds, XmlReader& e)
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
        } else if (tag == "variants") {
            while (e.readNextStartElement()) {
                const AsciiStringView tagv(e.name());
                if (tagv == "variant") {
                    DrumInstrumentVariant div;
                    div.pitch = e.attribute("pitch").toInt();
                    while (e.readNextStartElement()) {
                        const AsciiStringView taga(e.name());
                        if (taga == "articulation") {
                            AsciiStringView oldArticulationName = e.readAsciiText();
                            SymId oldId = Read206::articulationNames2SymId206(oldArticulationName);
                            div.articulationName = Articulation::symId2ArticulationName(oldId);
                        } else if (taga == "tremolo") {
                            div.tremolo = TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO);
                        }
                    }
                    ds->drum(pitch).addVariant(div);
                }
            }
        } else if (ds->readProperties(e, pitch)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readInstrument
//---------------------------------------------------------

static void readInstrument206(Instrument* i, Part* p, XmlReader& e, ReadContext& ctx)
{
    int bank    = 0;
    char volume  = 100;
    char pan     = 60;
    char chorus  = 30;
    char reverb  = 30;
    bool customDrumset = false;
    i->clearChannels();         // remove default channel
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Drum") {
            // if we see one of this tags, a custom drumset will
            // be created
            if (!i->drumset()) {
                i->setDrumset(new Drumset(*smDrumset));
            }
            if (!customDrumset) {
                i->drumset()->clear();
                customDrumset = true;
            }
            readDrumset206(i->drumset(), e);
        } else if (read400::TRead::readProperties(i, e, ctx, p, &customDrumset)) {
        } else {
            e.unknown();
        }
    }

    i->updateInstrumentId();

    // Read single-note dynamics from template
    i->setSingleNoteDynamicsFromTemplate();

    if (i->channel().empty()) {        // for backward compatibility
        InstrChannel* a = new InstrChannel;
        a->setName(String::fromUtf8(InstrChannel::DEFAULT_NAME));
        a->setProgram(i->recognizeMidiProgram());
        a->setBank(bank);
        a->setVolume(volume);
        a->setPan(pan);
        a->setReverb(reverb);
        a->setChorus(chorus);
        i->appendChannel(a);
    } else if (i->channel(0)->program() < 0) {
        i->channel(0)->setProgram(i->recognizeMidiProgram());
    }
    if (i->useDrumset()) {
        if (i->channel()[0]->bank() == 0) {
            i->channel()[0]->setBank(128);
        }
    }
}

//---------------------------------------------------------
//   readStaff
//---------------------------------------------------------

static void readStaff(Staff* staff, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "type") {        // obsolete
            int staffTypeIdx = e.readInt();
            LOGD("obsolete: Staff::read staffTypeIdx %d", staffTypeIdx);
        } else if (tag == "neverHide") {
            bool v = e.readInt();
            if (v) {
                staff->setHideWhenEmpty(Staff::HideMode::NEVER);
            }
        } else if (tag == "barLineSpan") {
            staff->setBarLineFrom(e.intAttribute("from", 0));
            staff->setBarLineTo(e.intAttribute("to", 0));
            int span     = e.readInt();
            staff->setBarLineSpan(span - 1);
        } else if (read400::TRead::readProperties(staff, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readPart
//---------------------------------------------------------

void Read206::readPart206(Part* part, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Instrument") {
            Instrument* i = part->m_instruments.instrument(/* tick */ -1);
            readInstrument206(i, part, e, ctx);
            Drumset* ds = i->drumset();
            Staff* s = part->staff(0);
            int lld = s ? std::round(s->lineDistance(Fraction(0, 1))) : 1;
            if (ds && s && lld > 1) {
                for (int j = 0; j < DRUM_INSTRUMENTS; ++j) {
                    ds->drum(j).line /= lld;
                }
            }
        } else if (tag == "Staff") {
            Staff* staff = Factory::createStaff(part);
            ctx.appendStaff(staff);
            readStaff(staff, e, ctx);
        } else if (TRead::readProperties(part, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readAmbitus
//---------------------------------------------------------

static void readAmbitus(Ambitus* ambitus, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "head") {
            ambitus->setNoteHeadGroup(Read206::convertHeadGroup(e.readInt()));
        } else if (tag == "headType") {
            ambitus->setNoteHeadType(convertHeadType(e.readInt()));
        } else if (read400::TRead::readProperties(ambitus, e, ctx)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readNote
//---------------------------------------------------------

static void readNote206(Note* note, XmlReader& e, ReadContext& ctx)
{
    note->setTpc1(Tpc::TPC_INVALID);
    note->setTpc2(Tpc::TPC_INVALID);

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Accidental") {
            Accidental* a = Factory::createAccidental(note);
            a->setTrack(note->track());
            Read206::readAccidental206(a, e, ctx);
            note->add(a);
        } else if (tag == "head") {
            int i = e.readInt();
            NoteHeadGroup val = Read206::convertHeadGroup(i);
            note->setHeadGroup(val);
        } else if (tag == "headType") {
            int i = e.readInt();
            NoteHeadType val = convertHeadType(i);
            note->setHeadType(val);
        } else if (Read206::readNoteProperties206(note, e, ctx)) {
        } else {
            e.unknown();
        }
    }
    // ensure sane values:
    note->setPitch(std::clamp(note->pitch(), 0, 127));

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
        Interval v = note->staff() ? note->staff()->transpose(tick) : Interval();
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
}

//---------------------------------------------------------
//   adjustPlacement
//---------------------------------------------------------

static void adjustPlacement(EngravingItem* e)
{
    if (!e || !e->staff()) {
        return;
    }

    // element to use to determine placement
    // for spanners, choose first segment
    EngravingItem* ee;
    Spanner* spanner;
    if (e->isSpanner()) {
        spanner = toSpanner(e);
        if (spanner->spannerSegments().empty()) {
            return;
        }
        ee = spanner->spannerSegments().front();
        if (!ee) {
            return;
        }
    } else {
        spanner = nullptr;
        ee = e;
    }

    // determine placement based on offset
    // anything below staff will be set to below
    double staffHeight = e->staff()->staffHeight();
    double threshold = staffHeight;
    double offsetAdjust = 0.0;
    PlacementV defaultPlacement = e->propertyDefault(Pid::PLACEMENT).value<PlacementV>();
    PlacementV newPlacement;
    // most offsets will be recorded as relative to top staff line
    // exceptions are styled offsets on elements with default placement below
    double normalize;
    if (defaultPlacement == PlacementV::BELOW && ee->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
        normalize = staffHeight;
    } else {
        normalize = 0.0;
    }
    double ypos = ee->offset().y() + normalize;
    if (ypos >= threshold) {
        newPlacement = PlacementV::BELOW;
        offsetAdjust -= staffHeight;
    } else {
        newPlacement = PlacementV::ABOVE;
    }

    // set placement
    e->setProperty(Pid::PLACEMENT, int(newPlacement));
    if (newPlacement != defaultPlacement) {
        e->setPropertyFlags(Pid::PLACEMENT, PropertyFlags::UNSTYLED);
    }

    // adjust offset
    if (spanner) {
        // adjust segments individually
        for (auto a : spanner->spannerSegments()) {
            // spanner segments share the placement setting of the spanner
            // just adjust offset
            if (defaultPlacement == PlacementV::BELOW && a->propertyFlags(Pid::OFFSET) == PropertyFlags::STYLED) {
                normalize = staffHeight;
            } else {
                normalize = 0.0;
            }
            double yp = a->offset().y() + normalize;
            a->ryoffset() += normalize + offsetAdjust;

            // if any segments are offset to opposite side of staff from placement,
            // or if they are within staff,
            // disable autoplace
            bool disableAutoplace;
            if (yp + a->height() <= 0.0) {
                disableAutoplace = (newPlacement == PlacementV::BELOW);
            } else if (yp > staffHeight) {
                disableAutoplace = (newPlacement == PlacementV::ABOVE);
            } else {
                disableAutoplace = true;
            }
            if (disableAutoplace) {
                a->setAutoplace(false);
            }
            // needed for https://musescore.org/en/node/281312
            // ideally we would rebase and calculate new offset
            // but this may not be possible
            // since original offset is relative to system
            a->rxoffset() = 0;
        }
    } else {
        e->ryoffset() += normalize + offsetAdjust;
        // if within staff, disable autoplace
        if (ypos + e->height() > 0.0 && ypos <= staffHeight) {
            e->setAutoplace(false);
        }
    }
}

bool Read206::readNoteProperties206(Note* note, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "pitch") {
        note->setPitch(e.readInt());
    } else if (tag == "tpc") {
        const int tpc = e.readInt();
        note->setTpc1(tpc);
        note->setTpc2(tpc);
    } else if (tag == "track") {          // for performance
        note->setTrack(e.readInt());
    } else if (tag == "Accidental") {
        Accidental* a = Factory::createAccidental(note);
        a->setTrack(note->track());
        read400::TRead::read(a, e, ctx);
        note->add(a);
    } else if (tag == "Tie") {
        Tie* tie = Factory::createTie(note);
        tie->setParent(note);
        tie->setTrack(note->track());
        readTie206(e, ctx, tie);
        tie->setStartNote(note);
        note->setTieFor(tie);
    } else if (tag == "tpc2") {
        note->setTpc2(e.readInt());
    } else if (tag == "small") {
        note->setSmall(e.readInt());
    } else if (tag == "mirror") {
        read400::TRead::readProperty(note, e, ctx, Pid::MIRROR_HEAD);
    } else if (tag == "dotPosition") {
        read400::TRead::readProperty(note, e, ctx, Pid::DOT_POSITION);
    } else if (tag == "fixed") {
        note->setFixed(e.readBool());
    } else if (tag == "fixedLine") {
        note->setFixedLine(e.readInt());
    } else if (tag == "head") {
        read400::TRead::readProperty(note, e, ctx, Pid::HEAD_GROUP);
    } else if (tag == "velocity") {
        // TODO: convert to MU4
        e.skipCurrentElement();
    } else if (tag == "play") {
        note->setPlay(e.readInt());
    } else if (tag == "tuning") {
        note->setTuning(e.readDouble());
    } else if (tag == "fret") {
        note->setFret(e.readInt());
    } else if (tag == "string") {
        note->setString(e.readInt());
    } else if (tag == "ghost") {
        note->setGhost(e.readInt());
    } else if (tag == "headType") {
        read400::TRead::readProperty(note, e, ctx, Pid::HEAD_TYPE);
    } else if (tag == "veloType") {
        read400::TRead::readProperty(note, e, ctx, Pid::VELO_TYPE);
    } else if (tag == "line") {
        note->setLine(e.readInt());
    } else if (tag == "Fingering") {
        Fingering* f = Factory::createFingering(note);
        f->setTrack(note->track());
        readText206(e, ctx, f, note);
        note->add(f);
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(note);
        s->setTrack(note->track());
        read400::TRead::read(s, e, ctx);
        note->add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(note);
            image->setTrack(note->track());
            read400::TRead::read(image, e, ctx);
            note->add(image);
        }
    } else if (tag == "Bend") {
        Bend* b = Factory::createBend(note);
        b->setTrack(note->track());
        read400::TRead::read(b, e, ctx);
        note->add(b);
    } else if (tag == "NoteDot") {
        NoteDot* dot = Factory::createNoteDot(note);
        read400::TRead::read(dot, e, ctx);
        note->add(dot);
    } else if (tag == "Events") {
        note->playEvents().clear();        // remove default event
        while (e.readNextStartElement()) {
            const AsciiStringView etag(e.name());
            if (etag == "Event") {
                NoteEvent ne;
                read400::TRead::read(&ne, e, ctx);
                note->playEvents().push_back(ne);
            } else {
                e.unknown();
            }
        }
        if (Chord* ch = note->chord()) {
            ch->setPlayEventType(PlayEventType::User);
        }
    } else if (tag == "endSpanner") {
        int id = e.intAttribute("id");
        Spanner* sp = ctx.findSpanner(id);
        if (sp) {
            sp->setEndElement(note);
            if (sp->isTie()) {
                note->setTieBack(toTie(sp));
            } else {
                bool isNoteAnchoredTextLine = sp->isNoteLine() && toNoteLine(sp)->enforceMinLength();
                if ((sp->isGlissando() || isNoteAnchoredTextLine) && note->explicitParent() && note->explicitParent()->isChord()) {
                    toChord(note->explicitParent())->setEndsNoteAnchoredLine(true);
                }
                note->addSpannerBack(sp);
            }
            ctx.removeSpanner(sp);
        } else {
            // End of a spanner whose start element will appear later;
            // may happen for cross-staff spanner from a lower to a higher staff
            // (for instance a glissando from bass to treble staff of piano).
            // Create a place-holder spanner with end data
            // (a TextLine is used only because both Spanner or SLine are abstract,
            // the actual class does not matter, as long as it is derived from Spanner)
            int id1 = e.intAttribute("id", -1);
            Staff* staff = note->staff();
            if (id1 != -1
                &&            // DISABLE if pasting into a staff with linked staves
                              // because the glissando is not properly cloned into the linked staves
                staff && (!ctx.pasteMode() || !staff->links() || staff->links()->empty())) {
                Spanner* placeholder = new TextLine(ctx.dummy());
                placeholder->setAnchor(Spanner::Anchor::NOTE);
                placeholder->setEndElement(note);
                placeholder->setTrack2(note->track());
                placeholder->setTick(Fraction(0, 1));
                placeholder->setTick2(ctx.tick());
                ctx.addSpanner(id1, placeholder);
            }
        }
        e.readNext();
    } else if (tag == "TextLine"
               || tag == "Glissando") {
        Spanner* sp = toSpanner(Factory::createItemByName(tag, ctx.dummy()));
        // check this is not a lower-to-higher cross-staff spanner we already got
        int id = e.intAttribute("id");
        Spanner* placeholder = ctx.findSpanner(id);
        if (placeholder && placeholder->endElement()) {
            // if it is, fill end data from place-holder
            sp->setAnchor(Spanner::Anchor::NOTE);                 // make sure we can set a Note as end element
            sp->setEndElement(placeholder->endElement());
            sp->setTrack2(placeholder->track2());
            sp->setTick(ctx.tick());                                // make sure tick2 will be correct
            sp->setTick2(placeholder->tick2());
            toNote(placeholder->endElement())->addSpannerBack(sp);
            // remove no longer needed place-holder before reading the new spanner,
            // as reading it also adds it to XML reader list of spanners,
            // which would overwrite the place-holder
            ctx.removeSpanner(placeholder);
            delete placeholder;
        }
        sp->setTrack(note->track());
        read400::TRead::readItem(sp, e, ctx);
        Staff* staff = note->staff();
        // DISABLE pasting of glissandi into staves with other linked staves
        // because the glissando is not properly cloned into the linked staves
        if (ctx.pasteMode() && staff && staff->links() && !staff->links()->empty()) {
            ctx.removeSpanner(sp);          // read() added the element to the XMLReader: remove it
            delete sp;
        } else {
            sp->setAnchor(Spanner::Anchor::NOTE);
            sp->setStartElement(note);
            sp->setTick(ctx.tick());
            note->addSpannerFor(sp);
            sp->setParent(note);
            adjustPlacement(sp);
        }
    } else if (tag == "offset") {
        TRead::readItemProperties(note, e, ctx);
    } else if (TRead::readItemProperties(note, e, ctx)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   ReadStyleName206
//    Retrieve the content of the "style" tag from the
//    String with the content of the whole text tag
//---------------------------------------------------------

static String ReadStyleName206(String xmlTag)
{
    size_t beginIdx = xmlTag.indexOf(u"<style>");
    if (beginIdx == muse::nidx) {
        return String();
    }
    beginIdx += String(u"<style>").size();

    size_t endIdx = xmlTag.indexOf(u"</style>");
    IF_ASSERT_FAILED(endIdx > 0) {
        return String();
    }

    String s = xmlTag.mid(beginIdx, (endIdx - beginIdx));
    return s;
}

//---------------------------------------------------------
//   readTextPropertyStyle206
//    This reads only the 'style' tag, so that it can be read
//    before setting anything else.
//---------------------------------------------------------

static bool readTextPropertyStyle206(String xmlTag, ReadContext& ctx, TextBase* t, EngravingItem* be)
{
    String s = ReadStyleName206(xmlTag);

    if (s.isEmpty()) {
        return false;
    }

    if (!be->isTuplet()) {        // Hack
        if (excessTextStyles206.find(s) != excessTextStyles206.end()) {
            // Init the text with a style that can't be stored as a user style
            // due to the limit on the number of user styles possible.
            // Use User-1, since it has all the possible user style pids
            t->initTextStyleType(TextStyleType::DEFAULT);
            std::map<Sid, PropertyValue> styleVals = excessTextStyles206[s];
            for (const auto& p : *textStyle(TextStyleType::USER1)) {
                if (t->getProperty(p.pid) == t->propertyDefault(p.pid) && styleVals.find(p.sid) != styleVals.end()) {
                    t->setProperty(p.pid, styleVals[p.sid]);
                }
            }
        } else {
            TextStyleType ss;
            ss = ctx.lookupUserTextStyle(s);
            if (ss == TextStyleType::TEXT_TYPES) {
                muse::ByteArray ba = s.toAscii();
                ss = TConv::fromXml(ba.constChar(), TextStyleType::DEFAULT);
            }
            if (ss != TextStyleType::TEXT_TYPES) {
                t->initTextStyleType(ss);
            }
        }
    }

    return true;
}

//---------------------------------------------------------
//   readTextProperties206
//---------------------------------------------------------

static bool readTextProperties206(XmlReader& e, ReadContext& ctx, TextBase* t)
{
    const AsciiStringView tag(e.name());
    if (tag == "style") {
        e.skipCurrentElement();     // read in readTextPropertyStyle206
    } else if (tag == "foregroundColor") { // same as "color" ?
        e.skipCurrentElement();
    } else if (tag == "frame") {
        t->setFrameType(e.readBool() ? FrameType::SQUARE : FrameType::NO_FRAME);
        t->setPropertyFlags(Pid::FRAME_TYPE, PropertyFlags::UNSTYLED);
    } else if (tag == "frameRound") {
        read400::TRead::readProperty(t, e, ctx, Pid::FRAME_ROUND);
    } else if (tag == "circle") {
        if (e.readBool()) {
            t->setFrameType(FrameType::CIRCLE);
        } else {
            if (t->circle()) {
                t->setFrameType(FrameType::SQUARE);
            }
        }
        t->setPropertyFlags(Pid::FRAME_TYPE, PropertyFlags::UNSTYLED);
    } else if (tag == "paddingWidthS") {
        read400::TRead::readProperty(t, e, ctx, Pid::FRAME_PADDING);
    } else if (tag == "frameWidthS") {
        read400::TRead::readProperty(t, e, ctx, Pid::FRAME_WIDTH);
    } else if (tag == "frameColor") {
        read400::TRead::readProperty(t, e, ctx, Pid::FRAME_FG_COLOR);
    } else if (tag == "backgroundColor") {
        read400::TRead::readProperty(t, e, ctx, Pid::FRAME_BG_COLOR);
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
    } else if (tag == "pos") {
        read400::TRead::readProperty(t, e, ctx, Pid::OFFSET);
        if (t->align() == AlignV::TOP) {
            t->ryoffset() += .5 * ctx.spatium();           // HACK: bbox is different in 2.x
        }
        adjustPlacement(t);
    } else if (!read400::TRead::readTextProperties(t, e, ctx)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   TextReaderContext206
//    For 2.x files, the style tag could be in a different
//    position with respect to 3.x files. Since seek
//    position is not reliable for readline in QIODevices (for
//    example because of non-single-byte characters in at least
//    one of the fields; some two-byte characters are counted as
//    two single-byte characters and thus the reading could
//    start at the wrong position), a copy of the text tag
//    is created and read in a separate XmlReader, while
//    the text style is extracted from a String containing
//    the whole text xml tag.
//    TextReaderContext206 takes care of this process
//---------------------------------------------------------

class TextReaderContext206
{
    XmlReader& origReader;
    XmlReader tagReader;
    String xmlTag;

public:
    TextReaderContext206(XmlReader& e)
        : origReader(e)
    {
        // Create a new xml document containing only the (text) xml chunk
        String name = String::fromAscii(origReader.name().ascii());
        int64_t additionalLines = origReader.lineNumber() - 2;     // Subtracting the 2 new lines that will be added
        xmlTag = origReader.readXml();
        xmlTag.prepend(u"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<" + name + u">");
        xmlTag.append(u"</" + name + u">\n");
        muse::ByteArray data = xmlTag.toUtf8();
        tagReader.setData(data);  // Add the xml data to the XmlReader
        // the additional lines are needed to output the correct line number
        // of the original file in case of error
        tagReader.setOffsetLines(additionalLines);
        copyProperties(origReader, tagReader);
        tagReader.readNextStartElement();     // read up to the first "name" tag
    }

    // Disable copying the TextReaderContext206
    TextReaderContext206(const TextReaderContext206&) = delete;
    TextReaderContext206& operator=(const TextReaderContext206&) = delete;

    ~TextReaderContext206()
    {
        // Ensure to copy back the potentially changed properties
        // to the original XmlReader before destruction
        copyProperties(tagReader, origReader);
    }

    XmlReader& reader() { return tagReader; }
    const String& tag() { return xmlTag; }
private:
    void copyProperties(XmlReader& original, XmlReader& derived);
};

//---------------------------------------------------------
//   copyProperties
//    Copy some of the XmlReader properties of an original
//    XmlReader object to a "derived" XmlReader object.
//    This is used for example when using the derived XmlReader
//    to read the element properties of a 2.x text, or to
//    update the base XmlReader object (for example, its
//    link list) after reading the properties of a 2.x text
//---------------------------------------------------------

void TextReaderContext206::copyProperties(XmlReader& original, XmlReader& derived)
{
    derived.setDocName(original.docName());
}

//---------------------------------------------------------
//   readText206
//---------------------------------------------------------

static void readText206(XmlReader& e, ReadContext& ctx, TextBase* t, EngravingItem* be)
{
    TextReaderContext206 tctx(e);
    readTextPropertyStyle206(tctx.tag(), ctx, t, be);
    while (tctx.reader().readNextStartElement()) {
        if (!readTextProperties206(tctx.reader(), ctx, t)) {
            tctx.reader().unknown();
        }
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

static void readTempoText(TempoText* t, XmlReader& e, ReadContext& ctx)
{
    TextReaderContext206 tctx(e);
    readTextPropertyStyle206(tctx.tag(), ctx, t, t);
    while (tctx.reader().readNextStartElement()) {
        const AsciiStringView tag(tctx.reader().name());
        if (tag == "tempo") {
            t->setTempo(tctx.reader().readDouble());
        } else if (tag == "followText") {
            t->setFollowText(tctx.reader().readInt());
        } else if (!readTextProperties206(tctx.reader(), ctx, t)) {
            tctx.reader().unknown();
        }
    }
    // check sanity
    if (t->xmlText().isEmpty()) {
        t->setXmlText(String(u"<sym>metNoteQuarterUp</sym> = %1").arg(int(lrint(t->tempo().toBPM().val))));
        t->setVisible(false);
    } else {
        t->setXmlText(t->xmlText().replace(u"<sym>unicode", u"<sym>met"));
    }
}

//---------------------------------------------------------
//   readMarker
//---------------------------------------------------------

static void readMarker(Marker* m, XmlReader& e, ReadContext& ctx)
{
    TextReaderContext206 tctx(e);
    readTextPropertyStyle206(tctx.tag(), ctx, m, m);
    MarkerType mt = MarkerType::SEGNO;

    while (tctx.reader().readNextStartElement()) {
        const AsciiStringView tag(tctx.reader().name());
        if (tag == "label") {
            AsciiStringView s(tctx.reader().readAsciiText());
            m->setLabel(String::fromAscii(s.ascii()));
            mt = TConv::fromXml(s, MarkerType::USER);
        } else if (!readTextProperties206(tctx.reader(), ctx, m)) {
            tctx.reader().unknown();
        }
    }
    m->setMarkerType(mt);
}

//---------------------------------------------------------
//   readDynamic
//---------------------------------------------------------

static void readDynamic(Dynamic* d, XmlReader& e, ReadContext& ctx)
{
    TextReaderContext206 tctx(e);
    readTextPropertyStyle206(tctx.tag(), ctx, d, d);
    while (tctx.reader().readNextStartElement()) {
        const AsciiStringView tag = tctx.reader().name();
        if (tag == "subtype") {
            d->setDynamicType(tctx.reader().readText());
        } else if (tag == "velocity") {
            d->setVelocity(tctx.reader().readInt());
        } else if (tag == "dynType") {
            d->setDynRange(TConv::fromXml(tctx.reader().readAsciiText(), DynamicRange::STAFF));
        } else if (tag == "size") {
            e.skipCurrentElement();
        } else if (!readTextProperties206(tctx.reader(), ctx, d)) {
            tctx.reader().unknown();
        }
    }
    d->setSize(10.0);
    if (d->xmlText().contains(u"<sym>") && !d->xmlText().contains(u"<font")) {
        d->setAlign(Align(AlignH::HCENTER, AlignV::BASELINE));
    }
}

//---------------------------------------------------------
//   readTuplet
//---------------------------------------------------------

static void readTuplet206(Tuplet* tuplet, XmlReader& e, ReadContext& ctx)
{
    tuplet->setId(e.intAttribute("id", 0));
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Number") {
            Text* _number = Factory::createText(tuplet);
            _number->setParent(tuplet);
            _number->setComposition(true);
            tuplet->setNumber(_number);
            // _number reads property defaults from parent tuplet as "composition" is set:
            tuplet->resetNumberProperty();
            readText206(e, ctx, _number, tuplet);
            _number->setVisible(tuplet->visible());           //?? override saved property
            _number->setColor(tuplet->color());
            _number->setTrack(tuplet->track());
            // move property flags from _number
            for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN, Pid::SIZE_SPATIUM_DEPENDENT }) {
                tuplet->setPropertyFlags(p, _number->propertyFlags(p));
            }
        } else if (!Read206::readTupletProperties206(e, ctx, tuplet)) {
            e.unknown();
        }
    }
    Fraction r = (tuplet->ratio() == Fraction(1, 1)) ? tuplet->ratio() : tuplet->ratio().reduced();
    Fraction f(r.denominator(), tuplet->baseLen().fraction().denominator());
    tuplet->setTicks(f.reduced());
}

//---------------------------------------------------------
//   readLyrics
//---------------------------------------------------------

static void readLyrics(Lyrics* lyrics, XmlReader& e, ReadContext& ctx)
{
    int iEndTick = 0;               // used for backward compatibility
    Text* _verseNumber = 0;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "endTick") {
            // store <endTick> tag value until a <ticks> tag has been read
            // which positions this lyrics element in the score
            iEndTick = e.readInt();
        } else if (tag == "Number") {
            _verseNumber = Factory::createText(lyrics);
            readText206(e, ctx, _verseNumber, lyrics);
            _verseNumber->setParent(lyrics);
        } else if (tag == "style") {
            e.readText();          // ignore style
        } else if (!read400::TRead::readProperties(lyrics, e, ctx)) {
            e.unknown();
        }
    }

    // if any endTick, make it relative to current tick
    if (iEndTick) {
        lyrics->setTicks(Fraction::fromTicks(iEndTick) - ctx.tick());
    }
    if (_verseNumber) {
        // TODO: add text to main text
        delete _verseNumber;
    }
    lyrics->setAutoplace(true);
    if (!lyrics->isStyled(Pid::OFFSET) && !ctx.pasteMode()) {
        // fix offset for pre-3.1 scores
        // 2.x and earlier: y offset was relative to staff; x offset was relative to center of notehead
        lyrics->rxoffset() -= lyrics->symWidth(SymId::noteheadBlack) * 0.5;
        //lyrics->ryoffset() -= lyrics->placeBelow() && lyrics->staff() ? lyrics->staff()->height() : 0.0;
        // temporarily set placement to above, since the original offset is relative to top of staff
        // depend on adjustPlacement() to change the placement if appropriate
        lyrics->setPlacement(PlacementV::ABOVE);
        adjustPlacement(lyrics);
    }
}

bool Read206::readDurationProperties206(XmlReader& e, ReadContext& ctx, DurationElement* de)
{
    if (e.name() == "Tuplet") {
        int i = e.readInt();
        Tuplet* t = ctx.findTuplet(i);
        if (!t) {
            LOGD("readDurationProperties206(): Tuplet id %d not found", i);
        }
        if (t) {
            de->setTuplet(t);
            if (!ctx.undoStackActive()) {         // HACK, also added in Undo::AddElement()
                t->add(de);
            }
        }
        return true;
    } else if (TRead::readItemProperties(de, e, ctx)) {
        return true;
    }
    return false;
}

bool Read206::readTupletProperties206(XmlReader& e, ReadContext& ctx, Tuplet* de)
{
    const AsciiStringView tag(e.name());

    if (read400::TRead::readStyledProperty(de, tag, e, ctx)) {
    } else if (tag == "normalNotes") {
        de->setProperty(Pid::NORMAL_NOTES, e.readInt());
    } else if (tag == "actualNotes") {
        de->setProperty(Pid::ACTUAL_NOTES, e.readInt());
    } else if (tag == "p1") {
        de->setProperty(Pid::P1, PropertyValue::fromValue(e.readPoint() * ctx.spatium()));
    } else if (tag == "p2") {
        de->setProperty(Pid::P2, PropertyValue::fromValue(e.readPoint() * ctx.spatium()));
    } else if (tag == "baseNote") {
        de->setBaseLen(TDuration(TConv::fromXml(e.readAsciiText(), DurationType::V_INVALID)));
    } else if (tag == "Number") {
        Text* _number = Factory::createText(de);
        de->setNumber(_number);
        _number->setComposition(true);
        _number->setParent(de);
//            _number->setSubStyleId(SubStyleId::TUPLET);
//            initSubStyle(SubStyleId::TUPLET);   // hack: initialize number
        for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN }) {
            _number->resetProperty(p);
        }
        readText206(e, ctx, _number, de);
        _number->setVisible(de->visible());         //?? override saved property
        _number->setTrack(de->track());
        // move property flags from _number
        for (auto p : { Pid::FONT_FACE, Pid::FONT_SIZE, Pid::FONT_STYLE, Pid::ALIGN }) {
            de->setPropertyFlags(p, _number->propertyFlags(p));
        }
    } else if (!readDurationProperties206(e, ctx, de)) {
        return false;
    }
    return true;
}

bool Read206::readChordRestProperties206(XmlReader& e, ReadContext& ctx, ChordRest* ch)
{
    const AsciiStringView tag(e.name());

    if (tag == "durationType") {
        ch->setDurationType(TConv::fromXml(e.readAsciiText(), DurationType::V_QUARTER));
        if (ch->actualDurationType().type() != DurationType::V_MEASURE) {
            if (ctx.mscVersion() < 112 && (ch->type() == ElementType::REST)
                &&            // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                ch->ticks().numerator() != 0
                &&            // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                (ch->actualDurationType() == DurationType::V_WHOLE && ch->ticks() <= Fraction(4, 4))) {
                // old pre 2.0 scores: convert
                ch->setDurationType(DurationType::V_MEASURE);
            } else {    // not from old score: set duration fraction from duration type
                ch->setTicks(ch->actualDurationType().fraction());
            }
        }
    } else if (tag == "BeamMode") {
        // 206 used begin32/begin64 for beam mode
        // 410 uses begin16/begin32
        auto txt = e.readAsciiText();
        if (txt == "begin64") {
            txt = "begin32";
        } else if (txt == "begin32") {
            txt = "begin16";
        }
        ch->setBeamMode(TConv::fromXml(txt, BeamMode::AUTO));
    } else if (tag == "Articulation") {
        EngravingItem* el = readArticulation(ch, e, ctx);
        if (el->isFermata()) {
            ch->segment()->add(el);
        } else {
            ch->add(el);
        }
    } else if (tag == "leadingSpace" || tag == "trailingSpace") {
        LOGD("ChordRest: %s obsolete", tag.ascii());
        e.skipCurrentElement();
    } else if (tag == "Beam") {
        int id = e.readInt();
        Beam* beam = ctx.findBeam(id);
        if (beam) {
            beam->add(ch);              // also calls ch->setBeam(beam)
        } else {
            LOGD("Beam id %d not found", id);
        }
    } else if (tag == "small") {
        ch->setSmall(e.readInt());
    } else if (tag == "duration") {
        ch->setTicks(e.readFraction());
    } else if (tag == "ticklen") {      // obsolete (version < 1.12)
        int mticks = ctx.compatTimeSigMap()->timesig(ctx.tick()).timesig().ticks();
        int i = e.readInt();
        if (i == 0) {
            i = mticks;
        }
        if ((ch->type() == ElementType::REST) && (mticks == i)) {
            ch->setDurationType(DurationType::V_MEASURE);
            ch->setTicks(Fraction::fromTicks(i));
        } else {
            Fraction f = Fraction::fromTicks(i);
            ch->setTicks(f);
            ch->setDurationType(TDuration(f));
        }
    } else if (tag == "dots") {
        ch->setDots(e.readInt());
    } else if (tag == "move") {
        ch->setStaffMove(e.readInt());
    } else if (tag == "Slur") {
        int id = e.intAttribute("id");
        if (id == 0) {
            id = e.intAttribute("number");                        // obsolete
        }
        Spanner* spanner = ctx.findSpanner(id);
        String atype(e.attribute("type"));

        if (!spanner) {
            if (atype == "stop") {
                SpannerValues sv;
                sv.spannerId = id;
                sv.track2    = ch->track();
                sv.tick2     = ctx.tick();
                ctx.addSpannerValues(sv);
            } else if (atype == "start") {
                LOGD("spanner: start without spanner");
            }
        } else {
            if (atype == "start") {
                if (spanner->ticks() > Fraction(0, 1) && spanner->tick() == Fraction(-1, 1)) {       // stop has been read first
                    spanner->setTicks(spanner->ticks() - ctx.tick() - Fraction::fromTicks(1));
                }
                spanner->setTick(ctx.tick());
                spanner->setTrack(ch->track());
                if (spanner->type() == ElementType::SLUR) {
                    spanner->setStartElement(ch);
                }
                if (ctx.pasteMode()) {
                    for (EngravingObject* el : spanner->linkList()) {
                        if (el == spanner) {
                            continue;
                        }
                        Spanner* ls = static_cast<Spanner*>(el);
                        ls->setTick(spanner->tick());
                        for (EngravingObject* ee : ch->linkList()) {
                            ChordRest* cr = toChordRest(ee);
                            if (cr->staffIdx() == ls->staffIdx()) {
                                ls->setTrack(cr->track());
                                if (ls->type() == ElementType::SLUR) {
                                    ls->setStartElement(cr);
                                }
                                break;
                            }
                        }
                    }
                }
            } else if (atype == "stop") {
                spanner->setTick2(ctx.tick());
                spanner->setTrack2(ch->track());
                if (spanner->isSlur()) {
                    spanner->setEndElement(ch);
                }
                ChordRest* start = toChordRest(spanner->startElement());
                if (start) {
                    spanner->setTrack(start->track());
                }
                if (ctx.pasteMode()) {
                    for (EngravingObject* el : spanner->linkList()) {
                        if (el == spanner) {
                            continue;
                        }
                        Spanner* ls = static_cast<Spanner*>(el);
                        ls->setTick2(spanner->tick2());
                        for (EngravingObject* ee : ch->linkList()) {
                            ChordRest* cr = toChordRest(ee);
                            if (cr->staffIdx() == ls->staffIdx()) {
                                ls->setTrack2(cr->track());
                                if (ls->type() == ElementType::SLUR) {
                                    ls->setEndElement(cr);
                                }
                                break;
                            }
                        }
                    }
                }
            } else {
                LOGD("readChordRestProperties206(): unknown Slur type <%s>", muPrintable(atype));
            }
        }
        e.readNext();
    } else if (tag == "Lyrics") {
        Lyrics* l = Factory::createLyrics(ch);
        l->setTrack(ctx.track());
        readLyrics(l, e, ctx);
        ch->add(l);
    } else if (tag == "pos") {
        PointF pt = e.readPoint();
        ch->setOffset(pt * ch->spatium());
    } else if (ch->isRest() && tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(ch);
            image->setTrack(ctx.track());
            read400::TRead::read(image, e, ctx);
            ch->add(image);
        }
    } else if (!readDurationProperties206(e, ctx, ch)) {
        return false;
    }
    return true;
}

bool Read206::readChordProperties206(XmlReader& e, ReadContext& ctx, Chord* ch)
{
    const AsciiStringView tag(e.name());

    if (tag == "Note") {
        Note* note = Factory::createNote(ch);
        // the note needs to know the properties of the track it belongs to
        note->setTrack(ch->track());
        readNote206(note, e, ctx);
        ch->add(note);
    } else if (readChordRestProperties206(e, ctx, ch)) {
    } else if (tag == "Stem") {
        Stem* s = Factory::createStem(ch);
        read400::TRead::read(s, e, ctx);
        ch->add(s);
    } else if (tag == "Hook") {
        Hook* hook = new Hook(ch);
        read400::TRead::read(hook, e, ctx);
        ch->add(hook);
    } else if (tag == "appoggiatura") {
        ch->setNoteType(NoteType::APPOGGIATURA);
        e.readNext();
    } else if (tag == "acciaccatura") {
        ch->setNoteType(NoteType::ACCIACCATURA);
        e.readNext();
    } else if (tag == "grace4") {
        ch->setNoteType(NoteType::GRACE4);
        e.readNext();
    } else if (tag == "grace16") {
        ch->setNoteType(NoteType::GRACE16);
        e.readNext();
    } else if (tag == "grace32") {
        ch->setNoteType(NoteType::GRACE32);
        e.readNext();
    } else if (tag == "grace8after") {
        ch->setNoteType(NoteType::GRACE8_AFTER);
        e.readNext();
    } else if (tag == "grace16after") {
        ch->setNoteType(NoteType::GRACE16_AFTER);
        e.readNext();
    } else if (tag == "grace32after") {
        ch->setNoteType(NoteType::GRACE32_AFTER);
        e.readNext();
    } else if (tag == "StemSlash") {
        StemSlash* ss = Factory::createStemSlash(ch);
        read400::TRead::read(ss, e, ctx);
        ch->add(ss);
    } else if (read400::TRead::readProperty(ch, tag, e, ctx, Pid::STEM_DIRECTION)) {
    } else if (tag == "noStem") {
        ch->setNoStem(e.readInt());
    } else if (tag == "Arpeggio") {
        Arpeggio* arpeggio = Factory::createArpeggio(ch);
        arpeggio->setTrack(ch->track());
        read400::TRead::read(arpeggio, e, ctx);
        arpeggio->setParent(ch);
        ch->add(arpeggio);
    }
    // old glissando format, chord-to-chord, attached to its final chord
    else if (tag == "Glissando") {
        // the measure we are reading is not inserted in the score yet
        // as well as, possibly, the glissando intended initial chord;
        // then we cannot fully link the glissando right now;
        // temporarily attach the glissando to its final note as a back spanner;
        // after the whole score is read, Score::connectTies() will look for
        // the suitable initial note
        Note* finalNote = ch->upNote();
        Glissando* gliss = Factory::createGlissando(ctx.dummy());
        read400::TRead::read(gliss, e, ctx);
        gliss->setAnchor(Spanner::Anchor::NOTE);
        gliss->setStartElement(nullptr);
        gliss->setEndElement(nullptr);
        // in TAB, use straight line with no text
        if (ctx.staff(static_cast<int>(ctx.track()) >> 2)->isTabStaff(ch->tick())) {
            gliss->setGlissandoType(GlissandoType::STRAIGHT);
            gliss->setShowText(false);
        }
        finalNote->addSpannerBack(gliss);
    } else if (tag == "Tremolo") {
        read400::TRead::TremoloCompat tcompat;
        tcompat.parent = ch;
        read400::TRead::read(tcompat, e, ctx);
        if (tcompat.two) {
            tcompat.two->setParent(ch);
            tcompat.two->setDurationType(ch->durationType());
            ch->setTremoloTwoChord(tcompat.two, false);
        } else if (tcompat.single) {
            tcompat.single->setParent(ch);
            tcompat.single->setDurationType(ch->durationType());
            ch->setTremoloSingleChord(tcompat.single);
        } else {
            UNREACHABLE;
        }
    } else if (tag == "tickOffset") {     // obsolete
    } else if (tag == "ChordLine") {
        ChordLine* cl = Factory::createChordLine(ch);
        read400::TRead::read(cl, e, ctx);
        PointF o = cl->offset();
        cl->setOffset(0.0, 0.0);
        ch->add(cl);
        ctx.fixOffsets().push_back({ cl, o });
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   convertDoubleArticulations
//    Replace double articulations with proper SMuFL
//    symbols which were not available for use prior to 3.0
//---------------------------------------------------------

static void convertDoubleArticulations(Chord* chord, XmlReader& e, ReadContext& ctx)
{
    UNUSED(e);
    std::vector<Articulation*> pairableArticulations;
    for (Articulation* a : chord->articulations()) {
        if (a->isStaccato() || a->isTenuto()
            || a->isAccent() || a->isMarcato()) {
            pairableArticulations.push_back(a);
        }
    }
    if (pairableArticulations.size() != 2) {
        // Do not replace triple articulation if this happens
        return;
    }

    SymId newSymId = SymId::noSym;
    for (int i = 0; i < 2; ++i) {
        if (newSymId != SymId::noSym) {
            break;
        }
        Articulation* ai = pairableArticulations[i];
        Articulation* aj = pairableArticulations[(i == 0) ? 1 : 0];
        if (ai->isStaccato()) {
            if (aj->isAccent()) {
                newSymId = SymId::articAccentStaccatoAbove;
            } else if (aj->isMarcato()) {
                newSymId = SymId::articMarcatoStaccatoAbove;
            } else if (aj->isTenuto()) {
                newSymId = SymId::articTenutoStaccatoAbove;
            }
        } else if (ai->isTenuto()) {
            if (aj->isAccent()) {
                newSymId = SymId::articTenutoAccentAbove;
            } else if (aj->isMarcato()) {
                newSymId = SymId::articMarcatoTenutoAbove;
            }
        }
    }

    if (newSymId != SymId::noSym) {
        // We reuse old articulation and change symbol ID
        // rather than constructing a new articulation
        // in order to preserve its other properties.
        Articulation* newArtic = pairableArticulations[0];
        for (Articulation* a : pairableArticulations) {
            chord->remove(a);
            if (a != newArtic) {
                if (LinkedObjects* link = a->links()) {
                    muse::remove(ctx.linkIds(), link->lid());
                }
                delete a;
            }
        }

        ArticulationAnchor anchor = newArtic->anchor();
        newArtic->setSymId(newSymId);
        newArtic->setAnchor(anchor);
        chord->add(newArtic);
    }
}

//---------------------------------------------------------
//   fixTies
//---------------------------------------------------------

static void fixTies(Chord* chord)
{
    std::vector<Note*> notes;
    for (Note* note : chord->notes()) {
        Tie* tie = note->tieBack();
        if (tie && tie->startNote()->pitch() != note->pitch()) {
            notes.push_back(tie->startNote());
        }
    }
    for (Note* note : notes) {
        Note* endNote = chord->findNote(note->pitch());
        Note* oldNote = note->tieFor()->endNote();
        if (oldNote) {
            oldNote->setTieBack(nullptr);
        }
        note->tieFor()->setEndNote(endNote);
    }
}

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

static void readChord(Chord* chord, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "Note") {
            Note* note = Factory::createNote(chord);
            // the note needs to know the properties of the track it belongs to
            note->setTrack(chord->track());
            readNote206(note, e, ctx);
            chord->add(note);
        } else if (tag == "Stem") {
            Stem* stem = Factory::createStem(chord);
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "subtype") {              // obsolete
                    e.skipCurrentElement();
                } else if (!read400::TRead::readProperties(stem, e, ctx)) {
                    e.unknown();
                }
            }
            chord->add(stem);
        } else if (tag == "Lyrics") {
            Lyrics* lyrics = Factory::createLyrics(chord);
            lyrics->setTrack(ctx.track());
            readLyrics(lyrics, e, ctx);
            chord->add(lyrics);
        } else if (Read206::readChordProperties206(e, ctx, chord)) {
        } else {
            e.unknown();
        }
    }
    convertDoubleArticulations(chord, e, ctx);
    fixTies(chord);
}

//---------------------------------------------------------
//   readRest
//---------------------------------------------------------

static void readRest(Rest* rest, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!Read206::readChordRestProperties206(e, ctx, rest)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readTextLineProperties
//---------------------------------------------------------

static bool readTextLineProperties(XmlReader& e, ReadContext& ctx, TextLineBase* tl)
{
    const AsciiStringView tag(e.name());

    if (tag == "beginText") {
        Text* text = Factory::createText(ctx.dummy(), TextStyleType::DEFAULT, false);
        readText206(e, ctx, text, tl);
        tl->setBeginText(text->xmlText());
        tl->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
        delete text;
    } else if (tag == "continueText") {
        Text* text = Factory::createText(ctx.dummy(), TextStyleType::DEFAULT, false);
        readText206(e, ctx, text, tl);
        tl->setContinueText(text->xmlText());
        tl->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::UNSTYLED);
        delete text;
    } else if (tag == "endText") {
        Text* text = Factory::createText(ctx.dummy(), TextStyleType::DEFAULT, false);
        readText206(e, ctx, text, tl);
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
    } else if (!read400::TRead::readProperties(tl, e, ctx)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   readVolta206
//---------------------------------------------------------

static void readVolta206(XmlReader& e, ReadContext& ctx, Volta* volta)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "endings") {
            String s = e.readText();
            StringList sl = s.split(u',', muse::SkipEmptyParts);
            volta->endings().clear();
            for (const String& l : sl) {
                int i = l.simplified().toInt();
                volta->endings().push_back(i);
            }
        } else if (tag == "lineWidth") {
            volta->setLineWidth(Spatium(e.readDouble()));
        } else if (!readTextLineProperties(e, ctx, volta)) {
            e.unknown();
        }
    }
    if (volta->anchor() != Volta::VOLTA_ANCHOR) {
        // Volta strictly assumes that its anchor is measure, so don't let old scores override this.
        LOGW("Correcting volta anchor type from %d to %d", int(volta->anchor()), int(Volta::VOLTA_ANCHOR));
        volta->setAnchor(Volta::VOLTA_ANCHOR);
    }
    adjustPlacement(volta);
}

//---------------------------------------------------------
//   readPedal
//---------------------------------------------------------

static void readPedal(XmlReader& e, ReadContext& ctx, Pedal* pedal)
{
    bool beginTextTag = false;
    bool continueTextTag = false;
    bool endTextTag = false;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (readTextLineProperties(e, ctx, pedal)) {
            beginTextTag = beginTextTag || tag == "beginText";
            continueTextTag = continueTextTag || tag == "continueText";
            endTextTag = endTextTag || tag == "endText";
        } else {
            e.unknown();
        }
    }

    // Set to the 206 defaults if no value was specified;
    // or follow the new style setting if the specified value matches it
    if (!beginTextTag) {
        pedal->setBeginText(String());
        pedal->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::UNSTYLED);
    } else if (pedal->beginText() == pedal->propertyDefault(Pid::BEGIN_TEXT).value<String>()) {
        pedal->setPropertyFlags(Pid::BEGIN_TEXT, PropertyFlags::STYLED);
    }
    if (!continueTextTag) {
        pedal->setContinueText(String());
        pedal->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::UNSTYLED);
    } else if (pedal->continueText() == pedal->propertyDefault(Pid::CONTINUE_TEXT).value<String>()) {
        pedal->setPropertyFlags(Pid::CONTINUE_TEXT, PropertyFlags::STYLED);
    }
    if (!endTextTag) {
        pedal->setEndText(String());
        pedal->setPropertyFlags(Pid::END_TEXT, PropertyFlags::UNSTYLED);
    } else if (pedal->endText() == pedal->propertyDefault(Pid::END_TEXT).value<String>()) {
        pedal->setPropertyFlags(Pid::END_TEXT, PropertyFlags::STYLED);
    }

    adjustPlacement(pedal);
}

//---------------------------------------------------------
//   readOttava
//---------------------------------------------------------

static void readOttava(XmlReader& e, ReadContext& ctx, Ottava* ottava)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            String s = e.readText();
            bool ok;
            int idx = s.toInt(&ok);
            if (!ok) {
                idx = 0;            // OttavaType::OTTAVA_8VA;
                int i = 0;
                for (auto p :  { u"8va", u"8vb", u"15ma", u"15mb", u"22ma", u"22mb" }) {
                    if (p == s) {
                        idx = i;
                        break;
                    }
                    ++i;
                }
            }
            ottava->setOttavaType(OttavaType(idx));
        } else if (tag == "numbersOnly") {
            ottava->setNumbersOnly(e.readBool());
        } else if (!readTextLineProperties(e, ctx, ottava)) {
            e.unknown();
        }
    }
    ottava->styleChanged();
    adjustPlacement(ottava);
}

void Read206::readHairpin206(XmlReader& e, ReadContext& ctx, Hairpin* h)
{
    bool useText = false;
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            h->setHairpinType(HairpinType(e.readInt()));
        } else if (tag == "lineWidth") {
            h->setLineWidth(Spatium(e.readDouble()));
            // lineWidthStyle = PropertyFlags::UNSTYLED;
        } else if (tag == "hairpinHeight") {
            h->setHairpinHeight(Spatium(e.readDouble()));
            // hairpinHeightStyle = PropertyFlags::UNSTYLED;
        } else if (tag == "hairpinContHeight") {
            h->setHairpinContHeight(Spatium(e.readDouble()));
            // hairpinContHeightStyle = PropertyFlags::UNSTYLED;
        } else if (tag == "hairpinCircledTip") {
            h->setHairpinCircledTip(e.readInt());
        } else if (tag == "veloChange") {
            h->setVeloChange(e.readInt());
        } else if (tag == "dynType") {
            h->setDynRange(DynamicRange(e.readInt()));
        } else if (tag == "useTextLine") {        // < 206
            e.readInt();
            if (h->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::CRESC_LINE);
            } else if (h->hairpinType() == HairpinType::DECRESC_HAIRPIN) {
                h->setHairpinType(HairpinType::DECRESC_LINE);
            }
            useText = true;
        } else if (!readTextLineProperties(e, ctx, h)) {
            e.unknown();
        }
    }
    if (!useText) {
        h->setBeginText(u"");
        h->setContinueText(u"");
        h->setEndText(u"");
    }
    adjustPlacement(h);
}

void Read206::readTrill206(XmlReader& e, ReadContext& ctx, Trill* t)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            t->setTrillType(TConv::fromXml(e.readAsciiText(), TrillType::TRILL_LINE));
        } else if (tag == "Accidental") {
            Accidental* _accidental = Factory::createAccidental(t);
            readAccidental206(_accidental, e, ctx);
            _accidental->setParent(t);
            t->setAccidental(_accidental);
            if (t->ornament()) {
                t->ornament()->setTrillOldCompatAccidental(_accidental);
            }
        } else if (tag == "ornamentStyle") {
            read400::TRead::readProperty(t, e, ctx, Pid::ORNAMENT_STYLE);
        } else if (tag == "play") {
            t->setPlaySpanner(e.readBool());
        } else if (!TRead::readProperties(static_cast<SLine*>(t), e, ctx)) {
            e.unknown();
        }
    }
    adjustPlacement(t);
}

void Read206::readTextLine206(XmlReader& e, ReadContext& ctx, TextLineBase* tlb)
{
    while (e.readNextStartElement()) {
        if (!readTextLineProperties(e, ctx, tlb)) {
            e.unknown();
        }
    }
    adjustPlacement(tlb);
}

//---------------------------------------------------------
//   setFermataPlacement
//    set fermata placement from old ArticulationAnchor
//    for backwards compatibility
//---------------------------------------------------------

static void setFermataPlacement(EngravingItem* el, ArticulationAnchor anchor, DirectionV direction)
{
    if (direction == DirectionV::UP) {
        el->setPlacement(PlacementV::ABOVE);
    } else if (direction == DirectionV::DOWN) {
        el->setPlacement(PlacementV::BELOW);
    } else {
        switch (anchor) {
        case ArticulationAnchor::TOP:
            el->setPlacement(PlacementV::ABOVE);
            break;

        case ArticulationAnchor::BOTTOM:
            el->setPlacement(PlacementV::BELOW);
            break;

        case ArticulationAnchor::AUTO:
            break;
        default:
            break;
        }
    }
}

EngravingItem* Read206::readArticulation(EngravingItem* parent, XmlReader& e, ReadContext& ctx)
{
    EngravingItem* el = nullptr;
    SymId sym = SymId::fermataAbove;            // default -- backward compatibility (no type = ufermata in 1.2)
    ArticulationAnchor anchor  = ArticulationAnchor::TOP;
    DirectionV direction = DirectionV::AUTO;
    track_idx_t track = parent->track();
    double timeStretch = 0.0;
    bool useDefaultPlacement = true;

    auto readProperties = [](EngravingItem* el, XmlReader& e, ReadContext& ctx)
    {
        if (el->isFermata()) {
            return read400::TRead::readProperties(dynamic_cast<Fermata*>(el), e, ctx);
        } else if (el->isArticulationFamily()) {
            return read400::TRead::readProperties(dynamic_cast<Articulation*>(el), e, ctx);
        }
        UNREACHABLE;
        return false;
    };

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            String s = e.readText();
            if (s.at(0).isDigit()) {
                int oldType = s.toInt();
                sym = articulationNames[oldType].id;
            } else {
                muse::ByteArray ba = s.toAscii();
                sym = articulationNames2SymId206(ba.constChar());
                if (sym == SymId::noSym) {
                    struct {
                        const char* name;
                        bool up;
                        SymId id;
                    } al[] =
                    {
                        { "fadein",                    true,  SymId::guitarFadeIn },
                        { "fadeout",                   true,  SymId::guitarFadeOut },
                        { "volumeswell",               true,  SymId::guitarVolumeSwell },
                        { "wigglesawtooth",            true,  SymId::wiggleSawtooth },
                        { "wigglesawtoothwide",        true,  SymId::wiggleSawtoothWide },
                        { "wigglevibratolargefaster",  true,  SymId::wiggleVibratoLargeFaster },
                        { "wigglevibratolargeslowest", true,  SymId::wiggleVibratoLargeSlowest },
                        { "umarcato",                  true,  SymId::articMarcatoAbove },
                        { "dmarcato",                  false, SymId::articMarcatoBelow },
                        { "ufermata",                  true,  SymId::fermataAbove },
                        { "dfermata",                  false, SymId::fermataBelow },
                        { "ushortfermata",             true,  SymId::fermataShortAbove },
                        { "dshortfermata",             false, SymId::fermataShortBelow },
                        { "ulongfermata",              true,  SymId::fermataLongAbove },
                        { "dlongfermata",              false, SymId::fermataLongBelow },
                        { "uverylongfermata",          true,  SymId::fermataVeryLongAbove },
                        { "dverylongfermata",          false, SymId::fermataVeryLongBelow },

                        // watch out, bug in 1.2 uportato and dportato are reversed
                        { "dportato",                  true,  SymId::articTenutoStaccatoAbove },
                        { "uportato",                  false, SymId::articTenutoStaccatoBelow },
                        { "ustaccatissimo",            true,  SymId::articStaccatissimoAbove },
                        { "dstaccatissimo",            false, SymId::articStaccatissimoBelow }
                    };
                    int i;
                    int n = sizeof(al) / sizeof(*al);
                    for (i = 0; i < n; ++i) {
                        if (s == al[i].name) {
                            sym       = al[i].id;
                            bool up   = al[i].up;
                            direction = up ? DirectionV::UP : DirectionV::DOWN;
                            if ((direction == DirectionV::DOWN) != (track & 1)) {
                                useDefaultPlacement = false;
                            }
                            break;
                        }
                    }
                    if (i == n) {
                        sym = SymNames::symIdByName(s);
                        if (sym == SymId::noSym) {
                            LOGD("Articulation: unknown type <%s>", muPrintable(s));
                        }
                    }
                }
            }
            switch (sym) {
            case SymId::fermataAbove:
            case SymId::fermataBelow:
            case SymId::fermataShortAbove:
            case SymId::fermataShortBelow:
            case SymId::fermataLongAbove:
            case SymId::fermataLongBelow:
            case SymId::fermataVeryLongAbove:
            case SymId::fermataVeryLongBelow: {
                Fermata* fe = Factory::createFermata(ctx.dummy());
                fe->setSymIdAndTimeStretch(sym);
                el = fe;
            } break;
            default:
                Articulation* ar = Factory::createArticulation(ctx.dummy()->chord());
                ar->setSymId(sym);
                ar->setDirection(direction);
                el = ar;
                break;
            }
        } else if (tag == "anchor") {
            useDefaultPlacement = false;
            if (!el || el->isFermata()) {
                anchor = CompatUtils::translateToNewArticulationAnchor(e.readInt());
            } else {
                readProperties(el, e, ctx);
            }
        } else if (tag == "direction") {
            useDefaultPlacement = false;
            if (!el || el->isFermata()) {
                direction = TConv::fromXml(e.readAsciiText(), DirectionV::AUTO);
            } else {
                readProperties(el, e, ctx);
            }
        } else if (tag == "timeStretch") {
            timeStretch = e.readDouble();
        } else {
            if (!el) {
                LOGD("not handled <%s>", tag.ascii());
            }
            if (!el || !readProperties(el, e, ctx)) {
                e.unknown();
            }
        }
    }
    // Special case for "no type" = ufermata, with missing subtype tag
    if (!el) {
        Fermata* f = Factory::createFermata(ctx.dummy());
        f->setSymIdAndTimeStretch(sym);
        el = f;
    }
    if (el->isFermata()) {
        if (!RealIsNull(timeStretch)) {
            el->setProperty(Pid::TIME_STRETCH, timeStretch);
        }
        if (useDefaultPlacement) {
            el->setPlacement(track & 1 ? PlacementV::BELOW : PlacementV::ABOVE);
        } else {
            setFermataPlacement(el, anchor, direction);
        }
    }
    el->setTrack(track);
    return el;
}

//---------------------------------------------------------
//   readSlurTieProperties
//---------------------------------------------------------

static bool readSlurTieProperties(XmlReader& e, ReadContext& ctx, SlurTie* st)
{
    const AsciiStringView tag(e.name());

    if (read400::TRead::readProperty(st, tag, e, ctx, Pid::SLUR_DIRECTION)) {
    } else if (tag == "lineType") {
        st->setStyleType(static_cast<SlurStyleType>(e.readInt()));
    } else if (tag == "SlurSegment") {
        SlurTieSegment* s = st->newSlurTieSegment(ctx.dummy()->system());
        read400::TRead::read(s, e, ctx);
        st->add(s);
    } else if (!TRead::readItemProperties(st, e, ctx)) {
        return false;
    }
    return true;
}

void Read206::readSlur206(XmlReader& e, ReadContext& ctx, Slur* s)
{
    s->setTrack(ctx.track());        // set staff
    ctx.addSpanner(e.intAttribute("id"), s);
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "track2") {
            s->setTrack2(e.readInt());
        } else if (tag == "startTrack") {       // obsolete
            s->setTrack(e.readInt());
        } else if (tag == "endTrack") {         // obsolete
            e.readInt();
        } else if (!readSlurTieProperties(e, ctx, s)) {
            e.unknown();
        }
    }
    if (s->track2() == muse::nidx) {
        s->setTrack2(s->track());
    }
}

void Read206::readTie206(XmlReader& e, ReadContext& ctx, Tie* t)
{
    ctx.addSpanner(e.intAttribute("id"), t);
    while (e.readNextStartElement()) {
        if (readSlurTieProperties(e, ctx, t)) {
        } else {
            e.unknown();
        }
    }
    if (ctx.mscVersion() <= 114 && t->spannerSegments().size() == 1) {
        // ignore manual adjustments to single-segment ties in older scores
        TieSegment* ss = t->frontSegment();
        PointF zeroP;
        ss->ups(Grip::START).off     = zeroP;
        ss->ups(Grip::BEZIER1).off   = zeroP;
        ss->ups(Grip::BEZIER2).off   = zeroP;
        ss->ups(Grip::END).off       = zeroP;
        ss->setOffset(zeroP);
        ss->setUserOff2(zeroP);
    }
}

//---------------------------------------------------------
//   readMeasure
//---------------------------------------------------------

static void readMeasure206(Measure* m, int staffIdx, XmlReader& e, ReadContext& ctx)
{
    Segment* segment = 0;
    double _spatium = m->spatium();

    std::vector<Chord*> graceNotes;
    ctx.tuplets().clear();
    ctx.setTrack(staffIdx * VOICES);

    m->createStaves(staffIdx);

    // tick is obsolete
    if (e.hasAttribute("tick")) {
        ctx.setTick(Fraction::fromTicks(ctx.fileDivision(e.intAttribute("tick"))));
    }

    bool irregular;
    if (e.hasAttribute("len")) {
        StringList sl = e.attribute("len").split(u'/');
        if (sl.size() == 2) {
            m->setTicks(Fraction(sl.at(0).toInt(), sl.at(1).toInt()));
        } else {
            LOGD("illegal measure size <%s>", muPrintable(e.attribute("len")));
        }
        irregular = true;
        ctx.compatTimeSigMap()->add(m->tick().ticks(), SigEvent(m->ticks(), m->timesig()));
        ctx.compatTimeSigMap()->add(m->endTick().ticks(), SigEvent(m->timesig()));
    } else {
        irregular = false;
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
            Fermata* fermataAbove = nullptr;
            Fermata* fermataBelow = nullptr;
            BarLine* bl = Factory::createBarLine(ctx.dummy()->segment());
            bl->setTrack(ctx.track());
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                if (t == "subtype") {
                    bl->setBarLineType(TConv::fromXml(e.readAsciiText(), BarLineType::NORMAL));
                } else if (t == "customSubtype") {                          // obsolete
                    e.readInt();
                } else if (t == "span") {
                    int span = e.readInt();
                    if (span) {
                        span--;
                    }
                    bl->setSpanStaff(span);
                } else if (t == "spanFromOffset") {
                    bl->setSpanFrom(e.readInt());
                } else if (t == "spanToOffset") {
                    bl->setSpanTo(e.readInt());
                } else if (t == "Articulation") {
                    EngravingItem* el = Read206::readArticulation(bl, e, ctx);
                    if (el->isFermata()) {
                        if (el->placement() == PlacementV::ABOVE) {
                            fermataAbove = toFermata(el);
                        } else {
                            fermataBelow = toFermata(el);
                            fermataBelow->setTrack((bl->staffIdx() + bl->spanStaff()) * VOICES);
                        }
                    } else {
                        bl->add(el);
                    }
                } else if (!TRead::readItemProperties(bl, e, ctx)) {
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
            } else if (bl->barLineType() == BarLineType::START_REPEAT && ctx.tick() == m->tick()) {
                st = SegmentType::StartRepeatBarLine;
            } else if (ctx.tick() == m->tick() && segment == 0) {
                st = SegmentType::BeginBarLine;
            } else {
                st = SegmentType::EndBarLine;
            }
            segment = m->getSegment(st, ctx.tick());
            segment->add(bl);
            bl->renderer()->layoutItem(bl);
            if (fermataAbove) {
                segment->add(fermataAbove);
            }
            if (fermataBelow) {
                segment->add(fermataBelow);
            }
        } else if (tag == "Chord") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            Chord* chord = Factory::createChord(segment);
            chord->setTrack(ctx.track());
            chord->setParent(segment);
            readChord(chord, e, ctx);
            if (chord->noteType() != NoteType::NORMAL) {
                graceNotes.push_back(chord);
            } else {
                segment->add(chord);
                for (size_t i = 0; i < graceNotes.size(); ++i) {
                    Chord* gc = graceNotes[i];
                    gc->setGraceIndex(i);
                    chord->add(gc);
                }
                graceNotes.clear();
                Fraction crticks = chord->actualTicks();
                lastTick         = ctx.tick();
                ctx.incTick(crticks);
            }
        } else if (tag == "Rest") {
            if (m->isMMRest()) {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                MMRest* mmr = Factory::createMMRest(segment);
                mmr->setParent(segment);
                mmr->setTrack(ctx.track());
                read400::TRead::read(mmr, e, ctx);
                mmr->setTicks(m->ticks());
                segment->add(mmr);
                lastTick = ctx.tick();
                ctx.incTick(mmr->actualTicks());
            } else {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                Rest* rest = Factory::createRest(segment);
                rest->setDurationType(DurationType::V_MEASURE);
                rest->setTicks(m->timesig() / timeStretch);
                rest->setTrack(ctx.track());
                rest->setParent(segment);
                readRest(rest, e, ctx);
                segment->add(rest);

                if (!rest->ticks().isValid()) {    // hack
                    rest->setTicks(m->timesig() / timeStretch);
                }

                lastTick = ctx.tick();
                ctx.incTick(rest->actualTicks());
            }
        } else if (tag == "Breath") {
            Breath* breath = Factory::createBreath(ctx.dummy()->segment());
            breath->setTrack(ctx.track());
            breath->setPlacement(PlacementV::ABOVE);
            Fraction tick = ctx.tick();
            read400::TRead::read(breath, e, ctx);
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
                if (spanner->track2() == muse::nidx) {
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
            Slur* sl = Factory::createSlur(ctx.dummy());
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
            Spanner* sp = toSpanner(Factory::createItemByName(tag, ctx.dummy()));
            sp->setTrack(ctx.track());
            sp->setTrack2(sp->track());
            sp->setTick(ctx.tick());
            sp->eraseSpannerSegments();
            ctx.addSpanner(e.intAttribute("id", -1), sp);

            if (tag == "Volta") {
                readVolta206(e, ctx, toVolta(sp));
            } else if (tag == "Pedal") {
                readPedal(e, ctx, toPedal(sp));
            } else if (tag == "Ottava") {
                readOttava(e, ctx, toOttava(sp));
            } else if (tag == "HairPin") {
                Read206::readHairpin206(e, ctx, toHairpin(sp));
            } else if (tag == "Trill") {
                Ornament* ornament = Factory::createOrnament(m->score()->dummy()->chord());
                toTrill(sp)->setOrnament(ornament);
                Read206::readTrill206(e, ctx, toTrill(sp));
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
            readRest(rm, e, ctx);
            rm->setNumMeasures(1);
            m->setMeasureRepeatCount(1, staffIdx);
            segment->add(rm);
            lastTick = ctx.tick();
            ctx.incTick(m->ticks());
        } else if (tag == "Clef") {
            Clef* clef = Factory::createClef(ctx.dummy()->segment());
            clef->setTrack(ctx.track());
            read400::TRead::read(clef, e, ctx);
            clef->setGenerated(false);
            if (ctx.tick().isZero()) {
                if (ctx.staff(staffIdx)->clef(Fraction(0, 1)) != clef->clefType()) {
                    ctx.staff(staffIdx)->setDefaultClefType(clef->clefType());
                }
                if (clef->links() && clef->links()->size() == 1) {
                    muse::remove(ctx.linkIds(), clef->links()->lid());
                    LOGD("remove link %d", clef->links()->lid());
                }
                delete clef;
                continue;
            }
            // there may be more than one clef segment for same tick position
            if (!segment) {
                // this is the first segment of measure
                segment = m->getSegment(SegmentType::Clef, ctx.tick());
            } else {
                bool firstSegment = false;
                // the first clef may be missing and is added later in layout
                for (Segment* s = m->segments().first(); s && s->tick() == ctx.tick(); s = s->next()) {
                    if (s->segmentType() == SegmentType::Clef
                        // hack: there may be other segment types which should
                        // generate a clef at current position
                        || s->segmentType() == SegmentType::StartRepeatBarLine
                        ) {
                        firstSegment = true;
                        break;
                    }
                }
                if (firstSegment) {
                    Segment* ns = 0;
                    if (segment->next()) {
                        ns = segment->next();
                        while (ns && ns->tick() < ctx.tick()) {
                            ns = ns->next();
                        }
                    }
                    segment = 0;
                    for (Segment* s = ns; s && s->tick() == ctx.tick(); s = s->next()) {
                        if (s->segmentType() == SegmentType::Clef) {
                            segment = s;
                            break;
                        }
                    }
                    if (!segment) {
                        segment = Factory::createSegment(m, SegmentType::Clef, ctx.tick() - m->tick());
                        m->segments().insert(segment, ns);
                    }
                } else {
                    // this is the first clef: move to left
                    segment = m->getSegment(SegmentType::Clef, ctx.tick());
                }
            }
            if (ctx.tick() != m->tick()) {
                clef->setSmall(true);                 // TODO: layout does this ?
            }
            segment->add(clef);
        } else if (tag == "TimeSig") {
            TimeSig* ts = Factory::createTimeSig(ctx.dummy()->segment());
            ts->setTrack(ctx.track());
            read400::TRead::read(ts, e, ctx);
            // if time sig not at beginning of measure => courtesy time sig
            Fraction currTick = ctx.tick();
            bool courtesySig = (currTick > m->tick());
            if (courtesySig) {
                // if courtesy sig., just add it without map processing
                segment = m->getSegment(SegmentType::TimeSigAnnounce, currTick);
                segment->add(ts);
            } else {
                // if 'real' time sig., do full process
                segment = m->getSegment(SegmentType::TimeSig, currTick);
                segment->add(ts);

                timeStretch = ts->stretch().reduced();
                m->setTimesig(ts->sig() / timeStretch);

                if (irregular) {
                    ctx.compatTimeSigMap()->add(m->tick().ticks(), SigEvent(m->ticks(), m->timesig()));
                    ctx.compatTimeSigMap()->add(m->endTick().ticks(), SigEvent(m->timesig()));
                } else {
                    m->setTicks(m->timesig());
                    ctx.compatTimeSigMap()->add(m->tick().ticks(), SigEvent(m->timesig()));
                }
            }
        } else if (tag == "KeySig") {
            KeySig* ks = Factory::createKeySig(ctx.dummy()->segment());
            ks->setTrack(ctx.track());
            read400::TRead::read(ks, e, ctx);
            Fraction curTick = ctx.tick();
            // if key sig not at beginning of measure => courtesy key sig
            bool courtesySig = (curTick == m->endTick());
            segment = m->getSegment(courtesySig ? SegmentType::KeySigAnnounce : SegmentType::KeySig, curTick);
            segment->add(ks);
            if (!courtesySig) {
                staff->setKey(curTick, ks->keySigEvent());
            }
        } else if (tag == "Text" || tag == "StaffText") {
            // MuseScore 3 has different types for system text and
            // staff text while MuseScore 2 didn't.
            // We need to decide first which one we should create.
            TextReaderContext206 tctx(e);
            String styleName = ReadStyleName206(tctx.tag());
            StaffTextBase* t;
            if (styleName == "System" || styleName == "Tempo"
                || styleName == "Marker" || styleName == "Jump"
                || styleName == "Volta") {    // TODO: is it possible to get it from style?
                t = Factory::createSystemText(ctx.dummy()->segment());
            } else {
                t = Factory::createStaffText(ctx.dummy()->segment());
            }
            t->setTrack(ctx.track());
            readTextPropertyStyle206(tctx.tag(), ctx, t, t);
            while (tctx.reader().readNextStartElement()) {
                if (!readTextProperties206(tctx.reader(), ctx, t)) {
                    tctx.reader().unknown();
                }
            }
            if (t->empty()) {
                if (t->links()) {
                    if (t->links()->size() == 1) {
                        LOGD("reading empty text: deleted lid = %d", t->links()->lid());
                        muse::remove(ctx.linkIds(), t->links()->lid());
                        delete t;
                    }
                }
            } else {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                segment->add(t);
            }
        }
        //----------------------------------------------------
        // Annotation
        else if (tag == "Dynamic") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            Dynamic* dyn = Factory::createDynamic(segment);
            dyn->setTrack(ctx.track());
            readDynamic(dyn, e, ctx);
            segment->add(dyn);
        } else if (tag == "RehearsalMark") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            RehearsalMark* el = Factory::createRehearsalMark(segment);
            el->setTrack(ctx.track());
            readText206(e, ctx, el, el);
            segment->add(el);
        } else if (tag == "Harmony"
                   || tag == "FretDiagram"
                   || tag == "TremoloBar"
                   || tag == "Symbol"
                   || tag == "InstrumentChange"
                   || tag == "StaffState"
                   || tag == "FiguredBass"
                   ) {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            EngravingItem* el = Factory::createItemByName(tag, segment);
            // hack - needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            el->setTrack(ctx.track());
            read400::TRead::readItem(el, e, ctx);
            if (el->staff() && (el->isHarmony() || el->isFretDiagram() || el->isInstrumentChange())) {
                adjustPlacement(el);
            }

            segment->add(el);
        } else if (tag == "Tempo") {
            segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
            TempoText* tt = Factory::createTempoText(segment);
            // hack - needed because tick tags are unreliable in 1.3 scores
            // for symbols attached to anything but a measure
            tt->setTrack(ctx.track());
            readTempoText(tt, e, ctx);
            segment->add(tt);
        } else if (tag == "Marker" || tag == "Jump") {
            EngravingItem* el = Factory::createItemByName(tag, ctx.dummy());
            el->setTrack(ctx.track());
            if (tag == "Marker") {
                Marker* ma = toMarker(el);
                readMarker(ma, e, ctx);
                EngravingItem* markerEl = toEngravingItem(ma);
                m->add(markerEl);
            } else {
                read400::TRead::readItem(el, e, ctx);
                m->add(el);
            }
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                segment = m->getSegment(SegmentType::ChordRest, ctx.tick());
                EngravingItem* el = Factory::createItemByName(tag, segment);
                el->setTrack(ctx.track());
                read400::TRead::readItem(el, e, ctx);
                segment->add(el);
            }
        } else if (tag == "stretch") {
            // Ignore measure stretch pre 4.0
            e.skipCurrentElement();
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
            readTuplet206(tuplet, e, ctx);
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
            read400::TRead::read(beam, e, ctx);
            beam->resetExplicitParent();
            ctx.addBeam(beam);
        } else if (tag == "Segment") {
            if (segment) {
                read400::TRead::read(segment, e, ctx);
            } else {
                e.unknown();
            }
        } else if (tag == "MeasureNumber") {
            MeasureNumber* noText = new MeasureNumber(m);
            readText206(e, ctx, noText, m);
            noText->setTrack(ctx.track());
            noText->setParent(m);
            m->setNoText(noText->staffIdx(), noText);
        } else if (tag == "SystemDivider") {
            SystemDivider* sd = new SystemDivider(ctx.dummy()->system());
            read400::TRead::read(sd, e, ctx);
            m->add(sd);
        } else if (tag == "Ambitus") {
            segment = m->getSegment(SegmentType::Ambitus, ctx.tick());
            Ambitus* range = Factory::createAmbitus(segment);
            readAmbitus(range, e, ctx);
            range->setParent(segment);                // a parent segment is needed for setTrack() to work
            range->setTrack(trackZeroVoice(ctx.track()));
            segment->add(range);
        } else if (tag == "multiMeasureRest") {
            m->setMMRestCount(e.readInt());
            // set tick to previous measure
            m->setTick(ctx.lastMeasure()->tick());
            ctx.setTick(ctx.lastMeasure()->tick());
        } else if (read400::TRead::readProperties(static_cast<MeasureBase*>(m), e, ctx)) {
        } else {
            e.unknown();
        }
    }
    ctx.checkTuplets();
    m->connectTremolo();
}

//---------------------------------------------------------
//   readBox
//---------------------------------------------------------

static void readBox(Box* b, XmlReader& e, ReadContext& ctx)
{
    b->setAutoSizeEnabled(false);      // didn't exist in Mu2

    b->setBoxHeight(Spatium(0));       // override default set in constructor
    b->setBoxWidth(Spatium(0));

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "HBox") {
            HBox* hb = Factory::createHBox(b->score()->dummy()->system());
            read400::TRead::read(hb, e, ctx);
            b->add(hb);
        } else if (tag == "VBox") {
            VBox* vb = Factory::createVBox(b->score()->dummy()->system());
            read400::TRead::read(vb, e, ctx);
            b->add(vb);
        } else if (tag == "Text") {
            Text* t;
            if (b->isTBox()) {
                t = toTBox(b)->text();
                readText206(e, ctx, t, t);
            } else {
                t = Factory::createText(b);
                readText206(e, ctx, t, t);
                if (t->empty()) {
                    LOGD("read empty text");
                } else {
                    b->add(t);
                }
            }
        } else if (!read400::TRead::readBoxProperties(b, e, ctx)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readStaffContent
//---------------------------------------------------------

static void readStaffContent206(Score* score, XmlReader& e, ReadContext& ctx)
{
    int staff = e.intAttribute("id", 1) - 1;
    ctx.setTick(Fraction(0, 1));
    ctx.setTrack(staff * VOICES);
    Box* lastReadBox = nullptr;
    bool readMeasureLast = false;

    if (staff == 0) {
        while (e.readNextStartElement()) {
            const AsciiStringView tag(e.name());

            if (tag == "Measure") {
                if (lastReadBox) {
                    lastReadBox->setBottomGap(lastReadBox->bottomGap() + lastReadBox->propertyDefault(Pid::BOTTOM_GAP).value<Spatium>());
                    lastReadBox = nullptr;
                }
                readMeasureLast = true;

                Measure* measure = nullptr;
                measure = Factory::createMeasure(score->dummy()->system());
                measure->setTick(ctx.tick());
                //
                // inherit timesig from previous measure
                //
                Measure* m = ctx.lastMeasure();         // measure->prevMeasure();
                Fraction f(m ? m->timesig() : Fraction(4, 4));
                measure->setTicks(f);
                measure->setTimesig(f);

                readMeasure206(measure, staff, e, ctx);
                measure->checkMeasure(staff);
                if (!measure->isMMRest()) {
                    score->measures()->add(measure);
                    if (m && m->mmRest()) {
                        m->mmRest()->setNext(measure);
                    }
                    ctx.setLastMeasure(measure);
                    ctx.setTick(measure->endTick());
                } else {
                    // this is a multi measure rest
                    // always preceded by the first measure it replaces
                    Measure* lm = ctx.lastMeasure();

                    if (lm) {
                        lm->setMMRest(measure);
                        measure->setTick(lm->tick());
                        measure->setPrev(lm->prev());
                    }
                }
            } else if (tag == "HBox" || tag == "VBox" || tag == "TBox" || tag == "FBox") {
                Box* b = toBox(Factory::createItemByName(tag, score->dummy()));
                readBox(b, e, ctx);
                b->setTick(ctx.tick());
                score->measures()->add(b);

                // If it's the first box, and comes before any measures, reset to
                // 301 default.
                if (!readMeasureLast && !lastReadBox) {
                    b->setTopGap(b->propertyDefault(Pid::TOP_GAP).value<Spatium>());
                    b->setPropertyFlags(Pid::TOP_GAP, PropertyFlags::STYLED);
                } else if (readMeasureLast) {
                    b->setTopGap(b->topGap() + b->propertyDefault(Pid::TOP_GAP).value<Spatium>());
                }

                lastReadBox = b;
                readMeasureLast = false;
            } else if (tag == "tick") {
                ctx.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    } else {
        Measure* measure = score->firstMeasure();
        while (e.readNextStartElement()) {
            const AsciiStringView tag(e.name());

            if (tag == "Measure") {
                if (measure == 0) {
                    LOGD("Score::readStaff(): missing measure!");
                    measure = Factory::createMeasure(score->dummy()->system());
                    measure->setTick(ctx.tick());
                    score->measures()->add(measure);
                }
                ctx.setTick(measure->tick());
                readMeasure206(measure, staff, e, ctx);
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
            } else if (tag == "tick") {
                ctx.setTick(Fraction::fromTicks(score->fileDivision(e.readInt())));
            } else {
                e.unknown();
            }
        }
    }
}

//---------------------------------------------------------
//   readStyle
//---------------------------------------------------------

static void readStyle206(MStyle* style, XmlReader& e, ReadContext& ctx, ReadChordListHook& readChordListHook)
{
    excessTextStyles206.clear();
    while (e.readNextStartElement()) {
        AsciiStringView tag = e.name();
        if (tag == "TextStyle") {
            Read206::readTextStyle206(style, e, ctx, excessTextStyles206);
        } else if (tag == "Spatium") {
            style->set(Sid::spatium, e.readDouble() * DPMM);
        } else if (tag == "page-layout") {
            readPageFormat206(style, e);
        } else if (tag == "displayInConcertPitch") {
            style->set(Sid::concertPitch, bool(e.readInt()));
        } else if (tag == "pedalY") {
            double y = e.readDouble();
            style->set(Sid::pedalPosBelow, PointF(0.0, y));
        } else if (tag == "lyricsDistance") {
            double y = e.readDouble();
            style->set(Sid::lyricsPosBelow, PointF(0.0, y));
        } else if (tag == "lyricsMinBottomDistance") {
            // no longer meaningful since it is now measured from skyline rather than staff
            //style->set(Sid::lyricsMinBottomDistance, PointF(0.0, y));
            e.skipCurrentElement();
        } else if (tag == "ottavaHook") {
            double y = std::abs(e.readDouble());
            style->set(Sid::ottavaHookAbove, y);
            style->set(Sid::ottavaHookBelow, -y);
        } else if (tag == "endBarDistance") {
            double d = e.readDouble();
            d += style->value(Sid::barWidth).toReal();
            d += style->value(Sid::endBarWidth).toReal();
            style->set(Sid::endBarDistance, d);
        } else if (tag == "ChordList") {
            readChordListHook.read(e);
        } else if (tag == "harmonyY") {
            double val = -e.readDouble();
            if (val > 0.0) {
                style->set(Sid::harmonyPlacement, PlacementV::BELOW);
                style->set(Sid::chordSymbolAPosBelow,  PointF(.0, val));
            } else {
                style->set(Sid::harmonyPlacement, PlacementV::ABOVE);
                style->set(Sid::chordSymbolAPosBelow,  PointF(.0, val));
            }
        } else {
            if (!ReadStyleHook::readStyleProperties(style, e)) {
                e.skipCurrentElement();
            }
        }
    }

    readChordListHook.validate();
}

bool Read206::readScore206(Score* score, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        ctx.setTrack(muse::nidx);
        const AsciiStringView tag(e.name());
        if (tag == "Staff") {
            readStaffContent206(score, e, ctx);
        } else if (tag == "siglist") {
            read400::TRead::read(ctx.compatTimeSigMap(), e, ctx);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            score->setAudio(new Audio);
            read400::TRead::read(score->audio(), e, ctx);
        } else if (tag == "showOmr") {
            e.skipCurrentElement();
        } else if (tag == "playMode") {
            score->setPlayMode(PlayMode(e.readInt()));
        } else if (tag == "LayerTag") {
            e.skipCurrentElement();
        } else if (tag == "Layer") {
            e.skipCurrentElement();
        } else if (tag == "currentLayer") {
            e.skipCurrentElement();
        } else if (tag == "Synthesizer") {
            score->synthesizerState().read(e);
        } else if (tag == "page-offset") {
            score->setPageNumberOffset(e.readInt());
        } else if (tag == "Division") {
            score->setFileDivision(e.readInt());
        } else if (tag == "showInvisible") {
            score->setShowInvisible(e.readInt());
        } else if (tag == "showUnprintable") {
            score->setShowUnprintable(e.readInt());
        } else if (tag == "showFrames") {
            score->setShowFrames(e.readInt());
        } else if (tag == "showMargins") {
            score->setShowPageborders(e.readInt());
        } else if (tag == "Style") {
            double sp = score->style().value(Sid::spatium).toReal();
            ReadChordListHook clhook(score);
            readStyle206(&score->style(), e, ctx, clhook);
            if (score->style().styleSt(Sid::musicalTextFont) == "MuseJazz") {
                score->style().set(Sid::musicalTextFont, "MuseJazz Text");
            }
            if (ctx.overrideSpatium()) {
                ctx.setOriginalSpatium(score->style().spatium());
                score->style().set(Sid::spatium, sp);
            }
            score->setEngravingFont(score->engravingFonts()->fontByName(score->style().styleSt(Sid::musicalSymbolFont).toStdString()));
        } else if (tag == "copyright" || tag == "rights") {
            Text* text = Factory::createText(score->dummy(), TextStyleType::DEFAULT, false);
            readText206(e, ctx, text, text);
            score->setMetaTag(u"copyright", text->xmlText());
            delete text;
        } else if (tag == "movement-number") {
            score->setMetaTag(u"movementNumber", e.readText());
        } else if (tag == "movement-title") {
            score->setMetaTag(u"movementTitle", e.readText());
        } else if (tag == "work-number") {
            score->setMetaTag(u"workNumber", e.readText());
        } else if (tag == "work-title") {
            score->setMetaTag(u"workTitle", e.readText());
        } else if (tag == "source") {
            score->setMetaTag(u"source", e.readText());
        } else if (tag == "metaTag") {
            String name = e.attribute("name");
            score->setMetaTag(name, e.readText());
        } else if (tag == "Part") {
            Part* part = new Part(score);
            Read206::readPart206(part, e, ctx);
            score->appendPart(part);
        } else if ((tag == "HairPin")     // TODO: do this elements exist here?
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Factory::createItemByName(tag, score->dummy()));
            if (tag == "HairPin") {
                readHairpin206(e, ctx, toHairpin(s));
            } else if (tag == "Ottava") {
                readOttava(e, ctx, toOttava(s));
            } else if (tag == "TextLine") {
                Read206::readTextLine206(e, ctx, toTextLine(s));
            } else if (tag == "Volta") {
                readVolta206(e, ctx, toVolta(s));
            } else if (tag == "Trill") {
                Read206::readTrill206(e, ctx, toTrill(s));
            } else if (tag == "Slur") {
                readSlur206(e, ctx, toSlur(s));
            } else {
                assert(tag == "Pedal");
                readPedal(e, ctx, toPedal(s));
            }
            score->addSpanner(s);
        } else if (tag == "Excerpt") {
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                if (score->isMaster()) {
                    MasterScore* mScore = static_cast<MasterScore*>(score);
                    Excerpt* ex = new Excerpt(mScore);
                    read400::TRead::read(ex, e, ctx);
                    mScore->excerpts().push_back(ex);
                } else {
                    LOGD("read206: readScore(): part cannot have parts");
                    e.skipCurrentElement();
                }
            }
        } else if (tag == "Score") {            // recursion
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                ctx.tracks().clear();
                ctx.clearUserTextStyles();
                MasterScore* m = score->masterScore();
                Score* s = m->createScore();
                ReadStyleHook::setupDefaultStyle(s);
                Excerpt* ex = new Excerpt(m);

                ex->setExcerptScore(s);
                ctx.setLastMeasure(nullptr);
                ReadContext exCtx(s);
                exCtx.setMasterCtx(&ctx);

                readScore206(s, e, exCtx);

                ex->setTracksMapping(ctx.tracks());
                m->addExcerpt(ex);
            }
        } else if (tag == "PageList") {
            e.skipCurrentElement();
        } else if (tag == "name") {
            String n = e.readText();
            if (!score->isMaster()) {                 //ignore the name if it's not a child score
                score->excerpt()->setName(n, /*saveAndNotify=*/ false);
            }
        } else if (tag == "layoutMode") {
            String s = e.readText();
            if (s == "line") {
                score->setLayoutMode(LayoutMode::LINE);
            } else if (s == "system") {
                score->setLayoutMode(LayoutMode::SYSTEM);
            } else {
                LOGD("layoutMode: %s", muPrintable(s));
            }
        } else {
            e.unknown();
        }
    }
    if (e.error() != muse::XmlStreamReader::NoError) {
        LOGE() << muse::String(u"XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber())
            .arg(muse::String::fromAscii(e.name().ascii()));
        return false;
    }

    score->connectTies();

    score->setFileDivision(Constants::DIVISION);

    score->setUpTempoMap();

    for (Part* p : score->parts()) {
        p->updateHarmonyChannels(false);
    }

    if (score->isMaster()) {
        MasterScore* ms = static_cast<MasterScore*>(score);
        ms->rebuildMidiMapping();
        ms->updateChannel();
    }

    if (score->isMaster()) {
        CompatUtils::assignInitialPartToExcerpts(score->masterScore()->excerpts());
    }

    return true;
}

Err Read206::readScore(Score* score, XmlReader& e, ReadInOutData* out)
{
    ReadContext ctx(score);
    if (out && out->overriddenSpatium.has_value()) {
        ctx.setSpatium(out->overriddenSpatium.value());
        ctx.setOverrideSpatium(true);
    }
    DEFER {
        if (out) {
            out->settingsCompat = std::move(ctx.settingCompat());
        }
    };

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "programVersion") {
            score->setMscoreVersion(e.readText());
        } else if (tag == "programRevision") {
            score->setMscoreRevision(e.readInt(nullptr, 16));
        } else if (tag == "Score") {
            if (!readScore206(score, e, ctx)) {
                return Err::FileBadFormat;
            }

            if (ctx.overrideSpatium() && out) {
                out->originalSpatium = ctx.originalSpatium();
            }
        } else if (tag == "Revision") {
            e.skipCurrentElement();
        }
    }

    int id = 1;
    for (auto& p : ctx.linkIds()) {
        LinkedObjects* le = p.second;
        le->setLid(score, id++);
    }

    for (Staff* s : score->staves()) {
        s->updateOttava();
    }

    // fix segment span
    SegmentType st = SegmentType::BarLineType;
    for (Segment* s = score->firstSegment(st); s; s = s->next1(st)) {
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            BarLine* b = toBarLine(s->element(staffIdx * VOICES));
            if (!b) {
                continue;
            }
            int sp = b->spanStaff();
            if (sp <= 0) {
                continue;
            }
            for (int span = 1; span <= sp; ++span) {
                BarLine* nb = toBarLine(s->element((staffIdx + span) * VOICES));
                if (!nb) {
                    nb = b->clone();
                    nb->setTrack((staffIdx + span) * VOICES);
                    s->add(nb);
                }
                nb->setSpanStaff(sp - span);
            }
            staffIdx += sp;
        }
    }
    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        Staff* s = score->staff(staffIdx);
        int sp = s->barLineSpan();
        if (sp <= 0) {
            continue;
        }
        for (int span = 1; span <= sp; ++span) {
            Staff* ns = score->staff(staffIdx + span);
            ns->setBarLineSpan(sp - span);
        }
        staffIdx += sp;
    }

    compat::CompatUtils::doCompatibilityConversions(score->masterScore());

    return Err::NoError;
}

bool Read206::pasteStaff(XmlReader&, Segment*, staff_idx_t, Fraction)
{
    UNREACHABLE;
    return false;
}

void Read206::pasteSymbols(XmlReader&, ChordRest*)
{
    UNREACHABLE;
}

void Read206::readTremoloCompat(compat::TremoloCompat*, XmlReader&)
{
    UNREACHABLE;
}

void Read206::doReadItem(EngravingItem*, XmlReader&)
{
    UNREACHABLE;
}
