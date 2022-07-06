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

#include "translation.h"

#include "symnames.h"

#include "log.h"

#ifdef ENGRAVING_NO_TRANSLATION
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(ctx, str) ""
#endif

using namespace mu;
using namespace mu::engraving;

template<typename T>
struct Item
{
    T type;
    AsciiStringView xml;
    const char* userName = nullptr;
};

template<typename T, typename C>
static const char* findUserNameByType(const C& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static const char* dummy = "";
        return dummy;
    }

    if (!it->userName || std::strlen(it->userName) == 0) {
        return it->xml.ascii();
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
    { ElementType::INVALID,              "invalid",              QT_TRANSLATE_NOOP("engraving", "Invalid") },
    { ElementType::BRACKET_ITEM,         "BracketItem",          QT_TRANSLATE_NOOP("engraving", "Bracket") },
    { ElementType::PART,                 "Part",                 QT_TRANSLATE_NOOP("engraving", "Part") },
    { ElementType::STAFF,                "Staff",                QT_TRANSLATE_NOOP("engraving", "Staff") },
    { ElementType::SCORE,                "Score",                QT_TRANSLATE_NOOP("engraving", "Score") },
    { ElementType::SYMBOL,               "Symbol",               QT_TRANSLATE_NOOP("engraving", "Symbol") },
    { ElementType::TEXT,                 "Text",                 QT_TRANSLATE_NOOP("engraving", "Text") },
    { ElementType::MEASURE_NUMBER,       "MeasureNumber",        QT_TRANSLATE_NOOP("engraving", "Measure number") },
    { ElementType::MMREST_RANGE,         "MMRestRange",          QT_TRANSLATE_NOOP("engraving", "Multimeasure rest range") },
    { ElementType::INSTRUMENT_NAME,      "InstrumentName",       QT_TRANSLATE_NOOP("engraving", "Instrument name") },
    { ElementType::SLUR_SEGMENT,         "SlurSegment",          QT_TRANSLATE_NOOP("engraving", "Slur segment") },
    { ElementType::TIE_SEGMENT,          "TieSegment",           QT_TRANSLATE_NOOP("engraving", "Tie segment") },
    { ElementType::BAR_LINE,             "BarLine",              QT_TRANSLATE_NOOP("engraving", "Barline") },
    { ElementType::STAFF_LINES,          "StaffLines",           QT_TRANSLATE_NOOP("engraving", "Staff lines") },
    { ElementType::SYSTEM_DIVIDER,       "SystemDivider",        QT_TRANSLATE_NOOP("engraving", "System divider") },
    { ElementType::STEM_SLASH,           "StemSlash",            QT_TRANSLATE_NOOP("engraving", "Stem slash") },
    { ElementType::ARPEGGIO,             "Arpeggio",             QT_TRANSLATE_NOOP("engraving", "Arpeggio") },
    { ElementType::ACCIDENTAL,           "Accidental",           QT_TRANSLATE_NOOP("engraving", "Accidental") },
    { ElementType::LEDGER_LINE,          "LedgerLine",           QT_TRANSLATE_NOOP("engraving", "Ledger line") },
    { ElementType::STEM,                 "Stem",                 QT_TRANSLATE_NOOP("engraving", "Stem") },
    { ElementType::NOTE,                 "Note",                 QT_TRANSLATE_NOOP("engraving", "Note") },
    { ElementType::CLEF,                 "Clef",                 QT_TRANSLATE_NOOP("engraving", "Clef") },
    { ElementType::KEYSIG,               "KeySig",               QT_TRANSLATE_NOOP("engraving", "Key signature") },
    { ElementType::AMBITUS,              "Ambitus",              QT_TRANSLATE_NOOP("engraving", "Ambitus") },
    { ElementType::TIMESIG,              "TimeSig",              QT_TRANSLATE_NOOP("engraving", "Time signature") },
    { ElementType::REST,                 "Rest",                 QT_TRANSLATE_NOOP("engraving", "Rest") },
    { ElementType::MMREST,               "MMRest",               QT_TRANSLATE_NOOP("engraving", "Multimeasure rest") },
    { ElementType::BREATH,               "Breath",               QT_TRANSLATE_NOOP("engraving", "Breath") },
    { ElementType::MEASURE_REPEAT,       "MeasureRepeat",        QT_TRANSLATE_NOOP("engraving", "Measure repeat") },
    { ElementType::TIE,                  "Tie",                  QT_TRANSLATE_NOOP("engraving", "Tie") },
    { ElementType::ARTICULATION,         "Articulation",         QT_TRANSLATE_NOOP("engraving", "Articulation") },
    { ElementType::FERMATA,              "Fermata",              QT_TRANSLATE_NOOP("engraving", "Fermata") },
    { ElementType::CHORDLINE,            "ChordLine",            QT_TRANSLATE_NOOP("engraving", "Chord line") },
    { ElementType::DYNAMIC,              "Dynamic",              QT_TRANSLATE_NOOP("engraving", "Dynamic") },
    { ElementType::BEAM,                 "Beam",                 QT_TRANSLATE_NOOP("engraving", "Beam") },
    { ElementType::HOOK,                 "Hook",                 QT_TRANSLATE_NOOP("engraving", "Flag") }, // internally called "Hook", but "Flag" in SMuFL, so here externally too
    { ElementType::LYRICS,               "Lyrics",               QT_TRANSLATE_NOOP("engraving", "Lyrics") },
    { ElementType::FIGURED_BASS,         "FiguredBass",          QT_TRANSLATE_NOOP("engraving", "Figured bass") },
    { ElementType::MARKER,               "Marker",               QT_TRANSLATE_NOOP("engraving", "Marker") },
    { ElementType::JUMP,                 "Jump",                 QT_TRANSLATE_NOOP("engraving", "Jump") },
    { ElementType::FINGERING,            "Fingering",            QT_TRANSLATE_NOOP("engraving", "Fingering") },
    { ElementType::TUPLET,               "Tuplet",               QT_TRANSLATE_NOOP("engraving", "Tuplet") },
    { ElementType::TEMPO_TEXT,           "Tempo",                QT_TRANSLATE_NOOP("engraving", "Tempo") },
    { ElementType::STAFF_TEXT,           "StaffText",            QT_TRANSLATE_NOOP("engraving", "Staff text") },
    { ElementType::SYSTEM_TEXT,          "SystemText",           QT_TRANSLATE_NOOP("engraving", "System text") },
    { ElementType::PLAYTECH_ANNOTATION,  "PlayTechAnnotation",   QT_TRANSLATE_NOOP("engraving", "Playing technique annotation") },
    { ElementType::TRIPLET_FEEL,         "TripletFeel",          QT_TRANSLATE_NOOP("engraving", "Triplet feel") },
    { ElementType::REHEARSAL_MARK,       "RehearsalMark",        QT_TRANSLATE_NOOP("engraving", "Rehearsal mark") },
    { ElementType::INSTRUMENT_CHANGE,    "InstrumentChange",     QT_TRANSLATE_NOOP("engraving", "Instrument change") },
    { ElementType::STAFFTYPE_CHANGE,     "StaffTypeChange",      QT_TRANSLATE_NOOP("engraving", "Staff type change") },
    { ElementType::HARMONY,              "Harmony",              QT_TRANSLATE_NOOP("engraving", "Chord symbol") },
    { ElementType::FRET_DIAGRAM,         "FretDiagram",          QT_TRANSLATE_NOOP("engraving", "Fretboard diagram") },
    { ElementType::BEND,                 "Bend",                 QT_TRANSLATE_NOOP("engraving", "Bend") },
    { ElementType::TREMOLOBAR,           "TremoloBar",           QT_TRANSLATE_NOOP("engraving", "Tremolo bar") },
    { ElementType::VOLTA,                "Volta",                QT_TRANSLATE_NOOP("engraving", "Volta") },
    { ElementType::HAIRPIN_SEGMENT,      "HairpinSegment",       QT_TRANSLATE_NOOP("engraving", "Hairpin segment") },
    { ElementType::OTTAVA_SEGMENT,       "OttavaSegment",        QT_TRANSLATE_NOOP("engraving", "Ottava segment") },
    { ElementType::TRILL_SEGMENT,        "TrillSegment",         QT_TRANSLATE_NOOP("engraving", "Trill segment") },
    { ElementType::LET_RING_SEGMENT,     "LetRingSegment",       QT_TRANSLATE_NOOP("engraving", "Let ring segment") },
    { ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, "GradualTempoChangeSegment", QT_TRANSLATE_NOOP("engraving",
                                                                                                "Gradual tempo change segment") },
    { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       QT_TRANSLATE_NOOP("engraving", "Vibrato segment") },
    { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      QT_TRANSLATE_NOOP("engraving", "Palm mute segment") },
    { ElementType::WHAMMY_BAR_SEGMENT,   "WhammyBarSegment",     QT_TRANSLATE_NOOP("engraving", "Whammy bar segment") },
    { ElementType::RASGUEADO_SEGMENT,    "RasgueadoSegment",     QT_TRANSLATE_NOOP("engraving", "Rasgueado segment") },
    { ElementType::HARMONIC_MARK_SEGMENT,    "HarmonicMarkSegment",    QT_TRANSLATE_NOOP("engraving", "Harmonic mark segment") },
    { ElementType::TEXTLINE_SEGMENT,     "TextLineSegment",      QT_TRANSLATE_NOOP("engraving", "Text line segment") },
    { ElementType::VOLTA_SEGMENT,        "VoltaSegment",         QT_TRANSLATE_NOOP("engraving", "Volta segment") },
    { ElementType::PEDAL_SEGMENT,        "PedalSegment",         QT_TRANSLATE_NOOP("engraving", "Pedal segment") },
    { ElementType::LYRICSLINE_SEGMENT,   "LyricsLineSegment",    QT_TRANSLATE_NOOP("engraving", "Melisma line segment") },
    { ElementType::GLISSANDO_SEGMENT,    "GlissandoSegment",     QT_TRANSLATE_NOOP("engraving", "Glissando segment") },
    { ElementType::LAYOUT_BREAK,         "LayoutBreak",          QT_TRANSLATE_NOOP("engraving", "Layout break") },
    { ElementType::SPACER,               "Spacer",               QT_TRANSLATE_NOOP("engraving", "Spacer") },
    { ElementType::STAFF_STATE,          "StaffState",           QT_TRANSLATE_NOOP("engraving", "Staff state") },
    { ElementType::NOTEHEAD,             "NoteHead",             QT_TRANSLATE_NOOP("engraving", "Notehead") },
    { ElementType::NOTEDOT,              "NoteDot",              QT_TRANSLATE_NOOP("engraving", "Note dot") },
    { ElementType::TREMOLO,              "Tremolo",              QT_TRANSLATE_NOOP("engraving", "Tremolo") },
    { ElementType::IMAGE,                "Image",                QT_TRANSLATE_NOOP("engraving", "Image") },
    { ElementType::MEASURE,              "Measure",              QT_TRANSLATE_NOOP("engraving", "Measure") },
    { ElementType::SELECTION,            "Selection",            QT_TRANSLATE_NOOP("engraving", "Selection") },
    { ElementType::LASSO,                "Lasso",                QT_TRANSLATE_NOOP("engraving", "Lasso") },
    { ElementType::SHADOW_NOTE,          "ShadowNote",           QT_TRANSLATE_NOOP("engraving", "Shadow note") },
    { ElementType::TAB_DURATION_SYMBOL,  "TabDurationSymbol",    QT_TRANSLATE_NOOP("engraving", "Tab duration symbol") },
    { ElementType::FSYMBOL,              "FSymbol",              QT_TRANSLATE_NOOP("engraving", "Font symbol") },
    { ElementType::PAGE,                 "Page",                 QT_TRANSLATE_NOOP("engraving", "Page") },
    { ElementType::HAIRPIN,              "HairPin",              QT_TRANSLATE_NOOP("engraving", "Hairpin") },
    { ElementType::OTTAVA,               "Ottava",               QT_TRANSLATE_NOOP("engraving", "Ottava") },
    { ElementType::PEDAL,                "Pedal",                QT_TRANSLATE_NOOP("engraving", "Pedal") },
    { ElementType::TRILL,                "Trill",                QT_TRANSLATE_NOOP("engraving", "Trill") },
    { ElementType::LET_RING,             "LetRing",              QT_TRANSLATE_NOOP("engraving", "Let ring") },
    { ElementType::GRADUAL_TEMPO_CHANGE, "GradualTempoChange",   QT_TRANSLATE_NOOP("engraving", "Gradual tempo change") },
    { ElementType::VIBRATO,              "Vibrato",              QT_TRANSLATE_NOOP("engraving", "Vibrato") },
    { ElementType::PALM_MUTE,            "PalmMute",             QT_TRANSLATE_NOOP("engraving", "Palm mute") },
    { ElementType::WHAMMY_BAR,           "WhammyBar",            QT_TRANSLATE_NOOP("engraving", "Whammy bar") },
    { ElementType::RASGUEADO,            "Rasgueado",            QT_TRANSLATE_NOOP("engraving", "Rasgueado") },
    { ElementType::HARMONIC_MARK,        "HarmonicMark",         QT_TRANSLATE_NOOP("engraving", "Harmonic Mark") },
    { ElementType::TEXTLINE,             "TextLine",             QT_TRANSLATE_NOOP("engraving", "Text line") },
    { ElementType::TEXTLINE_BASE,        "TextLineBase",         QT_TRANSLATE_NOOP("engraving", "Text line base") },    // remove
    { ElementType::NOTELINE,             "NoteLine",             QT_TRANSLATE_NOOP("engraving", "Note line") },
    { ElementType::LYRICSLINE,           "LyricsLine",           QT_TRANSLATE_NOOP("engraving", "Melisma line") },
    { ElementType::GLISSANDO,            "Glissando",            QT_TRANSLATE_NOOP("engraving", "Glissando") },
    { ElementType::BRACKET,              "Bracket",              QT_TRANSLATE_NOOP("engraving", "Bracket") },
    { ElementType::SEGMENT,              "Segment",              QT_TRANSLATE_NOOP("engraving", "Segment") },
    { ElementType::SYSTEM,               "System",               QT_TRANSLATE_NOOP("engraving", "System") },
    { ElementType::COMPOUND,             "Compound",             QT_TRANSLATE_NOOP("engraving", "Compound") },
    { ElementType::CHORD,                "Chord",                QT_TRANSLATE_NOOP("engraving", "Chord") },
    { ElementType::SLUR,                 "Slur",                 QT_TRANSLATE_NOOP("engraving", "Slur") },
    { ElementType::ELEMENT,              "EngravingItem",        QT_TRANSLATE_NOOP("engraving", "EngravingItem") },
    { ElementType::ELEMENT_LIST,         "ElementList",          QT_TRANSLATE_NOOP("engraving", "EngravingItem list") },
    { ElementType::STAFF_LIST,           "StaffList",            QT_TRANSLATE_NOOP("engraving", "Staff list") },
    { ElementType::MEASURE_LIST,         "MeasureList",          QT_TRANSLATE_NOOP("engraving", "Measure list") },
    { ElementType::HBOX,                 "HBox",                 QT_TRANSLATE_NOOP("engraving", "Horizontal frame") },
    { ElementType::VBOX,                 "VBox",                 QT_TRANSLATE_NOOP("engraving", "Vertical frame") },
    { ElementType::TBOX,                 "TBox",                 QT_TRANSLATE_NOOP("engraving", "Text frame") },
    { ElementType::FBOX,                 "FBox",                 QT_TRANSLATE_NOOP("engraving", "Fretboard diagram frame") },
    { ElementType::ACTION_ICON,          "ActionIcon",           QT_TRANSLATE_NOOP("engraving", "Action icon") },
    { ElementType::OSSIA,                "Ossia",                QT_TRANSLATE_NOOP("engraving", "Ossia") },
    { ElementType::BAGPIPE_EMBELLISHMENT, "BagpipeEmbellishment", QT_TRANSLATE_NOOP("engraving", "Bagpipe embellishment") },
    { ElementType::STICKING,             "Sticking",             QT_TRANSLATE_NOOP("engraving", "Sticking") },
    { ElementType::GRACE_NOTES_GROUP,    "GraceNotesGroup",      QT_TRANSLATE_NOOP("engraving", "Grace notes group") },
    { ElementType::ROOT_ITEM,            "RootItem",             QT_TRANSLATE_NOOP("engraving", "Root item") },
    { ElementType::DUMMY,                "Dummy",                QT_TRANSLATE_NOOP("engraving", "Dummy") },
};

