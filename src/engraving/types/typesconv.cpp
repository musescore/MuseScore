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

using namespace mu::engraving;
using namespace Ms;

template<typename T>
struct Item
{
    T type;
    QString xml;
    const char* userName = nullptr;
    SymId symId = SymId::noSym;
};

template<typename T>
static QString findUserNameByType(const std::vector<Item<T> >& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static QString dummy;
        return dummy;
    }

    if (!it->userName) {
        return it->xml;
    }

    return mu::qtrc("engraving", it->userName);
}

template<typename T>
static SymId findSymIdByType(const std::vector<Item<T> >& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        return SymId::noSym;
    }
    return it->symId;
}

template<typename T>
static QString findXmlTagByType(const std::vector<Item<T> >& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static QString dummy;
        return dummy;
    }
    return it->xml;
}

template<typename T>
static T findTypeByXmlTag(const std::vector<Item<T> >& cont, const QString& tag, T def)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [tag](const Item<T>& i) {
        return i.xml == tag;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        return def;
    }
    return it->type;
}

// ==========================================================
QString TConv::toUserName(SymId v)
{
    return SymNames::translatedUserNameForSymId(v);
}

QString TConv::toXml(SymId v)
{
    return SymNames::nameForSymId(v);
}

SymId TConv::fromXml(const QString& tag, SymId def)
{
    return SymNames::symIdByName(tag, def);
}

static const std::vector<Item<Orientation> > ORIENTATION = {
    { Orientation::VERTICAL,    "vertical",     QT_TRANSLATE_NOOP("engraving", "Vertical") },
    { Orientation::HORIZONTAL,  "horizontal",   QT_TRANSLATE_NOOP("engraving", "Horizontal") },
};

QString TConv::toUserName(Orientation v)
{
    return findUserNameByType<Orientation>(ORIENTATION, v);
}

QString TConv::toXml(Orientation v)
{
    return findXmlTagByType<Orientation>(ORIENTATION, v);
}

Orientation TConv::fromXml(const QString& tag, Orientation def)
{
    return findTypeByXmlTag<Orientation>(ORIENTATION, tag, def);
}

static const std::vector<Item<NoteHeadType> > NOTEHEAD_TYPES = {
    { NoteHeadType::HEAD_AUTO,      "auto",    QT_TRANSLATE_NOOP("engraving", "Auto") },
    { NoteHeadType::HEAD_WHOLE,     "whole",   QT_TRANSLATE_NOOP("engraving", "Whole") },
    { NoteHeadType::HEAD_HALF,      "half",    QT_TRANSLATE_NOOP("engraving", "Half") },
    { NoteHeadType::HEAD_QUARTER,   "quarter", QT_TRANSLATE_NOOP("engraving", "Quarter") },
    { NoteHeadType::HEAD_BREVIS,    "breve",   QT_TRANSLATE_NOOP("engraving", "Breve") },
};

QString TConv::toUserName(NoteHeadType v)
{
    return findUserNameByType<NoteHeadType>(NOTEHEAD_TYPES, v);
}

QString TConv::toXml(NoteHeadType v)
{
    return findXmlTagByType<NoteHeadType>(NOTEHEAD_TYPES, v);
}

NoteHeadType TConv::fromXml(const QString& tag, NoteHeadType def)
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

QString TConv::toUserName(NoteHeadScheme v)
{
    return findUserNameByType<NoteHeadScheme>(NOTEHEAD_SCHEMES, v);
}

QString TConv::toXml(NoteHeadScheme v)
{
    return findXmlTagByType<NoteHeadScheme>(NOTEHEAD_SCHEMES, v);
}

NoteHeadScheme TConv::fromXml(const QString& tag, NoteHeadScheme def)
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
    { NoteHeadGroup::HEAD_RE_NAME,      "re-name",  QT_TRANSLATE_NOOP("engraving",  "Re (Name)") },
    { NoteHeadGroup::HEAD_MI_NAME,      "mi-name",  QT_TRANSLATE_NOOP("engraving",  "Mi (Name)") },
    { NoteHeadGroup::HEAD_FA_NAME,      "fa-name",  QT_TRANSLATE_NOOP("engraving",  "Fa (Name)") },
    { NoteHeadGroup::HEAD_SOL_NAME,     "sol-name", QT_TRANSLATE_NOOP("engraving",  "Sol (Name)") },
    { NoteHeadGroup::HEAD_LA_NAME,      "la-name",  QT_TRANSLATE_NOOP("engraving",  "La (Name)") },
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

QString TConv::toUserName(NoteHeadGroup v)
{
    return findUserNameByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v);
}

QString TConv::toXml(NoteHeadGroup v)
{
    return findXmlTagByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v);
}

NoteHeadGroup TConv::fromXml(const QString& tag, NoteHeadGroup def)
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

QString TConv::toUserName(ClefType v)
{
    return findUserNameByType<ClefType>(CLEF_TYPES, v);
}

QString TConv::toXml(ClefType v)
{
    return findXmlTagByType<ClefType>(CLEF_TYPES, v);
}

ClefType TConv::fromXml(const QString& tag, ClefType def)
{
    return findTypeByXmlTag<ClefType>(CLEF_TYPES, tag, def);
}

