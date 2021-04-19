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

/**
 Symbol type to text translation.
 */

#include <QtCore/QString>

#include "symbols.h"

namespace Bww {
static const char* symTable[] =
{
    "COMMENT",
    "HEADER",
    "STRING",
    "CLEF",
    "KEY",
    "TEMPO",
    "TSIG",
    "PART",
    "BAR",
    "NOTE",
    "TIE",
    "TRIPLET",
    "DOT",
    "GRACE",
    "UNKNOWN",
    "NONE"
};

QString symbolToString(Symbol s)
{
    if (s < 0) {
        return "INVALID";
    }
    if (static_cast<unsigned>(s) > sizeof symTable) {
        return "INVALID";
    } else {
        return symTable[s];
    }
}
} // namespace Bww
