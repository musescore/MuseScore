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

#ifndef SYMBOLS_H
#define SYMBOLS_H

/**
 \file
 Definition of tokens types for bww lexer and parser
 */

namespace Bww {
enum Symbol
{
    COMMENT,
    HEADER,
    STRING,
    CLEF,
    KEY,
    TEMPO,
    TSIG,
    PART,
    BAR,
    NOTE,
    TIE,
    TRIPLET,
    DOT,
    GRACE,
    UNKNOWN,
    NONE
};

enum class StartStop
{
    ST_NONE,
    ST_START,
    ST_CONTINUE,
    ST_STOP
};

extern QString symbolToString(Symbol s);
} // namespace Bww

#endif // SYMBOLS_H
