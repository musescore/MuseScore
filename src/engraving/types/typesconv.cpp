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
#include "typesconv.h"

#include "types/translatablestring.h"

#include "symnames.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

template<typename T>
struct Item
{
    T type;
    AsciiStringView xml;
    TranslatableString userName;

    // NOTE Ideally we would write `TranslatableString userName = {};` and omit these constructors
    // But that causes internal compiler errors with certain versions of GCC/MinGW
    // See discussion at https://github.com/musescore/MuseScore/pull/12612

    Item() = default;
    Item(T type, AsciiStringView xml, const TranslatableString& userName = {})
        : type(type), xml(xml), userName(userName) {}
};

template<typename T, typename C>
static const TranslatableString& findUserNameByType(const C& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static TranslatableString dummy;
        return dummy;
    }

    return it->userName;
}

template<typename T, typename C>
static AsciiStringView findXmlTagByType(const C& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static AsciiStringView dummy;
        return dummy;
    }
    return it->xml;
}

template<typename T, typename C>
static T findTypeByXmlTag(const C& cont, const String& tag, T def)
{
    ByteArray ba = tag.toAscii();
    auto it = std::find_if(cont.cbegin(), cont.cend(), [ba](const Item<T>& i) {
        return i.xml == ba.constChar();
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        return def;
    }
    return it->type;
}

template<typename T, typename C>
static T findTypeByXmlTag(const C& cont, const AsciiStringView& tag, T def, bool silent = false)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [tag](const Item<T>& i) {
        return i.xml == tag;
    });

    if (it == cont.cend()) {
        if (!silent) {
            LOGE() << "not found type for tag: " << tag;
            assert(it != cont.cend());
        }
        return def;
    }

    return it->type;
}

// ==========================================================
String TConv::toXml(const std::vector<int>& v)
{
    StringList sl;
    for (int i : v) {
        sl << String::number(i);
    }
    return sl.join(u",");
}

std::vector<int> TConv::fromXml(const String& tag, const std::vector<int>& def)
{
    std::vector<int> list;
    StringList sl = tag.split(u',', mu::SkipEmptyParts);
    for (const String& s : sl) {
        bool ok = false;
        int i = s.simplified().toInt(&ok);
        if (!ok) {
            return def;
        }
        list.push_back(i);
    }
    return list;
}

static const std::vector<Item<ElementType> > ELEMENT_TYPES = {
    { ElementType::INVALID,              "invalid",              TranslatableString("engraving", "Invalid") },
    { ElementType::BRACKET_ITEM,         "BracketItem",          TranslatableString("engraving", "Bracket") },
    { ElementType::PART,                 "Part",                 TranslatableString("engraving", "Part") },
    { ElementType::STAFF,                "Staff",                TranslatableString("engraving", "Staff") },
    { ElementType::SCORE,                "Score",                TranslatableString("engraving", "Score") },
    { ElementType::SYMBOL,               "Symbol",               TranslatableString("engraving", "Symbol") },
    { ElementType::TEXT,                 "Text",                 TranslatableString("engraving", "Text") },
    { ElementType::MEASURE_NUMBER,       "MeasureNumber",        TranslatableString("engraving", "Measure number") },
    { ElementType::MMREST_RANGE,         "MMRestRange",          TranslatableString("engraving", "Multimeasure rest range") },
    { ElementType::INSTRUMENT_NAME,      "InstrumentName",       TranslatableString("engraving", "Instrument name") },
    { ElementType::SLUR_SEGMENT,         "SlurSegment",          TranslatableString("engraving", "Slur segment") },
    { ElementType::TIE_SEGMENT,          "TieSegment",           TranslatableString("engraving", "Tie segment") },
    { ElementType::BAR_LINE,             "BarLine",              TranslatableString("engraving", "Barline") },
    { ElementType::STAFF_LINES,          "StaffLines",           TranslatableString("engraving", "Staff lines") },
    { ElementType::SYSTEM_DIVIDER,       "SystemDivider",        TranslatableString("engraving", "System divider") },
    { ElementType::STEM_SLASH,           "StemSlash",            TranslatableString("engraving", "Stem slash") },
    { ElementType::ARPEGGIO,             "Arpeggio",             TranslatableString("engraving", "Arpeggio") },
    { ElementType::ACCIDENTAL,           "Accidental",           TranslatableString("engraving", "Accidental") },
    { ElementType::LEDGER_LINE,          "LedgerLine",           TranslatableString("engraving", "Ledger line") },
    { ElementType::STEM,                 "Stem",                 TranslatableString("engraving", "Stem") },
    { ElementType::NOTE,                 "Note",                 TranslatableString("engraving", "Note") },
    { ElementType::CLEF,                 "Clef",                 TranslatableString("engraving", "Clef") },
    { ElementType::KEYSIG,               "KeySig",               TranslatableString("engraving", "Key signature") },
    { ElementType::AMBITUS,              "Ambitus",              TranslatableString("engraving", "Ambitus") },
    { ElementType::TIMESIG,              "TimeSig",              TranslatableString("engraving", "Time signature") },
    { ElementType::REST,                 "Rest",                 TranslatableString("engraving", "Rest") },
    { ElementType::MMREST,               "MMRest",               TranslatableString("engraving", "Multimeasure rest") },
    { ElementType::DEAD_SLAPPED,         "DeadSlapped",          TranslatableString("engraving", "Dead slapped") },
    { ElementType::BREATH,               "Breath",               TranslatableString("engraving", "Breath") },
    { ElementType::MEASURE_REPEAT,       "MeasureRepeat",        TranslatableString("engraving", "Measure repeat") },
    { ElementType::TIE,                  "Tie",                  TranslatableString("engraving", "Tie") },
    { ElementType::ARTICULATION,         "Articulation",         TranslatableString("engraving", "Articulation") },
    { ElementType::FERMATA,              "Fermata",              TranslatableString("engraving", "Fermata") },
    { ElementType::CHORDLINE,            "ChordLine",            TranslatableString("engraving", "Chord line") },
    { ElementType::DYNAMIC,              "Dynamic",              TranslatableString("engraving", "Dynamic") },
    { ElementType::BEAM,                 "Beam",                 TranslatableString("engraving", "Beam") },
    { ElementType::HOOK,                 "Hook",                 TranslatableString("engraving", "Flag") }, // internally called "Hook", but "Flag" in SMuFL, so here externally too
    { ElementType::LYRICS,               "Lyrics",               TranslatableString("engraving", "Lyrics") },
    { ElementType::FIGURED_BASS,         "FiguredBass",          TranslatableString("engraving", "Figured bass") },
    { ElementType::MARKER,               "Marker",               TranslatableString("engraving", "Marker") },
    { ElementType::JUMP,                 "Jump",                 TranslatableString("engraving", "Jump") },
    { ElementType::FINGERING,            "Fingering",            TranslatableString("engraving", "Fingering") },
    { ElementType::TUPLET,               "Tuplet",               TranslatableString("engraving", "Tuplet") },
    { ElementType::TEMPO_TEXT,           "Tempo",                TranslatableString("engraving", "Tempo") },
    { ElementType::STAFF_TEXT,           "StaffText",            TranslatableString("engraving", "Staff text") },
    { ElementType::SYSTEM_TEXT,          "SystemText",           TranslatableString("engraving", "System text") },
    { ElementType::PLAYTECH_ANNOTATION,  "PlayTechAnnotation",   TranslatableString("engraving", "Playing technique annotation") },
    { ElementType::TRIPLET_FEEL,         "TripletFeel",          TranslatableString("engraving", "Triplet feel") },
    { ElementType::REHEARSAL_MARK,       "RehearsalMark",        TranslatableString("engraving", "Rehearsal mark") },
    { ElementType::INSTRUMENT_CHANGE,    "InstrumentChange",     TranslatableString("engraving", "Instrument change") },
    { ElementType::STAFFTYPE_CHANGE,     "StaffTypeChange",      TranslatableString("engraving", "Staff type change") },
    { ElementType::HARMONY,              "Harmony",              TranslatableString("engraving", "Chord symbol") },
    { ElementType::FRET_DIAGRAM,         "FretDiagram",          TranslatableString("engraving", "Fretboard diagram") },
    { ElementType::BEND,                 "Bend",                 TranslatableString("engraving", "Bend") },
    { ElementType::STRETCHED_BEND,       "Bend",                 TranslatableString("engraving", "Bend") },
    { ElementType::TREMOLOBAR,           "TremoloBar",           TranslatableString("engraving", "Tremolo bar") },
    { ElementType::VOLTA,                "Volta",                TranslatableString("engraving", "Volta") },
    { ElementType::HAIRPIN_SEGMENT,      "HairpinSegment",       TranslatableString("engraving", "Hairpin segment") },
    { ElementType::OTTAVA_SEGMENT,       "OttavaSegment",        TranslatableString("engraving", "Ottava segment") },
    { ElementType::TRILL_SEGMENT,        "TrillSegment",         TranslatableString("engraving", "Trill segment") },
    { ElementType::LET_RING_SEGMENT,     "LetRingSegment",       TranslatableString("engraving", "Let ring segment") },
    { ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, "GradualTempoChangeSegment",
      TranslatableString("engraving", "Gradual tempo change segment") },
    { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       TranslatableString("engraving", "Vibrato segment") },
    { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      TranslatableString("engraving", "Palm mute segment") },
    { ElementType::WHAMMY_BAR_SEGMENT,   "WhammyBarSegment",     TranslatableString("engraving", "Whammy bar segment") },
    { ElementType::RASGUEADO_SEGMENT,    "RasgueadoSegment",     TranslatableString("engraving", "Rasgueado segment") },
    { ElementType::HARMONIC_MARK_SEGMENT,    "HarmonicMarkSegment",    TranslatableString("engraving", "Harmonic mark segment") },
    { ElementType::PICK_SCRAPE_SEGMENT,    "PickScrapeSegment",    TranslatableString("engraving", "Pick scrape segment") },
    { ElementType::TEXTLINE_SEGMENT,     "TextLineSegment",      TranslatableString("engraving", "Text line segment") },
    { ElementType::VOLTA_SEGMENT,        "VoltaSegment",         TranslatableString("engraving", "Volta segment") },
    { ElementType::PEDAL_SEGMENT,        "PedalSegment",         TranslatableString("engraving", "Pedal segment") },
    { ElementType::LYRICSLINE_SEGMENT,   "LyricsLineSegment",    TranslatableString("engraving", "Melisma line segment") },
    { ElementType::GLISSANDO_SEGMENT,    "GlissandoSegment",     TranslatableString("engraving", "Glissando segment") },
    { ElementType::LAYOUT_BREAK,         "LayoutBreak",          TranslatableString("engraving", "Layout break") },
    { ElementType::SPACER,               "Spacer",               TranslatableString("engraving", "Spacer") },
    { ElementType::STAFF_STATE,          "StaffState",           TranslatableString("engraving", "Staff state") },
    { ElementType::NOTEHEAD,             "NoteHead",             TranslatableString("engraving", "Notehead") },
    { ElementType::NOTEDOT,              "NoteDot",              TranslatableString("engraving", "Note dot") },
    { ElementType::TREMOLO,              "Tremolo",              TranslatableString("engraving", "Tremolo") },
    { ElementType::IMAGE,                "Image",                TranslatableString("engraving", "Image") },
    { ElementType::MEASURE,              "Measure",              TranslatableString("engraving", "Measure") },
    { ElementType::SELECTION,            "Selection",            TranslatableString("engraving", "Selection") },
    { ElementType::LASSO,                "Lasso",                TranslatableString("engraving", "Lasso") },
    { ElementType::SHADOW_NOTE,          "ShadowNote",           TranslatableString("engraving", "Shadow note") },
    { ElementType::TAB_DURATION_SYMBOL,  "TabDurationSymbol",    TranslatableString("engraving", "Tab duration symbol") },
    { ElementType::FSYMBOL,              "FSymbol",              TranslatableString("engraving", "Font symbol") },
    { ElementType::PAGE,                 "Page",                 TranslatableString("engraving", "Page") },
    { ElementType::HAIRPIN,              "HairPin",              TranslatableString("engraving", "Hairpin") },
    { ElementType::OTTAVA,               "Ottava",               TranslatableString("engraving", "Ottava") },
    { ElementType::PEDAL,                "Pedal",                TranslatableString("engraving", "Pedal") },
    { ElementType::TRILL,                "Trill",                TranslatableString("engraving", "Trill") },
    { ElementType::LET_RING,             "LetRing",              TranslatableString("engraving", "Let ring") },
    { ElementType::GRADUAL_TEMPO_CHANGE, "GradualTempoChange",   TranslatableString("engraving", "Gradual tempo change") },
    { ElementType::VIBRATO,              "Vibrato",              TranslatableString("engraving", "Vibrato") },
    { ElementType::PALM_MUTE,            "PalmMute",             TranslatableString("engraving", "Palm mute") },
    { ElementType::WHAMMY_BAR,           "WhammyBar",            TranslatableString("engraving", "Whammy bar") },
    { ElementType::RASGUEADO,            "Rasgueado",            TranslatableString("engraving", "Rasgueado") },
    { ElementType::HARMONIC_MARK,        "HarmonicMark",         TranslatableString("engraving", "Harmonic mark") },
    { ElementType::PICK_SCRAPE,          "PickScrape",           TranslatableString("engraving", "Pick scrape out") },
    { ElementType::TEXTLINE,             "TextLine",             TranslatableString("engraving", "Text line") },
    { ElementType::TEXTLINE_BASE,        "TextLineBase",         TranslatableString("engraving", "Text line base") },    // remove
    { ElementType::NOTELINE,             "NoteLine",             TranslatableString("engraving", "Note line") },
    { ElementType::LYRICSLINE,           "LyricsLine",           TranslatableString("engraving", "Melisma line") },
    { ElementType::GLISSANDO,            "Glissando",            TranslatableString("engraving", "Glissando") },
    { ElementType::BRACKET,              "Bracket",              TranslatableString("engraving", "Bracket") },
    { ElementType::SEGMENT,              "Segment",              TranslatableString("engraving", "Segment") },
    { ElementType::SYSTEM,               "System",               TranslatableString("engraving", "System") },
    { ElementType::COMPOUND,             "Compound",             TranslatableString("engraving", "Compound") },
    { ElementType::CHORD,                "Chord",                TranslatableString("engraving", "Chord") },
    { ElementType::SLUR,                 "Slur",                 TranslatableString("engraving", "Slur") },
    { ElementType::ELEMENT,              "EngravingItem",        TranslatableString("engraving", "Element") },
    { ElementType::ELEMENT_LIST,         "ElementList",          TranslatableString("engraving", "Element list") },
    { ElementType::STAFF_LIST,           "StaffList",            TranslatableString("engraving", "Staff list") },
    { ElementType::MEASURE_LIST,         "MeasureList",          TranslatableString("engraving", "Measure list") },
    { ElementType::HBOX,                 "HBox",                 TranslatableString("engraving", "Horizontal frame") },
    { ElementType::VBOX,                 "VBox",                 TranslatableString("engraving", "Vertical frame") },
    { ElementType::TBOX,                 "TBox",                 TranslatableString("engraving", "Text frame") },
    { ElementType::FBOX,                 "FBox",                 TranslatableString("engraving", "Fretboard diagram frame") },
    { ElementType::ACTION_ICON,          "ActionIcon",           TranslatableString::untranslatable("Action icon") },
    { ElementType::OSSIA,                "Ossia",                TranslatableString("engraving", "Ossia") },
    { ElementType::BAGPIPE_EMBELLISHMENT, "BagpipeEmbellishment", TranslatableString("engraving", "Bagpipe embellishment") },
    { ElementType::STICKING,             "Sticking",             TranslatableString("engraving", "Sticking") },
    { ElementType::GRACE_NOTES_GROUP,    "GraceNotesGroup",      TranslatableString::untranslatable("Grace notes group") },
    { ElementType::FRET_CIRCLE,          "FretCircle",           TranslatableString::untranslatable("Fret circle") },
    { ElementType::ROOT_ITEM,            "RootItem",             TranslatableString::untranslatable("Root item") },
    { ElementType::DUMMY,                "Dummy",                TranslatableString::untranslatable("Dummy") },
};

