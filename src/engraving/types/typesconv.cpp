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
#include "typesconv.h"

#include "global/types/translatablestring.h"

#include "draw/types/drawtypes.h"

#include "symnames.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

template<typename T>
struct Item
{
    T type;
    AsciiStringView xml;
    muse::TranslatableString userName;

    // NOTE Ideally we would write `TranslatableString userName = {};` and omit these constructors
    // But that causes internal compiler errors with certain versions of GCC/MinGW
    // See discussion at https://github.com/musescore/MuseScore/pull/12612

    Item() = default;
    Item(T type, AsciiStringView xml, const muse::TranslatableString& userName = {})
        : type(type), xml(xml), userName(userName) {}
};

template<typename T, typename C>
static const muse::TranslatableString& findUserNameByType(const C& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static muse::TranslatableString dummy;
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
    muse::ByteArray ba = tag.toAscii();
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
    StringList sl = tag.split(u',', muse::SkipEmptyParts);
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

String TConv::toXml(const std::vector<string_idx_t>& v)
{
    std::vector<int> _v;
    for (string_idx_t string : v) {
        _v.push_back(static_cast<int>(string));
    }

    return toXml(_v);
}

std::vector<string_idx_t> TConv::fromXml(const String& tag, const std::vector<string_idx_t>& def)
{
    std::vector<int> _def;
    for (string_idx_t string : def) {
        _def.push_back(static_cast<int>(string));
    }

    std::vector<string_idx_t> v;
    std::vector<int> _v = fromXml(tag, _def);

    for (int string : _v) {
        v.push_back(static_cast<string_idx_t>(string));
    }

    return v;
}

static const std::vector<Item<ElementType> > ELEMENT_TYPES = {
    { ElementType::INVALID,              "invalid",              muse::TranslatableString("engraving", "Invalid") },
    { ElementType::BRACKET_ITEM,         "BracketItem",          muse::TranslatableString("engraving", "Bracket") },
    { ElementType::PART,                 "Part",                 muse::TranslatableString("engraving", "Part") },
    { ElementType::STAFF,                "Staff",                muse::TranslatableString("engraving", "Staff") },
    { ElementType::SCORE,                "Score",                muse::TranslatableString("engraving", "Score") },
    { ElementType::SYMBOL,               "Symbol",               muse::TranslatableString("engraving", "Symbol") },
    { ElementType::TEXT,                 "Text",                 muse::TranslatableString("engraving", "Text") },
    { ElementType::MEASURE_NUMBER,       "MeasureNumber",        muse::TranslatableString("engraving", "Measure number") },
    { ElementType::MMREST_RANGE,         "MMRestRange",          muse::TranslatableString("engraving", "Multimeasure rest range") },
    { ElementType::INSTRUMENT_NAME,      "InstrumentName",       muse::TranslatableString("engraving", "Instrument name") },
    { ElementType::SLUR_SEGMENT,         "SlurSegment",          muse::TranslatableString("engraving", "Slur segment") },
    { ElementType::TIE_SEGMENT,          "TieSegment",           muse::TranslatableString("engraving", "Tie segment") },
    { ElementType::LAISSEZ_VIB_SEGMENT,  "LaissezVibSegment",    muse::TranslatableString("engraving", "Laissez vibrer segment") },
    { ElementType::PARTIAL_TIE_SEGMENT,  "PartialTieSegment",    muse::TranslatableString("engraving", "Partial tie segment") },
    { ElementType::BAR_LINE,             "BarLine",              muse::TranslatableString("engraving", "Barline") },
    { ElementType::STAFF_LINES,          "StaffLines",           muse::TranslatableString("engraving", "Staff lines") },
    { ElementType::SYSTEM_DIVIDER,       "SystemDivider",        muse::TranslatableString("engraving", "System divider") },
    { ElementType::STEM_SLASH,           "StemSlash",            muse::TranslatableString("engraving", "Stem slash") },
    { ElementType::ARPEGGIO,             "Arpeggio",             muse::TranslatableString("engraving", "Arpeggio") },
    { ElementType::ACCIDENTAL,           "Accidental",           muse::TranslatableString("engraving", "Accidental") },
    { ElementType::LEDGER_LINE,          "LedgerLine",           muse::TranslatableString("engraving", "Ledger line") },
    { ElementType::STEM,                 "Stem",                 muse::TranslatableString("engraving", "Stem") },
    { ElementType::HOOK,                 "Hook",                 muse::TranslatableString("engraving", "Flag") }, // internally called "Hook", but "Flag" in SMuFL, so here externally too
    { ElementType::NOTE,                 "Note",                 muse::TranslatableString("engraving", "Note") },
    { ElementType::CLEF,                 "Clef",                 muse::TranslatableString("engraving", "Clef") },
    { ElementType::KEYSIG,               "KeySig",               muse::TranslatableString("engraving", "Key signature") },
    { ElementType::AMBITUS,              "Ambitus",              muse::TranslatableString("engraving", "Ambitus") },
    { ElementType::TIMESIG,              "TimeSig",              muse::TranslatableString("engraving", "Time signature") },
    { ElementType::REST,                 "Rest",                 muse::TranslatableString("engraving", "Rest") },
    { ElementType::MMREST,               "MMRest",               muse::TranslatableString("engraving", "Multimeasure rest") },
    { ElementType::DEAD_SLAPPED,         "DeadSlapped",          muse::TranslatableString("engraving", "Dead slapped") },
    { ElementType::BREATH,               "Breath",               muse::TranslatableString("engraving", "Breath") },
    { ElementType::MEASURE_REPEAT,       "MeasureRepeat",        muse::TranslatableString("engraving", "Measure repeat") },
    { ElementType::TIE,                  "Tie",                  muse::TranslatableString("engraving", "Tie") },
    { ElementType::LAISSEZ_VIB,          "LaissezVib",           muse::TranslatableString("engraving", "Laissez vibrer") },
    { ElementType::PARTIAL_TIE,          "PartialTie",           muse::TranslatableString("engraving", "Partial tie") },
    { ElementType::ARTICULATION,         "Articulation",         muse::TranslatableString("engraving", "Articulation") },
    { ElementType::ORNAMENT,             "Ornament",             muse::TranslatableString("engraving", "Ornament") },
    { ElementType::FERMATA,              "Fermata",              muse::TranslatableString("engraving", "Fermata") },
    { ElementType::CHORDLINE,            "ChordLine",            muse::TranslatableString("engraving", "Chord line") },
    { ElementType::DYNAMIC,              "Dynamic",              muse::TranslatableString("engraving", "Dynamic") },
    { ElementType::EXPRESSION,           "Expression",           muse::TranslatableString("engraving", "Expression") },
    { ElementType::BEAM,                 "Beam",                 muse::TranslatableString("engraving", "Beam") },
    { ElementType::LYRICS,               "Lyrics",               muse::TranslatableString("engraving", "Lyrics") },
    { ElementType::FIGURED_BASS,         "FiguredBass",          muse::TranslatableString("engraving", "Figured bass") },
    { ElementType::FIGURED_BASS_ITEM,    "FiguredBassItem",      muse::TranslatableString("engraving", "Figured bass item") },
    { ElementType::MARKER,               "Marker",               muse::TranslatableString("engraving", "Marker") },
    { ElementType::JUMP,                 "Jump",                 muse::TranslatableString("engraving", "Jump") },
    { ElementType::FINGERING,            "Fingering",            muse::TranslatableString("engraving", "Fingering") },
    { ElementType::TUPLET,               "Tuplet",               muse::TranslatableString("engraving", "Tuplet") },
    { ElementType::TEMPO_TEXT,           "Tempo",                muse::TranslatableString("engraving", "Tempo") },
    { ElementType::STAFF_TEXT,           "StaffText",            muse::TranslatableString("engraving", "Staff text") },
    { ElementType::SYSTEM_TEXT,          "SystemText",           muse::TranslatableString("engraving", "System text") },
    { ElementType::SOUND_FLAG,           "SoundFlag",            muse::TranslatableString("engraving", "Sound flag") },
    { ElementType::PLAYTECH_ANNOTATION,  "PlayTechAnnotation",   muse::TranslatableString("engraving", "Playing technique annotation") },
    { ElementType::CAPO,                 "Capo",                 muse::TranslatableString("engraving", "Capo") },
    { ElementType::STRING_TUNINGS,       "StringTunings",        muse::TranslatableString("engraving", "String tunings") },
    { ElementType::TRIPLET_FEEL,         "TripletFeel",          muse::TranslatableString("engraving", "Triplet feel") },
    { ElementType::REHEARSAL_MARK,       "RehearsalMark",        muse::TranslatableString("engraving", "Rehearsal mark") },
    { ElementType::INSTRUMENT_CHANGE,    "InstrumentChange",     muse::TranslatableString("engraving", "Instrument change") },
    { ElementType::STAFFTYPE_CHANGE,     "StaffTypeChange",      muse::TranslatableString("engraving", "Staff type change") },
    { ElementType::HARMONY,              "Harmony",              muse::TranslatableString("engraving", "Chord symbol") },
    { ElementType::FRET_DIAGRAM,         "FretDiagram",          muse::TranslatableString("engraving", "Fretboard diagram") },
    { ElementType::HARP_DIAGRAM,         "HarpPedalDiagram",     muse::TranslatableString("engraving", "Harp pedal diagram") },
    { ElementType::BEND,                 "Bend",                 muse::TranslatableString("engraving", "Bend") },
    { ElementType::STRETCHED_BEND,       "Bend",                 muse::TranslatableString("engraving", "Bend") },
    { ElementType::TREMOLOBAR,           "TremoloBar",           muse::TranslatableString("engraving", "Tremolo bar") },
    { ElementType::VOLTA,                "Volta",                muse::TranslatableString("engraving", "Volta") },
    { ElementType::HAIRPIN_SEGMENT,      "HairpinSegment",       muse::TranslatableString("engraving", "Hairpin segment") },
    { ElementType::OTTAVA_SEGMENT,       "OttavaSegment",        muse::TranslatableString("engraving", "Ottava segment") },
    { ElementType::TRILL_SEGMENT,        "TrillSegment",         muse::TranslatableString("engraving", "Trill segment") },
    { ElementType::LET_RING_SEGMENT,     "LetRingSegment",       muse::TranslatableString("engraving", "Let ring segment") },
    { ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, "GradualTempoChangeSegment",
      muse::TranslatableString("engraving", "Gradual tempo change segment") },
    { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       muse::TranslatableString("engraving", "Vibrato segment") },
    { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      muse::TranslatableString("engraving", "Palm mute segment") },
    { ElementType::WHAMMY_BAR_SEGMENT,   "WhammyBarSegment",     muse::TranslatableString("engraving", "Whammy bar segment") },
    { ElementType::RASGUEADO_SEGMENT,    "RasgueadoSegment",     muse::TranslatableString("engraving", "Rasgueado segment") },
    { ElementType::HARMONIC_MARK_SEGMENT,    "HarmonicMarkSegment",    muse::TranslatableString("engraving", "Harmonic mark segment") },
    { ElementType::PICK_SCRAPE_SEGMENT,    "PickScrapeSegment",    muse::TranslatableString("engraving", "Pick scrape segment") },
    { ElementType::TEXTLINE_SEGMENT,     "TextLineSegment",      muse::TranslatableString("engraving", "Text line segment") },
    { ElementType::VOLTA_SEGMENT,        "VoltaSegment",         muse::TranslatableString("engraving", "Volta segment") },
    { ElementType::PEDAL_SEGMENT,        "PedalSegment",         muse::TranslatableString("engraving", "Pedal segment") },
    { ElementType::LYRICSLINE_SEGMENT,   "LyricsLineSegment",    muse::TranslatableString("engraving", "Extension line segment") },
    { ElementType::PARTIAL_LYRICSLINE_SEGMENT,   "PartialLyricsLineSegment",    muse::TranslatableString("engraving",
                                                                                                         "Partial extension line segment") },
    { ElementType::GLISSANDO_SEGMENT,    "GlissandoSegment",     muse::TranslatableString("engraving", "Glissando segment") },
    { ElementType::NOTELINE_SEGMENT,     "NoteLineSegment",      muse::TranslatableString("engraving", "Note-anchored line segment") },
    { ElementType::LAYOUT_BREAK,         "LayoutBreak",          muse::TranslatableString("engraving", "Layout break") },
    { ElementType::SYSTEM_LOCK_INDICATOR, "systemLockIndicator", muse::TranslatableString("engraving", "System lock") },
    { ElementType::SPACER,               "Spacer",               muse::TranslatableString("engraving", "Spacer") },
    { ElementType::STAFF_STATE,          "StaffState",           muse::TranslatableString("engraving", "Staff state") },
    { ElementType::NOTEHEAD,             "NoteHead",             muse::TranslatableString("engraving", "Notehead") },
    { ElementType::NOTEDOT,              "NoteDot",              muse::TranslatableString("engraving", "Note dot") },
    { ElementType::IMAGE,                "Image",                muse::TranslatableString("engraving", "Image") },
    { ElementType::MEASURE,              "Measure",              muse::TranslatableString("engraving", "Measure") },
    { ElementType::SELECTION,            "Selection",            muse::TranslatableString("engraving", "Selection") },
    { ElementType::LASSO,                "Lasso",                muse::TranslatableString("engraving", "Lasso") },
    { ElementType::SHADOW_NOTE,          "ShadowNote",           muse::TranslatableString("engraving", "Shadow note") },
    { ElementType::TAB_DURATION_SYMBOL,  "TabDurationSymbol",    muse::TranslatableString("engraving", "Tab duration symbol") },
    { ElementType::FSYMBOL,              "FSymbol",              muse::TranslatableString("engraving", "Font symbol") },
    { ElementType::PAGE,                 "Page",                 muse::TranslatableString("engraving", "Page") },
    { ElementType::PARENTHESIS,          "Parenthesis",          muse::TranslatableString("engraving", "Parenthesis") },
    { ElementType::HAIRPIN,              "HairPin",              muse::TranslatableString("engraving", "Hairpin") },
    { ElementType::OTTAVA,               "Ottava",               muse::TranslatableString("engraving", "Ottava") },
    { ElementType::PEDAL,                "Pedal",                muse::TranslatableString("engraving", "Pedal") },
    { ElementType::TRILL,                "Trill",                muse::TranslatableString("engraving", "Trill") },
    { ElementType::LET_RING,             "LetRing",              muse::TranslatableString("engraving", "Let ring") },
    { ElementType::GRADUAL_TEMPO_CHANGE, "GradualTempoChange",   muse::TranslatableString("engraving", "Gradual tempo change") },
    { ElementType::VIBRATO,              "Vibrato",              muse::TranslatableString("engraving", "Vibrato") },
    { ElementType::PALM_MUTE,            "PalmMute",             muse::TranslatableString("engraving", "Palm mute") },
    { ElementType::WHAMMY_BAR,           "WhammyBar",            muse::TranslatableString("engraving", "Whammy bar") },
    { ElementType::RASGUEADO,            "Rasgueado",            muse::TranslatableString("engraving", "Rasgueado") },
    { ElementType::HARMONIC_MARK,        "HarmonicMark",         muse::TranslatableString("engraving", "Harmonic mark") },
    { ElementType::PICK_SCRAPE,          "PickScrape",           muse::TranslatableString("engraving", "Pick scrape out") },
    { ElementType::TEXTLINE,             "TextLine",             muse::TranslatableString("engraving", "Text line") },
    { ElementType::TEXTLINE_BASE,        "TextLineBase",         muse::TranslatableString("engraving", "Text line base") },    // remove
    { ElementType::NOTELINE,             "NoteLine",             muse::TranslatableString("engraving", "Note-anchored line") },
    { ElementType::LYRICSLINE,           "LyricsLine",           muse::TranslatableString("engraving", "Extension line") },
    { ElementType::PARTIAL_LYRICSLINE,   "PartialLyricsLine",    muse::TranslatableString("engraving", "Partial extension line") },
    { ElementType::GLISSANDO,            "Glissando",            muse::TranslatableString("engraving", "Glissando") },
    { ElementType::BRACKET,              "Bracket",              muse::TranslatableString("engraving", "Bracket") },
    { ElementType::SEGMENT,              "Segment",              muse::TranslatableString("engraving", "Segment") },
    { ElementType::SYSTEM,               "System",               muse::TranslatableString("engraving", "System") },
    { ElementType::CHORD,                "Chord",                muse::TranslatableString("engraving", "Chord") },
    { ElementType::SLUR,                 "Slur",                 muse::TranslatableString("engraving", "Slur") },
    { ElementType::HBOX,                 "HBox",                 muse::TranslatableString("engraving", "Horizontal frame") },
    { ElementType::VBOX,                 "VBox",                 muse::TranslatableString("engraving", "Vertical frame") },
    { ElementType::TBOX,                 "TBox",                 muse::TranslatableString("engraving", "Text frame") },
    { ElementType::FBOX,                 "FBox",                 muse::TranslatableString("engraving", "Fretboard diagram frame") },
    { ElementType::ACTION_ICON,          "ActionIcon",           muse::TranslatableString::untranslatable("Action icon") },
    { ElementType::BAGPIPE_EMBELLISHMENT, "BagpipeEmbellishment", muse::TranslatableString("engraving", "Bagpipe embellishment") },
    { ElementType::STICKING,             "Sticking",             muse::TranslatableString("engraving", "Sticking") },
    { ElementType::GRACE_NOTES_GROUP,    "GraceNotesGroup",      muse::TranslatableString::untranslatable("Grace notes group") },
    { ElementType::FRET_CIRCLE,          "FretCircle",           muse::TranslatableString::untranslatable("Fret circle") },
    { ElementType::GUITAR_BEND,          "GuitarBend",           muse::TranslatableString("engraving", "Guitar bend") },
    { ElementType::GUITAR_BEND_SEGMENT,  "GuitarBendSegment",    muse::TranslatableString("engraving", "Guitar bend segment") },
    { ElementType::GUITAR_BEND_HOLD,     "GuitarBendHold",           muse::TranslatableString("engraving", "Guitar bend hold") },
    { ElementType::GUITAR_BEND_HOLD_SEGMENT, "GuitarBendHoldSegment",
      muse::TranslatableString("engraving", "Guitar bend hold segment") },
    { ElementType::GUITAR_BEND_TEXT,     "GuitarBendText",       muse::TranslatableString("engraving", "Guitar bend text") },
    { ElementType::TREMOLO_SINGLECHORD,  "TremoloSingleChord",   muse::TranslatableString("engraving", "Tremolo") },
    { ElementType::TREMOLO_TWOCHORD,     "TremoloTwoChord",      muse::TranslatableString("engraving", "Tremolo") },
    { ElementType::TIME_TICK_ANCHOR,     "TimeTickAnchor",       muse::TranslatableString("engraving", "Time tick anchor") },
    { ElementType::ROOT_ITEM,            "RootItem",             muse::TranslatableString::untranslatable("Root item") },
    { ElementType::DUMMY,                "Dummy",                muse::TranslatableString::untranslatable("Dummy") },
};

static const std::unordered_map<ElementType, TranslatableString> ELEMENT_TYPE_PLURAL {
    { ElementType::REHEARSAL_MARK, muse::TranslatableString("engraving", "Rehearsal marks") },
    { ElementType::VOLTA, muse::TranslatableString("engraving", "Voltas") },
    { ElementType::JUMP, muse::TranslatableString("engraving", "Jumps") },
    { ElementType::MEASURE_NUMBER, muse::TranslatableString("engraving", "Measure numbers") },
};

const muse::TranslatableString& TConv::userName(ElementType v, bool plural)
{
    if (plural) {
        auto it = ELEMENT_TYPE_PLURAL.find(v);
        if (it != ELEMENT_TYPE_PLURAL.end()) {
            return it->second;
        }
    }

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

static const std::vector<Item<IntervalStep> > INTERVAL_STEP = {
    { IntervalStep::UNISON, "unison" },
    { IntervalStep::SECOND, "second" },
    { IntervalStep::THIRD, "third" },
    { IntervalStep::FOURTH, "fourth" },
    { IntervalStep::FIFTH, "fifth" },
    { IntervalStep::SIXTH, "sixth" },
    { IntervalStep::SEVENTH, "seventh" },
    { IntervalStep::OCTAVE, "octave" }
};

static const std::vector<Item<IntervalType> > INTERVAL_TYPE = {
    { IntervalType::AUTO, "auto" },
    { IntervalType::MINOR, "minor" },
    { IntervalType::MAJOR, "major" },
    { IntervalType::PERFECT, "perfect" },
    { IntervalType::DIMINISHED, "diminished" },
    { IntervalType::AUGMENTED, "augmented" },
};

String TConv::toXml(OrnamentInterval interval)
{
    StringList sl;
    sl << String::fromAscii(findXmlTagByType<IntervalStep>(INTERVAL_STEP, interval.step).ascii());
    sl << String::fromAscii(findXmlTagByType<IntervalType>(INTERVAL_TYPE, interval.type).ascii());
    return sl.join(u",");
}

OrnamentInterval TConv::fromXml(const String& str, OrnamentInterval def)
{
    StringList sl = str.split(',');
    if (sl.size() != 2) {
        LOGD() << "bad ornament interval value: " << str;
        return def;
    }

    OrnamentInterval interval;
    interval.step = findTypeByXmlTag<IntervalStep>(INTERVAL_STEP, sl.at(0), def.step);
    interval.type = findTypeByXmlTag<IntervalType>(INTERVAL_TYPE, sl.at(1), def.type);
    return interval;
}

IntervalStep TConv::fromXml(const AsciiStringView& tag, IntervalStep def)
{
    return findTypeByXmlTag<IntervalStep>(INTERVAL_STEP, tag, def);
}

IntervalType TConv::fromXml(const AsciiStringView& tag, IntervalType def)
{
    return findTypeByXmlTag<IntervalType>(INTERVAL_TYPE, tag, def);
}

static const std::vector<Item<TiePlacement> > TIE_PLACEMENT = {
    { TiePlacement::AUTO, "auto" },
    { TiePlacement::INSIDE, "inside" },
    { TiePlacement::OUTSIDE, "outside" },
};

AsciiStringView TConv::toXml(TiePlacement tiePlacement)
{
    return findXmlTagByType<TiePlacement>(TIE_PLACEMENT, tiePlacement);
}

TiePlacement TConv::fromXml(const AsciiStringView& str, TiePlacement def)
{
    return findTypeByXmlTag<TiePlacement>(TIE_PLACEMENT, str, def);
}

static const std::vector<Item<TieDotsPlacement> > TIE_DOTS_PLACEMENT = {
    { TieDotsPlacement::AUTO, "auto" },
    { TieDotsPlacement::BEFORE_DOTS, "before" },
    { TieDotsPlacement::AFTER_DOTS, "after" },
};

AsciiStringView TConv::toXml(TieDotsPlacement placement)
{
    return findXmlTagByType<TieDotsPlacement>(TIE_DOTS_PLACEMENT, placement);
}

TieDotsPlacement TConv::fromXml(const AsciiStringView& str, TieDotsPlacement def)
{
    return findTypeByXmlTag<TieDotsPlacement>(TIE_DOTS_PLACEMENT, str, def);
}

static const std::vector<Item<TimeSigPlacement> > TIMESIG_PLACEMENT = {
    { TimeSigPlacement::NORMAL, "normal" },
    { TimeSigPlacement::ABOVE_STAVES, "aboveStaves" },
    { TimeSigPlacement::ACROSS_STAVES, "acrossStaves" }
};
static const std::vector<Item<TimeSigStyle> > TIMESIG_STYLE = {
    { TimeSigStyle::NORMAL, "normal" },
    { TimeSigStyle::NARROW, "narrow" },
    { TimeSigStyle::LARGE, "large" }
};
static const std::vector<Item<TimeSigVSMargin> > TIMESIG_MARGIN = {
    { TimeSigVSMargin::HANG_INTO_MARGIN, "hangIntoMargin" },
    { TimeSigVSMargin::RIGHT_ALIGN_TO_BARLINE, "rightAlignToBarline" },
    { TimeSigVSMargin::CREATE_SPACE, "createSpace" },
};

AsciiStringView TConv::toXml(TimeSigPlacement timeSigPos)
{
    return findXmlTagByType<TimeSigPlacement>(TIMESIG_PLACEMENT, timeSigPos);
}

TimeSigPlacement TConv::fromXml(const AsciiStringView& str, TimeSigPlacement def)
{
    return findTypeByXmlTag<TimeSigPlacement>(TIMESIG_PLACEMENT, str, def);
}

AsciiStringView TConv::toXml(TimeSigStyle timeSigStyle)
{
    return findXmlTagByType<TimeSigStyle>(TIMESIG_STYLE, timeSigStyle);
}

TimeSigStyle TConv::fromXml(const AsciiStringView& str, TimeSigStyle def)
{
    return findTypeByXmlTag<TimeSigStyle>(TIMESIG_STYLE, str, def);
}

AsciiStringView TConv::toXml(TimeSigVSMargin timeSigVSMargin)
{
    return findXmlTagByType<TimeSigVSMargin>(TIMESIG_MARGIN, timeSigVSMargin);
}

TimeSigVSMargin TConv::fromXml(const AsciiStringView& str, TimeSigVSMargin def)
{
    return findTypeByXmlTag<TimeSigVSMargin>(TIMESIG_MARGIN, str, def);
}

static const std::vector<Item<VoiceAssignment> > VOICE_ASSIGNMENT = {
    { VoiceAssignment::ALL_VOICE_IN_INSTRUMENT, "allInInstrument" },
    { VoiceAssignment::ALL_VOICE_IN_STAFF,      "allInStaff" },
    { VoiceAssignment::CURRENT_VOICE_ONLY,      "currentVoiceOnly" }
};

AsciiStringView TConv::toXml(VoiceAssignment voiceAppl)
{
    return findXmlTagByType<VoiceAssignment>(VOICE_ASSIGNMENT, voiceAppl);
}

VoiceAssignment TConv::fromXml(const AsciiStringView& str, VoiceAssignment def)
{
    return findTypeByXmlTag<VoiceAssignment>(VOICE_ASSIGNMENT, str, def);
}

static const std::vector<Item<AutoOnOff> > AUTO_ON_OFF = {
    { AutoOnOff::AUTO, "auto" },
    { AutoOnOff::ON,   "on" },
    { AutoOnOff::OFF,  "off" },
};

AsciiStringView TConv::toXml(AutoOnOff autoOnOff)
{
    return findXmlTagByType<AutoOnOff>(AUTO_ON_OFF, autoOnOff);
}

AutoOnOff TConv::fromXml(const AsciiStringView& str, AutoOnOff def)
{
    return findTypeByXmlTag<AutoOnOff>(AUTO_ON_OFF, str, def);
}

static const std::vector<Item<ArticulationStemSideAlign> > ARTICULATION_STEM_SIDE_ALIGN = {
    { ArticulationStemSideAlign::STEM,     "stem" },
    { ArticulationStemSideAlign::NOTEHEAD, "notehead" },
    { ArticulationStemSideAlign::AVERAGE,  "average" },
};

AsciiStringView TConv::toXml(ArticulationStemSideAlign articulationStemSideAlign)
{
    return findXmlTagByType<ArticulationStemSideAlign>(ARTICULATION_STEM_SIDE_ALIGN, articulationStemSideAlign);
}

ArticulationStemSideAlign TConv::fromXml(const AsciiStringView& str, ArticulationStemSideAlign def)
{
    return findTypeByXmlTag<ArticulationStemSideAlign>(ARTICULATION_STEM_SIDE_ALIGN, str, def);
}

static const std::vector<Item<PartialSpannerDirection> > PARTIAL_SPANNER_DIRECTION = {
    { PartialSpannerDirection::NONE,     "none" },
    { PartialSpannerDirection::OUTGOING, "outgoing" },
    { PartialSpannerDirection::INCOMING, "incoming" },
    { PartialSpannerDirection::BOTH,     "both" }
};

AsciiStringView TConv::toXml(PartialSpannerDirection v)
{
    return findXmlTagByType<PartialSpannerDirection>(PARTIAL_SPANNER_DIRECTION, v);
}

PartialSpannerDirection TConv::fromXml(const AsciiStringView& str, PartialSpannerDirection def)
{
    return findTypeByXmlTag<PartialSpannerDirection>(PARTIAL_SPANNER_DIRECTION, str, def);
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
    { Orientation::VERTICAL,    "vertical",     muse::TranslatableString("engraving", "Vertical") },
    { Orientation::HORIZONTAL,  "horizontal",   muse::TranslatableString("engraving", "Horizontal") },
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
    { NoteHeadType::HEAD_AUTO,      "auto",    muse::TranslatableString("engraving", "Auto") },
    { NoteHeadType::HEAD_WHOLE,     "whole",   muse::TranslatableString("engraving/noteheadtype", "Whole") },
    { NoteHeadType::HEAD_HALF,      "half",    muse::TranslatableString("engraving/noteheadtype", "Half") },
    { NoteHeadType::HEAD_QUARTER,   "quarter", muse::TranslatableString("engraving/noteheadtype", "Quarter") },
    { NoteHeadType::HEAD_BREVIS,    "breve",   muse::TranslatableString("engraving/noteheadtype", "Breve") },
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
    { NoteHeadScheme::HEAD_AUTO,                "auto",              muse::TranslatableString("engraving", "Auto") },
    { NoteHeadScheme::HEAD_NORMAL,              "normal",            muse::TranslatableString("engraving/noteheadscheme", "Normal") },
    { NoteHeadScheme::HEAD_PITCHNAME,           "name-pitch",        muse::TranslatableString("engraving/noteheadscheme", "Pitch names") },
    { NoteHeadScheme::HEAD_PITCHNAME_GERMAN,    "name-pitch-german", muse::TranslatableString("engraving/noteheadscheme", "German pitch names") },
    { NoteHeadScheme::HEAD_SOLFEGE,             "solfege-movable",   muse::TranslatableString("engraving/noteheadscheme", "Solf\u00e8ge movable Do") },  // &egrave;
    { NoteHeadScheme::HEAD_SOLFEGE_FIXED,       "solfege-fixed",     muse::TranslatableString("engraving/noteheadscheme", "Solf\u00e8ge fixed Do") },    // &egrave;
    { NoteHeadScheme::HEAD_SHAPE_NOTE_4,        "shape-4",           muse::TranslatableString("engraving/noteheadscheme", "4-shape (Walker)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_AIKIN,  "shape-7-aikin",     muse::TranslatableString("engraving/noteheadscheme", "7-shape (Aikin)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_FUNK,   "shape-7-funk",      muse::TranslatableString("engraving/noteheadscheme", "7-shape (Funk)") },
    { NoteHeadScheme::HEAD_SHAPE_NOTE_7_WALKER, "shape-7-walker",    muse::TranslatableString("engraving/noteheadscheme", "7-shape (Walker)") }
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
    { NoteHeadGroup::HEAD_NORMAL,           "normal",         muse::TranslatableString("engraving/noteheadgroup", "Normal") },
    { NoteHeadGroup::HEAD_CROSS,            "cross",          muse::TranslatableString("engraving/noteheadgroup", "Cross") },
    { NoteHeadGroup::HEAD_PLUS,             "plus",           muse::TranslatableString("engraving/noteheadgroup", "Plus") },
    { NoteHeadGroup::HEAD_XCIRCLE,          "xcircle",        muse::TranslatableString("engraving/noteheadgroup", "XCircle") },
    { NoteHeadGroup::HEAD_WITHX,            "withx",          muse::TranslatableString("engraving/noteheadgroup", "With X") },
    { NoteHeadGroup::HEAD_TRIANGLE_UP,      "triangle-up",    muse::TranslatableString("engraving/noteheadgroup", "Triangle up") },
    { NoteHeadGroup::HEAD_TRIANGLE_DOWN,    "triangle-down",  muse::TranslatableString("engraving/noteheadgroup", "Triangle down") },
    { NoteHeadGroup::HEAD_SLASHED1,         "slashed1",       muse::TranslatableString("engraving/noteheadgroup", "Slashed (forwards)") },
    { NoteHeadGroup::HEAD_SLASHED2,         "slashed2",       muse::TranslatableString("engraving/noteheadgroup", "Slashed (backwards)") },
    { NoteHeadGroup::HEAD_DIAMOND,          "diamond",        muse::TranslatableString("engraving/noteheadgroup", "Diamond") },
    { NoteHeadGroup::HEAD_DIAMOND_OLD,      "diamond-old",    muse::TranslatableString("engraving/noteheadgroup", "Diamond (old)") },
    { NoteHeadGroup::HEAD_CIRCLED,          "circled",        muse::TranslatableString("engraving/noteheadgroup", "Circled") },
    { NoteHeadGroup::HEAD_CIRCLED_LARGE,    "circled-large",  muse::TranslatableString("engraving/noteheadgroup", "Circled large") },
    { NoteHeadGroup::HEAD_LARGE_ARROW,      "large-arrow",    muse::TranslatableString("engraving/noteheadgroup", "Large arrow") },
    { NoteHeadGroup::HEAD_BREVIS_ALT,       "altbrevis",      muse::TranslatableString("engraving/noteheadgroup", "Alt. brevis") },

    { NoteHeadGroup::HEAD_SLASH,            "slash",          muse::TranslatableString("engraving/noteheadgroup", "Slash") },
    { NoteHeadGroup::HEAD_LARGE_DIAMOND,    "large-diamond",  muse::TranslatableString("engraving/noteheadgroup", "Large diamond") },

    { NoteHeadGroup::HEAD_HEAVY_CROSS,      "heavy-cross",    muse::TranslatableString("engraving/noteheadgroup", "Heavy cross") },
    { NoteHeadGroup::HEAD_HEAVY_CROSS_HAT,  "heavy-cross-hat",    muse::TranslatableString("engraving/noteheadgroup", "Heavy cross hat") },

    // shape notes
    { NoteHeadGroup::HEAD_SOL,  "sol",       muse::TranslatableString("engraving/noteheadgroup", "Sol") },
    { NoteHeadGroup::HEAD_LA,   "la",        muse::TranslatableString("engraving/noteheadgroup", "La") },
    { NoteHeadGroup::HEAD_FA,   "fa",        muse::TranslatableString("engraving/noteheadgroup", "Fa") },
    { NoteHeadGroup::HEAD_MI,   "mi",        muse::TranslatableString("engraving/noteheadgroup", "Mi") },
    { NoteHeadGroup::HEAD_DO,   "do",        muse::TranslatableString("engraving/noteheadgroup", "Do") },
    { NoteHeadGroup::HEAD_RE,   "re",        muse::TranslatableString("engraving/noteheadgroup", "Re") },
    { NoteHeadGroup::HEAD_TI,   "ti",        muse::TranslatableString("engraving/noteheadgroup", "Ti") },

    // not exposed
    { NoteHeadGroup::HEAD_DO_WALKER,    "do-walker", muse::TranslatableString("engraving/noteheadgroup", "Do (Walker)") },
    { NoteHeadGroup::HEAD_RE_WALKER,    "re-walker", muse::TranslatableString("engraving/noteheadgroup", "Re (Walker)") },
    { NoteHeadGroup::HEAD_TI_WALKER,    "ti-walker", muse::TranslatableString("engraving/noteheadgroup", "Ti (Walker)") },
    { NoteHeadGroup::HEAD_DO_FUNK,      "do-funk",   muse::TranslatableString("engraving/noteheadgroup", "Do (Funk)") },
    { NoteHeadGroup::HEAD_RE_FUNK,      "re-funk",   muse::TranslatableString("engraving/noteheadgroup", "Re (Funk)") },
    { NoteHeadGroup::HEAD_TI_FUNK,      "ti-funk",   muse::TranslatableString("engraving/noteheadgroup", "Ti (Funk)") },

    // note name
    { NoteHeadGroup::HEAD_DO_NAME,      "do-name",  muse::TranslatableString("engraving/noteheadgroup",  "Do (Name)") },
    { NoteHeadGroup::HEAD_DI_NAME,      "di-name",  muse::TranslatableString("engraving/noteheadgroup",  "Di (Name)") },
    { NoteHeadGroup::HEAD_RA_NAME,      "ra-name",  muse::TranslatableString("engraving/noteheadgroup",  "Ra (Name)") },
    { NoteHeadGroup::HEAD_RE_NAME,      "re-name",  muse::TranslatableString("engraving/noteheadgroup",  "Re (Name)") },
    { NoteHeadGroup::HEAD_RI_NAME,      "ri-name",  muse::TranslatableString("engraving/noteheadgroup",  "Ri (Name)") },
    { NoteHeadGroup::HEAD_ME_NAME,      "me-name",  muse::TranslatableString("engraving/noteheadgroup",  "Me (Name)") },
    { NoteHeadGroup::HEAD_MI_NAME,      "mi-name",  muse::TranslatableString("engraving/noteheadgroup",  "Mi (Name)") },
    { NoteHeadGroup::HEAD_FA_NAME,      "fa-name",  muse::TranslatableString("engraving/noteheadgroup",  "Fa (Name)") },
    { NoteHeadGroup::HEAD_FI_NAME,      "fi-name",  muse::TranslatableString("engraving/noteheadgroup",  "Fi (Name)") },
    { NoteHeadGroup::HEAD_SE_NAME,      "se-name",  muse::TranslatableString("engraving/noteheadgroup",  "Se (Name)") },
    { NoteHeadGroup::HEAD_SOL_NAME,     "sol-name", muse::TranslatableString("engraving/noteheadgroup",  "Sol (Name)") },
    { NoteHeadGroup::HEAD_LE_NAME,      "le-name",  muse::TranslatableString("engraving/noteheadgroup",  "Le (Name)") },
    { NoteHeadGroup::HEAD_LA_NAME,      "la-name",  muse::TranslatableString("engraving/noteheadgroup",  "La (Name)") },
    { NoteHeadGroup::HEAD_LI_NAME,      "li-name",  muse::TranslatableString("engraving/noteheadgroup",  "Li (Name)") },
    { NoteHeadGroup::HEAD_TE_NAME,      "te-name",  muse::TranslatableString("engraving/noteheadgroup",  "Te (Name)") },
    { NoteHeadGroup::HEAD_TI_NAME,      "ti-name",  muse::TranslatableString("engraving/noteheadgroup",  "Ti (Name)") },
    { NoteHeadGroup::HEAD_SI_NAME,      "si-name",  muse::TranslatableString("engraving/noteheadgroup",  "Si (Name)") },

    { NoteHeadGroup::HEAD_A_SHARP,      "a-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "A♯ (Name)") },
    { NoteHeadGroup::HEAD_A,            "a-name",       muse::TranslatableString("engraving/noteheadgroup",  "A (Name)") },
    { NoteHeadGroup::HEAD_A_FLAT,       "a-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "A♭ (Name)") },
    { NoteHeadGroup::HEAD_B_SHARP,      "b-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "B♯ (Name)") },
    { NoteHeadGroup::HEAD_B,            "b-name",       muse::TranslatableString("engraving/noteheadgroup",  "B (Name)") },
    { NoteHeadGroup::HEAD_B_FLAT,       "b-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "B♭ (Name)") },
    { NoteHeadGroup::HEAD_C_SHARP,      "c-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "C♯ (Name)") },
    { NoteHeadGroup::HEAD_C,            "c-name",       muse::TranslatableString("engraving/noteheadgroup",  "C (Name)") },
    { NoteHeadGroup::HEAD_C_FLAT,       "c-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "C♭ (Name)") },
    { NoteHeadGroup::HEAD_D_SHARP,      "d-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "D♯ (Name)") },
    { NoteHeadGroup::HEAD_D,            "d-name",       muse::TranslatableString("engraving/noteheadgroup",  "D (Name)") },
    { NoteHeadGroup::HEAD_D_FLAT,       "d-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "D♭ (Name)") },
    { NoteHeadGroup::HEAD_E_SHARP,      "e-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "E♯ (Name)") },
    { NoteHeadGroup::HEAD_E,            "e-name",       muse::TranslatableString("engraving/noteheadgroup",  "E (Name)") },
    { NoteHeadGroup::HEAD_E_FLAT,       "e-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "E♭ (Name)") },
    { NoteHeadGroup::HEAD_F_SHARP,      "f-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "F♯ (Name)") },
    { NoteHeadGroup::HEAD_F,            "f-name",       muse::TranslatableString("engraving/noteheadgroup",  "F (Name)") },
    { NoteHeadGroup::HEAD_F_FLAT,       "f-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "F♭ (Name)") },
    { NoteHeadGroup::HEAD_G_SHARP,      "g-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "G♯ (Name)") },
    { NoteHeadGroup::HEAD_G,            "g-name",       muse::TranslatableString("engraving/noteheadgroup",  "G (Name)") },
    { NoteHeadGroup::HEAD_G_FLAT,       "g-flat-name",  muse::TranslatableString("engraving/noteheadgroup",  "G♭ (Name)") },
    { NoteHeadGroup::HEAD_H,            "h-name",       muse::TranslatableString("engraving/noteheadgroup",  "H (Name)") },
    { NoteHeadGroup::HEAD_H_SHARP,      "h-sharp-name", muse::TranslatableString("engraving/noteheadgroup",  "H♯ (Name)") },

    // Swiss rudiments
    { NoteHeadGroup::HEAD_SWISS_RUDIMENTS_FLAM,   "swiss-rudiments-flam",   muse::TranslatableString("engraving/noteheadgroup",
                                                                                                     "Swiss Rudiments Flam") },
    { NoteHeadGroup::HEAD_SWISS_RUDIMENTS_DOUBLE, "swiss-rudiments-double", muse::TranslatableString("engraving/noteheadgroup",
                                                                                                     "Swiss Rudiments Doublé") },

    { NoteHeadGroup::HEAD_CUSTOM,       "custom",       muse::TranslatableString("engraving",  "Custom") }
};

const muse::TranslatableString& TConv::userName(NoteHeadGroup v)
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

// table must be in sync with enum ClefType in types.h
static const std::vector<Item<ClefType> > CLEF_TYPES = {
    { ClefType::G,          "G",        muse::TranslatableString("engraving/cleftype", "Treble clef") },
    { ClefType::G15_MB,     "G15mb",    muse::TranslatableString("engraving/cleftype", "Treble clef 15ma bassa") },
    { ClefType::G8_VB,      "G8vb",     muse::TranslatableString("engraving/cleftype", "Treble clef 8va bassa") },
    { ClefType::G8_VA,      "G8va",     muse::TranslatableString("engraving/cleftype", "Treble clef 8va alta") },
    { ClefType::G15_MA,     "G15ma",    muse::TranslatableString("engraving/cleftype", "Treble clef 15ma alta") },
    { ClefType::G8_VB_O,    "G8vbo",    muse::TranslatableString("engraving/cleftype", "Double treble clef 8va bassa on 2nd line") },
    { ClefType::G8_VB_P,    "G8vbp",    muse::TranslatableString("engraving/cleftype", "Treble clef optional 8va bassa") },
    { ClefType::G_1,        "G1",       muse::TranslatableString("engraving/cleftype", "French violin clef") },
    { ClefType::C1,         "C1",       muse::TranslatableString("engraving/cleftype", "Soprano clef") },
    { ClefType::C2,         "C2",       muse::TranslatableString("engraving/cleftype", "Mezzo-soprano clef") },
    { ClefType::C3,         "C3",       muse::TranslatableString("engraving/cleftype", "Alto clef") },
    { ClefType::C4,         "C4",       muse::TranslatableString("engraving/cleftype", "Tenor clef") },
    { ClefType::C5,         "C5",       muse::TranslatableString("engraving/cleftype", "Baritone clef (C clef)") },
    { ClefType::C_19C,      "C_19C",    muse::TranslatableString("engraving/cleftype", "C clef, H shape (19th century)") },
    { ClefType::C1_F18C,    "C1_F18C",  muse::TranslatableString("engraving/cleftype", "Soprano clef (French, 18th century)") },
    { ClefType::C3_F18C,    "C3_F18C",  muse::TranslatableString("engraving/cleftype", "Alto clef (French, 18th century)") },
    { ClefType::C4_F18C,    "C4_F18C",  muse::TranslatableString("engraving/cleftype", "Tenor clef (French, 18th century)") },
    { ClefType::C1_F20C,    "C1_F20C",  muse::TranslatableString("engraving/cleftype", "Soprano clef (French, 20th century)") },
    { ClefType::C3_F20C,    "C3_F20C",  muse::TranslatableString("engraving/cleftype", "Alto clef (French, 20th century)") },
    { ClefType::C4_F20C,    "C4_F20C",  muse::TranslatableString("engraving/cleftype", "Tenor clef (French, 20th century)") },
    { ClefType::F,          "F",        muse::TranslatableString("engraving/cleftype", "Bass clef") },
    { ClefType::F15_MB,     "F15mb",    muse::TranslatableString("engraving/cleftype", "Bass clef 15ma bassa") },
    { ClefType::F8_VB,      "F8vb",     muse::TranslatableString("engraving/cleftype", "Bass clef 8va bassa") },
    { ClefType::F_8VA,      "F8va",     muse::TranslatableString("engraving/cleftype", "Bass clef 8va alta") },
    { ClefType::F_15MA,     "F15ma",    muse::TranslatableString("engraving/cleftype", "Bass clef 15ma alta") },
    { ClefType::F_B,        "F3",       muse::TranslatableString("engraving/cleftype", "Baritone clef (F clef)") },
    { ClefType::F_C,        "F5",       muse::TranslatableString("engraving/cleftype", "Subbass clef") },
    { ClefType::F_F18C,     "F_F18C",   muse::TranslatableString("engraving/cleftype", "F clef (French, 18th century)") },
    { ClefType::F_19C,      "F_19C",    muse::TranslatableString("engraving/cleftype", "F clef (19th century)") },

    { ClefType::PERC,       "PERC",     muse::TranslatableString("engraving/cleftype", "Percussion") },
    { ClefType::PERC2,      "PERC2",    muse::TranslatableString("engraving/cleftype", "Percussion 2") },

    { ClefType::TAB,        "TAB",      muse::TranslatableString("engraving/cleftype", "Tablature") },
    { ClefType::TAB4,       "TAB4",     muse::TranslatableString("engraving/cleftype", "Tablature 4 lines") },
    { ClefType::TAB_SERIF,  "TAB2",     muse::TranslatableString("engraving/cleftype", "Tablature Serif") },
    { ClefType::TAB4_SERIF, "TAB4_SERIF", muse::TranslatableString("engraving/cleftype", "Tablature Serif 4 lines") },

    { ClefType::C4_8VB,     "C4_8VB",   muse::TranslatableString("engraving/cleftype", "Tenor clef 8va bassa") },
};

const muse::TranslatableString& TConv::userName(ClefType v)
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
    muse::TranslatableString userName;
};

static const std::vector<DynamicItem> DYNAMIC_TYPES = {
    { DynamicType::OTHER,   "other-dynamics",   SymId::noSym,                      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "Other dynamic") },
    { DynamicType::PPPPPP,  "pppppp",           SymId::dynamicPPPPPP,              muse::TranslatableString("engraving/dynamictype",
                                                                                                            "pppppp (Pianissississississimo)") },
    { DynamicType::PPPPP,   "ppppp",            SymId::dynamicPPPPP,               muse::TranslatableString("engraving/dynamictype",
                                                                                                            "ppppp (Pianississississimo)") },
    { DynamicType::PPPP,    "pppp",             SymId::dynamicPPPP,                muse::TranslatableString("engraving/dynamictype",
                                                                                                            "pppp (Pianissississimo)") },
    { DynamicType::PPP,     "ppp",              SymId::dynamicPPP,                 muse::TranslatableString("engraving/dynamictype",
                                                                                                            "ppp (Pianississimo)") },
    { DynamicType::PP,      "pp",               SymId::dynamicPP,                  muse::TranslatableString("engraving/dynamictype",
                                                                                                            "pp (Pianissimo)") },
    { DynamicType::P,       "p",                SymId::dynamicPiano,               muse::TranslatableString("engraving/dynamictype",
                                                                                                            "p (Piano)") },

    { DynamicType::MP,      "mp",               SymId::dynamicMP,                  muse::TranslatableString("engraving/dynamictype",
                                                                                                            "mp (Mezzo-piano)") },
    { DynamicType::MF,      "mf",               SymId::dynamicMF,                  muse::TranslatableString("engraving/dynamictype",
                                                                                                            "mf (Mezzo-forte)") },

    { DynamicType::F,       "f",                SymId::dynamicForte,               muse::TranslatableString("engraving/dynamictype",
                                                                                                            "f (Forte)") },
    { DynamicType::FF,      "ff",               SymId::dynamicFF,                  muse::TranslatableString("engraving/dynamictype",
                                                                                                            "ff (Fortissimo)") },
    { DynamicType::FFF,     "fff",              SymId::dynamicFFF,                 muse::TranslatableString("engraving/dynamictype",
                                                                                                            "fff (Fortississimo)") },
    { DynamicType::FFFF,    "ffff",             SymId::dynamicFFFF,                muse::TranslatableString("engraving/dynamictype",
                                                                                                            "ffff (Fortissississimo)") },
    { DynamicType::FFFFF,   "fffff",            SymId::dynamicFFFFF,               muse::TranslatableString("engraving/dynamictype",
                                                                                                            "fffff (Fortississississimo)") },
    { DynamicType::FFFFFF,  "ffffff",           SymId::dynamicFFFFFF,              muse::TranslatableString("engraving/dynamictype",
                                                                                                            "ffffff (Fortissississississimo)") },

    { DynamicType::FP,      "fp",               SymId::dynamicFortePiano,          muse::TranslatableString("engraving/dynamictype",
                                                                                                            "fp (Fortepiano)") },
    { DynamicType::PF,      "pf",               SymId::noSym,                      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "pf (Pianoforte)") },

    { DynamicType::SF,      "sf",               SymId::dynamicSforzando1,          muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sf (Sforzando)") },
    { DynamicType::SFZ,     "sfz",              SymId::dynamicSforzato,            muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sfz (Sforzato)") },
    { DynamicType::SFF,     "sff",              SymId::noSym,                      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sff (Sforzando)") },
    { DynamicType::SFFZ,    "sffz",             SymId::dynamicSforzatoFF,          muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sffz (Sforzato)") },
    { DynamicType::SFFF,    "sfff",             SymId::noSym,                      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sfff (Sforzando)") },
    { DynamicType::SFFFZ,   "sfffz",            SymId::noSym,                      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sfffz (Sforzato)") },
    { DynamicType::SFP,     "sfp",              SymId::dynamicSforzandoPiano,      muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sfp (Sforzando-piano)") },
    { DynamicType::SFPP,    "sfpp",             SymId::dynamicSforzandoPianissimo, muse::TranslatableString("engraving/dynamictype",
                                                                                                            "sfpp (Sforzando-pianissimo)") },

    { DynamicType::RFZ,     "rfz",              SymId::dynamicRinforzando2,        muse::TranslatableString("engraving/dynamictype",
                                                                                                            "rfz (Rinforzando)") },
    { DynamicType::RF,      "rf",               SymId::dynamicRinforzando1,        muse::TranslatableString("engraving/dynamictype",
                                                                                                            "rf (Rinforzando)") },
    { DynamicType::FZ,      "fz",               SymId::dynamicForzando,            muse::TranslatableString("engraving/dynamictype",
                                                                                                            "fz (Forzando)") },
    { DynamicType::M,       "m",                SymId::dynamicMezzo,               muse::TranslatableString("engraving/dynamictype",
                                                                                                            "m (Mezzo)") },
    { DynamicType::R,       "r",                SymId::dynamicRinforzando,         muse::TranslatableString("engraving/dynamictype",
                                                                                                            "r (Rinforzando)") },
    { DynamicType::S,       "s",                SymId::dynamicSforzando,           muse::TranslatableString("engraving/dynamictype",
                                                                                                            "s (Sforzando)") },
    { DynamicType::Z,       "z",                SymId::dynamicZ,                   muse::TranslatableString("engraving/dynamictype",
                                                                                                            "z (Forzando)") },
    { DynamicType::N,       "n",                SymId::dynamicNiente,              muse::TranslatableString("engraving/dynamictype",
                                                                                                            "n (Niente)") },
};

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

bool TConv::dynamicValid(const AsciiStringView& tag)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [tag](const DynamicItem& i) {
        return i.xml == tag;
    });

    if (it != DYNAMIC_TYPES.cend()) {
        return true;
    }
    return false;
}

const muse::TranslatableString& TConv::userName(DynamicType v)
{
    auto it = std::find_if(DYNAMIC_TYPES.cbegin(), DYNAMIC_TYPES.cend(), [v](const DynamicItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != DYNAMIC_TYPES.cend()) {
        static TranslatableString dummy;
        return dummy;
    }
    return it->userName;
}

String TConv::translatedUserName(DynamicType v)
{
    return userName(v).translated();
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
        PenStyle penStyle = static_cast<PenStyle>(v);
        switch (penStyle) {
        case PenStyle::NoPen:
            return def;
        case PenStyle::SolidLine:
            return LineType::SOLID;
        case PenStyle::DashLine:
        case PenStyle::DashDotLine:
        case PenStyle::CustomDashLine:
            return LineType::DASHED;
        case PenStyle::DotLine:
        case PenStyle::DashDotDotLine:
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
    { TextStyleType::DEFAULT,           "default",              muse::TranslatableString("engraving", "Default") },
    { TextStyleType::TITLE,             "title",                muse::TranslatableString("engraving", "Title") },
    { TextStyleType::SUBTITLE,          "subtitle",             muse::TranslatableString("engraving", "Subtitle") },
    { TextStyleType::COMPOSER,          "composer",             muse::TranslatableString("engraving", "Composer") },
    { TextStyleType::LYRICIST,          "poet",                 muse::TranslatableString("engraving", "Lyricist") },
    { TextStyleType::TRANSLATOR,        "translator",           muse::TranslatableString("engraving", "Translator") },
    { TextStyleType::FRAME,             "frame",                muse::TranslatableString("engraving", "Frame") },
    { TextStyleType::INSTRUMENT_EXCERPT, "instrument_excerpt",  muse::TranslatableString("engraving", "Instrument name (Part)") },
    { TextStyleType::INSTRUMENT_LONG,   "instrument_long",      muse::TranslatableString("engraving", "Instrument name (Long)") },
    { TextStyleType::INSTRUMENT_SHORT,  "instrument_short",     muse::TranslatableString("engraving", "Instrument name (Short)") },
    { TextStyleType::INSTRUMENT_CHANGE, "instrument_change",    muse::TranslatableString("engraving", "Instrument change") },
    { TextStyleType::HEADER,            "header",               muse::TranslatableString("engraving", "Header") },
    { TextStyleType::FOOTER,            "footer",               muse::TranslatableString("engraving", "Footer") },
    { TextStyleType::COPYRIGHT,         "copyright",            muse::TranslatableString("engraving", "Copyright") },
    { TextStyleType::PAGE_NUMBER,       "page_number",          muse::TranslatableString("engraving", "Page number") },

    { TextStyleType::MEASURE_NUMBER,    "measure_number",       muse::TranslatableString("engraving", "Measure number") },
    { TextStyleType::MMREST_RANGE,      "mmrest_range",         muse::TranslatableString("engraving", "Multimeasure rest range") },

    { TextStyleType::TEMPO,             "tempo",                muse::TranslatableString("engraving", "Tempo") },
    { TextStyleType::TEMPO_CHANGE,      "tempo change",         muse::TranslatableString("engraving", "Gradual tempo change") },
    { TextStyleType::METRONOME,         "metronome",            muse::TranslatableString("engraving", "Metronome") },
    { TextStyleType::REPEAT_LEFT,       "repeat_left",          muse::TranslatableString("engraving", "Repeat text left") },
    { TextStyleType::REPEAT_RIGHT,      "repeat_right",         muse::TranslatableString("engraving", "Repeat text right") },
    { TextStyleType::REHEARSAL_MARK,    "rehearsal_mark",       muse::TranslatableString("engraving", "Rehearsal mark") },
    { TextStyleType::SYSTEM,            "system",               muse::TranslatableString("engraving", "System") },

    { TextStyleType::STAFF,             "staff",                muse::TranslatableString("engraving", "Staff") },
    { TextStyleType::EXPRESSION,        "expression",           muse::TranslatableString("engraving", "Expression") },
    { TextStyleType::DYNAMICS,          "dynamics",             muse::TranslatableString("engraving", "Dynamics") },
    { TextStyleType::HAIRPIN,           "hairpin",              muse::TranslatableString("engraving", "Hairpin") },
    { TextStyleType::LYRICS_ODD,        "lyrics_odd",           muse::TranslatableString("engraving", "Lyrics odd lines") },
    { TextStyleType::LYRICS_EVEN,       "lyrics_even",          muse::TranslatableString("engraving", "Lyrics even lines") },
    { TextStyleType::HARMONY_A,         "harmony_a",            muse::TranslatableString("engraving", "Chord symbol") },
    { TextStyleType::HARMONY_B,         "harmony_b",            muse::TranslatableString("engraving", "Chord symbol (alternate)") },
    { TextStyleType::HARMONY_ROMAN,     "harmony_roman",        muse::TranslatableString("engraving", "Roman numeral analysis") },
    { TextStyleType::HARMONY_NASHVILLE, "harmony_nashville",    muse::TranslatableString("engraving", "Nashville number") },

    { TextStyleType::TUPLET,            "tuplet",               muse::TranslatableString("engraving", "Tuplet") },
    { TextStyleType::STICKING,          "sticking",             muse::TranslatableString("engraving", "Sticking") },
    { TextStyleType::FINGERING,         "fingering",            muse::TranslatableString("engraving", "Fingering") },
    { TextStyleType::LH_GUITAR_FINGERING, "guitar_fingering_lh", muse::TranslatableString("engraving", "LH guitar fingering") },
    { TextStyleType::RH_GUITAR_FINGERING, "guitar_fingering_rh", muse::TranslatableString("engraving", "RH guitar fingering") },
    { TextStyleType::STRING_NUMBER,     "string_number",        muse::TranslatableString("engraving", "String number") },
    { TextStyleType::STRING_TUNINGS,    "string_tunings", muse::TranslatableString("engraving", "String tunings") },
    { TextStyleType::FRET_DIAGRAM_FINGERING, "fret_diagram_fingering",
      muse::TranslatableString("engraving", "Fretboard diagram fingering") },
    { TextStyleType::FRET_DIAGRAM_FRET_NUMBER, "fret_diagram_fret_number",
      muse::TranslatableString("engraving", "Fretboard diagram fret number") },
    { TextStyleType::HARP_PEDAL_DIAGRAM, "harp_pedal_diagram",  muse::TranslatableString("engraving", "Harp pedal diagram") },
    { TextStyleType::HARP_PEDAL_TEXT_DIAGRAM, "harp_pedal_text_diagram", muse::TranslatableString("engraving", "Harp pedal text diagram") },

    { TextStyleType::TEXTLINE,          "textline",             muse::TranslatableString("engraving", "Text line") },
    { TextStyleType::NOTELINE,          "noteline",             muse::TranslatableString("engraving", "Note-anchored line") },
    { TextStyleType::VOLTA,             "volta",                muse::TranslatableString("engraving", "Volta") },
    { TextStyleType::OTTAVA,            "ottava",               muse::TranslatableString("engraving", "Ottava") },
    { TextStyleType::GLISSANDO,         "glissando",            muse::TranslatableString("engraving", "Glissando") },
    { TextStyleType::PEDAL,             "pedal",                muse::TranslatableString("engraving", "Pedal") },
    { TextStyleType::BEND,              "bend",                 muse::TranslatableString("engraving", "Bend") },
    { TextStyleType::LET_RING,          "let_ring",             muse::TranslatableString("engraving", "Let ring") },
    { TextStyleType::PALM_MUTE,         "palm_mute",            muse::TranslatableString("engraving", "Palm mute") },

    { TextStyleType::USER1,             "user_1",               muse::TranslatableString("engraving", "User-1") },
    { TextStyleType::USER2,             "user_2",               muse::TranslatableString("engraving", "User-2") },
    { TextStyleType::USER3,             "user_3",               muse::TranslatableString("engraving", "User-3") },
    { TextStyleType::USER4,             "user_4",               muse::TranslatableString("engraving", "User-4") },
    { TextStyleType::USER5,             "user_5",               muse::TranslatableString("engraving", "User-5") },
    { TextStyleType::USER6,             "user_6",               muse::TranslatableString("engraving", "User-6") },
    { TextStyleType::USER7,             "user_7",               muse::TranslatableString("engraving", "User-7") },
    { TextStyleType::USER8,             "user_8",               muse::TranslatableString("engraving", "User-8") },
    { TextStyleType::USER9,             "user_9",               muse::TranslatableString("engraving", "User-9") },
    { TextStyleType::USER10,            "user_10",              muse::TranslatableString("engraving", "User-10") },
    { TextStyleType::USER11,            "user_11",              muse::TranslatableString("engraving", "User-11") },
    { TextStyleType::USER12,            "user_12",              muse::TranslatableString("engraving", "User-12") },
};

const muse::TranslatableString& TConv::userName(TextStyleType v)
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
        { "Lyricist", TextStyleType::LYRICIST },
        { "Translator", TextStyleType::TRANSLATOR },
        { "Frame", TextStyleType::FRAME },
        { "Instrument Name (Part)", TextStyleType::INSTRUMENT_EXCERPT },
        { "Instrument Name (Long)", TextStyleType::INSTRUMENT_LONG },
        { "Instrument Name (Short)", TextStyleType::INSTRUMENT_SHORT },
        { "Instrument Change", TextStyleType::INSTRUMENT_CHANGE },
        { "Header", TextStyleType::HEADER },
        { "Footer", TextStyleType::FOOTER },
        { "Copyright", TextStyleType::COPYRIGHT },
        { "Page Number", TextStyleType::PAGE_NUMBER },

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
        { "Note-anchored Line", TextStyleType::NOTELINE },
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
        if (muse::RealIsEqual(x, 1.f)) {
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
        return {};
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
    { DurationType::V_QUARTER,  "quarter",  muse::TranslatableString("engraving", "Quarter") },
    { DurationType::V_EIGHTH,   "eighth",   muse::TranslatableString("engraving", "Eighth") },
    { DurationType::V_1024TH,   "1024th",   muse::TranslatableString("engraving", "1024th") },
    { DurationType::V_512TH,    "512th",    muse::TranslatableString("engraving", "512th") },
    { DurationType::V_256TH,    "256th",    muse::TranslatableString("engraving", "256th") },
    { DurationType::V_128TH,    "128th",    muse::TranslatableString("engraving", "128th") },
    { DurationType::V_64TH,     "64th",     muse::TranslatableString("engraving", "64th") },
    { DurationType::V_32ND,     "32nd",     muse::TranslatableString("engraving", "32nd") },
    { DurationType::V_16TH,     "16th",     muse::TranslatableString("engraving", "16th") },
    { DurationType::V_HALF,     "half",     muse::TranslatableString("engraving", "Half") },
    { DurationType::V_WHOLE,    "whole",    muse::TranslatableString("engraving", "Whole") },
    { DurationType::V_MEASURE,  "measure",  muse::TranslatableString("engraving", "Measure") },
    { DurationType::V_BREVE,    "breve",    muse::TranslatableString("engraving", "Breve") },
    { DurationType::V_LONG,     "long",     muse::TranslatableString("engraving", "Longa") },
    { DurationType::V_ZERO,     "",         muse::TranslatableString("engraving", "Zero") },
    { DurationType::V_INVALID,  "",         muse::TranslatableString("engraving", "Invalid") },
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
    { PlayingTechniqueType::Undefined,     "undefined",      muse::TranslatableString::untranslatable("Undefined") },
    { PlayingTechniqueType::Natural,       "natural",        muse::TranslatableString("engraving/playtechtype", "Normal") },
    { PlayingTechniqueType::Pizzicato,     "pizzicato",      muse::TranslatableString("engraving/playtechtype", "Pizzicato") },
    //: For brass and plucked string instruments: staff text that prescribes to play without mute, see https://en.wikipedia.org/wiki/Mute_(music)
    { PlayingTechniqueType::Open,          "open",           muse::TranslatableString("engraving/playtechtype", "Open") },
    //: For brass and plucked string instruments: staff text that prescribes to use mute while playing, see https://en.wikipedia.org/wiki/Mute_(music)
    { PlayingTechniqueType::Mute,          "mute",           muse::TranslatableString("engraving/playtechtype", "Mute") },
    { PlayingTechniqueType::Tremolo,       "tremolo",        muse::TranslatableString("engraving/playtechtype", "Tremolo") },
    { PlayingTechniqueType::Detache,       "detache",        muse::TranslatableString("engraving/playtechtype", "Détaché") },
    { PlayingTechniqueType::Martele,       "martele",        muse::TranslatableString("engraving/playtechtype", "Martelé") },
    { PlayingTechniqueType::ColLegno,      "col_legno",      muse::TranslatableString("engraving/playtechtype", "Col legno") },
    { PlayingTechniqueType::SulPonticello, "sul_ponticello", muse::TranslatableString("engraving/playtechtype", "Sul ponticello") },
    { PlayingTechniqueType::SulTasto,      "sul_tasto",      muse::TranslatableString("engraving/playtechtype", "Sul tasto") },
    { PlayingTechniqueType::Vibrato,       "vibrato",        muse::TranslatableString("engraving/playtechtype", "Vibrato") },
    { PlayingTechniqueType::Legato,        "legato",         muse::TranslatableString("engraving/playtechtype", "Legato") },
    { PlayingTechniqueType::Distortion,    "distortion",     muse::TranslatableString("engraving/playtechtype", "Distortion") },
    { PlayingTechniqueType::Overdrive,     "overdrive",      muse::TranslatableString("engraving/playtechtype", "Overdrive") },
    { PlayingTechniqueType::Harmonics,     "harmonics",      muse::TranslatableString("engraving/playtechtype", "Harmonics") },
    { PlayingTechniqueType::JazzTone,      "jazz_tone",      muse::TranslatableString("engraving/playtechtype", "Jazz tone") },
};

const muse::TranslatableString& TConv::userName(PlayingTechniqueType v)
{
    return findUserNameByType<PlayingTechniqueType>(PLAY_TECH_TYPES, v);
}

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
    { GradualTempoChangeType::Accelerando, "accelerando", muse::TranslatableString("engraving/gradualtempochangetype", "accel.") },
    { GradualTempoChangeType::Allargando, "allargando", muse::TranslatableString("engraving/gradualtempochangetype", "allarg.") },
    { GradualTempoChangeType::Calando, "calando", muse::TranslatableString("engraving/gradualtempochangetype", "calando") },
    { GradualTempoChangeType::Lentando, "lentando", muse::TranslatableString("engraving/gradualtempochangetype", "lentando") },
    { GradualTempoChangeType::Morendo, "morendo", muse::TranslatableString("engraving/gradualtempochangetype", "morendo") },
    { GradualTempoChangeType::Precipitando, "precipitando", muse::TranslatableString("engraving/gradualtempochangetype", "precipitando") },
    { GradualTempoChangeType::Rallentando, "rallentando", muse::TranslatableString("engraving/gradualtempochangetype", "rall.") },
    { GradualTempoChangeType::Ritardando, "ritardando", muse::TranslatableString("engraving/gradualtempochangetype", "rit.") },
    { GradualTempoChangeType::Smorzando, "smorzando", muse::TranslatableString("engraving/gradualtempochangetype", "smorz.") },
    { GradualTempoChangeType::Sostenuto, "sostenuto", muse::TranslatableString("engraving/gradualtempochangetype", "sost.") },
    { GradualTempoChangeType::Stringendo, "stringendo", muse::TranslatableString("engraving/gradualtempochangetype", "string.") }
};

const muse::TranslatableString& TConv::userName(GradualTempoChangeType v)
{
    return findUserNameByType<GradualTempoChangeType>(TEMPO_CHANGE_TYPES, v);
}

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
    { PlacementH::LEFT,   "left" },
    { PlacementH::RIGHT,  "center" },
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
    { TextPlace::AUTO,  "auto" },
    { TextPlace::ABOVE, "above" },
    { TextPlace::BELOW, "below" },
    { TextPlace::LEFT,  "left" }
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
        { TextPlace::AUTO,  "0" },
        { TextPlace::ABOVE, "1" },
        { TextPlace::BELOW, "2" },
        { TextPlace::LEFT,  "3" }
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
    { DirectionV::AUTO, "auto",     muse::TranslatableString("engraving", "Auto") },
    { DirectionV::UP,   "up",       muse::TranslatableString("engraving", "Up") },
    { DirectionV::DOWN, "down",     muse::TranslatableString("engraving", "Down") },
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
    { DirectionH::AUTO,  "auto",  muse::TranslatableString("engraving", "Auto") },
    { DirectionH::RIGHT, "right", muse::TranslatableString("engraving", "Right") },
    { DirectionH::LEFT,  "left",  muse::TranslatableString("engraving", "Left") },
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
    { LayoutBreakType::LINE,    "line",    muse::TranslatableString("engraving/layoutbreaktype", "System break") },
    { LayoutBreakType::PAGE,    "page",    muse::TranslatableString("engraving/layoutbreaktype", "Page break") },
    { LayoutBreakType::SECTION, "section", muse::TranslatableString("engraving/layoutbreaktype", "Section break") },
    { LayoutBreakType::NOBREAK, "nobreak", muse::TranslatableString("engraving/layoutbreaktype", "Keep measures on the same system") }
};

const muse::TranslatableString& TConv::userName(LayoutBreakType v)
{
    return findUserNameByType<LayoutBreakType>(LAYOUTBREAK_TYPES, v);
}

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
    { BeamMode::BEGIN16, "begin16" },
    { BeamMode::BEGIN32, "begin32" },
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
    { TremoloType::R8,              "r8",       muse::TranslatableString("engraving/tremolotype", "Eighth through stem") },
    { TremoloType::R16,             "r16",      muse::TranslatableString("engraving/tremolotype", "16th through stem") },
    { TremoloType::R32,             "r32",      muse::TranslatableString("engraving/tremolotype", "32nd through stem") },
    { TremoloType::R64,             "r64",      muse::TranslatableString("engraving/tremolotype", "64th through stem") },
    { TremoloType::BUZZ_ROLL,       "buzzroll", muse::TranslatableString("engraving/tremolotype", "Buzz roll") },
    { TremoloType::C8,              "c8",       muse::TranslatableString("engraving/tremolotype", "Eighth between notes") },
    { TremoloType::C16,             "c16",      muse::TranslatableString("engraving/tremolotype", "16th between notes") },
    { TremoloType::C32,             "c32",      muse::TranslatableString("engraving/tremolotype", "32nd between notes") },
    { TremoloType::C64,             "c64",      muse::TranslatableString("engraving/tremolotype", "64th between notes") }
} };

const muse::TranslatableString& TConv::userName(TremoloType v)
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
    { BracketType::NORMAL,     "Normal",    muse::TranslatableString("engraving/brackettype", "Normal") },
    { BracketType::BRACE,      "Brace",     muse::TranslatableString("engraving/brackettype", "Brace") },
    { BracketType::SQUARE,     "Square",    muse::TranslatableString("engraving/brackettype", "Square") },
    { BracketType::LINE,       "Line",      muse::TranslatableString("engraving/brackettype", "Line") },
    { BracketType::NO_BRACKET, "NoBracket", muse::TranslatableString("engraving/brackettype", "No bracket") }
};

const muse::TranslatableString& TConv::userName(BracketType v)
{
    return findUserNameByType<BracketType>(BRACKET_TYPES, v);
}

String TConv::translatedUserName(BracketType v)
{
    return findUserNameByType<BracketType>(BRACKET_TYPES, v).translated();
}

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
    { ArpeggioType::NORMAL,         "0",     muse::TranslatableString("engraving", "Arpeggio") },
    { ArpeggioType::UP,             "1",     muse::TranslatableString("engraving", "Up arpeggio") },
    { ArpeggioType::DOWN,           "2",     muse::TranslatableString("engraving", "Down arpeggio") },
    { ArpeggioType::BRACKET,        "3",     muse::TranslatableString("engraving", "Bracket arpeggio") },
    { ArpeggioType::UP_STRAIGHT,    "4",     muse::TranslatableString("engraving", "Up arpeggio straight") },
    { ArpeggioType::DOWN_STRAIGHT,  "5",     muse::TranslatableString("engraving", "Down arpeggio straight") }
} };

const muse::TranslatableString& TConv::userName(ArpeggioType v)
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
    muse::TranslatableString name;
    AsciiStringView notes;
};

// TODO: Can't use .arg, because Palettes use these strings and doesn't support TranslatableString
static const std::vector<EmbelItem> EMBELLISHMENT_TYPES = {
    // Single Grace notes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace low G"), "LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace low A"), "LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace B"), "B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace C"), "C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace D"), "D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace E"), "E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace F"), "F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace high G"), "HG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Single grace high A"), "HA" },

    // Double Grace notes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "E D" },

    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "F E" },

    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HG F" },

    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA B" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double grace"), "HA HG" },

    // Half Doublings
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on low G"), "LG D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on low A"), "LA D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on B"), "B D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on C"), "C D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on D"), "D E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on E"), "E F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on F"), "F HG" },
    // ? { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on high G"), "HG F" },
    // ? { muse::TranslatableString("engraving/bagpipeembellishment", "Half doubling on high A"), "HA HG" },

    // Regular Doublings
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on high G"), "HG F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on high A"), "HA HG" },

    // Half Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on low A"), "LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on B"), "B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on C"), "C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on D"), "D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on D"), "D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on E"), "E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on F"), "F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half strike on high G"), "HG F" },

    // Regular Grip
    { muse::TranslatableString("engraving/bagpipeembellishment", "Grip"), "D LG" },

    // D Throw
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half D throw"), "D C" },

    // Regular Doublings (continued)
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on low G"),  "HG LG D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on low A"),  "HG LA D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on B"),      "HG B D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on C"),      "HG C D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on D"),      "HG D E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on E"),      "HG E F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Doubling on F"),      "HG F HG" },

    // Thumb Doublings
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on low G"), "HA LG D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on low A"), "HA LA D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on B"), "HA B D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on C"), "HA C D" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on D"), "HA D E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on E"), "HA E F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on F"), "HA F HG" },
    // ? { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb doubling on high G"), "HA HG F" },

    // G Grace note Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on low A"), "HG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on B"), "HG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on C"), "HG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on D"), "HG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on D"), "HG D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on E"), "HG E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note on F"), "HG F E" },

    // Regular Double Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on low A"), "LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on B"), "LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on C"), "LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on D"), "LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on D"), "C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on E"), "LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on F"), "E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on high G"), "F HG F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Double strike on high A"), "HG HA HG" },

    // Thumb Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on low A"), "HA LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on B"), "HA B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on C"), "HA C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on D"), "HA D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on D"), "HA D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on E"), "HA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on F"), "HA F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb strike on high G"), "HA HG F" },

    // Regular Grips (continued)
    { muse::TranslatableString("engraving/bagpipeembellishment", "Grip"), "LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Grip"), "LG B LG" },

    // Taorluath and Birl
    { muse::TranslatableString("engraving/bagpipeembellishment", "Birl"), "LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "D throw"), "LG D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half heavy D throw"), "D LG C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "D LG E" },

    // Birl, Bubbly, D Throws (cont/bagpipeembellishmentinued) and Taorluaths (continued)
    { muse::TranslatableString("engraving/bagpipeembellishment", "Birl"), "LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Bubbly"), "D LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Heavy D throw"), "LG D LG C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "LG D LG E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Taorluath"), "LG B LG E" },

    // Half Double Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on low A"), "LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on B"), "B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on C"), "C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on D"), "D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on D"), "D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on E"), "E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on F"), "F E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on high G"), "HG F HG F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half double strike on high A"), "HA HG HA HG" },

    // Half Grips
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on low A"), "LA LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on B"), "B LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on C"), "C LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on D"), "D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on D"), "D LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on E"), "E LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on F"), "F LG F LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on high G"), "HG LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half grip on high A"), "HA LG D LG" },

    // Half Peles
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on low A"), "LA E LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on B"), "B E B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on C"), "C E C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on D"), "D E D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on D"), "D E D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on E"), "E F E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on F"), "F HG F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half pele on high G"), "HG HA HG F" },

    // G Grace note Grips
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on low A"), "HG LA LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on B"), "HG B LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on C"), "HG C LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on D"), "HG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on D"), "HG D LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on E"), "HG E LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note grip on F"), "HG F LG F LG" },

    // Thumb Grips
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on low A"), "HA LA LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on B"), "HA B LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on C"), "HA C LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on D"), "HA D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on D"), "HA D LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on E"), "HA E LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on F"), "HA F LG F LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grip on high G"), "HA HG LG F LG" },

    // Bubbly
    { muse::TranslatableString("engraving/bagpipeembellishment", "Bubbly"), "LG D LG C LG" },

    //  Birls
    { muse::TranslatableString("engraving/bagpipeembellishment", "Birl"), "HG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Birl"), "HA LA LG LA LG" },

    // Regular Peles
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on low A"), "HG LA E LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on B"), "HG B E B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on C"), "HG C E C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on D"), "HG D E D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on D"), "HG D E D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on E"), "HG E F E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Pele on F"), "HG F HG F E" },

    // Thumb Grace Note Peles
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on low A"), "HA LA E LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on B"), "HA B E B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on C"), "HA C E C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on D"), "HA D E D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on D"), "HA D E D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on E"), "HA E F E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on F"), "HA F HG F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb grace note pele on high G"), "HA HG HA HG F" },

    // G Grace note Double Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on low A"), "HG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on B"), "HG B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on C"), "HG C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on D"), "HG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on D"), "HG D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on E"), "HG E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note double strike on F"), "HG F E F E" },

    // Thumb Double Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on low A"), "HA LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on B"), "HA B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on C"), "HA C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on D"), "HA D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on D"), "HA D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on E"), "HA E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on F"), "HA F E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb double strike on high G"), "HA HG F HG F" },

    // Regular Triple Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on low A"), "LG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on B"), "LG B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on C"), "LG C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on D"), "LG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on D"), "C D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on E"), "LA E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on F"), "E F E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on high G"), "F HG F HG F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Triple strike on high A"), "HG HA HG HA HG" },

    // Half Triple Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on low A"), "LA LG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on B"), "B LG B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on C"), "C LG C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on D"), "D LG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on D"), "D C D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on E"), "E LA E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on F"), "F E F E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on high G"), "HG F HG F HG F" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Half triple strike on high A"), "HA HG HA HG HA HG" },

    // G Grace note Triple Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on low A"), "HG LA LG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on B"), "HG B LG B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on C"), "HG C LG C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on D"), "HG D LG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on D"), "HG D C D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on E"), "HG E LA E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "G grace note triple strike on F"), "HG F E F E F E" },

    // Thumb Triple Strikes
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on low A"),  "HA LA LG LA LG LA LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on B"),      "HA B LG B LG B LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on C"),      "HA C LG C LG C LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on D"),      "HA D LG D LG D LG" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on D"),      "HA D C D C D C" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on E"),      "HA E LA E LA E LA" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on F"),      "HA F E F E F E" },
    { muse::TranslatableString("engraving/bagpipeembellishment", "Thumb triple strike on high G"), "HA HG F HG F HG F" },
};

