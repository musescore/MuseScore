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
using namespace mu;
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
static T findTypeBySymId(const std::vector<Item<T> >& cont, const SymId& v, T def)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.symId == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        return def;
    }
    return it->type;
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
QString TConv::toXml(const QList<int>& v)
{
    QStringList sl;
    for (int i : v) {
        sl << QString::number(i);
    }
    return sl.join(",");
}

QList<int> TConv::fromXml(const QString& tag, const QList<int>& def)
{
    QList<int> list;
    QStringList sl = tag.split(",", Qt::SkipEmptyParts);
    for (const QString& s : qAsConst(sl)) {
        bool ok = false;
        int i = s.simplified().toInt(&ok);
        if (!ok) {
            return def;
        }
        list << i;
    }
    return list;
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

AlignH TConv::fromXml(const QString& tag, AlignH def)
{
    return findTypeByXmlTag<AlignH>(ALIGN_H, tag, def);
}

AlignV TConv::fromXml(const QString& tag, AlignV def)
{
    return findTypeByXmlTag<AlignV>(ALIGN_V, tag, def);
}

QString TConv::toXml(Align v)
{
    QStringList sl;
    sl << findXmlTagByType<AlignH>(ALIGN_H, v.horizontal);
    sl << findXmlTagByType<AlignV>(ALIGN_V, v.vertical);
    return sl.join(",");
}

Align TConv::fromXml(const QString& str, Align def)
{
    QStringList sl = str.split(',');
    if (sl.size() != 2) {
        LOGD() << "bad align value: " << str;
        return def;
    }

    Align a;
    a.horizontal = findTypeByXmlTag<AlignH>(ALIGN_H, sl.at(0), def.horizontal);
    a.vertical = findTypeByXmlTag<AlignV>(ALIGN_V, sl.at(1), def.vertical);
    return a;
}

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

DynamicType TConv::dynamicType(SymId s)
{
    return findTypeBySymId<DynamicType>(DYNAMIC_TYPES, s, DynamicType::OTHER);
}

DynamicType TConv::dynamicType(const std::string& string)
{
    static const std::unordered_map<std::string, DynamicType> DYNAMIC_STR_MAP = {
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

    auto search = DYNAMIC_STR_MAP.find(string);
    if (search != DYNAMIC_STR_MAP.cend()) {
        return search->second;
    }

    return DynamicType::OTHER;
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

QString TConv::toUserName(KeyMode v)
{
    return findUserNameByType<KeyMode>(KEY_MODES, v);
}

QString TConv::toXml(KeyMode v)
{
    return findXmlTagByType<KeyMode>(KEY_MODES, v);
}

KeyMode TConv::fromXml(const QString& tag, KeyMode def)
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

QString TConv::toUserName(TextStyleType v)
{
    return findUserNameByType<TextStyleType>(TEXTSTYLE_TYPES, v);
}

QString TConv::toXml(TextStyleType v)
{
    return findXmlTagByType<TextStyleType>(TEXTSTYLE_TYPES, v);
}

TextStyleType TConv::fromXml(const QString& tag, TextStyleType def)
{
    auto it = std::find_if(TEXTSTYLE_TYPES.cbegin(), TEXTSTYLE_TYPES.cend(), [tag](const Item<TextStyleType>& i) {
        return i.xml == tag;
    });

    if (it != TEXTSTYLE_TYPES.cend()) {
        return it->type;
    }

    // compatibility

    static const std::map<QString, TextStyleType> OLD_TST_TAGS = {
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

    float durationStep = ticksDuration / static_cast<float>(stepsCount);

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

QString TConv::toUserName(ChangeMethod v)
{
    return findUserNameByType<ChangeMethod>(CHANGE_METHODS, v);
}

QString TConv::toXml(ChangeMethod v)
{
    return findXmlTagByType<ChangeMethod>(CHANGE_METHODS, v);
}

ChangeMethod TConv::fromXml(const QString& tag, ChangeMethod def)
{
    return findTypeByXmlTag<ChangeMethod>(CHANGE_METHODS, tag, def);
}

QString TConv::toXml(const PitchValue& v)
{
    return QString("point time=\"%1\" pitch=\"%2\" vibrato=\"%3\"")
           .arg(v.time).arg(v.pitch).arg(v.vibrato);
}

QString TConv::toXml(AccidentalRole v)
{
    return QString::number(static_cast<int>(v));
}

AccidentalRole TConv::fromXml(const QString& tag, AccidentalRole def)
{
    bool ok = false;
    int r = tag.toInt(&ok);
    return ok ? static_cast<AccidentalRole>(r) : def;
}

QString TConv::toXml(BeatsPerSecond v)
{
    return QString::number(v.val);
}

BeatsPerSecond TConv::fromXml(const QString& tag, BeatsPerSecond def)
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
    { DurationType::V_MEASURE,  "measure",  QT_TRANSLATE_NOOP("engraving", "Quarter") },
    { DurationType::V_BREVE,    "breve",    QT_TRANSLATE_NOOP("engraving", "Breve") },
    { DurationType::V_LONG,     "long",     QT_TRANSLATE_NOOP("engraving", "Longa") },
    { DurationType::V_ZERO,     "",         QT_TRANSLATE_NOOP("engraving", "Zero") },
    { DurationType::V_INVALID,  "",         QT_TRANSLATE_NOOP("engraving", "Invalid") },
};

QString TConv::toUserName(DurationType v)
{
    return findUserNameByType<DurationType>(DURATION_TYPES, v);
}

QString TConv::toXml(DurationType v)
{
    return findXmlTagByType<DurationType>(DURATION_TYPES, v);
}

DurationType TConv::fromXml(const QString& tag, DurationType def)
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

QString TConv::toXml(PlayingTechniqueType v)
{
    return findXmlTagByType<PlayingTechniqueType>(PLAY_TECH_TYPES, v);
}

PlayingTechniqueType TConv::fromXml(const QString& tag, PlayingTechniqueType def)
{
    return findTypeByXmlTag<PlayingTechniqueType>(PLAY_TECH_TYPES, tag, def);
}

static const std::vector<Item<TempoTechniqueType> > TEMPO_CHANGE_TYPES = {
    { TempoTechniqueType::Undefined, "undefined" },
    { TempoTechniqueType::Accelerando, "accelerando" },
    { TempoTechniqueType::Allargando, "allargando" },
    { TempoTechniqueType::Calando, "calando" },
    { TempoTechniqueType::Lentando, "lentando" },
    { TempoTechniqueType::Morendo, "morendo" },
    { TempoTechniqueType::Precipitando, "precipitando" },
    { TempoTechniqueType::Rallentando, "rallentando" },
    { TempoTechniqueType::Ritardando, "ritardando" },
    { TempoTechniqueType::Smorzando, "smorzando" },
    { TempoTechniqueType::Sostenuto, "sostenuto" },
    { TempoTechniqueType::Stringendo, "stringendo" }
};

QString TConv::toXml(TempoTechniqueType v)
{
    return findXmlTagByType<TempoTechniqueType>(TEMPO_CHANGE_TYPES, v);
}

TempoTechniqueType TConv::fromXml(const QString& tag, TempoTechniqueType def)
{
    return findTypeByXmlTag<TempoTechniqueType>(TEMPO_CHANGE_TYPES, tag, def);
}