String TConv::translatedUserName(ElementType v)
{
    return mtrc("engraving", findUserNameByType<ElementType>(ELEMENT_TYPES, v));
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
    { Orientation::VERTICAL,    "vertical",     QT_TRANSLATE_NOOP("engraving", "Vertical") },
    { Orientation::HORIZONTAL,  "horizontal",   QT_TRANSLATE_NOOP("engraving", "Horizontal") },
} };

String TConv::translatedUserName(Orientation v)
{
    return mtrc("engraving", findUserNameByType<Orientation>(ORIENTATION, v));
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
    { NoteHeadType::HEAD_AUTO,      "auto",    QT_TRANSLATE_NOOP("engraving", "Auto") },
    { NoteHeadType::HEAD_WHOLE,     "whole",   QT_TRANSLATE_NOOP("engraving", "Whole") },
    { NoteHeadType::HEAD_HALF,      "half",    QT_TRANSLATE_NOOP("engraving", "Half") },
    { NoteHeadType::HEAD_QUARTER,   "quarter", QT_TRANSLATE_NOOP("engraving", "Quarter") },
    { NoteHeadType::HEAD_BREVIS,    "breve",   QT_TRANSLATE_NOOP("engraving", "Breve") },
} };

String TConv::translatedUserName(NoteHeadType v)
{
    return mtrc("engraving", findUserNameByType<NoteHeadType>(NOTEHEAD_TYPES, v));
}