const TranslatableString& TConv::userName(ElementType v)
{
    return findUserNameByType<ElementType>(ELEMENT_TYPES, v);
}

AsciiStringView TConv::toXml(ElementType v)
{
    return findXmlTagByType<ElementType>(ELEMENT_TYPES, v);
}

ElementType TConv::fromXml(const AsciiStringView& tag, ElementType def, bool silent)
{
    return findTypeByXmlTag<ElementType>(ELEMENT_TYPES, tag, def, silent);
}

static const std::vector<Item<AlignH> > ALIGN_H = {
    { AlignH::LEFT,     "left" },
    { AlignH::RIGHT,    "right" },
    { AlignH::HCENTER,  "center" },
};

static const std::vector<Item<AlignV> > ALIGN_V = {
    { AlignV::TOP,      "top" },
    { AlignV::VCENTER,  "center" },
    { AlignV::BOTTOM,   "bottom" },
    { AlignV::BASELINE, "baseline" },
};

AlignH TConv::fromXml(const AsciiStringView& tag, AlignH def)
{
    return findTypeByXmlTag<AlignH>(ALIGN_H, tag, def);
}

AlignV TConv::fromXml(const AsciiStringView& tag, AlignV def)
{
    return findTypeByXmlTag<AlignV>(ALIGN_V, tag, def);
}

String TConv::toXml(Align v)
{
    StringList sl;
    sl << String::fromAscii(findXmlTagByType<AlignH>(ALIGN_H, v.horizontal).ascii());
    sl << String::fromAscii(findXmlTagByType<AlignV>(ALIGN_V, v.vertical).ascii());
    return sl.join(u",");
}

Align TConv::fromXml(const String& str, Align def)
{
    StringList sl = str.split(',');
    if (sl.size() != 2) {
        LOGD() << "bad align value: " << str;
        return def;
    }

    Align a;
    a.horizontal = findTypeByXmlTag<AlignH>(ALIGN_H, sl.at(0), def.horizontal);
    a.vertical = findTypeByXmlTag<AlignV>(ALIGN_V, sl.at(1), def.vertical);
    return a;
}

String TConv::translatedUserName(SymId v)
{
    return SymNames::translatedUserNameForSymId(v);
}

AsciiStringView TConv::toXml(SymId v)
{
    return SymNames::nameForSymId(v);
}

SymId TConv::fromXml(const AsciiStringView& tag, SymId def)
{
    return SymNames::symIdByName(tag, def);
}

static const std::array<Item<Orientation>, 2> ORIENTATION = { {
    { Orientation::VERTICAL,    "vertical",     TranslatableString("engraving", "Vertical") },
    { Orientation::HORIZONTAL,  "horizontal",   TranslatableString("engraving", "Horizontal") },
} };

String TConv::translatedUserName(Orientation v)
{
    return findUserNameByType<Orientation>(ORIENTATION, v).translated();
}

AsciiStringView TConv::toXml(Orientation v)
{
    return findXmlTagByType<Orientation>(ORIENTATION, v);
}

Orientation TConv::fromXml(const AsciiStringView& tag, Orientation def)
{
    return findTypeByXmlTag<Orientation>(ORIENTATION, tag, def);
}

static const std::array<Item<NoteHeadType>, 5> NOTEHEAD_TYPES = { {
    { NoteHeadType::HEAD_AUTO,      "auto",    TranslatableString("engraving", "Auto") },
    { NoteHeadType::HEAD_WHOLE,     "whole",   TranslatableString("engraving/noteheadtype", "Whole") },
    { NoteHeadType::HEAD_HALF,      "half",    TranslatableString("engraving/noteheadtype", "Half") },
    { NoteHeadType::HEAD_QUARTER,   "quarter", TranslatableString("engraving/noteheadtype", "Quarter") },
    { NoteHeadType::HEAD_BREVIS,    "breve",   TranslatableString("engraving/noteheadtype", "Breve") },
} };

String TConv::translatedUserName(NoteHeadType v)
{
    return findUserNameByType<NoteHeadType>(NOTEHEAD_TYPES, v).translated();
}

AsciiStringView TConv::toXml(NoteHeadType v)
{
    return findXmlTagByType<NoteHeadType>(NOTEHEAD_TYPES, v);
}

NoteHeadType TConv::fromXml(const AsciiStringView& tag, NoteHeadType def)
{
    return findTypeByXmlTag<NoteHeadType>(NOTEHEAD_TYPES, tag, def);
}

/* *INDENT-OFF* */
static const std::vector<Item<NoteHeadScheme> > NOTEHEAD_SCHEMES = {
    { NoteHeadScheme::HEAD_AUTO,                "auto",              TranslatableString("engraving", "Auto") },
    { NoteHeadScheme::HEAD_NORMAL,              "normal",            TranslatableString("engraving/noteheadscheme", "Normal") },
    { NoteHeadScheme::HEAD_PITCHNAME,           "name-pitch",        TranslatableString("engraving/noteheadscheme", "Pitch names") },
    { NoteHeadScheme::HEAD_PITCHNAME_GERMAN,    "name-pitch-german", TranslatableString("engraving/noteheadscheme", "German pitch names") },
    { NoteHeadScheme::HEAD_SOLFEGE,             "solfege-movable",   TranslatableString("engraving/noteheadscheme", "Solf\u00e8ge movable Do") },  // &egrave;
    { NoteHeadScheme::HEAD_SOLFEGE_FIXED,       "solfege-fixed",     TranslatableString("engraving/noteheadscheme", "Solf\u00e8ge fixed Do") },    // &egrave;
    { NoteHeadScheme::HEAD_SHAPE_NOTE_4,        "shape-4",           TranslatableString("engraving/noteheadscheme", "4-shape (Walker)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,  "shape-7-aikin",     TranslatableString("engraving/noteheadscheme", "7-shape (Aikin)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,   "shape-7-funk",      TranslatableString("engraving/noteheadscheme", "7-shape (Funk)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER, "shape-7-walker",    TranslatableString("engraving/noteheadscheme", "7-shape (Walker)") }
};
/* *INDENT-ON* */

String TConv::translatedUserName(NoteHeadScheme v)
{
    return findUserNameByType<NoteHeadScheme>(NOTEHEAD_SCHEMES, v).translated();
}

AsciiStringView TConv::toXml(NoteHeadScheme v)
{
    return findXmlTagByType<NoteHeadScheme>(NOTEHEAD_SCHEMES, v);
}

NoteHeadScheme TConv::fromXml(const AsciiStringView& tag, NoteHeadScheme def)
{
    return findTypeByXmlTag<NoteHeadScheme>(NOTEHEAD_SCHEMES, tag, def);
}

static const std::vector<Item<NoteHeadGroup> > NOTEHEAD_GROUPS = {
    { NoteHeadGroup::HEAD_NORMAL,           "normal",         TranslatableString("engraving/noteheadgroup", "Normal") },
    { NoteHeadGroup::HEAD_CROSS,            "cross",          TranslatableString("engraving/noteheadgroup", "Cross") },
    { NoteHeadGroup::HEAD_PLUS,             "plus",           TranslatableString("engraving/noteheadgroup", "Plus") },
    { NoteHeadGroup::HEAD_XCIRCLE,          "xcircle",        TranslatableString("engraving/noteheadgroup", "XCircle") },
    { NoteHeadGroup::HEAD_WITHX,            "withx",          TranslatableString("engraving/noteheadgroup", "With X") },
    { NoteHeadGroup::HEAD_TRIANGLE_UP,      "triangle-up",    TranslatableString("engraving/noteheadgroup", "Triangle up") },
    { NoteHeadGroup::HEAD_TRIANGLE_DOWN,    "triangle-down",  TranslatableString("engraving/noteheadgroup", "Triangle down") },
    { NoteHeadGroup::HEAD_SLASHED1,         "slashed1",       TranslatableString("engraving/noteheadgroup", "Slashed (forwards)") },
    { NoteHeadGroup::HEAD_SLASHED2,         "slashed2",       TranslatableString("engraving/noteheadgroup", "Slashed (backwards)") },
    { NoteHeadGroup::HEAD_DIAMOND,          "diamond",        TranslatableString("engraving/noteheadgroup", "Diamond") },
    { NoteHeadGroup::HEAD_DIAMOND_OLD,      "diamond-old",    TranslatableString("engraving/noteheadgroup", "Diamond (old)") },
    { NoteHeadGroup::HEAD_CIRCLED,          "circled",        TranslatableString("engraving/noteheadgroup", "Circled") },
    { NoteHeadGroup::HEAD_CIRCLED_LARGE,    "circled-large",  TranslatableString("engraving/noteheadgroup", "Circled large") },
    { NoteHeadGroup::HEAD_LARGE_ARROW,      "large-arrow",    TranslatableString("engraving/noteheadgroup", "Large arrow") },
    { NoteHeadGroup::HEAD_BREVIS_ALT,       "altbrevis",      TranslatableString("engraving/noteheadgroup", "Alt. brevis") },

    { NoteHeadGroup::HEAD_SLASH,            "slash",          TranslatableString("engraving/noteheadgroup", "Slash") },
    { NoteHeadGroup::HEAD_LARGE_DIAMOND,    "large-diamond",  TranslatableString("engraving/noteheadgroup", "Large diamond") },

    { NoteHeadGroup::HEAD_HEAVY_CROSS,      "heavy-cross",    TranslatableString("engraving/noteheadgroup", "Heavy cross") },
    { NoteHeadGroup::HEAD_HEAVY_CROSS_HAT,  "heavy-cross-hat",    TranslatableString("engraving/noteheadgroup", "Heavy cross hat") },

    // shape notes
    { NoteHeadGroup::HEAD_SOL,  "sol",       TranslatableString("engraving/noteheadgroup", "Sol") },
    { NoteHeadGroup::HEAD_LA,   "la",        TranslatableString("engraving/noteheadgroup", "La") },
    { NoteHeadGroup::HEAD_FA,   "fa",        TranslatableString("engraving/noteheadgroup", "Fa") },
    { NoteHeadGroup::HEAD_MI,   "mi",        TranslatableString("engraving/noteheadgroup", "Mi") },
    { NoteHeadGroup::HEAD_DO,   "do",        TranslatableString("engraving/noteheadgroup", "Do") },
    { NoteHeadGroup::HEAD_RE,   "re",        TranslatableString("engraving/noteheadgroup", "Re") },
    { NoteHeadGroup::HEAD_TI,   "ti",        TranslatableString("engraving/noteheadgroup", "Ti") },

    // not exposed
    { NoteHeadGroup::HEAD_DO_WALKER,    "do-walker", TranslatableString("engraving/noteheadgroup", "Do (Walker)") },
    { NoteHeadGroup::HEAD_RE_WALKER,    "re-walker", TranslatableString("engraving/noteheadgroup", "Re (Walker)") },
    { NoteHeadGroup::HEAD_TI_WALKER,    "ti-walker", TranslatableString("engraving/noteheadgroup", "Ti (Walker)") },
    { NoteHeadGroup::HEAD_DO_FUNK,      "do-funk",   TranslatableString("engraving/noteheadgroup", "Do (Funk)") },
    { NoteHeadGroup::HEAD_RE_FUNK,      "re-funk",   TranslatableString("engraving/noteheadgroup", "Re (Funk)") },
    { NoteHeadGroup::HEAD_TI_FUNK,      "ti-funk",   TranslatableString("engraving/noteheadgroup", "Ti (Funk)") },

    // note name
    { NoteHeadGroup::HEAD_DO_NAME,      "do-name",  TranslatableString("engraving/noteheadgroup",  "Do (Name)") },
    { NoteHeadGroup::HEAD_DI_NAME,      "di-name",  TranslatableString("engraving/noteheadgroup",  "Di (Name)") },
    { NoteHeadGroup::HEAD_RA_NAME,      "ra-name",  TranslatableString("engraving/noteheadgroup",  "Ra (Name)") },
    { NoteHeadGroup::HEAD_RE_NAME,      "re-name",  TranslatableString("engraving/noteheadgroup",  "Re (Name)") },
    { NoteHeadGroup::HEAD_RI_NAME,      "ri-name",  TranslatableString("engraving/noteheadgroup",  "Ri (Name)") },
    { NoteHeadGroup::HEAD_ME_NAME,      "me-name",  TranslatableString("engraving/noteheadgroup",  "Me (Name)") },
    { NoteHeadGroup::HEAD_MI_NAME,      "mi-name",  TranslatableString("engraving/noteheadgroup",  "Mi (Name)") },
    { NoteHeadGroup::HEAD_FA_NAME,      "fa-name",  TranslatableString("engraving/noteheadgroup",  "Fa (Name)") },
    { NoteHeadGroup::HEAD_FI_NAME,      "fi-name",  TranslatableString("engraving/noteheadgroup",  "Fi (Name)") },
    { NoteHeadGroup::HEAD_SE_NAME,      "se-name",  TranslatableString("engraving/noteheadgroup",  "Se (Name)") },
    { NoteHeadGroup::HEAD_SOL_NAME,     "sol-name", TranslatableString("engraving/noteheadgroup",  "Sol (Name)") },
    { NoteHeadGroup::HEAD_LE_NAME,      "le-name",  TranslatableString("engraving/noteheadgroup",  "Le (Name)") },
    { NoteHeadGroup::HEAD_LA_NAME,      "la-name",  TranslatableString("engraving/noteheadgroup",  "La (Name)") },
    { NoteHeadGroup::HEAD_LI_NAME,      "li-name",  TranslatableString("engraving/noteheadgroup",  "Li (Name)") },
    { NoteHeadGroup::HEAD_TE_NAME,      "te-name",  TranslatableString("engraving/noteheadgroup",  "Te (Name)") },
    { NoteHeadGroup::HEAD_TI_NAME,      "ti-name",  TranslatableString("engraving/noteheadgroup",  "Ti (Name)") },
    { NoteHeadGroup::HEAD_SI_NAME,      "si-name",  TranslatableString("engraving/noteheadgroup",  "Si (Name)") },

    { NoteHeadGroup::HEAD_A_SHARP,      "a-sharp-name", TranslatableString("engraving/noteheadgroup",  "A♯ (Name)") },
    { NoteHeadGroup::HEAD_A,            "a-name",       TranslatableString("engraving/noteheadgroup",  "A (Name)") },
    { NoteHeadGroup::HEAD_A_FLAT,       "a-flat-name",  TranslatableString("engraving/noteheadgroup",  "A♭ (Name)") },
    { NoteHeadGroup::HEAD_B_SHARP,      "b-sharp-name", TranslatableString("engraving/noteheadgroup",  "B♯ (Name)") },
    { NoteHeadGroup::HEAD_B,            "b-name",       TranslatableString("engraving/noteheadgroup",  "B (Name)") },
    { NoteHeadGroup::HEAD_B_FLAT,       "b-flat-name",  TranslatableString("engraving/noteheadgroup",  "B♭ (Name)") },
    { NoteHeadGroup::HEAD_C_SHARP,      "c-sharp-name", TranslatableString("engraving/noteheadgroup",  "C♯ (Name)") },
    { NoteHeadGroup::HEAD_C,            "c-name",       TranslatableString("engraving/noteheadgroup",  "C (Name)") },
    { NoteHeadGroup::HEAD_C_FLAT,       "c-flat-name",  TranslatableString("engraving/noteheadgroup",  "C♭ (Name)") },
    { NoteHeadGroup::HEAD_D_SHARP,      "d-sharp-name", TranslatableString("engraving/noteheadgroup",  "D♯ (Name)") },
    { NoteHeadGroup::HEAD_D,            "d-name",       TranslatableString("engraving/noteheadgroup",  "D (Name)") },
    { NoteHeadGroup::HEAD_D_FLAT,       "d-flat-name",  TranslatableString("engraving/noteheadgroup",  "D♭ (Name)") },
    { NoteHeadGroup::HEAD_E_SHARP,      "e-sharp-name", TranslatableString("engraving/noteheadgroup",  "E♯ (Name)") },
    { NoteHeadGroup::HEAD_E,            "e-name",       TranslatableString("engraving/noteheadgroup",  "E (Name)") },
    { NoteHeadGroup::HEAD_E_FLAT,       "e-flat-name",  TranslatableString("engraving/noteheadgroup",  "E♭ (Name)") },
    { NoteHeadGroup::HEAD_F_SHARP,      "f-sharp-name", TranslatableString("engraving/noteheadgroup",  "F♯ (Name)") },
    { NoteHeadGroup::HEAD_F,            "f-name",       TranslatableString("engraving/noteheadgroup",  "F (Name)") },
    { NoteHeadGroup::HEAD_F_FLAT,       "f-flat-name",  TranslatableString("engraving/noteheadgroup",  "F♭ (Name)") },
    { NoteHeadGroup::HEAD_G_SHARP,      "g-sharp-name", TranslatableString("engraving/noteheadgroup",  "G♯ (Name)") },
    { NoteHeadGroup::HEAD_G,            "g-name",       TranslatableString("engraving/noteheadgroup",  "G (Name)") },
    { NoteHeadGroup::HEAD_G_FLAT,       "g-flat-name",  TranslatableString("engraving/noteheadgroup",  "G♭ (Name)") },
    { NoteHeadGroup::HEAD_H,            "h-name",       TranslatableString("engraving/noteheadgroup",  "H (Name)") },
    { NoteHeadGroup::HEAD_H_SHARP,      "h-sharp-name", TranslatableString("engraving/noteheadgroup",  "H♯ (Name)") },

    { NoteHeadGroup::HEAD_CUSTOM,       "custom",       TranslatableString("engraving",  "Custom") }
};

const TranslatableString& TConv::userName(NoteHeadGroup v)
{
    return findUserNameByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v);
}

String TConv::translatedUserName(NoteHeadGroup v)
{
    return findUserNameByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v).translated();
}

AsciiStringView TConv::toXml(NoteHeadGroup v)
{
    return findXmlTagByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v);
}

