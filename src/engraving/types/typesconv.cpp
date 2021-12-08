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

#include "log.h"

using namespace mu::engraving;

template<typename T>
struct Item
{
    T type;
    QString xmlTag;
    QString userName;
};

template<typename T>
const Item<T>& findItemByType(const std::vector<Item<T> >& cont, const T& v)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [v](const Item<T>& i) {
        return i.type == v;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        static Item<T> dummy;
        return dummy;
    }
    return *it;
}

template<typename T>
T findTypeByXmlTag(const std::vector<Item<T> >& cont, const QString& tag, T def)
{
    auto it = std::find_if(cont.cbegin(), cont.cend(), [tag](const Item<T>& i) {
        return i.xmlTag == tag;
    });

    IF_ASSERT_FAILED(it != cont.cend()) {
        return def;
    }
    return it->type;
}

// ==========================================================
// NoteHeadGroup
// ==========================================================
static const std::vector<Item<NoteHeadGroup> > NOTEHEAD_GROUPS = {
    { NoteHeadGroup::HEAD_NORMAL,           "normal",         QT_TRANSLATE_NOOP("noteheadnames", "Normal") },
    { NoteHeadGroup::HEAD_CROSS,            "cross",          QT_TRANSLATE_NOOP("noteheadnames", "Cross") },
    { NoteHeadGroup::HEAD_PLUS,             "plus",           QT_TRANSLATE_NOOP("noteheadnames", "Plus") },
    { NoteHeadGroup::HEAD_XCIRCLE,          "xcircle",        QT_TRANSLATE_NOOP("noteheadnames", "XCircle") },
    { NoteHeadGroup::HEAD_WITHX,            "withx",          QT_TRANSLATE_NOOP("noteheadnames", "With X") },
    { NoteHeadGroup::HEAD_TRIANGLE_UP,      "triangle-up",    QT_TRANSLATE_NOOP("noteheadnames", "Triangle up") },
    { NoteHeadGroup::HEAD_TRIANGLE_DOWN,    "triangle-down",  QT_TRANSLATE_NOOP("noteheadnames", "Triangle down") },
    { NoteHeadGroup::HEAD_SLASHED1,         "slashed1",       QT_TRANSLATE_NOOP("noteheadnames", "Slashed (forwards)") },
    { NoteHeadGroup::HEAD_SLASHED2,         "slashed2",       QT_TRANSLATE_NOOP("noteheadnames", "Slashed (backwards)") },
    { NoteHeadGroup::HEAD_DIAMOND,          "diamond",        QT_TRANSLATE_NOOP("noteheadnames", "Diamond") },
    { NoteHeadGroup::HEAD_DIAMOND_OLD,      "diamond-old",    QT_TRANSLATE_NOOP("noteheadnames", "Diamond (old)") },
    { NoteHeadGroup::HEAD_CIRCLED,          "circled",        QT_TRANSLATE_NOOP("noteheadnames", "Circled") },
    { NoteHeadGroup::HEAD_CIRCLED_LARGE,    "circled-large",  QT_TRANSLATE_NOOP("noteheadnames", "Circled large") },
    { NoteHeadGroup::HEAD_LARGE_ARROW,      "large-arrow",    QT_TRANSLATE_NOOP("noteheadnames", "Large arrow") },
    { NoteHeadGroup::HEAD_BREVIS_ALT,       "altbrevis",      QT_TRANSLATE_NOOP("noteheadnames", "Alt. brevis") },

    { NoteHeadGroup::HEAD_SLASH,            "slash",          QT_TRANSLATE_NOOP("noteheadnames", "Slash") },

    // shape notes
    { NoteHeadGroup::HEAD_SOL,  "sol",       QT_TRANSLATE_NOOP("noteheadnames", "Sol") },
    { NoteHeadGroup::HEAD_LA,   "la",        QT_TRANSLATE_NOOP("noteheadnames", "La") },
    { NoteHeadGroup::HEAD_FA,   "fa",        QT_TRANSLATE_NOOP("noteheadnames", "Fa") },
    { NoteHeadGroup::HEAD_MI,   "mi",        QT_TRANSLATE_NOOP("noteheadnames", "Mi") },
    { NoteHeadGroup::HEAD_DO,   "do",        QT_TRANSLATE_NOOP("noteheadnames", "Do") },
    { NoteHeadGroup::HEAD_RE,   "re",        QT_TRANSLATE_NOOP("noteheadnames", "Re") },
    { NoteHeadGroup::HEAD_TI,   "ti",        QT_TRANSLATE_NOOP("noteheadnames", "Ti") },

    // not exposed
    { NoteHeadGroup::HEAD_DO_WALKER,    "do-walker", QT_TRANSLATE_NOOP("noteheadnames", "Do (Walker)") },
    { NoteHeadGroup::HEAD_RE_WALKER,    "re-walker", QT_TRANSLATE_NOOP("noteheadnames", "Re (Walker)") },
    { NoteHeadGroup::HEAD_TI_WALKER,    "ti-walker", QT_TRANSLATE_NOOP("noteheadnames", "Ti (Walker)") },
    { NoteHeadGroup::HEAD_DO_FUNK,      "do-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Do (Funk)") },
    { NoteHeadGroup::HEAD_RE_FUNK,      "re-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Re (Funk)") },
    { NoteHeadGroup::HEAD_TI_FUNK,      "ti-funk",   QT_TRANSLATE_NOOP("noteheadnames", "Ti (Funk)") },

    // note name
    { NoteHeadGroup::HEAD_DO_NAME,      "do-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Do (Name)") },
    { NoteHeadGroup::HEAD_RE_NAME,      "re-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Re (Name)") },
    { NoteHeadGroup::HEAD_MI_NAME,      "mi-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Mi (Name)") },
    { NoteHeadGroup::HEAD_FA_NAME,      "fa-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Fa (Name)") },
    { NoteHeadGroup::HEAD_SOL_NAME,     "sol-name", QT_TRANSLATE_NOOP("noteheadnames",  "Sol (Name)") },
    { NoteHeadGroup::HEAD_LA_NAME,      "la-name",  QT_TRANSLATE_NOOP("noteheadnames",  "La (Name)") },
    { NoteHeadGroup::HEAD_TI_NAME,      "ti-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Ti (Name)") },
    { NoteHeadGroup::HEAD_SI_NAME,      "si-name",  QT_TRANSLATE_NOOP("noteheadnames",  "Si (Name)") },

    { NoteHeadGroup::HEAD_A_SHARP,      "a-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "A♯ (Name)") },
    { NoteHeadGroup::HEAD_A,            "a-name",       QT_TRANSLATE_NOOP("noteheadnames",  "A (Name)") },
    { NoteHeadGroup::HEAD_A_FLAT,       "a-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "A♭ (Name)") },
    { NoteHeadGroup::HEAD_B_SHARP,      "b-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "B♯ (Name)") },
    { NoteHeadGroup::HEAD_B,            "b-name",       QT_TRANSLATE_NOOP("noteheadnames",  "B (Name)") },
    { NoteHeadGroup::HEAD_B_FLAT,       "b-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "B♭ (Name)") },
    { NoteHeadGroup::HEAD_C_SHARP,      "c-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "C♯ (Name)") },
    { NoteHeadGroup::HEAD_C,            "c-name",       QT_TRANSLATE_NOOP("noteheadnames",  "C (Name)") },
    { NoteHeadGroup::HEAD_C_FLAT,       "c-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "C♭ (Name)") },
    { NoteHeadGroup::HEAD_D_SHARP,      "d-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "D♯ (Name)") },
    { NoteHeadGroup::HEAD_D,            "d-name",       QT_TRANSLATE_NOOP("noteheadnames",  "D (Name)") },
    { NoteHeadGroup::HEAD_D_FLAT,       "d-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "D♭ (Name)") },
    { NoteHeadGroup::HEAD_E_SHARP,      "e-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "E♯ (Name)") },
    { NoteHeadGroup::HEAD_E,            "e-name",       QT_TRANSLATE_NOOP("noteheadnames",  "E (Name)") },
    { NoteHeadGroup::HEAD_E_FLAT,       "e-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "E♭ (Name)") },
    { NoteHeadGroup::HEAD_F_SHARP,      "f-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "F♯ (Name)") },
    { NoteHeadGroup::HEAD_F,            "f-name",       QT_TRANSLATE_NOOP("noteheadnames",  "F (Name)") },
    { NoteHeadGroup::HEAD_F_FLAT,       "f-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "F♭ (Name)") },
    { NoteHeadGroup::HEAD_G_SHARP,      "g-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "G♯ (Name)") },
    { NoteHeadGroup::HEAD_G,            "g-name",       QT_TRANSLATE_NOOP("noteheadnames",  "G (Name)") },
    { NoteHeadGroup::HEAD_G_FLAT,       "g-flat-name",  QT_TRANSLATE_NOOP("noteheadnames",  "G♭ (Name)") },
    { NoteHeadGroup::HEAD_H,            "h-name",       QT_TRANSLATE_NOOP("noteheadnames",  "H (Name)") },
    { NoteHeadGroup::HEAD_H_SHARP,      "h-sharp-name", QT_TRANSLATE_NOOP("noteheadnames",  "H♯ (Name)") },

    { NoteHeadGroup::HEAD_CUSTOM,       "custom",       QT_TRANSLATE_NOOP("noteheadnames",  "Custom") }
};

QString TConv::toUserName(NoteHeadGroup v)
{
    return findItemByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v).userName;
}

QString TConv::toXmlTag(NoteHeadGroup v)
{
    return findItemByType<NoteHeadGroup>(NOTEHEAD_GROUPS, v).xmlTag;
}

NoteHeadGroup TConv::fromXmlTag(const QString& tag, NoteHeadGroup def)
{
    return findTypeByXmlTag<NoteHeadGroup>(NOTEHEAD_GROUPS, tag, def);
}