AsciiStringView TConv::toXml(NoteHeadType v)
{
    return findXmlTagByType<NoteHeadType>(NOTEHEAD_TYPES, v);
}

NoteHeadType TConv::fromXml(const AsciiStringView& tag, NoteHeadType def)
{
    return findTypeByXmlTag<NoteHeadType>(NOTEHEAD_TYPES, tag, def);
}

static const std::vector<Item<NoteHeadScheme> > NOTEHEAD_SCHEMES = {
    { NoteHeadScheme::HEAD_AUTO,                "auto",                QT_TRANSLATE_NOOP("engraving", "Auto") },
    { NoteHeadScheme::HEAD_NORMAL,              "normal",              QT_TRANSLATE_NOOP("engraving", "Normal") },
    { NoteHeadScheme::HEAD_PITCHNAME,           "name-pitch",          QT_TRANSLATE_NOOP("engraving", "Pitch names") },
    { NoteHeadScheme::HEAD_PITCHNAME_GERMAN,    "name-pitch-german",   QT_TRANSLATE_NOOP("engraving", "German Pitch names") },
    { NoteHeadScheme::HEAD_SOLFEGE,             "solfege-movable",     QT_TRANSLATE_NOOP("engraving", "Solf\u00e8ge movable Do") },  // &egrave;
    { NoteHeadScheme::HEAD_SOLFEGE_FIXED,       "solfege-fixed",       QT_TRANSLATE_NOOP("engraving", "Solf\u00e8ge fixed Do") },    // &egrave;
    { NoteHeadScheme::HEAD_SHAPE_NOTE_4,        "shape-4",             QT_TRANSLATE_NOOP("engraving", "4-shape (Walker)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,  "shape-7-aikin",       QT_TRANSLATE_NOOP("engraving", "7-shape (Aikin)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,   "shape-7-funk",        QT_TRANSLATE_NOOP("engraving", "7-shape (Funk)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER, "shape-7-walker",      QT_TRANSLATE_NOOP("engraving", "7-shape (Walker)") }
};

String TConv::translatedUserName(NoteHeadScheme v)
{
    return mtrc("engraving", findUserNameByType<NoteHeadScheme>(NOTEHEAD_SCHEMES, v));
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
    { NoteHeadGroup::HEAD_NORMAL,           "normal",         QT_TRANSLATE_NOOP("engraving", "Normal") },
    { NoteHeadGroup::HEAD_CROSS,            "cross",          QT_TRANSLATE_NOOP("engraving", "Cross") },
    { NoteHeadGroup::HEAD_PLUS,             "plus",           QT_TRANSLATE_NOOP("engraving", "Plus") },
    { NoteHeadGroup::HEAD_XCIRCLE,          "xcircle",        QT_TRANSLATE_NOOP("engraving", "XCircle") },
    { NoteHeadGroup::HEAD_WITHX,            "withx",          QT_TRANSLATE_NOOP("engraving", "With X") },
    { NoteHeadGroup::HEAD_TRIANGLE_UP,      "triangle-up",    QT_TRANSLATE_NOOP("engraving", "Triangle up") },
    { NoteHeadGroup::HEAD_TRIANGLE_DOWN,    "triangle-down",  QT_TRANSLATE_NOOP("engraving", "Triangle down") },
    { NoteHeadGroup::HEAD_SLASHED1,         "slashed1",       QT_TRANSLATE_NOOP("engraving", "Slashed (forwards)") },
    { NoteHeadGroup::HEAD_SLASHED2,         "slashed2",       QT_TRANSLATE_NOOP("engraving", "Slashed (backwards)") },
    { NoteHeadGroup::HEAD_DIAMOND,          "diamond",        QT_TRANSLATE_NOOP("engraving", "Diamond") },
    { NoteHeadGroup::HEAD_DIAMOND_OLD,      "diamond-old",    QT_TRANSLATE_NOOP("engraving", "Diamond (old)") },
    { NoteHeadGroup::HEAD_CIRCLED,          "circled",        QT_TRANSLATE_NOOP("engraving", "Circled") },
    { NoteHeadGroup::HEAD_CIRCLED_LARGE,    "circled-large",  QT_TRANSLATE_NOOP("engraving", "Circled large") },
    { NoteHeadGroup::HEAD_LARGE_ARROW,      "large-arrow",    QT_TRANSLATE_NOOP("engraving", "Large arrow") },
    { NoteHeadGroup::HEAD_BREVIS_ALT,       "altbrevis",      QT_TRANSLATE_NOOP("engraving", "Alt. brevis") },

    { NoteHeadGroup::HEAD_SLASH,            "slash",          QT_TRANSLATE_NOOP("engraving", "Slash") },

    { NoteHeadGroup::HEAD_HEAVY_CROSS,      "heavy-cross",    QT_TRANSLATE_NOOP("engraving", "Heavy Cross") },
    { NoteHeadGroup::HEAD_HEAVY_CROSS_HAT,  "heavy-cross-hat",    QT_TRANSLATE_NOOP("engraving", "Heavy Cross Hat") },

    // shape notes
    { NoteHeadGroup::HEAD_SOL,  "sol",       QT_TRANSLATE_NOOP("engraving", "Sol") },
    { NoteHeadGroup::HEAD_LA,   "la",        QT_TRANSLATE_NOOP("engraving", "La") },
    { NoteHeadGroup::HEAD_FA,   "fa",        QT_TRANSLATE_NOOP("engraving", "Fa") },
    { NoteHeadGroup::HEAD_MI,   "mi",        QT_TRANSLATE_NOOP("engraving", "Mi") },
    { NoteHeadGroup::HEAD_DO,   "do",        QT_TRANSLATE_NOOP("engraving", "Do") },
    { NoteHeadGroup::HEAD_RE,   "re",        QT_TRANSLATE_NOOP("engraving", "Re") },
    { NoteHeadGroup::HEAD_TI,   "ti",        QT_TRANSLATE_NOOP("engraving", "Ti") },

    // not exposed
    { NoteHeadGroup::HEAD_DO_WALKER,    "do-walker", QT_TRANSLATE_NOOP("engraving", "Do (Walker)") },
    { NoteHeadGroup::HEAD_RE_WALKER,    "re-walker", QT_TRANSLATE_NOOP("engraving", "Re (Walker)") },
    { NoteHeadGroup::HEAD_TI_WALKER,    "ti-walker", QT_TRANSLATE_NOOP("engraving", "Ti (Walker)") },
    { NoteHeadGroup::HEAD_DO_FUNK,      "do-funk",   QT_TRANSLATE_NOOP("engraving", "Do (Funk)") },
    { NoteHeadGroup::HEAD_RE_FUNK,      "re-funk",   QT_TRANSLATE_NOOP("engraving", "Re (Funk)") },
    { NoteHeadGroup::HEAD_TI_FUNK,      "ti-funk",   QT_TRANSLATE_NOOP("engraving", "Ti (Funk)") },

    // note name
    { NoteHeadGroup::HEAD_DO_NAME,      "do-name",  QT_TRANSLATE_NOOP("engraving",  "Do (Name)") },
    { NoteHeadGroup::HEAD_DI_NAME,      "di-name",  QT_TRANSLATE_NOOP("engraving",  "Di (Name)") },
    { NoteHeadGroup::HEAD_RA_NAME,      "ra-name",  QT_TRANSLATE_NOOP("engraving",  "Ra (Name)") },
    { NoteHeadGroup::HEAD_RE_NAME,      "re-name",  QT_TRANSLATE_NOOP("engraving",  "Re (Name)") },
    { NoteHeadGroup::HEAD_RI_NAME,      "ri-name",  QT_TRANSLATE_NOOP("engraving",  "Ri (Name)") },
    { NoteHeadGroup::HEAD_ME_NAME,      "me-name",  QT_TRANSLATE_NOOP("engraving",  "Me (Name)") },
    { NoteHeadGroup::HEAD_MI_NAME,      "mi-name",  QT_TRANSLATE_NOOP("engraving",  "Mi (Name)") },
    { NoteHeadGroup::HEAD_FA_NAME,      "fa-name",  QT_TRANSLATE_NOOP("engraving",  "Fa (Name)") },
    { NoteHeadGroup::HEAD_FI_NAME,      "fi-name",  QT_TRANSLATE_NOOP("engraving",  "Fi (Name)") },
    { NoteHeadGroup::HEAD_SE_NAME,      "se-name",  QT_TRANSLATE_NOOP("engraving",  "Se (Name)") },
    { NoteHeadGroup::HEAD_SOL_NAME,     "sol-name", QT_TRANSLATE_NOOP("engraving",  "Sol (Name)") },
    { NoteHeadGroup::HEAD_LE_NAME,      "le-name",  QT_TRANSLATE_NOOP("engraving",  "Le (Name)") },
    { NoteHeadGroup::HEAD_LA_NAME,      "la-name",  QT_TRANSLATE_NOOP("engraving",  "La (Name)") },
    { NoteHeadGroup::HEAD_LI_NAME,      "li-name",  QT_TRANSLATE_NOOP("engraving",  "Li (Name)") },
    { NoteHeadGroup::HEAD_TE_NAME,      "te-name",  QT_TRANSLATE_NOOP("engraving",  "Te (Name)") },
    { NoteHeadGroup::HEAD_TI_NAME,      "ti-name",  QT_TRANSLATE_NOOP("engraving",  "Ti (Name)") },
    { NoteHeadGroup::HEAD_SI_NAME,      "si-name",  QT_TRANSLATE_NOOP("engraving",  "Si (Name)") },

    { NoteHeadGroup::HEAD_A_SHARP,      "a-sharp-name", QT_TRANSLATE_NOOP("engraving",  "A♯ (Name)") },
    { NoteHeadGroup::HEAD_A,            "a-name",       QT_TRANSLATE_NOOP("engraving",  "A (Name)") },
    { NoteHeadGroup::HEAD_A_FLAT,       "a-flat-name",  QT_TRANSLATE_NOOP("engraving",  "A♭ (Name)") },
    { NoteHeadGroup::HEAD_B_SHARP,      "b-sharp-name", QT_TRANSLATE_NOOP("engraving",  "B♯ (Name)") },
    { NoteHeadGroup::HEAD_B,            "b-name",       QT_TRANSLATE_NOOP("engraving",  "B (Name)") },
    { NoteHeadGroup::HEAD_B_FLAT,       "b-flat-name",  QT_TRANSLATE_NOOP("engraving",  "B♭ (Name)") },
    { NoteHeadGroup::HEAD_C_SHARP,      "c-sharp-name", QT_TRANSLATE_NOOP("engraving",  "C♯ (Name)") },
    { NoteHeadGroup::HEAD_C,            "c-name",       QT_TRANSLATE_NOOP("engraving",  "C (Name)") },
    { NoteHeadGroup::HEAD_C_FLAT,       "c-flat-name",  QT_TRANSLATE_NOOP("engraving",  "C♭ (Name)") },
    { NoteHeadGroup::HEAD_D_SHARP,      "d-sharp-name", QT_TRANSLATE_NOOP("engraving",  "D♯ (Name)") },
    { NoteHeadGroup::HEAD_D,            "d-name",       QT_TRANSLATE_NOOP("engraving",  "D (Name)") },
    { NoteHeadGroup::HEAD_D_FLAT,       "d-flat-name",  QT_TRANSLATE_NOOP("engraving",  "D♭ (Name)") },
    { NoteHeadGroup::HEAD_E_SHARP,      "e-sharp-name", QT_TRANSLATE_NOOP("engraving",  "E♯ (Name)") },
    { NoteHeadGroup::HEAD_E,            "e-name",       QT_TRANSLATE_NOOP("engraving",  "E (Name)") },
    { NoteHeadGroup::HEAD_E_FLAT,       "e-flat-name",  QT_TRANSLATE_NOOP("engraving",  "E♭ (Name)") },
    { NoteHeadGroup::HEAD_F_SHARP,      "f-sharp-name", QT_TRANSLATE_NOOP("engraving",  "F♯ (Name)") },
    { NoteHeadGroup::HEAD_F,            "f-name",       QT_TRANSLATE_NOOP("engraving",  "F (Name)") },
    { NoteHeadGroup::HEAD_F_FLAT,       "f-flat-name",  QT_TRANSLATE_NOOP("engraving",  "F♭ (Name)") },
    { NoteHeadGroup::HEAD_G_SHARP,      "g-sharp-name", QT_TRANSLATE_NOOP("engraving",  "G♯ (Name)") },
    { NoteHeadGroup::HEAD_G,            "g-name",       QT_TRANSLATE_NOOP("engraving",  "G (Name)") },
    { NoteHeadGroup::HEAD_G_FLAT,       "g-flat-name",  QT_TRANSLATE_NOOP("engraving",  "G♭ (Name)") },
    { NoteHeadGroup::HEAD_H,            "h-name",       QT_TRANSLATE_NOOP("engraving",  "H (Name)") },
    { NoteHeadGroup::HEAD_H_SHARP,      "h-sharp-name", QT_TRANSLATE_NOOP("engraving",  "H♯ (Name)") },

    { NoteHeadGroup::HEAD_CUSTOM,       "custom",       QT_TRANSLATE_NOOP("engraving",  "Custom") }
};

String TConv::translatedUserName(NoteHeadGroup v)
{
    return mtrc("engraving", findUserNameByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v));
}

AsciiStringView TConv::toXml(NoteHeadGroup v)
{
    return findXmlTagByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v);
}

NoteHeadGroup TConv::fromXml(const AsciiStringView& tag, NoteHeadGroup def)
{
    return findTypeByXmlTag<NoteHeadGroup>(NOTEHEAD_GROUPS, tag, def);
}

static const std::vector<Item<ClefType> > CLEF_TYPES = {
    { ClefType::G,          "G",        QT_TRANSLATE_NOOP("engraving", "Treble clef") },
    { ClefType::G15_MB,     "G15mb",    QT_TRANSLATE_NOOP("engraving", "Treble clef 15ma bassa") },
    { ClefType::G8_VB,      "G8vb",     QT_TRANSLATE_NOOP("engraving", "Treble clef 8va bassa") },
    { ClefType::G8_VA,      "G8va",     QT_TRANSLATE_NOOP("engraving", "Treble clef 8va alta") },
    { ClefType::G15_MA,     "G15ma",    QT_TRANSLATE_NOOP("engraving", "Treble clef 15ma alta") },
    { ClefType::G8_VB_O,    "G8vbo",    QT_TRANSLATE_NOOP("engraving", "Double treble clef 8va bassa on 2nd line") },
    { ClefType::G8_VB_P,    "G8vbp",    QT_TRANSLATE_NOOP("engraving", "Treble clef optional 8va bassa") },
    { ClefType::G_1,        "G1",       QT_TRANSLATE_NOOP("engraving", "French violin clef") },
    { ClefType::C1,         "C1",       QT_TRANSLATE_NOOP("engraving", "Soprano clef") },
    { ClefType::C2,         "C2",       QT_TRANSLATE_NOOP("engraving", "Mezzo-soprano clef") },
    { ClefType::C3,         "C3",       QT_TRANSLATE_NOOP("engraving", "Alto clef") },
    { ClefType::C4,         "C4",       QT_TRANSLATE_NOOP("engraving", "Tenor clef") },
    { ClefType::C5,         "C5",       QT_TRANSLATE_NOOP("engraving", "Baritone clef (C clef)") },
    { ClefType::C_19C,      "C_19C",    QT_TRANSLATE_NOOP("engraving", "C clef, H shape (19th century)") },
    { ClefType::C1_F18C,    "C1_F18C",  QT_TRANSLATE_NOOP("engraving", "Soprano clef (French, 18th century)") },
    { ClefType::C3_F18C,    "C3_F18C",  QT_TRANSLATE_NOOP("engraving", "Alto clef (French, 18th century)") },
    { ClefType::C4_F18C,    "C4_F18C",  QT_TRANSLATE_NOOP("engraving", "Tenor clef (French, 18th century)") },
    { ClefType::C1_F20C,    "C1_F20C",  QT_TRANSLATE_NOOP("engraving", "Soprano clef (French, 20th century)") },
    { ClefType::C3_F20C,    "C3_F20C",  QT_TRANSLATE_NOOP("engraving", "Alto clef (French, 20th century)") },
    { ClefType::C4_F20C,    "C4_F20C",  QT_TRANSLATE_NOOP("engraving", "Tenor clef (French, 20th century)") },
    { ClefType::F,          "F",        QT_TRANSLATE_NOOP("engraving", "Bass clef") },
    { ClefType::F15_MB,     "F15mb",    QT_TRANSLATE_NOOP("engraving", "Bass clef 15ma bassa") },
    { ClefType::F8_VB,      "F8vb",     QT_TRANSLATE_NOOP("engraving", "Bass clef 8va bassa") },
    { ClefType::F_8VA,      "F8va",     QT_TRANSLATE_NOOP("engraving", "Bass clef 8va alta") },
    { ClefType::F_15MA,     "F15ma",    QT_TRANSLATE_NOOP("engraving", "Bass clef 15ma alta") },
    { ClefType::F_B,        "F3",       QT_TRANSLATE_NOOP("engraving", "Baritone clef (F clef)") },
    { ClefType::F_C,        "F5",       QT_TRANSLATE_NOOP("engraving", "Subbass clef") },
    { ClefType::F_F18C,     "F_F18C",   QT_TRANSLATE_NOOP("engraving", "F clef (French, 18th century)") },
    { ClefType::F_19C,      "F_19C",    QT_TRANSLATE_NOOP("engraving", "F clef (19th century)") },

    { ClefType::PERC,       "PERC",     QT_TRANSLATE_NOOP("engraving", "Percussion") },
    { ClefType::PERC2,      "PERC2",    QT_TRANSLATE_NOOP("engraving", "Percussion 2") },

    { ClefType::TAB,        "TAB",      QT_TRANSLATE_NOOP("engraving", "Tablature") },
    { ClefType::TAB4,       "TAB4",     QT_TRANSLATE_NOOP("engraving", "Tablature 4 lines") },
    { ClefType::TAB_SERIF,  "TAB2",     QT_TRANSLATE_NOOP("engraving", "Tablature Serif") },
    { ClefType::TAB4_SERIF, "TAB4_SERIF", QT_TRANSLATE_NOOP("engraving", "Tablature Serif 4 lines") },
};

String TConv::translatedUserName(ClefType v)
{
    return mtrc("engraving", findUserNameByType<ClefType>(CLEF_TYPES, v));
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
    return mtrc("engraving", findUserNameByType<DynamicRange>(DYNAMIC_RANGES, v));
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
    return mtrc("engraving", findUserNameByType<DynamicSpeed>(DYNAMIC_SPEEDS, v));
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
    return mtrc("engraving", findUserNameByType<HookType>(HOOK_TYPES, v));
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
    return mtrc("engraving", findUserNameByType<KeyMode>(KEY_MODES, v));
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
    { TextStyleType::DEFAULT,           "default",              QT_TRANSLATE_NOOP("engraving", "Default") },
    { TextStyleType::TITLE,             "title",                QT_TRANSLATE_NOOP("engraving", "Title") },
    { TextStyleType::SUBTITLE,          "subtitle",             QT_TRANSLATE_NOOP("engraving", "Subtitle") },
    { TextStyleType::COMPOSER,          "composer",             QT_TRANSLATE_NOOP("engraving", "Composer") },
    { TextStyleType::POET,              "poet",                 QT_TRANSLATE_NOOP("engraving", "Poet") },
    { TextStyleType::TRANSLATOR,        "translator",           QT_TRANSLATE_NOOP("engraving", "Translator") },
    { TextStyleType::FRAME,             "frame",                QT_TRANSLATE_NOOP("engraving", "Frame") },
    { TextStyleType::INSTRUMENT_EXCERPT, "instrument_excerpt",  QT_TRANSLATE_NOOP("engraving", "Instrument Name (Part)") },
    { TextStyleType::INSTRUMENT_LONG,   "instrument_long",      QT_TRANSLATE_NOOP("engraving", "Instrument Name (Long)") },
    { TextStyleType::INSTRUMENT_SHORT,  "instrument_short",     QT_TRANSLATE_NOOP("engraving", "Instrument Name (Short)") },
    { TextStyleType::INSTRUMENT_CHANGE, "instrument_change",    QT_TRANSLATE_NOOP("engraving", "Instrument Change") },
    { TextStyleType::HEADER,            "header",               QT_TRANSLATE_NOOP("engraving", "Header") },
    { TextStyleType::FOOTER,            "footer",               QT_TRANSLATE_NOOP("engraving", "Footer") },

    { TextStyleType::MEASURE_NUMBER,    "measure_number",       QT_TRANSLATE_NOOP("engraving", "Measure Number") },
    { TextStyleType::MMREST_RANGE,      "mmrest_range",         QT_TRANSLATE_NOOP("engraving", "Multimeasure Rest Range") },

    { TextStyleType::TEMPO,             "tempo",                QT_TRANSLATE_NOOP("engraving", "Tempo") },
    { TextStyleType::METRONOME,         "metronome",            QT_TRANSLATE_NOOP("engraving", "Metronome") },
    { TextStyleType::REPEAT_LEFT,       "repeat_left",          QT_TRANSLATE_NOOP("engraving", "Repeat Text Left") },
    { TextStyleType::REPEAT_RIGHT,      "repeat_right",         QT_TRANSLATE_NOOP("engraving", "Repeat Text Right") },
    { TextStyleType::REHEARSAL_MARK,    "rehearsal_mark",       QT_TRANSLATE_NOOP("engraving", "Rehearsal Mark") },
    { TextStyleType::SYSTEM,            "system",               QT_TRANSLATE_NOOP("engraving", "System") },

    { TextStyleType::STAFF,             "staff",                QT_TRANSLATE_NOOP("engraving", "Staff") },
    { TextStyleType::EXPRESSION,        "expression",           QT_TRANSLATE_NOOP("engraving", "Expression") },
    { TextStyleType::DYNAMICS,          "dynamics",             QT_TRANSLATE_NOOP("engraving", "Dynamics") },
    { TextStyleType::HAIRPIN,           "hairpin",              QT_TRANSLATE_NOOP("engraving", "Hairpin") },
    { TextStyleType::LYRICS_ODD,        "lyrics_odd",           QT_TRANSLATE_NOOP("engraving", "Lyrics Odd Lines") },
    { TextStyleType::LYRICS_EVEN,       "lyrics_even",          QT_TRANSLATE_NOOP("engraving", "Lyrics Even Lines") },
    { TextStyleType::HARMONY_A,         "harmony_a",            QT_TRANSLATE_NOOP("engraving", "Chord Symbol") },
    { TextStyleType::HARMONY_B,         "harmony_b",            QT_TRANSLATE_NOOP("engraving", "Chord Symbol (Alternate)") },
    { TextStyleType::HARMONY_ROMAN,     "harmony_roman",        QT_TRANSLATE_NOOP("engraving", "Roman Numeral Analysis") },
    { TextStyleType::HARMONY_NASHVILLE, "harmony_nashville",    QT_TRANSLATE_NOOP("engraving", "Nashville Number") },

    { TextStyleType::TUPLET,            "tuplet",               QT_TRANSLATE_NOOP("engraving", "Tuplet") },
    { TextStyleType::STICKING,          "sticking",             QT_TRANSLATE_NOOP("engraving", "Sticking") },
    { TextStyleType::FINGERING,         "fingering",            QT_TRANSLATE_NOOP("engraving", "Fingering") },
    { TextStyleType::LH_GUITAR_FINGERING, "guitar_fingering_lh", QT_TRANSLATE_NOOP("engraving", "LH Guitar Fingering") },
    { TextStyleType::RH_GUITAR_FINGERING, "guitar_fingering_rh", QT_TRANSLATE_NOOP("engraving", "RH Guitar Fingering") },
    { TextStyleType::STRING_NUMBER,     "string_number",        QT_TRANSLATE_NOOP("engraving", "String Number") },

    { TextStyleType::TEXTLINE,          "textline",             QT_TRANSLATE_NOOP("engraving", "Text Line") },
    { TextStyleType::VOLTA,             "volta",                QT_TRANSLATE_NOOP("engraving", "Volta") },
    { TextStyleType::OTTAVA,            "ottava",               QT_TRANSLATE_NOOP("engraving", "Ottava") },
    { TextStyleType::GLISSANDO,         "glissando",            QT_TRANSLATE_NOOP("engraving", "Glissando") },
    { TextStyleType::PEDAL,             "pedal",                QT_TRANSLATE_NOOP("engraving", "Pedal") },
    { TextStyleType::BEND,              "bend",                 QT_TRANSLATE_NOOP("engraving", "Bend") },
    { TextStyleType::LET_RING,          "let_ring",             QT_TRANSLATE_NOOP("engraving", "Let Ring") },
    { TextStyleType::PALM_MUTE,         "palm_mute",            QT_TRANSLATE_NOOP("engraving", "Palm Mute") },

    { TextStyleType::USER1,             "user_1",               QT_TRANSLATE_NOOP("engraving", "User-1") },
    { TextStyleType::USER2,             "user_2",               QT_TRANSLATE_NOOP("engraving", "User-2") },
    { TextStyleType::USER3,             "user_3",               QT_TRANSLATE_NOOP("engraving", "User-3") },
    { TextStyleType::USER4,             "user_4",               QT_TRANSLATE_NOOP("engraving", "User-4") },
    { TextStyleType::USER5,             "user_5",               QT_TRANSLATE_NOOP("engraving", "User-5") },
    { TextStyleType::USER6,             "user_6",               QT_TRANSLATE_NOOP("engraving", "User-6") },
    { TextStyleType::USER7,             "user_7",               QT_TRANSLATE_NOOP("engraving", "User-7") },
    { TextStyleType::USER8,             "user_8",               QT_TRANSLATE_NOOP("engraving", "User-8") },
    { TextStyleType::USER9,             "user_9",               QT_TRANSLATE_NOOP("engraving", "User-9") },
    { TextStyleType::USER10,            "user_10",              QT_TRANSLATE_NOOP("engraving", "User-10") },
    { TextStyleType::USER11,            "user_11",              QT_TRANSLATE_NOOP("engraving", "User-11") },
    { TextStyleType::USER12,            "user_12",              QT_TRANSLATE_NOOP("engraving", "User-12") },
};

String TConv::translatedUserName(TextStyleType v)
{
    return mtrc("engraving", findUserNameByType<TextStyleType>(TEXTSTYLE_TYPES, v));
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
    };

    auto old = OLD_TST_TAGS.find(tag);
    if (old != OLD_TST_TAGS.cend()) {
        return old->second;
    }

    if (tag == "Technique") {
        return TextStyleType::EXPRESSION;
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

String TConv::translatedUserName(ChangeMethod v)
{
    return mtrc("engraving", findUserNameByType<ChangeMethod>(CHANGE_METHODS, v));
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
    { DurationType::V_QUARTER,  "quarter",  QT_TRANSLATE_NOOP("engraving", "Quarter") },
    { DurationType::V_EIGHTH,   "eighth",   QT_TRANSLATE_NOOP("engraving", "Eighth") },
    { DurationType::V_1024TH,   "1024th",   QT_TRANSLATE_NOOP("engraving", "1024th") },
    { DurationType::V_512TH,    "512th",    QT_TRANSLATE_NOOP("engraving", "512th") },
    { DurationType::V_256TH,    "256th",    QT_TRANSLATE_NOOP("engraving", "256th") },
    { DurationType::V_128TH,    "128th",    QT_TRANSLATE_NOOP("engraving", "128th") },
    { DurationType::V_64TH,     "64th",     QT_TRANSLATE_NOOP("engraving", "64th") },
    { DurationType::V_32ND,     "32nd",     QT_TRANSLATE_NOOP("engraving", "32nd") },
    { DurationType::V_16TH,     "16th",     QT_TRANSLATE_NOOP("engraving", "16th") },
    { DurationType::V_HALF,     "half",     QT_TRANSLATE_NOOP("engraving", "Half") },
    { DurationType::V_WHOLE,    "whole",    QT_TRANSLATE_NOOP("engraving", "Whole") },
    { DurationType::V_MEASURE,  "measure",  QT_TRANSLATE_NOOP("engraving", "Measure") },
    { DurationType::V_BREVE,    "breve",    QT_TRANSLATE_NOOP("engraving", "Breve") },
    { DurationType::V_LONG,     "long",     QT_TRANSLATE_NOOP("engraving", "Longa") },
    { DurationType::V_ZERO,     "",         QT_TRANSLATE_NOOP("engraving", "Zero") },
    { DurationType::V_INVALID,  "",         QT_TRANSLATE_NOOP("engraving", "Invalid") },
};

String TConv::translatedUserName(DurationType v)
{
    return mtrc("engraving", findUserNameByType<DurationType>(DURATION_TYPES, v));
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
    { PlayingTechniqueType::Overdrive,           "overdrive" }
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

static const std::vector<Item<DirectionV> > DIRECTIONV_TYPES = {
    { DirectionV::AUTO, "auto" },
    { DirectionV::UP, "up" },
    { DirectionV::DOWN, "down" }
};

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
    { DirectionH::AUTO, "auto" },
    { DirectionH::RIGHT, "right" },
    { DirectionH::LEFT, "left" }
};

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
    { TremoloType::INVALID_TREMOLO, "", "" },
    { TremoloType::R8,              "r8",       QT_TRANSLATE_NOOP("engraving", "Eighth through stem") },
    { TremoloType::R16,             "r16",      QT_TRANSLATE_NOOP("engraving", "16th through stem") },
    { TremoloType::R32,             "r32",      QT_TRANSLATE_NOOP("engraving", "32nd through stem") },
    { TremoloType::R64,             "r64",      QT_TRANSLATE_NOOP("engraving", "64th through stem") },
    { TremoloType::BUZZ_ROLL,       "buzzroll", QT_TRANSLATE_NOOP("engraving", "Buzz roll") },
    { TremoloType::C8,              "c8",       QT_TRANSLATE_NOOP("engraving", "Eighth between notes") },
    { TremoloType::C16,             "c16",      QT_TRANSLATE_NOOP("engraving", "16th between notes") },
    { TremoloType::C32,             "c32",      QT_TRANSLATE_NOOP("engraving", "32nd between notes") },
    { TremoloType::C64,             "c64",      QT_TRANSLATE_NOOP("engraving", "64th between notes") }
} };