NoteHeadGroup TConv::fromXml(const AsciiStringView& tag, NoteHeadGroup def)
{
    auto it = std::find_if(NOTEHEAD_GROUPS.cbegin(), NOTEHEAD_GROUPS.cend(), [tag](const Item<NoteHeadGroup>& i) {
        return i.xml == tag;
    });

    if (it != NOTEHEAD_GROUPS.cend()) {
        return it->type;
    }

    // compatibility
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<NoteHeadGroup>(v);
    }

    return def;
}

static const std::vector<Item<ClefType> > CLEF_TYPES = {
    { ClefType::G,          "G",        TranslatableString("engraving/cleftype", "Treble clef") },
    { ClefType::G15_MB,     "G15mb",    TranslatableString("engraving/cleftype", "Treble clef 15ma bassa") },
    { ClefType::G8_VB,      "G8vb",     TranslatableString("engraving/cleftype", "Treble clef 8va bassa") },
    { ClefType::G8_VA,      "G8va",     TranslatableString("engraving/cleftype", "Treble clef 8va alta") },
    { ClefType::G15_MA,     "G15ma",    TranslatableString("engraving/cleftype", "Treble clef 15ma alta") },
    { ClefType::G8_VB_O,    "G8vbo",    TranslatableString("engraving/cleftype", "Double treble clef 8va bassa on 2nd line") },
    { ClefType::G8_VB_P,    "G8vbp",    TranslatableString("engraving/cleftype", "Treble clef optional 8va bassa") },
    { ClefType::G_1,        "G1",       TranslatableString("engraving/cleftype", "French violin clef") },
    { ClefType::C1,         "C1",       TranslatableString("engraving/cleftype", "Soprano clef") },
    { ClefType::C2,         "C2",       TranslatableString("engraving/cleftype", "Mezzo-soprano clef") },
    { ClefType::C3,         "C3",       TranslatableString("engraving/cleftype", "Alto clef") },
    { ClefType::C4,         "C4",       TranslatableString("engraving/cleftype", "Tenor clef") },
    { ClefType::C5,         "C5",       TranslatableString("engraving/cleftype", "Baritone clef (C clef)") },
    { ClefType::C_19C,      "C_19C",    TranslatableString("engraving/cleftype", "C clef, H shape (19th century)") },
    { ClefType::C1_F18C,    "C1_F18C",  TranslatableString("engraving/cleftype", "Soprano clef (French, 18th century)") },
    { ClefType::C3_F18C,    "C3_F18C",  TranslatableString("engraving/cleftype", "Alto clef (French, 18th century)") },
    { ClefType::C4_F18C,    "C4_F18C",  TranslatableString("engraving/cleftype", "Tenor clef (French, 18th century)") },
    { ClefType::C1_F20C,    "C1_F20C",  TranslatableString("engraving/cleftype", "Soprano clef (French, 20th century)") },
    { ClefType::C3_F20C,    "C3_F20C",  TranslatableString("engraving/cleftype", "Alto clef (French, 20th century)") },
    { ClefType::C4_F20C,    "C4_F20C",  TranslatableString("engraving/cleftype", "Tenor clef (French, 20th century)") },
    { ClefType::F,          "F",        TranslatableString("engraving/cleftype", "Bass clef") },
    { ClefType::F15_MB,     "F15mb",    TranslatableString("engraving/cleftype", "Bass clef 15ma bassa") },
    { ClefType::F8_VB,      "F8vb",     TranslatableString("engraving/cleftype", "Bass clef 8va bassa") },
    { ClefType::F_8VA,      "F8va",     TranslatableString("engraving/cleftype", "Bass clef 8va alta") },
    { ClefType::F_15MA,     "F15ma",    TranslatableString("engraving/cleftype", "Bass clef 15ma alta") },
    { ClefType::F_B,        "F3",       TranslatableString("engraving/cleftype", "Baritone clef (F clef)") },
    { ClefType::F_C,        "F5",       TranslatableString("engraving/cleftype", "Subbass clef") },
    { ClefType::F_F18C,     "F_F18C",   TranslatableString("engraving/cleftype", "F clef (French, 18th century)") },
    { ClefType::F_19C,      "F_19C",    TranslatableString("engraving/cleftype", "F clef (19th century)") },

    { ClefType::PERC,       "PERC",     TranslatableString("engraving/cleftype", "Percussion") },
    { ClefType::PERC2,      "PERC2",    TranslatableString("engraving/cleftype", "Percussion 2") },

    { ClefType::TAB,        "TAB",      TranslatableString("engraving/cleftype", "Tablature") },
    { ClefType::TAB4,       "TAB4",     TranslatableString("engraving/cleftype", "Tablature 4 lines") },
    { ClefType::TAB_SERIF,  "TAB2",     TranslatableString("engraving/cleftype", "Tablature Serif") },
    { ClefType::TAB4_SERIF, "TAB4_SERIF", TranslatableString("engraving/cleftype", "Tablature Serif 4 lines") },
};

const TranslatableString& TConv::userName(ClefType v)
{
    return findUserNameByType<ClefType>(CLEF_TYPES, v);
}

String TConv::translatedUserName(ClefType v)
{
    return findUserNameByType<ClefType>(CLEF_TYPES, v).translated();
}

AsciiStringView TConv::toXml(ClefType v)
{
    return findXmlTagByType<ClefType>(CLEF_TYPES, v);
}

ClefType TConv::fromXml(const AsciiStringView& tag, ClefType def)
{
    auto it = std::find_if(CLEF_TYPES.cbegin(), CLEF_TYPES.cend(), [tag](const Item<ClefType>& i) {
        return i.xml == tag;
    });

    if (it != CLEF_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<ClefType>(v);
    }

    return def;
}

struct DynamicItem
{
    DynamicType type;
    AsciiStringView xml;
    SymId symId;
};

static const std::vector<DynamicItem> DYNAMIC_TYPES = {
    { DynamicType::OTHER,   "other-dynamics",   SymId::noSym },
    { DynamicType::PPPPPP,  "pppppp",           SymId::dynamicPPPPPP },
    { DynamicType::PPPPP,   "ppppp",            SymId::dynamicPPPPP },
    { DynamicType::PPPP,    "pppp",             SymId::dynamicPPPP },
    { DynamicType::PPP,     "ppp",              SymId::dynamicPPP },
    { DynamicType::PP,      "pp",               SymId::dynamicPP },
    { DynamicType::P,       "p",                SymId::dynamicPiano },

    { DynamicType::MP,      "mp",               SymId::dynamicMP },
    { DynamicType::MF,      "mf",               SymId::dynamicMF },

    { DynamicType::F,       "f",                SymId::dynamicForte },
    { DynamicType::FF,      "ff",               SymId::dynamicFF },
    { DynamicType::FFF,     "fff",              SymId::dynamicFFF },
    { DynamicType::FFFF,    "ffff",             SymId::dynamicFFFF },
    { DynamicType::FFFFF,   "fffff",            SymId::dynamicFFFFF },
    { DynamicType::FFFFFF,  "ffffff",           SymId::dynamicFFFFFF },

    { DynamicType::FP,      "fp",               SymId::dynamicFortePiano },
    { DynamicType::PF,      "pf",               SymId::noSym },

    { DynamicType::SF,      "sf",               SymId::dynamicSforzando1 },
    { DynamicType::SFZ,     "sfz",              SymId::dynamicSforzato },
    { DynamicType::SFF,     "sff",              SymId::noSym },
    { DynamicType::SFFZ,    "sffz",             SymId::dynamicSforzatoFF },
    { DynamicType::SFP,     "sfp",              SymId::dynamicSforzandoPiano },
    { DynamicType::SFPP,    "sfpp",             SymId::dynamicSforzandoPianissimo },

    { DynamicType::RFZ,     "rfz",              SymId::dynamicRinforzando2 },
    { DynamicType::RF,      "rf",               SymId::dynamicRinforzando1 },
    { DynamicType::FZ,      "fz",               SymId::dynamicForzando },
    { DynamicType::M,       "m",                SymId::dynamicMezzo },
    { DynamicType::R,       "r",                SymId::dynamicRinforzando },
    { DynamicType::S,       "s",                SymId::dynamicSforzando },
    { DynamicType::Z,       "z",                SymId::dynamicZ },
    { DynamicType::N,       "n",                SymId::dynamicNiente },
};

