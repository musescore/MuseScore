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
#include "pagestyle.h"

namespace Ms {
QSet<Sid> pageStyles()
{
    static const QSet<Sid> styles {
        Sid::pageWidth,
        Sid::pageHeight,
        Sid::pagePrintableWidth,
        Sid::pageEvenTopMargin,
        Sid::pageEvenBottomMargin,
        Sid::pageEvenLeftMargin,
        Sid::pageOddTopMargin,
        Sid::pageOddBottomMargin,
        Sid::pageOddLeftMargin,
        Sid::pageTwosided,
        Sid::spatium
    };

    return styles;
}
}