const char* TConv::userName(TremoloType v)
{
    return findUserNameByType<TremoloType>(TREMOLO_TYPES, v);
}

String TConv::translatedUserName(TremoloType v)
{
    return mtrc("engraving", userName(v));
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
    { ArpeggioType::NORMAL,         "0",     QT_TRANSLATE_NOOP("engraving", "Arpeggio") },
    { ArpeggioType::UP,             "1",     QT_TRANSLATE_NOOP("engraving", "Up arpeggio") },
    { ArpeggioType::DOWN,           "2",     QT_TRANSLATE_NOOP("engraving", "Down arpeggio") },
    { ArpeggioType::BRACKET,        "3",     QT_TRANSLATE_NOOP("engraving", "Bracket arpeggio") },
    { ArpeggioType::UP_STRAIGHT,    "4",     QT_TRANSLATE_NOOP("engraving", "Up arpeggio straight") },
    { ArpeggioType::DOWN_STRAIGHT,  "5",     QT_TRANSLATE_NOOP("engraving", "Down arpeggio straight") }
} };

String TConv::translatedUserName(ArpeggioType v)
{
    return mtrc("engraving", findUserNameByType<ArpeggioType>(ARPEGGIO_TYPES, v));
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
    const char* name = nullptr;
    AsciiStringView notes;
};