String TConv::translatedUserName(DynamicType v)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [v](const DynamicItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        return String();
    }
    return String::fromAscii(it->xml.ascii());
}

SymId TConv::symId(DynamicType v)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [v](const DynamicItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        return SymId::noSym;
    }
    return it->symId;
}

DynamicType TConv::dynamicType(SymId v)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [v](const DynamicItem& i) {
        return i.symId == v;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        return DynamicType::OTHER;
    }
    return it->type;
}

DynamicType TConv::dynamicType(const AsciiStringView& tag)
{
    static const std::map<AsciiStringView, DynamicType> DYNAMIC_STR_MAP = {
        {
            "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
            DynamicType::PPPPPP },
        { "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
          DynamicType::PPPPP, },
        { "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
          DynamicType::PPPP },
        { "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
          DynamicType::PPP },
        { "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
          DynamicType::PP },
        { "<sym>dynamicPiano</sym>",
          DynamicType::P },

        { "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>",
          DynamicType::MP },
        { "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>",
          DynamicType::MF },

        { "<sym>dynamicForte</sym>",
          DynamicType::F },
        { "<sym>dynamicForte</sym><sym>dynamicForte</sym>",
          DynamicType::FF },
        { "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
          DynamicType::FFF },
        { "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
          DynamicType::FFFF },
        { "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
          DynamicType::FFFFF },
        {
            "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
            DynamicType::FFFFFF },

        { "<sym>dynamicForte</sym><sym>dynamicPiano</sym>",
          DynamicType::FP },
        { "<sym>dynamicPiano</sym><sym>dynamicForte</sym>",
          DynamicType::PF },

        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>",
          DynamicType::SF },
        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>",
          DynamicType::SFZ },
        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>",
          DynamicType::SFF },
        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>",
          DynamicType::SFFZ },
        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>",
          DynamicType::SFP },
        { "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>",
          DynamicType::SFPP },

        { "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>",
          DynamicType::RFZ },
        { "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>",
          DynamicType::RF },
        { "<sym>dynamicForte</sym><sym>dynamicZ</sym>",
          DynamicType::FZ },

        { "<sym>dynamicMezzo</sym>", DynamicType::M },
        { "<sym>dynamicRinforzando</sym>", DynamicType::R },
        { "<sym>dynamicSforzando</sym>", DynamicType::S },
        { "<sym>dynamicZ</sym>", DynamicType::Z },
        { "<sym>dynamicNiente</sym>", DynamicType::N }
    };

    auto search = DYNAMIC_STR_MAP.find(tag);
    if (search != DYNAMIC_STR_MAP.cend()) {
        return search->second;
    }

    return DynamicType::OTHER;
}

AsciiStringView TConv::toXml(DynamicType v)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [v](const DynamicItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        static AsciiStringView dummy;
        return dummy;
    }
    return it->xml;
}

DynamicType TConv::fromXml(const AsciiStringView& tag, DynamicType def)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [tag](const DynamicItem& i) {
        return i.xml == tag;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        return def;
    }
    return it->type;
}

static const std::vector<Item<DynamicRange> > DYNAMIC_RANGES = {
    { DynamicRange::STAFF,  "staff" },
    { DynamicRange::PART,   "part" },
    { DynamicRange::SYSTEM, "system" },
};

String TConv::translatedUserName(DynamicRange v)
{
    return findUserNameByType<DynamicRange>(DYNAMIC_RANGES, v).translated();
}

String TConv::toXml(DynamicRange v)
{
    return String::number(static_cast<int>(v));
}

DynamicRange TConv::fromXml(const AsciiStringView& tag, DynamicRange def)
{
    bool ok = false;
    int v = tag.toInt(&ok);
    return ok ? DynamicRange(v) : def;
}

static const std::vector<Item<DynamicSpeed> > DYNAMIC_SPEEDS = {
    { DynamicSpeed::NORMAL, "normal" },
    { DynamicSpeed::SLOW,   "slow" },
    { DynamicSpeed::FAST,   "fast" },
};

String TConv::translatedUserName(DynamicSpeed v)
{
    return findUserNameByType<DynamicSpeed>(DYNAMIC_SPEEDS, v).translated();
}

AsciiStringView TConv::toXml(DynamicSpeed v)
{
    return findXmlTagByType<DynamicSpeed>(DYNAMIC_SPEEDS, v);
}

DynamicSpeed TConv::fromXml(const AsciiStringView& tag, DynamicSpeed def)
{
    return findTypeByXmlTag<DynamicSpeed>(DYNAMIC_SPEEDS, tag, def);
}

static const std::vector<Item<HookType> > HOOK_TYPES = {
    { HookType::NONE,       "hook_none" },
    { HookType::HOOK_90,    "hook_90" },
    { HookType::HOOK_45,    "hook_45" },
    { HookType::HOOK_90T,   "hook_90t" },
};

String TConv::translatedUserName(HookType v)
{
    return findUserNameByType<HookType>(HOOK_TYPES, v).translated();
}

String TConv::toXml(HookType v)
{
    return String::number(static_cast<int>(v));
}

HookType TConv::fromXml(const AsciiStringView& tag, HookType def)
{
    bool ok = false;
    int v = tag.toInt(&ok);
    return ok ? HookType(v) : def;
}

static const std::vector<Item<LineType> > LINE_TYPES = {
    { LineType::SOLID, "solid" },
    { LineType::DASHED, "dashed" },
    { LineType::DOTTED, "dotted" }
};

AsciiStringView TConv::toXml(LineType v)
{
    return findXmlTagByType(LINE_TYPES, v);
}

LineType TConv::fromXml(const AsciiStringView& tag, LineType def)
{
    // Pre-4.0 files
    bool ok = false;
    if (int v = tag.toInt(&ok); ok) {
        draw::PenStyle penStyle = static_cast<draw::PenStyle>(v);
        switch (penStyle) {
        case draw::PenStyle::NoPen:
            return def;
        case draw::PenStyle::SolidLine:
            return LineType::SOLID;
        case draw::PenStyle::DashLine:
        case draw::PenStyle::DashDotLine:
        case draw::PenStyle::CustomDashLine:
            return LineType::DASHED;
        case draw::PenStyle::DotLine:
        case draw::PenStyle::DashDotDotLine:
            return LineType::DOTTED;
        }
    }

    return findTypeByXmlTag(LINE_TYPES, tag, def);
}

static const std::vector<Item<KeyMode> > KEY_MODES = {
    { KeyMode::UNKNOWN,     "unknown" },
    { KeyMode::NONE,        "none" },
    { KeyMode::MAJOR,       "major" },
    { KeyMode::MINOR,       "minor" },
    { KeyMode::DORIAN,      "dorian" },
    { KeyMode::PHRYGIAN,    "phrygian" },
    { KeyMode::LYDIAN,      "lydian" },
    { KeyMode::MIXOLYDIAN,  "mixolydian" },
    { KeyMode::AEOLIAN,     "aeolian" },
    { KeyMode::IONIAN,      "ionian" },
    { KeyMode::LOCRIAN,     "locrian" },
};

String TConv::translatedUserName(KeyMode v)
{
    return findUserNameByType<KeyMode>(KEY_MODES, v).translated();
}

AsciiStringView TConv::toXml(KeyMode v)
{
    return findXmlTagByType<KeyMode>(KEY_MODES, v);
}

KeyMode TConv::fromXml(const AsciiStringView& tag, KeyMode def)
{
    return findTypeByXmlTag<KeyMode>(KEY_MODES, tag, def);
}

static const std::vector<Item<TextStyleType> > TEXTSTYLE_TYPES = {
    { TextStyleType::DEFAULT,           "default",              TranslatableString("engraving", "Default") },
    { TextStyleType::TITLE,             "title",                TranslatableString("engraving", "Title") },
    { TextStyleType::SUBTITLE,          "subtitle",             TranslatableString("engraving", "Subtitle") },
    { TextStyleType::COMPOSER,          "composer",             TranslatableString("engraving", "Composer") },
    { TextStyleType::POET,              "poet",                 TranslatableString("engraving", "Lyricist") },
    { TextStyleType::TRANSLATOR,        "translator",           TranslatableString("engraving", "Translator") },
    { TextStyleType::FRAME,             "frame",                TranslatableString("engraving", "Frame") },
    { TextStyleType::INSTRUMENT_EXCERPT, "instrument_excerpt",  TranslatableString("engraving", "Instrument name (Part)") },
    { TextStyleType::INSTRUMENT_LONG,   "instrument_long",      TranslatableString("engraving", "Instrument name (Long)") },
    { TextStyleType::INSTRUMENT_SHORT,  "instrument_short",     TranslatableString("engraving", "Instrument name (Short)") },
    { TextStyleType::INSTRUMENT_CHANGE, "instrument_change",    TranslatableString("engraving", "Instrument change") },
    { TextStyleType::HEADER,            "header",               TranslatableString("engraving", "Header") },
    { TextStyleType::FOOTER,            "footer",               TranslatableString("engraving", "Footer") },

    { TextStyleType::MEASURE_NUMBER,    "measure_number",       TranslatableString("engraving", "Measure number") },
    { TextStyleType::MMREST_RANGE,      "mmrest_range",         TranslatableString("engraving", "Multimeasure rest range") },

    { TextStyleType::TEMPO,             "tempo",                TranslatableString("engraving", "Tempo") },
    { TextStyleType::METRONOME,         "metronome",            TranslatableString("engraving", "Metronome") },
    { TextStyleType::REPEAT_LEFT,       "repeat_left",          TranslatableString("engraving", "Repeat text left") },
    { TextStyleType::REPEAT_RIGHT,      "repeat_right",         TranslatableString("engraving", "Repeat text right") },
    { TextStyleType::REHEARSAL_MARK,    "rehearsal_mark",       TranslatableString("engraving", "Rehearsal mark") },
    { TextStyleType::SYSTEM,            "system",               TranslatableString("engraving", "System") },

    { TextStyleType::STAFF,             "staff",                TranslatableString("engraving", "Staff") },
    { TextStyleType::EXPRESSION,        "expression",           TranslatableString("engraving", "Expression") },
    { TextStyleType::DYNAMICS,          "dynamics",             TranslatableString("engraving", "Dynamics") },
    { TextStyleType::HAIRPIN,           "hairpin",              TranslatableString("engraving", "Hairpin") },
    { TextStyleType::LYRICS_ODD,        "lyrics_odd",           TranslatableString("engraving", "Lyrics odd lines") },
    { TextStyleType::LYRICS_EVEN,       "lyrics_even",          TranslatableString("engraving", "Lyrics even lines") },
    { TextStyleType::HARMONY_A,         "harmony_a",            TranslatableString("engraving", "Chord symbol") },
    { TextStyleType::HARMONY_B,         "harmony_b",            TranslatableString("engraving", "Chord symbol (alternate)") },
    { TextStyleType::HARMONY_ROMAN,     "harmony_roman",        TranslatableString("engraving", "Roman numeral analysis") },
    { TextStyleType::HARMONY_NASHVILLE, "harmony_nashville",    TranslatableString("engraving", "Nashville number") },

    { TextStyleType::TUPLET,            "tuplet",               TranslatableString("engraving", "Tuplet") },
    { TextStyleType::STICKING,          "sticking",             TranslatableString("engraving", "Sticking") },
    { TextStyleType::FINGERING,         "fingering",            TranslatableString("engraving", "Fingering") },
    { TextStyleType::LH_GUITAR_FINGERING, "guitar_fingering_lh", TranslatableString("engraving", "LH guitar fingering") },
    { TextStyleType::RH_GUITAR_FINGERING, "guitar_fingering_rh", TranslatableString("engraving", "RH guitar fingering") },
    { TextStyleType::STRING_NUMBER,     "string_number",        TranslatableString("engraving", "String number") },

    { TextStyleType::TEXTLINE,          "textline",             TranslatableString("engraving", "Text line") },
    { TextStyleType::VOLTA,             "volta",                TranslatableString("engraving", "Volta") },
    { TextStyleType::OTTAVA,            "ottava",               TranslatableString("engraving", "Ottava") },
    { TextStyleType::GLISSANDO,         "glissando",            TranslatableString("engraving", "Glissando") },
    { TextStyleType::PEDAL,             "pedal",                TranslatableString("engraving", "Pedal") },
    { TextStyleType::BEND,              "bend",                 TranslatableString("engraving", "Bend") },
    { TextStyleType::LET_RING,          "let_ring",             TranslatableString("engraving", "Let Ring") },
    { TextStyleType::PALM_MUTE,         "palm_mute",            TranslatableString("engraving", "Palm Mute") },

    { TextStyleType::USER1,             "user_1",               TranslatableString("engraving", "User-1") },
    { TextStyleType::USER2,             "user_2",               TranslatableString("engraving", "User-2") },
    { TextStyleType::USER3,             "user_3",               TranslatableString("engraving", "User-3") },
    { TextStyleType::USER4,             "user_4",               TranslatableString("engraving", "User-4") },
    { TextStyleType::USER5,             "user_5",               TranslatableString("engraving", "User-5") },
    { TextStyleType::USER6,             "user_6",               TranslatableString("engraving", "User-6") },
    { TextStyleType::USER7,             "user_7",               TranslatableString("engraving", "User-7") },
    { TextStyleType::USER8,             "user_8",               TranslatableString("engraving", "User-8") },
    { TextStyleType::USER9,             "user_9",               TranslatableString("engraving", "User-9") },
    { TextStyleType::USER10,            "user_10",              TranslatableString("engraving", "User-10") },
    { TextStyleType::USER11,            "user_11",              TranslatableString("engraving", "User-11") },
    { TextStyleType::USER12,            "user_12",              TranslatableString("engraving", "User-12") },
};

const TranslatableString& TConv::userName(TextStyleType v)
{
    return findUserNameByType<TextStyleType>(TEXTSTYLE_TYPES, v);
}

String TConv::translatedUserName(TextStyleType v)
{
    return findUserNameByType<TextStyleType>(TEXTSTYLE_TYPES, v).translated();
}