const muse::TranslatableString& TConv::userName(EmbellishmentType v)
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

struct ChordLineNameType {
    ChordLineType type;
    bool straight;
    bool wavy;

    bool operator==(const ChordLineNameType& other) const
    {
        return type == other.type && straight == other.straight && wavy == other.wavy;
    }
};

//! TODO Add xml names
static const std::array<Item<ChordLineNameType>, 15> CHORDLINE_TYPES = { {
    { { ChordLineType::NOTYPE, false, false },    "0" },
    { { ChordLineType::FALL, false, false },      "1",     muse::TranslatableString("engraving", "Fall") },
    { { ChordLineType::DOIT, false, false },      "2",     muse::TranslatableString("engraving", "Doit") },
    { { ChordLineType::PLOP, false, false },      "3",     muse::TranslatableString("engraving", "Plop") },
    { { ChordLineType::SCOOP, false, false },     "4",     muse::TranslatableString("engraving", "Scoop") },
    { { ChordLineType::NOTYPE, true, false },     "0" },
    { { ChordLineType::FALL, true, false },       "1",     muse::TranslatableString("engraving", "Slide out down") },
    { { ChordLineType::DOIT, true, false },       "2",     muse::TranslatableString("engraving", "Slide out up") },
    { { ChordLineType::PLOP, true, false },       "3",     muse::TranslatableString("engraving", "Slide in above") },
    { { ChordLineType::SCOOP, true, false },      "4",     muse::TranslatableString("engraving", "Slide in below") },
    { { ChordLineType::NOTYPE, true, true },      "0" },
    { { ChordLineType::FALL, true, true },        "1",     muse::TranslatableString("engraving", "Slide out down (rough)") },
    { { ChordLineType::DOIT, true, true },        "2",     muse::TranslatableString("engraving", "Slide out up (rough)") },
    { { ChordLineType::PLOP, true, true },        "3",     muse::TranslatableString("engraving", "Slide in above (rough)") },
    { { ChordLineType::SCOOP, true, true },       "4",     muse::TranslatableString("engraving", "Slide in below (rough)") }
} };