static const std::vector<EmbelItem> EMBELLISHMENT_TYPES = {
    // Single Grace notes
    { QT_TRANSLATE_NOOP("engraving", "Single grace low G"), "LG" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace low A"), "LA" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace B"), "B" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace C"), "C" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace D"), "D" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace E"), "E" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace F"), "F" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace high G"), "HG" },
    { QT_TRANSLATE_NOOP("engraving", "Single grace high A"), "HA" },

    // Double Grace notes
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "D LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "D B" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "D C" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "E LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "E B" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "E C" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "E D" },

    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F B" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F C" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F D" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "F E" },

    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG B" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG C" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG D" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG E" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HG F" },

    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA B" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA C" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA D" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA E" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA F" },
    { QT_TRANSLATE_NOOP("engraving", "Double grace"), "HA HG" },

    // Half Doublings
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on low G"), "LG D" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on low A"), "LA D" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on B"), "B D" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on C"), "C D" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on D"), "D E" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on E"), "E F" },
    { QT_TRANSLATE_NOOP("engraving", "Half doubling on F"), "F HG" },
    // ? { QT_TRANSLATE_NOOP("engraving", "Half doubling on high G"), "HG F" },
    // ? { QT_TRANSLATE_NOOP("engraving", "Half doubling on high A"), "HA HG" },

    // Regular Doublings
    { QT_TRANSLATE_NOOP("engraving", "Doubling on high G"), "HG F" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on high A"), "HA HG" },

    // Half Strikes
    { QT_TRANSLATE_NOOP("engraving", "Half strike on low A"), "LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on B"), "B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on C"), "C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on D"), "D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on D"), "D C" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on E"), "E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on F"), "F E" },
    { QT_TRANSLATE_NOOP("engraving", "Half strike on high G"), "HG F" },

    // Regular Grip
    { QT_TRANSLATE_NOOP("engraving", "Grip"), "D LG" },

    // D Throw
    { QT_TRANSLATE_NOOP("engraving", "Half D throw"), "D C" },

    // Regular Doublings (continued)
    { QT_TRANSLATE_NOOP("engraving", "Doubling on low G"),  "HG LG D" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on low A"),  "HG LA D" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on B"),      "HG B D" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on C"),      "HG C D" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on D"),      "HG D E" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on E"),      "HG E F" },
    { QT_TRANSLATE_NOOP("engraving", "Doubling on F"),      "HG F HG" },

    // Thumb Doublings
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on low G"), "HA LG D" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on low A"), "HA LA D" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on B"), "HA B D" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on C"), "HA C D" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on D"), "HA D E" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on E"), "HA E F" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on F"), "HA F HG" },
    // ? { QT_TRANSLATE_NOOP("engraving", "Thumb doubling on high G"), "HA HG F" },

    // G Grace note Strikes
    { QT_TRANSLATE_NOOP("engraving", "G grace note on low A"), "HG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on B"), "HG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on C"), "HG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on D"), "HG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on D"), "HG D C" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on E"), "HG E LA" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note on F"), "HG F E" },

    // Regular Double Strikes
    { QT_TRANSLATE_NOOP("engraving", "Double strike on low A"), "LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on B"), "LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on C"), "LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on D"), "LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on D"), "C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on E"), "LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on F"), "E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on high G"), "F HG F" },
    { QT_TRANSLATE_NOOP("engraving", "Double strike on high A"), "HG HA HG" },

    // Thumb Strikes
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on low A"), "HA LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on B"), "HA B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on C"), "HA C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on D"), "HA D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on D"), "HA D C" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on E"), "HA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on F"), "HA F E" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb strike on high G"), "HA HG F" },

    // Regular Grips (continued)
    { QT_TRANSLATE_NOOP("engraving", "Grip"), "LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Grip"), "LG B LG" },

    // Taorluath and Birl
    { QT_TRANSLATE_NOOP("engraving", "Birl"), "LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "D throw"), "LG D C" },
    { QT_TRANSLATE_NOOP("engraving", "Half heavy D throw"), "D LG C" },
    { QT_TRANSLATE_NOOP("engraving", "Taorluath"), "D LG E" },

    // Birl, Bubbly, D Throws (continued) and Taorluaths (continued)
    { QT_TRANSLATE_NOOP("engraving", "Birl"), "LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Bubbly"), "D LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Heavy D throw"), "LG D LG C" },
    { QT_TRANSLATE_NOOP("engraving", "Taorluath"), "LG D LG E" },
    { QT_TRANSLATE_NOOP("engraving", "Taorluath"), "LG B LG E" },

    // Half Double Strikes
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on low A"), "LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on B"), "B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on C"), "C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on D"), "D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on D"), "D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on E"), "E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on F"), "F E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on high G"), "HG F HG F" },
    { QT_TRANSLATE_NOOP("engraving", "Half double strike on high A"), "HA HG HA HG" },

    // Half Grips
    { QT_TRANSLATE_NOOP("engraving", "Half grip on low A"), "LA LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on B"), "B LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on C"), "C LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on D"), "D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on D"), "D LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on E"), "E LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on F"), "F LG F LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on high G"), "HG LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half grip on high A"), "HA LG D LG" },

    // Half Peles
    { QT_TRANSLATE_NOOP("engraving", "Half pele on low A"), "LA E LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on B"), "B E B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on C"), "C E C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on D"), "D E D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on D"), "D E D C" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on E"), "E F E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on F"), "F HG F E" },
    { QT_TRANSLATE_NOOP("engraving", "Half pele on high G"), "HG HA HG F" },

    // G Grace note Grips
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on low A"), "HG LA LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on B"), "HG B LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on C"), "HG C LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on D"), "HG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on D"), "HG D LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on E"), "HG E LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note grip on F"), "HG F LG F LG" },

    // Thumb Grips
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on low A"), "HA LA LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on B"), "HA B LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on C"), "HA C LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on D"), "HA D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on D"), "HA D LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on E"), "HA E LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on F"), "HA F LG F LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grip on high G"), "HA HG LG F LG" },

    // Bubbly
    { QT_TRANSLATE_NOOP("engraving", "Bubbly"), "LG D LG C LG" },

    //  Birls
    { QT_TRANSLATE_NOOP("engraving", "Birl"), "HG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Birl"), "HA LA LG LA LG" },

    // Regular Peles
    { QT_TRANSLATE_NOOP("engraving", "Pele on low A"), "HG LA E LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on B"), "HG B E B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on C"), "HG C E C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on D"), "HG D E D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on D"), "HG D E D C" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on E"), "HG E F E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Pele on F"), "HG F HG F E" },

    // Thumb Grace Note Peles
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on low A"), "HA LA E LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on B"), "HA B E B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on C"), "HA C E C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on D"), "HA D E D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on D"), "HA D E D C" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on E"), "HA E F E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on F"), "HA F HG F E" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb grace note pele on high G"), "HA HG HA HG F" },

    // G Grace note Double Strikes
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on low A"), "HG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on B"), "HG B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on C"), "HG C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on D"), "HG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on D"), "HG D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on E"), "HG E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note double strike on F"), "HG F E F E" },

    // Thumb Double Strikes
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on low A"), "HA LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on B"), "HA B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on C"), "HA C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on D"), "HA D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on D"), "HA D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on E"), "HA E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on F"), "HA F E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb double strike on high G"), "HA HG F HG F" },

    // Regular Triple Strikes
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on low A"), "LG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on B"), "LG B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on C"), "LG C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on D"), "LG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on D"), "C D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on E"), "LA E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on F"), "E F E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on high G"), "F HG F HG F" },
    { QT_TRANSLATE_NOOP("engraving", "Triple strike on high A"), "HG HA HG HA HG" },

    // Half Triple Strikes
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on low A"), "LA LG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on B"), "B LG B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on C"), "C LG C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on D"), "D LG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on D"), "D C D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on E"), "E LA E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on F"), "F E F E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on high G"), "HG F HG F HG F" },
    { QT_TRANSLATE_NOOP("engraving", "Half triple strike on high A"), "HA HG HA HG HA HG" },

    // G Grace note Triple Strikes
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on low A"), "HG LA LG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on B"), "HG B LG B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on C"), "HG C LG C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on D"), "HG D LG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on D"), "HG D C D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on E"), "HG E LA E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "G grace note triple strike on F"), "HG F E F E F E" },

    // Thumb Triple Strikes
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on low A"),  "HA LA LG LA LG LA LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on B"),      "HA B LG B LG B LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on C"),      "HA C LG C LG C LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on D"),      "HA D LG D LG D LG" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on D"),      "HA D C D C D C" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on E"),      "HA E LA E LA E LA" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on F"),      "HA F E F E F E" },
    { QT_TRANSLATE_NOOP("engraving", "Thumb triple strike on high G"), "HA HG F HG F HG F" },
};