AsciiStringView TConv::toXml(TextStyleType v)
{
    return findXmlTagByType<TextStyleType>(TEXTSTYLE_TYPES, v);
}

TextStyleType TConv::fromXml(const AsciiStringView& tag, TextStyleType def)
{
    auto it = std::find_if(TEXTSTYLE_TYPES.cbegin(), TEXTSTYLE_TYPES.cend(), [tag](const Item<TextStyleType>& i) {
        return i.xml == tag;
    });

    if (it != TEXTSTYLE_TYPES.cend()) {
        return it->type;
    }

    // compatibility

    static const std::map<AsciiStringView, TextStyleType> OLD_TST_TAGS = {
        { "Default", TextStyleType::DEFAULT },
        { "Title", TextStyleType::TITLE },
        { "Subtitle", TextStyleType::SUBTITLE },
        { "Composer", TextStyleType::COMPOSER },
        { "Lyricist", TextStyleType::POET },
        { "Translator", TextStyleType::TRANSLATOR },
        { "Frame", TextStyleType::FRAME },
        { "Instrument Name (Part)", TextStyleType::INSTRUMENT_EXCERPT },
        { "Instrument Name (Long)", TextStyleType::INSTRUMENT_LONG },
        { "Instrument Name (Short)", TextStyleType::INSTRUMENT_SHORT },
        { "Instrument Change", TextStyleType::INSTRUMENT_CHANGE },
        { "Header", TextStyleType::HEADER },
        { "Footer", TextStyleType::FOOTER },

        { "Measure Number", TextStyleType::MEASURE_NUMBER },
        { "Multimeasure Rest Range", TextStyleType::MMREST_RANGE },

        { "Tempo", TextStyleType::TEMPO },
        { "Metronome", TextStyleType::METRONOME },
        { "Repeat Text Left", TextStyleType::REPEAT_LEFT },
        { "Repeat Text Right", TextStyleType::REPEAT_RIGHT },
        { "Rehearsal Mark", TextStyleType::REHEARSAL_MARK },
        { "System", TextStyleType::SYSTEM },

        { "Staff", TextStyleType::STAFF },
        { "Expression", TextStyleType::EXPRESSION },
        { "Dynamics", TextStyleType::DYNAMICS },
        { "Hairpin", TextStyleType::HAIRPIN },
        { "Lyrics Odd Lines", TextStyleType::LYRICS_ODD },
        { "Lyrics Even Lines", TextStyleType::LYRICS_EVEN },
        { "Chord Symbol", TextStyleType::HARMONY_A },
        { "Chord Symbol (Alternate)", TextStyleType::HARMONY_B },
        { "Roman Numeral Analysis", TextStyleType::HARMONY_ROMAN },
        { "Nashville Number", TextStyleType::HARMONY_NASHVILLE },

        { "Tuplet", TextStyleType::TUPLET },
        { "Sticking", TextStyleType::STICKING },
        { "Fingering", TextStyleType::FINGERING },
        { "LH Guitar Fingering", TextStyleType::LH_GUITAR_FINGERING },
        { "RH Guitar Fingering", TextStyleType::RH_GUITAR_FINGERING },
        { "String Number", TextStyleType::STRING_NUMBER },

        { "Text Line", TextStyleType::TEXTLINE },
        { "Volta", TextStyleType::VOLTA },
        { "Ottava", TextStyleType::OTTAVA },
        { "Glissando", TextStyleType::GLISSANDO },
        { "Pedal", TextStyleType::PEDAL },
        { "Bend", TextStyleType::BEND },
        { "Let Ring", TextStyleType::LET_RING },
        { "Palm Mute", TextStyleType::PALM_MUTE },

        { "User-1", TextStyleType::USER1 },
        { "User-2", TextStyleType::USER2 },
        { "User-3", TextStyleType::USER3 },
        { "User-4", TextStyleType::USER4 },
        { "User-5", TextStyleType::USER5 },
        { "User-6", TextStyleType::USER6 },
        { "User-7", TextStyleType::USER7 },
        { "User-8", TextStyleType::USER8 },
        { "User-9", TextStyleType::USER9 },
        { "User-10", TextStyleType::USER10 },
        { "User-11", TextStyleType::USER11 },
        { "User-12", TextStyleType::USER12 },

        { "Technique", TextStyleType::EXPRESSION },

        { "12", TextStyleType::DYNAMICS },
        { "26", TextStyleType::STAFF }
    };

    auto old = OLD_TST_TAGS.find(tag);
    if (old != OLD_TST_TAGS.cend()) {
        return old->second;
    }

    LOGE() << "not found type for tag: " << tag;
    UNREACHABLE;
    return def;
}

static const std::vector<Item<ChangeMethod> > CHANGE_METHODS = {
    { ChangeMethod::NORMAL,           "normal" },
    { ChangeMethod::EASE_IN,          "ease-in" },
    { ChangeMethod::EASE_OUT,         "ease-out" },
    { ChangeMethod::EASE_IN_OUT,      "ease-in-out" },
    { ChangeMethod::EXPONENTIAL,      "exponential" },
};

static float easingFactor(const float x, const ChangeMethod method)
{
    switch (method) {
    case ChangeMethod::NORMAL:
        return x;
    case ChangeMethod::EASE_IN:
        return 1 - std::sqrt(1 - std::pow(x, 2));
    case ChangeMethod::EASE_OUT:
        return std::sqrt(1 - std::pow(x - 1, 2));
    case ChangeMethod::EASE_IN_OUT:
        if (x < 0.5) {
            return (1.f - std::sqrt(1 - std::pow(2 * x, 2))) / 2;
        } else {
            return (std::sqrt(1.f - std::pow(-2 * x + 2, 2)) + 1) / 2;
        }
    case ChangeMethod::EXPONENTIAL:
        if (RealIsEqual(x, 1.f)) {
            return x;
        } else {
            return 1.f - std::pow(2, -10 * x);
        }
    }

    return 1.f;
}

template<typename T>
static std::map<int /*tickPosition*/, T> buildEasedValueCurve(const int ticksDuration, const int stepsCount, const T amplitude,
                                                              const ChangeMethod method)
{
    if (stepsCount <= 0) {
        static std::map<int, T> empty;
        return empty;
    }

    std::map<int, T> result;

    float durationStep = static_cast<float>(ticksDuration) / static_cast<float>(stepsCount);

    for (int i = 0; i <= stepsCount; ++i) {
        result.emplace(i * durationStep, easingFactor(i / static_cast<float>(stepsCount), method) * amplitude);
    }

    return result;
}

std::map<int, int> TConv::easingValueCurve(const int ticksDuration, const int stepsCount, const int amplitude,
                                           const ChangeMethod method)
{
    return buildEasedValueCurve(ticksDuration, stepsCount, amplitude, method);
}

std::map<int, double> TConv::easingValueCurve(const int ticksDuration, const int stepsCount, const double amplitude,
                                              const ChangeMethod method)
{
    return buildEasedValueCurve(ticksDuration, stepsCount, amplitude, method);
}

AsciiStringView TConv::toXml(ChangeMethod v)
{
    return findXmlTagByType<ChangeMethod>(CHANGE_METHODS, v);
}

ChangeMethod TConv::fromXml(const AsciiStringView& tag, ChangeMethod def)
{
    return findTypeByXmlTag<ChangeMethod>(CHANGE_METHODS, tag, def);
}

String TConv::toXml(const PitchValue& v)
{
    return String(u"point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"").arg(v.time).arg(v.pitch).arg(v.vibrato);
}

struct AccidentalUnicodeItem {
    AccidentalVal accidental;
    const char* name = nullptr;
    const char* fullName = nullptr;
};

static const std::vector<AccidentalUnicodeItem> ACCIDENTALS_NAMES = {
    { AccidentalVal::SHARP3,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "triple ♯"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "triple sharp") },
    { AccidentalVal::SHARP2,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "double ♯"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "double sharp") },
    { AccidentalVal::SHARP,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "♯"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "sharp") },
    { AccidentalVal::NATURAL,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "♮"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "natural") },
    { AccidentalVal::FLAT,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "♭"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "flat") },
    { AccidentalVal::FLAT2,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "double ♭"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "double flat") },
    { AccidentalVal::FLAT3,
      //: Visible text in the UI. Please preserve the accidental symbol in the translation
      QT_TRANSLATE_NOOP("engraving", "triple ♭"),
      //: Accessible text for screen readers. Please avoid using accidental symbols in the translation
      QT_TRANSLATE_NOOP("engraving", "triple flat") }
};

const char* TConv::userName(AccidentalVal accidental, bool full)
{
    auto it = std::find_if(ACCIDENTALS_NAMES.cbegin(), ACCIDENTALS_NAMES.cend(), [accidental](const AccidentalUnicodeItem& i) {
        return i.accidental == accidental;
    });

    if (it != ACCIDENTALS_NAMES.cend()) {
        return full ? it->fullName : it->name;
    }

    return "";
}

String TConv::toXml(AccidentalRole v)
{
    return String::number(static_cast<int>(v));
}

AccidentalRole TConv::fromXml(const AsciiStringView& tag, AccidentalRole def)
{
    bool ok = false;
    int r = tag.toInt(&ok);
    return ok ? static_cast<AccidentalRole>(r) : def;
}

String TConv::toXml(BeatsPerSecond v)
{
    return String::number(v.val);
}

BeatsPerSecond TConv::fromXml(const AsciiStringView& tag, BeatsPerSecond def)
{
    bool ok = false;
    double v = tag.toDouble(&ok);
    return ok ? BeatsPerSecond(v) : def;
}

static const std::vector<Item<DurationType> > DURATION_TYPES = {
    { DurationType::V_QUARTER,  "quarter",  TranslatableString("engraving", "Quarter") },
    { DurationType::V_EIGHTH,   "eighth",   TranslatableString("engraving", "Eighth") },
    { DurationType::V_1024TH,   "1024th",   TranslatableString("engraving", "1024th") },
    { DurationType::V_512TH,    "512th",    TranslatableString("engraving", "512th") },
    { DurationType::V_256TH,    "256th",    TranslatableString("engraving", "256th") },
    { DurationType::V_128TH,    "128th",    TranslatableString("engraving", "128th") },
    { DurationType::V_64TH,     "64th",     TranslatableString("engraving", "64th") },
    { DurationType::V_32ND,     "32nd",     TranslatableString("engraving", "32nd") },
    { DurationType::V_16TH,     "16th",     TranslatableString("engraving", "16th") },
    { DurationType::V_HALF,     "half",     TranslatableString("engraving", "Half") },
    { DurationType::V_WHOLE,    "whole",    TranslatableString("engraving", "Whole") },
    { DurationType::V_MEASURE,  "measure",  TranslatableString("engraving", "Measure") },
    { DurationType::V_BREVE,    "breve",    TranslatableString("engraving", "Breve") },
    { DurationType::V_LONG,     "long",     TranslatableString("engraving", "Longa") },
    { DurationType::V_ZERO,     "",         TranslatableString("engraving", "Zero") },
    { DurationType::V_INVALID,  "",         TranslatableString("engraving", "Invalid") },
};

String TConv::translatedUserName(DurationType v)
{
    return findUserNameByType<DurationType>(DURATION_TYPES, v).translated();
}

AsciiStringView TConv::toXml(DurationType v)
{
    return findXmlTagByType<DurationType>(DURATION_TYPES, v);
}

DurationType TConv::fromXml(const AsciiStringView& tag, DurationType def)
{
    return findTypeByXmlTag<DurationType>(DURATION_TYPES, tag, def);
}

static const std::vector<Item<PlayingTechniqueType> > PLAY_TECH_TYPES = {
    { PlayingTechniqueType::Undefined,           "undefined" },
    { PlayingTechniqueType::Natural,             "natural" },
    { PlayingTechniqueType::Pizzicato,           "pizzicato" },
    { PlayingTechniqueType::Open,                "open" },
    { PlayingTechniqueType::Mute,                "mute" },
    { PlayingTechniqueType::Tremolo,             "tremolo" },
    { PlayingTechniqueType::Detache,             "detache" },
    { PlayingTechniqueType::Martele,             "martele" },
    { PlayingTechniqueType::ColLegno,            "col_legno" },
    { PlayingTechniqueType::SulPonticello,       "sul_ponticello" },
    { PlayingTechniqueType::SulTasto,            "sul_tasto" },
    { PlayingTechniqueType::Vibrato,             "vibrato" },
    { PlayingTechniqueType::Legato,              "legato" },
    { PlayingTechniqueType::Distortion,          "distortion" },
    { PlayingTechniqueType::Overdrive,           "overdrive" },
    { PlayingTechniqueType::Harmonics,           "harmonics" },
    { PlayingTechniqueType::JazzTone,            "jazz_tone" },
};

AsciiStringView TConv::toXml(PlayingTechniqueType v)
{
    return findXmlTagByType<PlayingTechniqueType>(PLAY_TECH_TYPES, v);
}

PlayingTechniqueType TConv::fromXml(const AsciiStringView& tag, PlayingTechniqueType def)
{
    return findTypeByXmlTag<PlayingTechniqueType>(PLAY_TECH_TYPES, tag, def);
}

static const std::vector<Item<GradualTempoChangeType> > TEMPO_CHANGE_TYPES = {
    { GradualTempoChangeType::Undefined, "undefined" },
    { GradualTempoChangeType::Accelerando, "accelerando" },
    { GradualTempoChangeType::Allargando, "allargando" },
    { GradualTempoChangeType::Calando, "calando" },
    { GradualTempoChangeType::Lentando, "lentando" },
    { GradualTempoChangeType::Morendo, "morendo" },
    { GradualTempoChangeType::Precipitando, "precipitando" },
    { GradualTempoChangeType::Rallentando, "rallentando" },
    { GradualTempoChangeType::Ritardando, "ritardando" },
    { GradualTempoChangeType::Smorzando, "smorzando" },
    { GradualTempoChangeType::Sostenuto, "sostenuto" },
    { GradualTempoChangeType::Stringendo, "stringendo" }
};

AsciiStringView TConv::toXml(GradualTempoChangeType v)
{
    return findXmlTagByType<GradualTempoChangeType>(TEMPO_CHANGE_TYPES, v);
}

