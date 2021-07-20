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

#ifndef MU_UI_MUSICALSYMBOLCODE_H
#define MU_UI_MUSICALSYMBOLCODE_H

#include <QObject>

namespace mu::ui {
/**
 * @brief The MusicalSymbolCodes class simplifies access to the icons from the musical font
 *
 * @details Each enum value is a UTF-16-like address of the icon in the icon font.
 *          The current default musical font (Leland.otf) is located in the 'MuseScore/fonts/leland' folder,
 *          The most actual version can be found by this persistent URL: @link https://www.dropbox.com/s/fhiu26x7sgboxjf/Leland.otf?dl=0
 */

class MusicalSymbolCodes
{
    Q_GADGET

public:
    enum class Code : char16_t {
        ZERO = 0xE080,
        ONE = 0xE081,
        TWO = 0xE082,
        THREE = 0xE083,
        FOUR = 0xE084,
        FIVE = 0xE085,
        SIX = 0xE086,
        SEVEN = 0xE087,
        EIGHT = 0xE088,
        NINE = 0xE089,
        TIMESIG_COMMON = 0xE08A,
        TIMESIG_CUT = 0xE08B,

        SEMIBREVE = 0xE1D2,
        MINIM = 0xE1D3,
        CROTCHET = 0xE1D5,
        QUAVER = 0xE1D7,
        SEMIQUAVER = 0xE1D9,
        DEMISEMIQUAVER = 0xE1DB,
        DOT = 0xE1E7,

        NONE
    };

    Q_ENUM(Code)
};

inline QString musicalSymbolToString(MusicalSymbolCodes::Code noteIcon, bool withDot = false)
{
    QString noteSymbol = QString(QChar(static_cast<char16_t>(noteIcon)));

    if (withDot) {
        noteSymbol += QChar(static_cast<char16_t>(MusicalSymbolCodes::Code::DOT));
    }

    return noteSymbol;
}
}

#endif // MU_UI_MUSICALSYMBOLCODE_H