const char* TConv::userName(EmbellishmentType v)
{
    return EMBELLISHMENT_TYPES.at(static_cast<size_t>(v)).name;
}

String TConv::translatedUserName(EmbellishmentType v)
{
    return mtrc("engraving", userName(v));
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
static const std::array<Item<ChordLineType>, 6> CHORDLINE_TYPES = { {
    { ChordLineType::NOTYPE,    "0",     "" },
    { ChordLineType::FALL,      "1",     QT_TRANSLATE_NOOP("engraving", "Fall") },
    { ChordLineType::DOIT,      "2",     QT_TRANSLATE_NOOP("engraving", "Doit") },
    { ChordLineType::PLOP,      "3",     QT_TRANSLATE_NOOP("engraving", "Plop") },
    { ChordLineType::SCOOP,     "4",     QT_TRANSLATE_NOOP("engraving", "Scoop") }
} };

const char* TConv::userName(ChordLineType v)
{
    return findUserNameByType<ChordLineType>(CHORDLINE_TYPES, v);
}

String TConv::translatedUserName(ChordLineType v)
{
    return mtrc("engraving", userName(v));
}

AsciiStringView TConv::toXml(ChordLineType v)
{
    return findXmlTagByType<ChordLineType>(CHORDLINE_TYPES, v);
}

ChordLineType TConv::fromXml(const AsciiStringView& tag, ChordLineType def)
{
    return findTypeByXmlTag<ChordLineType>(CHORDLINE_TYPES, tag, def);
}

struct DrumPitchItem {
    DrumPitch pitch;
    const char* userName = nullptr;
};

static const std::vector<DrumPitchItem> DRUMPITCHS = {
    { DrumPitch(35),       QT_TRANSLATE_NOOP("engraving", "Acoustic Bass Drum") },
    { DrumPitch(36),       QT_TRANSLATE_NOOP("engraving", "Bass Drum 1") },
    { DrumPitch(37),       QT_TRANSLATE_NOOP("engraving", "Side Stick") },
    { DrumPitch(38),       QT_TRANSLATE_NOOP("engraving", "Acoustic Snare") },

    { DrumPitch(40),       QT_TRANSLATE_NOOP("engraving", "Electric Snare") },
    { DrumPitch(41),       QT_TRANSLATE_NOOP("engraving", "Low Floor Tom") },
    { DrumPitch(42),       QT_TRANSLATE_NOOP("engraving", "Closed Hi-Hat") },
    { DrumPitch(43),       QT_TRANSLATE_NOOP("engraving", "High Floor Tom") },
    { DrumPitch(44),       QT_TRANSLATE_NOOP("engraving", "Pedal Hi-Hat") },
    { DrumPitch(45),       QT_TRANSLATE_NOOP("engraving", "Low Tom") },
    { DrumPitch(46),       QT_TRANSLATE_NOOP("engraving", "Open Hi-Hat") },
    { DrumPitch(47),       QT_TRANSLATE_NOOP("engraving", "Low-Mid Tom") },
    { DrumPitch(48),       QT_TRANSLATE_NOOP("engraving", "Hi-Mid Tom") },
    { DrumPitch(49),       QT_TRANSLATE_NOOP("engraving", "Crash Cymbal 1") },

    { DrumPitch(50),       QT_TRANSLATE_NOOP("engraving", "High Tom") },
    { DrumPitch(51),       QT_TRANSLATE_NOOP("engraving", "Ride Cymbal 1") },
    { DrumPitch(52),       QT_TRANSLATE_NOOP("engraving", "Chinese Cymbal") },
    { DrumPitch(53),       QT_TRANSLATE_NOOP("engraving", "Ride Bell") },
    { DrumPitch(54),       QT_TRANSLATE_NOOP("engraving", "Tambourine") },
    { DrumPitch(55),       QT_TRANSLATE_NOOP("engraving", "Splash Cymbal") },
    { DrumPitch(56),       QT_TRANSLATE_NOOP("engraving", "Cowbell") },
    { DrumPitch(57),       QT_TRANSLATE_NOOP("engraving", "Crash Cymbal 2") },

    { DrumPitch(59),       QT_TRANSLATE_NOOP("engraving", "Ride Cymbal 2") },

    { DrumPitch(63),       QT_TRANSLATE_NOOP("engraving", "Open Hi Conga") },
    { DrumPitch(64),       QT_TRANSLATE_NOOP("engraving", "Low Conga") },
};

const char* TConv::userName(DrumPitch v)
{
    auto it = std::find_if(DRUMPITCHS.cbegin(), DRUMPITCHS.cend(), [v](const DrumPitchItem& i) {
        return i.pitch == v;
    });

    IF_ASSERT_FAILED(it != DRUMPITCHS.cend()) {
        static const char* dummy = "";
        return dummy;
    }
    return it->userName;
}

//! TODO Add xml names
static const std::array<Item<GlissandoType>, 2> GLISSANDO_TYPES = { {
    { GlissandoType::STRAIGHT,  "0",     QT_TRANSLATE_NOOP("engraving", "Straight glissando") },
    { GlissandoType::WAVY,      "1",     QT_TRANSLATE_NOOP("engraving", "Wavy glissando") }
} };

const char* TConv::userName(GlissandoType v)
{
    return findUserNameByType<GlissandoType>(GLISSANDO_TYPES, v);
}

String TConv::translatedUserName(GlissandoType v)
{
    return mtrc("engraving", userName(v));
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
    { JumpType::DC,             "", QT_TRANSLATE_NOOP("engraving", "Da Capo") },
    { JumpType::DC_AL_FINE,     "", QT_TRANSLATE_NOOP("engraving", "Da Capo al Fine") },
    { JumpType::DC_AL_CODA,     "", QT_TRANSLATE_NOOP("engraving", "Da Capo al Coda") },
    { JumpType::DS_AL_CODA,     "", QT_TRANSLATE_NOOP("engraving", "D.S. al Coda") },
    { JumpType::DS_AL_FINE,     "", QT_TRANSLATE_NOOP("engraving", "D.S. al Fine") },
    { JumpType::DS,             "", QT_TRANSLATE_NOOP("engraving", "D.S.") },

    { JumpType::DC_AL_DBLCODA,  "", QT_TRANSLATE_NOOP("engraving", "Da Capo al Double Coda") },
    { JumpType::DS_AL_DBLCODA,  "", QT_TRANSLATE_NOOP("engraving", "Da Segno al Double Coda") },
    { JumpType::DSS,            "", QT_TRANSLATE_NOOP("engraving", "Dal Segno Segno") },
    { JumpType::DSS_AL_CODA,    "", QT_TRANSLATE_NOOP("engraving", "Dal Segno Segno al Coda") },
    { JumpType::DSS_AL_DBLCODA, "", QT_TRANSLATE_NOOP("engraving", "Dal Segno Segno al Double Coda") },
    { JumpType::DSS_AL_FINE,    "", QT_TRANSLATE_NOOP("engraving", "Dal Segno Segno al Fine") },
    { JumpType::DCODA,          "", QT_TRANSLATE_NOOP("engraving", "Da Coda") },
    { JumpType::DDBLCODA,       "", QT_TRANSLATE_NOOP("engraving", "Da Double Coda") },

    { JumpType::USER,           "", QT_TRANSLATE_NOOP("engraving", "Custom") }
};

const char* TConv::userName(JumpType v)
{
    return findUserNameByType<JumpType>(JUMP_TYPES, v);
}

String TConv::translatedUserName(JumpType v)
{
    return mtrc("engraving", userName(v));
}

static const std::array<Item<MarkerType>, 9> MARKET_TYPES = { {
    { MarkerType::SEGNO,        "segno",    QT_TRANSLATE_NOOP("engraving", "Segno") },
    { MarkerType::VARSEGNO,     "varsegno", QT_TRANSLATE_NOOP("engraving", "Segno variation") },
    { MarkerType::CODA,         "codab",    QT_TRANSLATE_NOOP("engraving", "Coda") },
    { MarkerType::VARCODA,      "varcoda",  QT_TRANSLATE_NOOP("engraving", "Varied coda") },
    { MarkerType::CODETTA,      "codetta",  QT_TRANSLATE_NOOP("engraving", "Codetta") },
    { MarkerType::FINE,         "fine",     QT_TRANSLATE_NOOP("engraving", "Fine") },
    { MarkerType::TOCODA,       "coda",     QT_TRANSLATE_NOOP("engraving", "To coda") },
    { MarkerType::TOCODASYM,    "",         QT_TRANSLATE_NOOP("engraving", "To coda (symbol)") },
    { MarkerType::USER,         "",         QT_TRANSLATE_NOOP("engraving", "Custom") }
} };

const char* TConv::userName(MarkerType v)
{
    return findUserNameByType<MarkerType>(MARKET_TYPES, v);
}

String TConv::translatedUserName(MarkerType v)
{
    return mtrc("engraving", userName(v));
}

AsciiStringView TConv::toXml(MarkerType v)
{
    return findXmlTagByType<MarkerType>(MARKET_TYPES, v);
}

MarkerType TConv::fromXml(const AsciiStringView& tag, MarkerType def)
{
    return findTypeByXmlTag<MarkerType>(MARKET_TYPES, tag, def);
}

static const std::array<Item<StaffGroup>, 3> STAFFGROUP_TYPES = { {
    { StaffGroup::STANDARD,     "pitched",    QT_TRANSLATE_NOOP("engraving", "Standard") },
    { StaffGroup::PERCUSSION,   "percussion", QT_TRANSLATE_NOOP("engraving", "Percussion") },
    { StaffGroup::TAB,          "tablature",  QT_TRANSLATE_NOOP("engraving", "Tablature") },
} };

const char* TConv::userName(StaffGroup v)
{
    return findUserNameByType<StaffGroup>(STAFFGROUP_TYPES, v);
}

String TConv::translatedUserName(StaffGroup v)
{
    return mtrc("engraving", userName(v));
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
    { TrillType::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("engraving", "Trill line") },
    { TrillType::UPPRALL_LINE,    "upprall",    QT_TRANSLATE_NOOP("engraving", "Upprall line") },
    { TrillType::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("engraving", "Downprall line") },
    { TrillType::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("engraving", "Prallprall line") }
} };

const char* TConv::userName(TrillType v)
{
    return findUserNameByType<TrillType>(TRILL_TYPES, v);
}

String TConv::translatedUserName(TrillType v)
{
    return mtrc("engraving", userName(v));
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
    { VibratoType::GUITAR_VIBRATO,        "guitarVibrato",       QT_TRANSLATE_NOOP("engraving", "Guitar vibrato") },
    { VibratoType::GUITAR_VIBRATO_WIDE,   "guitarVibratoWide",   QT_TRANSLATE_NOOP("engraving", "Guitar vibrato wide") },
    { VibratoType::VIBRATO_SAWTOOTH,      "vibratoSawtooth",     QT_TRANSLATE_NOOP("engraving", "Vibrato sawtooth") },
    { VibratoType::VIBRATO_SAWTOOTH_WIDE, "vibratoSawtoothWide", QT_TRANSLATE_NOOP("engraving", "Tremolo sawtooth wide") }
} };

const char* TConv::userName(VibratoType v)
{
    return findUserNameByType<VibratoType>(VIBRATO_TYPES, v);
}

String TConv::translatedUserName(VibratoType v)
{
    return mtrc("engraving", userName(v));
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