GradualTempoChangeType TConv::fromXml(const AsciiStringView& tag, GradualTempoChangeType def)
{
    return findTypeByXmlTag<GradualTempoChangeType>(TEMPO_CHANGE_TYPES, tag, def);
}

static const std::vector<Item<OrnamentStyle> > ORNAMENTSTYLE_TYPES = {
    { OrnamentStyle::BAROQUE, "baroque" },
    { OrnamentStyle::DEFAULT, "default" }
};

AsciiStringView TConv::toXml(OrnamentStyle v)
{
    return findXmlTagByType<OrnamentStyle>(ORNAMENTSTYLE_TYPES, v);
}

OrnamentStyle TConv::fromXml(const AsciiStringView& tag, OrnamentStyle def)
{
    return findTypeByXmlTag<OrnamentStyle>(ORNAMENTSTYLE_TYPES, tag, def);
}

static const std::vector<Item<PlacementV> > PLACEMENTV_TYPES = {
    { PlacementV::ABOVE, "above" },
    { PlacementV::BELOW, "below" }
};

AsciiStringView TConv::toXml(PlacementV v)
{
    return findXmlTagByType<PlacementV>(PLACEMENTV_TYPES, v);
}

PlacementV TConv::fromXml(const AsciiStringView& tag, PlacementV def)
{
    return findTypeByXmlTag<PlacementV>(PLACEMENTV_TYPES, tag, def);
}

static const std::vector<Item<PlacementH> > PLACEMENTH_TYPES = {
    { PlacementH::LEFT, "left" },
    { PlacementH::RIGHT, "center" },
    { PlacementH::CENTER, "right" }
};

AsciiStringView TConv::toXml(PlacementH v)
{
    return findXmlTagByType<PlacementH>(PLACEMENTH_TYPES, v);
}

PlacementH TConv::fromXml(const AsciiStringView& tag, PlacementH def)
{
    return findTypeByXmlTag<PlacementH>(PLACEMENTH_TYPES, tag, def);
}

static const std::vector<Item<TextPlace> > TEXTPLACE_TYPES = {
    { TextPlace::AUTO, "auto" },
    { TextPlace::ABOVE, "above" },
    { TextPlace::BELOW, "below" },
    { TextPlace::LEFT, "left" }
};

AsciiStringView TConv::toXml(TextPlace v)
{
    return findXmlTagByType<TextPlace>(TEXTPLACE_TYPES, v);
}

TextPlace TConv::fromXml(const AsciiStringView& tag, TextPlace def)
{
    auto it = std::find_if(TEXTPLACE_TYPES.cbegin(), TEXTPLACE_TYPES.cend(), [tag](const Item<TextPlace>& i) {
        return i.xml == tag;
    });

    if (it != TEXTPLACE_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    static const std::vector<Item<TextPlace> > OLD_TEXTPLACE_TYPES = {
        { TextPlace::AUTO, "0" },
        { TextPlace::ABOVE, "1" },
        { TextPlace::BELOW, "2" },
        { TextPlace::LEFT, "3" }
    };

    auto oldit = std::find_if(OLD_TEXTPLACE_TYPES.cbegin(), OLD_TEXTPLACE_TYPES.cend(), [tag](const Item<TextPlace>& i) {
        return i.xml == tag;
    });

    IF_ASSERT_FAILED(oldit != OLD_TEXTPLACE_TYPES.cend()) {
        return def;
    }
    return oldit->type;
}

static const std::array<Item<DirectionV>, 3 > DIRECTIONV_TYPES = { {
    { DirectionV::AUTO, "auto",     TranslatableString("engraving", "Auto") },
    { DirectionV::UP, "up",         TranslatableString("engraving", "Up") },
    { DirectionV::DOWN, "down",     TranslatableString("engraving", "Down") },
} };

String TConv::translatedUserName(DirectionV v)
{
    return findUserNameByType<DirectionV>(DIRECTIONV_TYPES, v).translated();
}

AsciiStringView TConv::toXml(DirectionV v)
{
    return findXmlTagByType<DirectionV>(DIRECTIONV_TYPES, v);
}

DirectionV TConv::fromXml(const AsciiStringView& tag, DirectionV def)
{
    auto it = std::find_if(DIRECTIONV_TYPES.cbegin(), DIRECTIONV_TYPES.cend(), [tag](const Item<DirectionV>& i) {
        return i.xml == tag;
    });

    if (it != DIRECTIONV_TYPES.cend()) {
        return it->type;
    }

    // compatibility
//    bool ok = false;
//    int v = tag.toInt(&ok);
//    if (ok) {
//        return static_cast<DirectionV>(v);
//    }

    return def;
}

static const std::vector<Item<DirectionH> > DIRECTIONH_TYPES = {
    { DirectionH::AUTO,  "auto",  TranslatableString("engraving", "Auto") },
    { DirectionH::RIGHT, "right", TranslatableString("engraving", "Right") },
    { DirectionH::LEFT,  "left",  TranslatableString("engraving", "Left") },
};

String TConv::translatedUserName(DirectionH v)
{
    return findUserNameByType<DirectionH>(DIRECTIONH_TYPES, v).translated();
}

AsciiStringView TConv::toXml(DirectionH v)
{
    return findXmlTagByType<DirectionH>(DIRECTIONH_TYPES, v);
}

DirectionH TConv::fromXml(const AsciiStringView& tag, DirectionH def)
{
    auto it = std::find_if(DIRECTIONH_TYPES.cbegin(), DIRECTIONH_TYPES.cend(), [tag](const Item<DirectionH>& i) {
        return i.xml == tag;
    });

    if (it != DIRECTIONH_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<DirectionH>(v);
    }

    return def;
}

static const std::vector<Item<LayoutBreakType> > LAYOUTBREAK_TYPES = {
    { LayoutBreakType::LINE, "line" },
    { LayoutBreakType::PAGE, "page" },
    { LayoutBreakType::SECTION, "section" },
    { LayoutBreakType::NOBREAK, "nobreak" }
};

AsciiStringView TConv::toXml(LayoutBreakType v)
{
    return findXmlTagByType<LayoutBreakType>(LAYOUTBREAK_TYPES, v);
}

LayoutBreakType TConv::fromXml(const AsciiStringView& tag, LayoutBreakType def)
{
    return findTypeByXmlTag<LayoutBreakType>(LAYOUTBREAK_TYPES, tag, def);
}

static const std::vector<Item<VeloType> > VELO_TYPES = {
    { VeloType::OFFSET_VAL, "offset" },
    { VeloType::USER_VAL, "user" }
};

AsciiStringView TConv::toXml(VeloType v)
{
    return findXmlTagByType<VeloType>(VELO_TYPES, v);
}

VeloType TConv::fromXml(const AsciiStringView& tag, VeloType def)
{
    return findTypeByXmlTag<VeloType>(VELO_TYPES, tag, def);
}

static const std::vector<Item<BeamMode> > BEAMMODE_TYPES = {
    { BeamMode::AUTO, "auto" },
    { BeamMode::BEGIN, "begin" },
    { BeamMode::MID, "mid" },
    { BeamMode::END, "end" },
    { BeamMode::NONE, "no" },
    { BeamMode::BEGIN32, "begin32" },
    { BeamMode::BEGIN64, "begin64" },
    { BeamMode::INVALID, "invalid" }
};

AsciiStringView TConv::toXml(BeamMode v)
{
    return findXmlTagByType<BeamMode>(BEAMMODE_TYPES, v);
}

BeamMode TConv::fromXml(const AsciiStringView& tag, BeamMode def)
{
    auto it = std::find_if(BEAMMODE_TYPES.cbegin(), BEAMMODE_TYPES.cend(), [tag](const Item<BeamMode>& i) {
        return i.xml == tag;
    });

    if (it != BEAMMODE_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<BeamMode>(v);
    }

    return def;
}

static const std::vector<Item<GlissandoStyle> > GLISSANDOSTYLE_TYPES = {
    { GlissandoStyle::BLACK_KEYS, "blackkeys" },
    { GlissandoStyle::WHITE_KEYS, "whitekeys" },
    { GlissandoStyle::DIATONIC, "diatonic" },
    { GlissandoStyle::PORTAMENTO, "portamento" },
    { GlissandoStyle::CHROMATIC, "chromatic" }
};

AsciiStringView TConv::toXml(GlissandoStyle v)
{
    return findXmlTagByType<GlissandoStyle>(GLISSANDOSTYLE_TYPES, v);
}

GlissandoStyle TConv::fromXml(const AsciiStringView& tag, GlissandoStyle def)
{
    auto it = std::find_if(GLISSANDOSTYLE_TYPES.cbegin(), GLISSANDOSTYLE_TYPES.cend(), [tag](const Item<GlissandoStyle>& i) {
        return i.xml == tag;
    });

    if (it != GLISSANDOSTYLE_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    if (tag == "Chromatic") {
        return GlissandoStyle::CHROMATIC;
    }

    return def;
}

static const std::vector<Item<BarLineType> > BARLINE_TYPES = {
    { BarLineType::NORMAL, "normal" },
    { BarLineType::DOUBLE, "double" },
    { BarLineType::START_REPEAT, "start-repeat" },
    { BarLineType::END_REPEAT, "end-repeat" },
    { BarLineType::BROKEN, "dashed" },
    { BarLineType::END, "end" },
    { BarLineType::END_START_REPEAT, "end-start-repeat" },
    { BarLineType::DOTTED, "dotted" },
    { BarLineType::REVERSE_END, "reverse-end" },
    { BarLineType::HEAVY, "heavy" },
    { BarLineType::DOUBLE_HEAVY, "double-heavy" }
};

AsciiStringView TConv::toXml(BarLineType v)
{
    return findXmlTagByType<BarLineType>(BARLINE_TYPES, v);
}

BarLineType TConv::fromXml(const AsciiStringView& tag, BarLineType def)
{
    auto it = std::find_if(BARLINE_TYPES.cbegin(), BARLINE_TYPES.cend(), [tag](const Item<BarLineType>& i) {
        return i.xml == tag;
    });

    if (it != BARLINE_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<BarLineType>(v);
    }

    return def;
}

static const std::array<Item<TremoloType>, 10> TREMOLO_TYPES = { {
    { TremoloType::INVALID_TREMOLO, "" },
    { TremoloType::R8,              "r8",       TranslatableString("engraving/tremolotype", "Eighth through stem") },
    { TremoloType::R16,             "r16",      TranslatableString("engraving/tremolotype", "16th through stem") },
    { TremoloType::R32,             "r32",      TranslatableString("engraving/tremolotype", "32nd through stem") },
    { TremoloType::R64,             "r64",      TranslatableString("engraving/tremolotype", "64th through stem") },
    { TremoloType::BUZZ_ROLL,       "buzzroll", TranslatableString("engraving/tremolotype", "Buzz roll") },
    { TremoloType::C8,              "c8",       TranslatableString("engraving/tremolotype", "Eighth between notes") },
    { TremoloType::C16,             "c16",      TranslatableString("engraving/tremolotype", "16th between notes") },
    { TremoloType::C32,             "c32",      TranslatableString("engraving/tremolotype", "32nd between notes") },
    { TremoloType::C64,             "c64",      TranslatableString("engraving/tremolotype", "64th between notes") }
} };

const TranslatableString& TConv::userName(TremoloType v)
{
    return findUserNameByType<TremoloType>(TREMOLO_TYPES, v);
}

AsciiStringView TConv::toXml(TremoloType v)
{
    return findXmlTagByType<TremoloType>(TREMOLO_TYPES, v);
}

TremoloType TConv::fromXml(const AsciiStringView& tag, TremoloType def)
{
    return findTypeByXmlTag<TremoloType>(TREMOLO_TYPES, tag, def);
}

static const std::vector<Item<BracketType> > BRACKET_TYPES = {
    { BracketType::NORMAL, "Normal" },
    { BracketType::BRACE, "Brace" },
    { BracketType::SQUARE, "Square" },
    { BracketType::LINE, "Line" },
    { BracketType::NO_BRACKET, "NoBracket" }
};

AsciiStringView TConv::toXml(BracketType v)
{
    return findXmlTagByType<BracketType>(BRACKET_TYPES, v);
}

BracketType TConv::fromXml(const AsciiStringView& tag, BracketType def)
{
    auto it = std::find_if(BRACKET_TYPES.cbegin(), BRACKET_TYPES.cend(), [tag](const Item<BracketType>& i) {
        return i.xml == tag;
    });

    if (it != BRACKET_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    if (tag == "Akkolade") {
        return BracketType::BRACE;
    }

    return def;
}

//! TODO Add xml names
static const std::array<Item<ArpeggioType>, 6> ARPEGGIO_TYPES = { {
    { ArpeggioType::NORMAL,         "0",     TranslatableString("engraving", "Arpeggio") },
    { ArpeggioType::UP,             "1",     TranslatableString("engraving", "Up arpeggio") },
    { ArpeggioType::DOWN,           "2",     TranslatableString("engraving", "Down arpeggio") },
    { ArpeggioType::BRACKET,        "3",     TranslatableString("engraving", "Bracket arpeggio") },
    { ArpeggioType::UP_STRAIGHT,    "4",     TranslatableString("engraving", "Up arpeggio straight") },
    { ArpeggioType::DOWN_STRAIGHT,  "5",     TranslatableString("engraving", "Down arpeggio straight") }
} };

const TranslatableString& TConv::userName(ArpeggioType v)
{
    return findUserNameByType<ArpeggioType>(ARPEGGIO_TYPES, v);
}

AsciiStringView TConv::toXml(ArpeggioType v)
{
    return findXmlTagByType<ArpeggioType>(ARPEGGIO_TYPES, v);
}

ArpeggioType TConv::fromXml(const AsciiStringView& tag, ArpeggioType def)
{
    return findTypeByXmlTag<ArpeggioType>(ARPEGGIO_TYPES, tag, def);
}

struct EmbelItem
{
    TranslatableString name;
    AsciiStringView notes;
};

// TODO: Can't use .arg, because Palettes use these strings and doesn't support TranslatableString
static const std::vector<EmbelItem> EMBELLISHMENT_TYPES = {
    // Single Grace notes
    { TranslatableString("engraving/bagpipeembellishment", "Single grace low G"), "LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace low A"), "LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace B"), "B" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace C"), "C" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace D"), "D" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace E"), "E" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace F"), "F" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace high G"), "HG" },
    { TranslatableString("engraving/bagpipeembellishment", "Single grace high A"), "HA" },

    // Double Grace notes
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D B" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E B" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E D" },

    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F B" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F D" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F E" },

    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG B" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG D" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG E" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG F" },

    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA B" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA D" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA E" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA F" },
    { TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA HG" },

    // Half Doublings
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on low G"), "LG D" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on low A"), "LA D" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on B"), "B D" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on C"), "C D" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on D"), "D E" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on E"), "E F" },
    { TranslatableString("engraving/bagpipeembellishment", "Half doubling on F"), "F HG" },
    // ? { TranslatableString("engraving/bagpipeembellishment", "Half doubling on high G"), "HG F" },
    // ? { TranslatableString("engraving/bagpipeembellishment", "Half doubling on high A"), "HA HG" },

    // Regular Doublings
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on high G"), "HG F" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on high A"), "HA HG" },

    // Half Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on low A"), "LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on B"), "B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on C"), "C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on D"), "D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on D"), "D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on E"), "E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on F"), "F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Half strike on high G"), "HG F" },

    // Regular Grip
    { TranslatableString("engraving/bagpipeembellishment", "Grip"), "D LG" },

    // D Throw
    { TranslatableString("engraving/bagpipeembellishment", "Half D throw"), "D C" },

    // Regular Doublings (continued)
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on low G"),  "HG LG D" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on low A"),  "HG LA D" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on B"),      "HG B D" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on C"),      "HG C D" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on D"),      "HG D E" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on E"),      "HG E F" },
    { TranslatableString("engraving/bagpipeembellishment", "Doubling on F"),      "HG F HG" },

    // Thumb Doublings
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on low G"), "HA LG D" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on low A"), "HA LA D" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on B"), "HA B D" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on C"), "HA C D" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on D"), "HA D E" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on E"), "HA E F" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on F"), "HA F HG" },
    // ? { TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on high G"), "HA HG F" },

    // G Grace note Strikes
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on low A"), "HG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on B"), "HG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on C"), "HG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on D"), "HG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on D"), "HG D C" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on E"), "HG E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note on F"), "HG F E" },

    // Regular Double Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on low A"), "LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on B"), "LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on C"), "LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on D"), "LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on D"), "C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on E"), "LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on F"), "E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on high G"), "F HG F" },
    { TranslatableString("engraving/bagpipeembellishment", "Double strike on high A"), "HG HA HG" },

    // Thumb Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on low A"), "HA LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on B"), "HA B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on C"), "HA C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on D"), "HA D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on D"), "HA D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on E"), "HA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on F"), "HA F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb strike on high G"), "HA HG F" },

    // Regular Grips (continued)
    { TranslatableString("engraving/bagpipeembellishment", "Grip"), "LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Grip"), "LG B LG" },

    // Taorluath and Birl
    { TranslatableString("engraving/bagpipeembellishment", "Birl"), "LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "D throw"), "LG D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Half heavy D throw"), "D LG C" },
    { TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "D LG E" },

    // Birl, Bubbly, D Throws (cont/bagpipeembellishmentinued) and Taorluaths (continued)
    { TranslatableString("engraving/bagpipeembellishment", "Birl"), "LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Bubbly"), "D LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Heavy D throw"), "LG D LG C" },
    { TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "LG D LG E" },
    { TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "LG B LG E" },

    // Half Double Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on low A"), "LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on B"), "B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on C"), "C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on D"), "D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on D"), "D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on E"), "E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on F"), "F E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on high G"), "HG F HG F" },
    { TranslatableString("engraving/bagpipeembellishment", "Half double strike on high A"), "HA HG HA HG" },

    // Half Grips
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on low A"), "LA LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on B"), "B LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on C"), "C LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on D"), "D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on D"), "D LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on E"), "E LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on F"), "F LG F LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on high G"), "HG LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half grip on high A"), "HA LG D LG" },

    // Half Peles
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on low A"), "LA E LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on B"), "B E B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on C"), "C E C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on D"), "D E D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on D"), "D E D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on E"), "E F E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on F"), "F HG F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Half pele on high G"), "HG HA HG F" },

    // G Grace note Grips
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on low A"), "HG LA LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on B"), "HG B LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on C"), "HG C LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on D"), "HG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on D"), "HG D LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on E"), "HG E LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note grip on F"), "HG F LG F LG" },

    // Thumb Grips
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on low A"), "HA LA LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on B"), "HA B LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on C"), "HA C LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on D"), "HA D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on D"), "HA D LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on E"), "HA E LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on F"), "HA F LG F LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grip on high G"), "HA HG LG F LG" },

    // Bubbly
    { TranslatableString("engraving/bagpipeembellishment", "Bubbly"), "LG D LG C LG" },

    //  Birls
    { TranslatableString("engraving/bagpipeembellishment", "Birl"), "HG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Birl"), "HA LA LG LA LG" },

    // Regular Peles
    { TranslatableString("engraving/bagpipeembellishment", "Pele on low A"), "HG LA E LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on B"), "HG B E B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on C"), "HG C E C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on D"), "HG D E D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on D"), "HG D E D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on E"), "HG E F E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Pele on F"), "HG F HG F E" },

    // Thumb Grace Note Peles
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on low A"), "HA LA E LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on B"), "HA B E B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on C"), "HA C E C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on D"), "HA D E D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on D"), "HA D E D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on E"), "HA E F E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on F"), "HA F HG F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on high G"), "HA HG HA HG F" },

    // G Grace note Double Strikes
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on low A"), "HG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on B"), "HG B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on C"), "HG C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on D"), "HG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on D"), "HG D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on E"), "HG E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on F"), "HG F E F E" },

    // Thumb Double Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on low A"), "HA LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on B"), "HA B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on C"), "HA C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on D"), "HA D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on D"), "HA D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on E"), "HA E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on F"), "HA F E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on high G"), "HA HG F HG F" },

    // Regular Triple Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on low A"), "LG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on B"), "LG B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on C"), "LG C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on D"), "LG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on D"), "C D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on E"), "LA E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on F"), "E F E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on high G"), "F HG F HG F" },
    { TranslatableString("engraving/bagpipeembellishment", "Triple strike on high A"), "HG HA HG HA HG" },

    // Half Triple Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on low A"), "LA LG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on B"), "B LG B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on C"), "C LG C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on D"), "D LG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on D"), "D C D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on E"), "E LA E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on F"), "F E F E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on high G"), "HG F HG F HG F" },
    { TranslatableString("engraving/bagpipeembellishment", "Half triple strike on high A"), "HA HG HA HG HA HG" },

    // G Grace note Triple Strikes
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on low A"), "HG LA LG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on B"), "HG B LG B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on C"), "HG C LG C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on D"), "HG D LG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on D"), "HG D C D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on E"), "HG E LA E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on F"), "HG F E F E F E" },

    // Thumb Triple Strikes
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on low A"),  "HA LA LG LA LG LA LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on B"),      "HA B LG B LG B LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on C"),      "HA C LG C LG C LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on D"),      "HA D LG D LG D LG" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on D"),      "HA D C D C D C" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on E"),      "HA E LA E LA E LA" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on F"),      "HA F E F E F E" },
    { TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on high G"), "HA HG F HG F HG F" },
};

