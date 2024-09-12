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
#pragma once

#include <memory>
#include <variant>

#include "types/ret.h"

#include "../types/types.h"
#include "xmlreader.h"

namespace mu::engraving {
class Score;
class EngravingItem;

class Accidental;
class ChordRest;
class Harmony;
class Segment;
class Spanner;
class StaffTypeChange;
class Symbol;
class Text;
class Tuplet;

class Chord;
class TremoloSingleChord;
class TremoloTwoChord;
}

namespace mu::engraving::compat {
struct TremoloCompat;
}

namespace mu::engraving::rw {
struct ReadInOutData;
class IReader
{
public:
    virtual ~IReader() = default;

    virtual muse::Ret readScore(Score* score, XmlReader& xml, rw::ReadInOutData* out) = 0;

    using Supported = std::variant<std::monostate,
                                   Accidental*,
                                   ChordRest*,
                                   Harmony*,
                                   Spanner*,
                                   StaffTypeChange*,
                                   Symbol*,
                                   Text*,
                                   Tuplet*
                                   >;

    template<typename T>
    static void check_supported_static(T item)
    {
        if constexpr (std::is_same<T, EngravingItem*>::value) {
            // supported
        } else {
            Supported check(item);
            (void)check;
        }
    }

    template<typename T>
    void readItem(T item, XmlReader& xml)
    {
#ifndef NDEBUG
        check_supported_static(item);
#endif
        doReadItem(static_cast<EngravingItem*>(item), xml);
    }

    //! NOTE Needs refactoring - reading should be separated from insertion
    //! (we read the elements into some structure, then inserted them)
    virtual bool pasteStaff(XmlReader& e, Segment* dst, staff_idx_t dstStaff, Fraction scale) = 0;
    virtual void pasteSymbols(XmlReader& e, ChordRest* dst) = 0;

    // compat
    virtual void readTremoloCompat(compat::TremoloCompat* item, XmlReader& xml) = 0;

private:
    virtual void doReadItem(EngravingItem* item, XmlReader& xml) = 0;
};

using IReaderPtr = std::shared_ptr<IReader>;
}
