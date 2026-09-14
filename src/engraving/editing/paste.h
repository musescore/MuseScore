/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "../types/fraction.h"
#include "../types/types.h"
#include "../rw/ireader.h"

namespace muse {
class ByteArray;
}

namespace mu::engraving {
class ChordRest;
class IMimeData;
class MuseScoreView;
class Score;
class Segment;
class Transaction;
class XmlReader;

class Paste
{
public:
    static bool paste(Transaction& tx, Score* score, const IMimeData* ms, MuseScoreView* view, Fraction scale = Fraction(1, 1),
                      rw::PasteMode mode = rw::PasteMode::Default);

    static bool pasteSymbol(Transaction& tx, Score* score, muse::ByteArray& data, MuseScoreView* view, Fraction scale = Fraction(1, 1),
                            rw::PasteMode mode = rw::PasteMode::Default);
    static bool pasteStaffList(Transaction& tx, Score* score, muse::ByteArray& data, Fraction scale = Fraction(1, 1),
                               rw::PasteMode mode = rw::PasteMode::Default);
    static bool pasteSymbolList(Transaction& tx, Score* score, muse::ByteArray& data, rw::PasteMode mode = rw::PasteMode::Default);

    static bool pasteStaff(Transaction& tx, Score* score, XmlReader& e, Segment* dst, staff_idx_t staffIdx, Fraction scale = Fraction(1,
                                                                                                                                      1),
                           rw::PasteMode mode = rw::PasteMode::Default);
    static void pasteSymbols(Transaction& tx, XmlReader& e, ChordRest* dst, rw::PasteMode mode = rw::PasteMode::Default);

    static void pasteChordRest(Transaction& tx, Score* score, ChordRest* cr, const Fraction& tick);

    static bool repeatListSelection(Transaction& tx, Score* score);
};
}