const TranslatableString& TConv::userName(EmbellishmentType v)
{
    return EMBELLISHMENT_TYPES.at(static_cast<size_t>(v)).name;
}

String TConv::translatedUserName(EmbellishmentType v)
{
    return EMBELLISHMENT_TYPES.at(static_cast<size_t>(v)).name.translated();
}

String TConv::toXml(EmbellishmentType v)
{
    return String::number(static_cast<size_t>(v));
}

EmbellishmentType TConv::fromXml(const AsciiStringView& tag, EmbellishmentType def)
{
    bool ok = false;
    int v = tag.toInt(&ok);
    if (ok) {
        return static_cast<EmbellishmentType>(v);
    }
    return def;
}

StringList TConv::embellishmentNotes(EmbellishmentType v)
{
    return String::fromAscii(EMBELLISHMENT_TYPES.at(v).notes.ascii()).split(u' ');
}

size_t TConv::embellishmentsCount()
{
    return EMBELLISHMENT_TYPES.size();
}

//! TODO Add xml names
static const std::array<Item<std::pair<ChordLineType, bool /*straight*/> >, 10> CHORDLINE_TYPES = { {
    { { ChordLineType::NOTYPE, false },    "0" },
    { { ChordLineType::FALL, false },      "1",     TranslatableString("engraving", "Fall") },
    { { ChordLineType::DOIT, false },      "2",     TranslatableString("engraving", "Doit") },
    { { ChordLineType::PLOP, false },      "3",     TranslatableString("engraving", "Plop") },
    { { ChordLineType::SCOOP, false },     "4",     TranslatableString("engraving", "Scoop") },
    { { ChordLineType::NOTYPE, true },     "0" },
    { { ChordLineType::FALL, true },       "1",     TranslatableString("engraving", "Slide out down") },
    { { ChordLineType::DOIT, true },       "2",     TranslatableString("engraving", "Slide out up") },
    { { ChordLineType::PLOP, true },       "3",     TranslatableString("engraving", "Slide in above") },
    { { ChordLineType::SCOOP, true },      "4",     TranslatableString("engraving", "Slide in below") }
} };

const TranslatableString& TConv::userName(ChordLineType v, bool straight)
{
    return findUserNameByType<std::pair<ChordLineType, bool> >(CHORDLINE_TYPES, { v, straight });
}

AsciiStringView TConv::toXml(ChordLineType v)
{
    return findXmlTagByType<std::pair<ChordLineType, bool> >(CHORDLINE_TYPES, { v, false });
}

ChordLineType TConv::fromXml(const AsciiStringView& tag, ChordLineType def)
{
    return findTypeByXmlTag<std::pair<ChordLineType, bool> >(CHORDLINE_TYPES, tag, { def, false }).first;
}

struct DrumPitchItem {
    DrumNum num = DrumNum(0);
    const char* userName;
};