const muse::TranslatableString& TConv::userName(ChordLineType v, bool straight, bool wavy)
{
    return findUserNameByType<ChordLineNameType>(CHORDLINE_TYPES, { v, straight, wavy });
}

AsciiStringView TConv::toXml(ChordLineType v)
{
    return findXmlTagByType<ChordLineNameType>(CHORDLINE_TYPES, { v, false, false });
}

ChordLineType TConv::fromXml(const AsciiStringView& tag, ChordLineType def)
{
    return findTypeByXmlTag<ChordLineNameType>(CHORDLINE_TYPES, tag, { def, false, false }).type;
}

struct DrumPitchItem {
    DrumNum num = DrumNum(0);
    String userName;
};

// TODO: Can't use TranslatableString, because Drumset uses these strings and doesn't support TranslatableString
static const std::vector<DrumPitchItem> DRUMPITCHS = {
    { DrumNum(27),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "High Q") },
    { DrumNum(28),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Slap") },
    { DrumNum(29),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Scratch Push") },

    { DrumNum(30),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Scratch Pull") },
    { DrumNum(31),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Sticks") },
    { DrumNum(32),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Square Click") },
    { DrumNum(33),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Metronome Click") },
    { DrumNum(34),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Metronome Bell") },
    { DrumNum(35),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Acoustic Bass Drum") },
    { DrumNum(36),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Bass Drum 1") },
    { DrumNum(37),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Side Stick") },
    { DrumNum(38),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Acoustic Snare") },
    { DrumNum(39),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Hand Clap") },

    { DrumNum(40),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Electric Snare") },
    { DrumNum(41),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Floor Tom") },
    { DrumNum(42),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Closed Hi-Hat") },
    { DrumNum(43),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "High Floor Tom") },
    { DrumNum(44),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Pedal Hi-Hat") },
    { DrumNum(45),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Tom") },
    { DrumNum(46),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Open Hi-Hat") },
    { DrumNum(47),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low-Mid Tom") },
    { DrumNum(48),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Hi-Mid Tom") },
    { DrumNum(49),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Crash Cymbal 1") },

    { DrumNum(50),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "High Tom") },
    { DrumNum(51),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Ride Cymbal 1") },
    { DrumNum(52),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Chinese Cymbal") },
    { DrumNum(53),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Ride Bell") },
    { DrumNum(54),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Tambourine") },
    { DrumNum(55),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Splash Cymbal") },
    { DrumNum(56),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Cowbell") },
    { DrumNum(57),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Crash Cymbal 2") },
    { DrumNum(58),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Vibraslap") },
    { DrumNum(59),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Ride Cymbal 2") },

    { DrumNum(60),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Hi Bongo") },
    { DrumNum(61),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Bongo") },
    { DrumNum(62),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Mute Hi Conga") },
    { DrumNum(63),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Open Hi Conga") },
    { DrumNum(64),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Conga") },
    { DrumNum(65),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "High Timbale") },
    { DrumNum(66),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Timbale") },
    { DrumNum(67),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "High Agogo") },
    { DrumNum(68),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Agogo") },
    { DrumNum(69),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Cabasa") },

    { DrumNum(70),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Maracas") },
    { DrumNum(71),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Short Whistle") },
    { DrumNum(72),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Long Whistle") },
    { DrumNum(73),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Short Güiro") },
    { DrumNum(74),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Long Güiro") },
    { DrumNum(75),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Claves") },
    { DrumNum(76),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Hi Wood Block") },
    { DrumNum(77),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Low Wood Block") },
    { DrumNum(78),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Mute Cuica") },
    { DrumNum(79),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Open Cuica") },

    { DrumNum(80),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Mute Triangle") },
    { DrumNum(81),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Open Triangle") },
    { DrumNum(82),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Shaker") },
    { DrumNum(83),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Sleigh Bell") },
    { DrumNum(84),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Mark Tree") },
    { DrumNum(85),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Castanets") },
    { DrumNum(86),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Mute Surdo") },
    { DrumNum(87),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Open Surdo") },

    { DrumNum(91),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Snare (Rim shot)") },

    { DrumNum(93),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Ride (Edge)") },

    { DrumNum(99),       QT_TRANSLATE_NOOP_U16("engraving/drumset", "Cowbell Low") },

    { DrumNum(102),      QT_TRANSLATE_NOOP_U16("engraving/drumset", "Cowbell High") },
};

const String& TConv::userName(DrumNum v)
{
    auto it = std::find_if(DRUMPITCHS.cbegin(), DRUMPITCHS.cend(), [v](const DrumPitchItem& i) {
        return i.num == v;
    });

    IF_ASSERT_FAILED(it != DRUMPITCHS.cend()) {
        static const String dummy;
        return dummy;
    }
    return it->userName;
}

//! TODO Add xml names
static const std::array<Item<GlissandoType>, 2> GLISSANDO_TYPES = { {
    { GlissandoType::STRAIGHT,  "0",     muse::TranslatableString("engraving", "Straight glissando") },
    { GlissandoType::WAVY,      "1",     muse::TranslatableString("engraving", "Wavy glissando") }
} };

const muse::TranslatableString& TConv::userName(GlissandoType v)
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
    { JumpType::DC,             "", muse::TranslatableString("engraving", "Da Capo") },
    { JumpType::DC_AL_FINE,     "", muse::TranslatableString("engraving", "Da Capo al Fine") },
    { JumpType::DC_AL_CODA,     "", muse::TranslatableString("engraving", "Da Capo al Coda") },
    { JumpType::DS_AL_CODA,     "", muse::TranslatableString("engraving", "D.S. al Coda") },
    { JumpType::DS_AL_FINE,     "", muse::TranslatableString("engraving", "D.S. al Fine") },
    { JumpType::DS,             "", muse::TranslatableString("engraving", "D.S.") },

    { JumpType::DC_AL_DBLCODA,  "", muse::TranslatableString("engraving", "Da Capo al Doppia Coda") },
    { JumpType::DS_AL_DBLCODA,  "", muse::TranslatableString("engraving", "Dal Segno al Doppia Coda") },
    { JumpType::DSS,            "", muse::TranslatableString("engraving", "Dal Doppio Segno") },
    { JumpType::DSS_AL_CODA,    "", muse::TranslatableString("engraving", "Dal Doppio Segno al Coda") },
    { JumpType::DSS_AL_DBLCODA, "", muse::TranslatableString("engraving", "Dal Doppio Segno al Doppia Coda") },
    { JumpType::DSS_AL_FINE,    "", muse::TranslatableString("engraving", "Dal Doppio Segno al Fine") },

    { JumpType::USER,           "", muse::TranslatableString("engraving", "Custom") }
};

const muse::TranslatableString& TConv::userName(JumpType v)
{
    return findUserNameByType<JumpType>(JUMP_TYPES, v);
}

String TConv::translatedUserName(JumpType v)
{
    return findUserNameByType<JumpType>(JUMP_TYPES, v).translated();
}

static const std::array<Item<MarkerType>, 11> MARKER_TYPES = { {
    { MarkerType::SEGNO,        "segno",    muse::TranslatableString("engraving", "Segno") },
    { MarkerType::VARSEGNO,     "varsegno", muse::TranslatableString("engraving", "Segno variation") },
    { MarkerType::CODA,         "codab",    muse::TranslatableString("engraving", "Coda") },
    { MarkerType::VARCODA,      "varcoda",  muse::TranslatableString("engraving", "Varied coda") },
    { MarkerType::CODETTA,      "codetta",  muse::TranslatableString("engraving", "Doppia Coda") },
    { MarkerType::FINE,         "fine",     muse::TranslatableString("engraving", "Fine") },
    { MarkerType::TOCODA,       "coda",     muse::TranslatableString("engraving", "To coda") },
    { MarkerType::TOCODASYM,    "",         muse::TranslatableString("engraving", "To coda (symbol)") },
    { MarkerType::DA_CODA,      "",         muse::TranslatableString("engraving", "Da Coda") },
    { MarkerType::DA_DBLCODA,   "",         muse::TranslatableString("engraving", "Da Doppia Coda") },
    { MarkerType::USER,         "",         muse::TranslatableString("engraving", "Custom") }
} };

const muse::TranslatableString& TConv::userName(MarkerType v)
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
    { StaffGroup::STANDARD,     "pitched",    muse::TranslatableString("engraving/staffgroup", "Standard") },
    { StaffGroup::PERCUSSION,   "percussion", muse::TranslatableString("engraving/staffgroup", "Percussion") },
    { StaffGroup::TAB,          "tablature",  muse::TranslatableString("engraving/staffgroup", "Tablature") },
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
    { TrillType::TRILL_LINE,      "trill",      muse::TranslatableString("engraving/trilltype", "Trill line") },
    { TrillType::UPPRALL_LINE,    "upprall",    muse::TranslatableString("engraving/trilltype", "Upprall line") },
    { TrillType::DOWNPRALL_LINE,  "downprall",  muse::TranslatableString("engraving/trilltype", "Downprall line") },
    { TrillType::PRALLPRALL_LINE, "prallprall", muse::TranslatableString("engraving/trilltype", "Prallprall line") }
} };

const muse::TranslatableString& TConv::userName(TrillType v)
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
    { VibratoType::GUITAR_VIBRATO,        "guitarVibrato",       muse::TranslatableString("engraving/vibratotype", "Guitar vibrato") },
    { VibratoType::GUITAR_VIBRATO_WIDE,   "guitarVibratoWide",   muse::TranslatableString("engraving/vibratotype", "Guitar vibrato wide") },
    { VibratoType::VIBRATO_SAWTOOTH,      "vibratoSawtooth",     muse::TranslatableString("engraving/vibratotype", "Vibrato sawtooth") },
    { VibratoType::VIBRATO_SAWTOOTH_WIDE, "vibratoSawtoothWide",
      muse::TranslatableString("engraving/vibratotype", "Tremolo sawtooth wide") }
} };

const muse::TranslatableString& TConv::userName(VibratoType v)
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

// Note about "engraving/sym": they need to be in this context because PaletteCell::translationContext expects them there
struct ArticulationTextTypeItem {
    ArticulationTextType type;
    AsciiStringView xml;
    String text;
    muse::TranslatableString name;
};

const std::array<ArticulationTextTypeItem, 3> ARTICULATIONTEXT_TYPES = { {
    { ArticulationTextType::TAP,    "Tap",  String(u"T"),  muse::TranslatableString("engraving/sym", "Tap") },
    { ArticulationTextType::SLAP,   "Slap", String(u"S"),  muse::TranslatableString("engraving/sym", "Slap") },
    { ArticulationTextType::POP,    "Pop",  String(u"P"),  muse::TranslatableString("engraving/sym", "Pop") }
} };

const muse::TranslatableString& TConv::userName(ArticulationTextType v)
{
    auto it = std::find_if(ARTICULATIONTEXT_TYPES.cbegin(), ARTICULATIONTEXT_TYPES.cend(), [v](const ArticulationTextTypeItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != ARTICULATIONTEXT_TYPES.cend()) {
        static muse::TranslatableString dummy;
        return dummy;
    }
    return it->name;
}

String TConv::text(ArticulationTextType v)
{
    auto it = std::find_if(ARTICULATIONTEXT_TYPES.cbegin(), ARTICULATIONTEXT_TYPES.cend(), [v](const ArticulationTextTypeItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != ARTICULATIONTEXT_TYPES.cend()) {
        static String dummy;
        return dummy;
    }
    return it->text;
}

AsciiStringView TConv::toXml(ArticulationTextType v)
{
    auto it = std::find_if(ARTICULATIONTEXT_TYPES.cbegin(), ARTICULATIONTEXT_TYPES.cend(), [v](const ArticulationTextTypeItem& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != ARTICULATIONTEXT_TYPES.cend()) {
        static AsciiStringView dummy;
        return dummy;
    }
    return it->xml;
}

ArticulationTextType TConv::fromXml(const AsciiStringView& tag, ArticulationTextType def)
{
    auto it = std::find_if(ARTICULATIONTEXT_TYPES.cbegin(), ARTICULATIONTEXT_TYPES.cend(), [tag](const ArticulationTextTypeItem& i) {
        return i.xml == tag;
    });

    if (it != ARTICULATIONTEXT_TYPES.cend()) {
        return it->type;
    }

    // compatibility
    return def;
}

const std::array<Item<LyricsSyllabic>, 4> LYRICSSYLLABIC_TYPES = { {
    { LyricsSyllabic::SINGLE,   "single" },
    { LyricsSyllabic::BEGIN,    "begin" },
    { LyricsSyllabic::END,      "end" },
    { LyricsSyllabic::MIDDLE,   "middle" }
} };

AsciiStringView TConv::toXml(LyricsSyllabic v)
{
    return findXmlTagByType<LyricsSyllabic>(LYRICSSYLLABIC_TYPES, v);
}

LyricsSyllabic TConv::fromXml(const AsciiStringView& tag, LyricsSyllabic def)
{
    return findTypeByXmlTag<LyricsSyllabic>(LYRICSSYLLABIC_TYPES, tag, def);
}

const std::array<Item<LyricsDashSystemStart>, 3> LYRICS_DASH_SYSTEM_START_TYPES = { {
    { LyricsDashSystemStart::STANDARD,   "standard" },
    { LyricsDashSystemStart::UNDER_HEADER,   "underHeader" },
    { LyricsDashSystemStart::UNDER_FIRST_NOTE,   "underFirstNote" },
} };

AsciiStringView TConv::toXml(LyricsDashSystemStart v)
{
    return findXmlTagByType<LyricsDashSystemStart>(LYRICS_DASH_SYSTEM_START_TYPES, v);
}

LyricsDashSystemStart TConv::fromXml(const AsciiStringView& tag, LyricsDashSystemStart def)
{
    return findTypeByXmlTag<LyricsDashSystemStart>(LYRICS_DASH_SYSTEM_START_TYPES, tag, def);
}

const std::array<const muse::TranslatableString, 17 > KEY_NAMES = { {
    muse::TranslatableString("engraving", "C♭ major, A♭ minor"),
    muse::TranslatableString("engraving", "G♭ major, E♭ minor"),
    muse::TranslatableString("engraving", "D♭ major, B♭ minor"),
    muse::TranslatableString("engraving", "A♭ major, F minor"),
    muse::TranslatableString("engraving", "E♭ major, C minor"),
    muse::TranslatableString("engraving", "B♭ major, G minor"),
    muse::TranslatableString("engraving", "F major, D minor"),
    muse::TranslatableString("engraving", "C major, A minor"),
    muse::TranslatableString("engraving", "G major, E minor"),
    muse::TranslatableString("engraving", "D major, B minor"),
    muse::TranslatableString("engraving", "A major, F♯ minor"),
    muse::TranslatableString("engraving", "E major, C♯ minor"),
    muse::TranslatableString("engraving", "B major, G♯ minor"),
    muse::TranslatableString("engraving", "F♯ major, D♯ minor"),
    muse::TranslatableString("engraving", "C♯ major, A♯ minor"),
    muse::TranslatableString("engraving", "Open/Atonal"),
    muse::TranslatableString("engraving", "Custom")
} };

const muse::TranslatableString& TConv::userName(Key v, bool isAtonal, bool isCustom)
{
    if (isAtonal) {
        return KEY_NAMES[15];
    } else if (isCustom) {
        return KEY_NAMES[16];
    }

    int keyInt = static_cast<int>(v);
    return KEY_NAMES[keyInt + 7];
}

String TConv::translatedUserName(Key v, bool isAtonal, bool isCustom)
{
    return userName(v, isAtonal, isCustom).translated();
}