static const std::vector<Item<DynamicType> > DYNAMIC_TYPES = {
    { DynamicType::OTHER,   "other-dynamics",   nullptr, SymId::noSym },
    { DynamicType::PPPPPP,  "pppppp",           nullptr, SymId::dynamicPPPPPP },
    { DynamicType::PPPPP,   "ppppp",            nullptr, SymId::dynamicPPPPP },
    { DynamicType::PPPP,    "pppp",             nullptr, SymId::dynamicPPPP },
    { DynamicType::PPP,     "ppp",              nullptr, SymId::dynamicPPP },
    { DynamicType::PP,      "pp",               nullptr, SymId::dynamicPP },
    { DynamicType::P,       "p",                nullptr, SymId::dynamicPiano },

    { DynamicType::MP,      "mp",               nullptr, SymId::dynamicMP },
    { DynamicType::MF,      "mf",               nullptr, SymId::dynamicMF },

    { DynamicType::F,       "f",                nullptr, SymId::dynamicForte },
    { DynamicType::FF,      "ff",               nullptr, SymId::dynamicFF },
    { DynamicType::FFF,     "fff",              nullptr, SymId::dynamicFFF },
    { DynamicType::FFFF,    "ffff",             nullptr, SymId::dynamicFFFF },
    { DynamicType::FFFFF,   "fffff",            nullptr, SymId::dynamicFFFFF },
    { DynamicType::FFFFFF,  "ffffff",           nullptr, SymId::dynamicFFFFFF },

    { DynamicType::FP,      "fp",               nullptr, SymId::dynamicFortePiano },
    { DynamicType::PF,      "pf",               nullptr, SymId::noSym },

    { DynamicType::SF,      "sf",               nullptr, SymId::dynamicSforzando1 },
    { DynamicType::SFZ,     "sfz",              nullptr, SymId::dynamicSforzato },
    { DynamicType::SFF,     "sff",              nullptr, SymId::noSym },
    { DynamicType::SFFZ,    "sffz",             nullptr, SymId::dynamicSforzatoFF },
    { DynamicType::SFP,     "sfp",              nullptr, SymId::dynamicSforzandoPiano },
    { DynamicType::SFPP,    "sfpp",             nullptr, SymId::dynamicSforzandoPianissimo },

    { DynamicType::RFZ,     "rfz",              nullptr, SymId::dynamicRinforzando2 },
    { DynamicType::RF,      "rf",               nullptr, SymId::dynamicRinforzando1 },
    { DynamicType::FZ,      "fz",               nullptr, SymId::dynamicForzando },
    { DynamicType::M,       "m",                nullptr, SymId::dynamicMezzo },
    { DynamicType::R,       "r",                nullptr, SymId::dynamicRinforzando },
    { DynamicType::S,       "s",                nullptr, SymId::dynamicSforzando },
    { DynamicType::Z,       "z",                nullptr, SymId::dynamicZ },
    { DynamicType::N,       "n",                nullptr, SymId::dynamicNiente },
};

QString TConv::toUserName(DynamicType v)
{
    return findUserNameByType<DynamicType>(DYNAMIC_TYPES, v);
}

SymId TConv::symId(DynamicType v)
{
    return findSymIdByType<DynamicType>(DYNAMIC_TYPES, v);
}

QString TConv::toXml(DynamicType v)
{
    return findXmlTagByType<DynamicType>(DYNAMIC_TYPES, v);
}

DynamicType TConv::fromXml(const QString& tag, DynamicType def)
{
    return findTypeByXmlTag<DynamicType>(DYNAMIC_TYPES, tag, def);
}

static const std::vector<Item<DynamicRange> > DYNAMIC_RANGES = {
    { DynamicRange::STAFF,  "staff" },
    { DynamicRange::PART,   "part" },
    { DynamicRange::SYSTEM, "system" },
};

QString TConv::toUserName(DynamicRange v)
{
    return findUserNameByType<DynamicRange>(DYNAMIC_RANGES, v);
}

QString TConv::toXml(DynamicRange v)
{
    return QString::number(static_cast<int>(v));
}

DynamicRange TConv::fromXml(const QString& tag, DynamicRange def)
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

QString TConv::toUserName(DynamicSpeed v)
{
    return findUserNameByType<DynamicSpeed>(DYNAMIC_SPEEDS, v);
}

QString TConv::toXml(DynamicSpeed v)
{
    return findXmlTagByType<DynamicSpeed>(DYNAMIC_SPEEDS, v);
}

DynamicSpeed TConv::fromXml(const QString& tag, DynamicSpeed def)
{
    return findTypeByXmlTag<DynamicSpeed>(DYNAMIC_SPEEDS, tag, def);
}

static const std::vector<Item<HookType> > HOOK_TYPES = {
    { HookType::NONE,       "hook_none" },
    { HookType::HOOK_90,    "hook_90" },
    { HookType::HOOK_45,    "hook_45" },
    { HookType::HOOK_90T,   "hook_90t" },
};

QString TConv::toUserName(HookType v)
{
    return findUserNameByType<HookType>(HOOK_TYPES, v);
}

QString TConv::toXml(HookType v)
{
    return QString::number(static_cast<int>(v));
}

HookType TConv::fromXml(const QString& tag, HookType def)
{
    bool ok = false;
    int v = tag.toInt(&ok);
    return ok ? HookType(v) : def;
}