// TODO: Can't use TranslatableString, because Drumset uses these strings and doesn't support TranslatableString
static const std::vector<DrumPitchItem> DRUMPITCHS = {
    { DrumNum(27),       QT_TRANSLATE_NOOP("engraving/drumset", "High Q") },
    { DrumNum(28),       QT_TRANSLATE_NOOP("engraving/drumset", "Slap") },
    { DrumNum(29),       QT_TRANSLATE_NOOP("engraving/drumset", "Scratch Push") },

    { DrumNum(30),       QT_TRANSLATE_NOOP("engraving/drumset", "Scratch Pull") },
    { DrumNum(31),       QT_TRANSLATE_NOOP("engraving/drumset", "Sticks") },
    { DrumNum(32),       QT_TRANSLATE_NOOP("engraving/drumset", "Square Click") },
    { DrumNum(33),       QT_TRANSLATE_NOOP("engraving/drumset", "Metronome Click") },
    { DrumNum(34),       QT_TRANSLATE_NOOP("engraving/drumset", "Metronome Bell") },
    { DrumNum(35),       QT_TRANSLATE_NOOP("engraving/drumset", "Acoustic Bass Drum") },
    { DrumNum(36),       QT_TRANSLATE_NOOP("engraving/drumset", "Bass Drum 1") },
    { DrumNum(37),       QT_TRANSLATE_NOOP("engraving/drumset", "Side Stick") },
    { DrumNum(38),       QT_TRANSLATE_NOOP("engraving/drumset", "Acoustic Snare") },
    { DrumNum(39),       QT_TRANSLATE_NOOP("engraving/drumset", "Hand Clap") },

    { DrumNum(40),       QT_TRANSLATE_NOOP("engraving/drumset", "Electric Snare") },
    { DrumNum(41),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Floor Tom") },
    { DrumNum(42),       QT_TRANSLATE_NOOP("engraving/drumset", "Closed Hi-Hat") },
    { DrumNum(43),       QT_TRANSLATE_NOOP("engraving/drumset", "High Floor Tom") },
    { DrumNum(44),       QT_TRANSLATE_NOOP("engraving/drumset", "Pedal Hi-Hat") },
    { DrumNum(45),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Tom") },
    { DrumNum(46),       QT_TRANSLATE_NOOP("engraving/drumset", "Open Hi-Hat") },
    { DrumNum(47),       QT_TRANSLATE_NOOP("engraving/drumset", "Low-Mid Tom") },
    { DrumNum(48),       QT_TRANSLATE_NOOP("engraving/drumset", "Hi-Mid Tom") },
    { DrumNum(49),       QT_TRANSLATE_NOOP("engraving/drumset", "Crash Cymbal 1") },

    { DrumNum(50),       QT_TRANSLATE_NOOP("engraving/drumset", "High Tom") },
    { DrumNum(51),       QT_TRANSLATE_NOOP("engraving/drumset", "Ride Cymbal 1") },
    { DrumNum(52),       QT_TRANSLATE_NOOP("engraving/drumset", "Chinese Cymbal") },
    { DrumNum(53),       QT_TRANSLATE_NOOP("engraving/drumset", "Ride Bell") },
    { DrumNum(54),       QT_TRANSLATE_NOOP("engraving/drumset", "Tambourine") },
    { DrumNum(55),       QT_TRANSLATE_NOOP("engraving/drumset", "Splash Cymbal") },
    { DrumNum(56),       QT_TRANSLATE_NOOP("engraving/drumset", "Cowbell") },
    { DrumNum(57),       QT_TRANSLATE_NOOP("engraving/drumset", "Crash Cymbal 2") },
    { DrumNum(58),       QT_TRANSLATE_NOOP("engraving/drumset", "Vibraslap") },
    { DrumNum(59),       QT_TRANSLATE_NOOP("engraving/drumset", "Ride Cymbal 2") },

    { DrumNum(60),       QT_TRANSLATE_NOOP("engraving/drumset", "Hi Bongo") },
    { DrumNum(61),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Bongo") },
    { DrumNum(62),       QT_TRANSLATE_NOOP("engraving/drumset", "Mute Hi Conga") },
    { DrumNum(63),       QT_TRANSLATE_NOOP("engraving/drumset", "Open Hi Conga") },
    { DrumNum(64),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Conga") },
    { DrumNum(65),       QT_TRANSLATE_NOOP("engraving/drumset", "High Timbale") },
    { DrumNum(66),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Timbale") },
    { DrumNum(67),       QT_TRANSLATE_NOOP("engraving/drumset", "High Agogo") },
    { DrumNum(68),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Agogo") },
    { DrumNum(69),       QT_TRANSLATE_NOOP("engraving/drumset", "Cabasa") },

    { DrumNum(70),       QT_TRANSLATE_NOOP("engraving/drumset", "Maracas") },
    { DrumNum(71),       QT_TRANSLATE_NOOP("engraving/drumset", "Short Whistle") },
    { DrumNum(72),       QT_TRANSLATE_NOOP("engraving/drumset", "Long Whistle") },
    { DrumNum(73),       QT_TRANSLATE_NOOP("engraving/drumset", "Short Güiro") },
    { DrumNum(74),       QT_TRANSLATE_NOOP("engraving/drumset", "Long Güiro") },
    { DrumNum(75),       QT_TRANSLATE_NOOP("engraving/drumset", "Claves") },
    { DrumNum(76),       QT_TRANSLATE_NOOP("engraving/drumset", "Hi Wood Block") },
    { DrumNum(77),       QT_TRANSLATE_NOOP("engraving/drumset", "Low Wood Block") },
    { DrumNum(78),       QT_TRANSLATE_NOOP("engraving/drumset", "Mute Cuica") },
    { DrumNum(79),       QT_TRANSLATE_NOOP("engraving/drumset", "Open Cuica") },

    { DrumNum(80),       QT_TRANSLATE_NOOP("engraving/drumset", "Mute Triangle") },
    { DrumNum(81),       QT_TRANSLATE_NOOP("engraving/drumset", "Open Triangle") },
    { DrumNum(82),       QT_TRANSLATE_NOOP("engraving/drumset", "Shaker") },
    { DrumNum(83),       QT_TRANSLATE_NOOP("engraving/drumset", "Sleigh Bell") },
    { DrumNum(84),       QT_TRANSLATE_NOOP("engraving/drumset", "Mark Tree") },
    { DrumNum(85),       QT_TRANSLATE_NOOP("engraving/drumset", "Castanets") },
    { DrumNum(86),       QT_TRANSLATE_NOOP("engraving/drumset", "Mute Surdo") },
    { DrumNum(87),       QT_TRANSLATE_NOOP("engraving/drumset", "Open Surdo") },

    { DrumNum(91),       QT_TRANSLATE_NOOP("engraving/drumset", "Snare (Rim shot)") },

    { DrumNum(93),       QT_TRANSLATE_NOOP("engraving/drumset", "Ride (Edge)") },

    { DrumNum(99),       QT_TRANSLATE_NOOP("engraving/drumset", "Cowbell Low") },

    { DrumNum(102),      QT_TRANSLATE_NOOP("engraving/drumset", "Cowbell High") },
};

const char* TConv::userName(DrumNum v)
{
    auto it = std::find_if(DRUMPITCHS.cbegin(), DRUMPITCHS.cend(), [v](const DrumPitchItem& i) {
        return i.num == v;
    });

    IF_ASSERT_FAILED(it != DRUMPITCHS.cend()) {
        static const char* dummy = "";
        return dummy;
    }
    return it->userName;
}

//! TODO Add xml names
static const std::array<Item<GlissandoType>, 2> GLISSANDO_TYPES = { {
    { GlissandoType::STRAIGHT,  "0",     TranslatableString("engraving", "Straight glissando") },
    { GlissandoType::WAVY,      "1",     TranslatableString("engraving", "Wavy glissando") }
} };

const TranslatableString& TConv::userName(GlissandoType v)
{
    return findUserNameByType<GlissandoType>(GLISSANDO_TYPES, v);
}

AsciiStringView TConv::toXml(GlissandoType v)
{
    return findXmlTagByType<GlissandoType>(GLISSANDO_TYPES, v);
}

GlissandoType TConv::fromXml(const AsciiStringView& tag, GlissandoType def)
{
    return findTypeByXmlTag<GlissandoType>(GLISSANDO_TYPES, tag, def);
}

static const std::vector<Item<JumpType> > JUMP_TYPES = {
    { JumpType::DC,             "", TranslatableString("engraving", "Da Capo") },
    { JumpType::DC_AL_FINE,     "", TranslatableString("engraving", "Da Capo al Fine") },
    { JumpType::DC_AL_CODA,     "", TranslatableString("engraving", "Da Capo al Coda") },
    { JumpType::DS_AL_CODA,     "", TranslatableString("engraving", "D.S. al Coda") },
    { JumpType::DS_AL_FINE,     "", TranslatableString("engraving", "D.S. al Fine") },
    { JumpType::DS,             "", TranslatableString("engraving", "D.S.") },

    { JumpType::DC_AL_DBLCODA,  "", TranslatableString("engraving", "Da Capo al Double Coda") },
    { JumpType::DS_AL_DBLCODA,  "", TranslatableString("engraving", "Dal Segno al Double Coda") },
    { JumpType::DSS,            "", TranslatableString("engraving", "Dal Segno Segno") },
    { JumpType::DSS_AL_CODA,    "", TranslatableString("engraving", "Dal Segno Segno al Coda") },
    { JumpType::DSS_AL_DBLCODA, "", TranslatableString("engraving", "Dal Segno Segno al Double Coda") },
    { JumpType::DSS_AL_FINE,    "", TranslatableString("engraving", "Dal Segno Segno al Fine") },
    { JumpType::DCODA,          "", TranslatableString("engraving", "Da Coda") },
    { JumpType::DDBLCODA,       "", TranslatableString("engraving", "Da Double Coda") },

    { JumpType::USER,           "", TranslatableString("engraving", "Custom") }
};

const TranslatableString& TConv::userName(JumpType v)
{
    return findUserNameByType<JumpType>(JUMP_TYPES, v);
}

String TConv::translatedUserName(JumpType v)
{
    return findUserNameByType<JumpType>(JUMP_TYPES, v).translated();
}

static const std::array<Item<MarkerType>, 9> MARKER_TYPES = { {
    { MarkerType::SEGNO,        "segno",    TranslatableString("engraving", "Segno") },
    { MarkerType::VARSEGNO,     "varsegno", TranslatableString("engraving", "Segno variation") },
    { MarkerType::CODA,         "codab",    TranslatableString("engraving", "Coda") },
    { MarkerType::VARCODA,      "varcoda",  TranslatableString("engraving", "Varied coda") },
    { MarkerType::CODETTA,      "codetta",  TranslatableString("engraving", "Codetta") },
    { MarkerType::FINE,         "fine",     TranslatableString("engraving", "Fine") },
    { MarkerType::TOCODA,       "coda",     TranslatableString("engraving", "To coda") },
    { MarkerType::TOCODASYM,    "",         TranslatableString("engraving", "To coda (symbol)") },
    { MarkerType::USER,         "",         TranslatableString("engraving", "Custom") }
} };

const TranslatableString& TConv::userName(MarkerType v)
{
    return findUserNameByType<MarkerType>(MARKER_TYPES, v);
}

String TConv::translatedUserName(MarkerType v)
{
    return findUserNameByType<MarkerType>(MARKER_TYPES, v).translated();
}

AsciiStringView TConv::toXml(MarkerType v)
{
    return findXmlTagByType<MarkerType>(MARKER_TYPES, v);
}

MarkerType TConv::fromXml(const AsciiStringView& tag, MarkerType def)
{
    auto it = std::find_if(MARKER_TYPES.cbegin(), MARKER_TYPES.cend(), [tag](const Item<MarkerType>& i) {
        return i.xml == tag;
    });

    if (it != MARKER_TYPES.cend()) {
        return it->type;
    }

    // compatibility

    if (tag == "Repeat") {
        return MarkerType::TOCODA;
    }

    return def;
}

static const std::array<Item<StaffGroup>, 3> STAFFGROUP_TYPES = { {
    { StaffGroup::STANDARD,     "pitched",    TranslatableString("engraving/staffgroup", "Standard") },
    { StaffGroup::PERCUSSION,   "percussion", TranslatableString("engraving/staffgroup", "Percussion") },
    { StaffGroup::TAB,          "tablature",  TranslatableString("engraving/staffgroup", "Tablature") },
} };

String TConv::translatedUserName(StaffGroup v)
{
    return findUserNameByType<StaffGroup>(STAFFGROUP_TYPES, v).translated();
}

AsciiStringView TConv::toXml(StaffGroup v)
{
    return findXmlTagByType<StaffGroup>(STAFFGROUP_TYPES, v);
}

StaffGroup TConv::fromXml(const AsciiStringView& tag, StaffGroup def)
{
    return findTypeByXmlTag<StaffGroup>(STAFFGROUP_TYPES, tag, def);
}

const std::array<Item<TrillType>, 4> TRILL_TYPES = { {
    { TrillType::TRILL_LINE,      "trill",      TranslatableString("engraving/trilltype", "Trill line") },
    { TrillType::UPPRALL_LINE,    "upprall",    TranslatableString("engraving/trilltype", "Upprall line") },
    { TrillType::DOWNPRALL_LINE,  "downprall",  TranslatableString("engraving/trilltype", "Downprall line") },
    { TrillType::PRALLPRALL_LINE, "prallprall", TranslatableString("engraving/trilltype", "Prallprall line") }
} };

const TranslatableString& TConv::userName(TrillType v)
{
    return findUserNameByType<TrillType>(TRILL_TYPES, v);
}

String TConv::translatedUserName(TrillType v)
{
    return findUserNameByType<TrillType>(TRILL_TYPES, v).translated();
}

AsciiStringView TConv::toXml(TrillType v)
{
    return findXmlTagByType<TrillType>(TRILL_TYPES, v);
}

TrillType TConv::fromXml(const AsciiStringView& tag, TrillType def)
{
    auto it = std::find_if(TRILL_TYPES.cbegin(), TRILL_TYPES.cend(), [tag](const Item<TrillType>& i) {
        return i.xml == tag;
    });

    if (it != TRILL_TYPES.cend()) {
        return it->type;
    }

    // compatibility

    if (tag == "0") {
        return TrillType::TRILL_LINE;
    } else if (tag == "pure") {
        return TrillType::PRALLPRALL_LINE;     // obsolete, compatibility only
    }

    return def;
}

const std::array<Item<VibratoType>, 4> VIBRATO_TYPES = { {
    { VibratoType::GUITAR_VIBRATO,        "guitarVibrato",       TranslatableString("engraving/vibratotype", "Guitar vibrato") },
    { VibratoType::GUITAR_VIBRATO_WIDE,   "guitarVibratoWide",   TranslatableString("engraving/vibratotype", "Guitar vibrato wide") },
    { VibratoType::VIBRATO_SAWTOOTH,      "vibratoSawtooth",     TranslatableString("engraving/vibratotype", "Vibrato sawtooth") },
    { VibratoType::VIBRATO_SAWTOOTH_WIDE, "vibratoSawtoothWide", TranslatableString("engraving/vibratotype", "Tremolo sawtooth wide") }
} };

const TranslatableString& TConv::userName(VibratoType v)
{
    return findUserNameByType<VibratoType>(VIBRATO_TYPES, v);
}

String TConv::translatedUserName(VibratoType v)
{
    return findUserNameByType<VibratoType>(VIBRATO_TYPES, v).translated();
}

AsciiStringView TConv::toXml(VibratoType v)
{
    return findXmlTagByType<VibratoType>(VIBRATO_TYPES, v);
}

VibratoType TConv::fromXml(const AsciiStringView& tag, VibratoType def)
{
    return findTypeByXmlTag<VibratoType>(VIBRATO_TYPES, tag, def);
}

const std::array<const char*, 17> KEY_NAMES = { {
    QT_TRANSLATE_NOOP("engraving", "G major, E minor"),
    QT_TRANSLATE_NOOP("engraving", "C♭ major, A♭ minor"),
    QT_TRANSLATE_NOOP("engraving", "D major, B minor"),
    QT_TRANSLATE_NOOP("engraving", "G♭ major, E♭ minor"),
    QT_TRANSLATE_NOOP("engraving", "A major, F♯ minor"),
    QT_TRANSLATE_NOOP("engraving", "D♭ major, B♭ minor"),
    QT_TRANSLATE_NOOP("engraving", "E major, C♯ minor"),
    QT_TRANSLATE_NOOP("engraving", "A♭ major, F minor"),
    QT_TRANSLATE_NOOP("engraving", "B major, G♯ minor"),
    QT_TRANSLATE_NOOP("engraving", "E♭ major, C minor"),
    QT_TRANSLATE_NOOP("engraving", "F♯ major, D♯ minor"),
    QT_TRANSLATE_NOOP("engraving", "B♭ major, G minor"),
    QT_TRANSLATE_NOOP("engraving", "C♯ major, A♯ minor"),
    QT_TRANSLATE_NOOP("engraving", "F major, D minor"),
    QT_TRANSLATE_NOOP("engraving", "C major, A minor"),
    QT_TRANSLATE_NOOP("engraving", "Open/Atonal"),
    QT_TRANSLATE_NOOP("engraving", "Custom")
} };

const char* TConv::userName(Key v, bool isAtonal, bool isCustom)
{
    if (isAtonal) {
        return KEY_NAMES[15];
    } else if (isCustom) {
        return KEY_NAMES[16];
    }

    if (v == Key::C) {
        return KEY_NAMES[14];
    }

    int keyInt = static_cast<int>(v);
    if (keyInt < 0) {
        return KEY_NAMES[(7 + keyInt) * 2 + 1];
    } else {
        return KEY_NAMES[(keyInt - 1) * 2];
    }
}

String TConv::translatedUserName(Key v, bool isAtonal, bool isCustom)
{
    return mtrc("engraving", userName(v, isAtonal, isCustom));
}
