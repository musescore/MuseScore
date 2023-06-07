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
#ifndef MU_ENGRAVING_ILAYOUT_H
#define MU_ENGRAVING_ILAYOUT_H

#include <variant>

#include "modularity/imoduleinterface.h"

#include "layoutoptions.h"

namespace mu::engraving {
class Score;
class EngravingItem;

class Arpeggio;

class BarLine;
class Box;
class Bracket;

class Clef;

class Dynamic;

class FiguredBassItem;

class Harmony;

class Image;

class LedgerLine;
class SLine;
class LineSegment;
class Lyrics;

class NoteDot;

class Rest;

class Spanner;
class Slur;
class SlurTie;
class Stem;

class TextBase;
class Text;
}

namespace mu::engraving::layout {
class ILayout : MODULE_INTERNAL_INTERFACE
{
    INTERFACE_ID(IEngravingLayout)

public:
    virtual ~ILayout() = default;

    // Layout Score
    virtual void layoutRange(Score* score, const LayoutOptions& options, const Fraction&, const Fraction&) = 0;

    using Supported = std::variant<std::monostate,
                                   Arpeggio*,
                                   BarLine*,
                                   Box*,
                                   Bracket*,
                                   Clef*,
                                   Dynamic*,
                                   FiguredBassItem*,
                                   Harmony*,
                                   Image*,
                                   LedgerLine*,
                                   SLine*,
                                   LineSegment*,
                                   Lyrics*,
                                   NoteDot*,
                                   Rest*,
                                   Spanner*,
                                   Slur*,
                                   SlurTie*,
                                   Stem*,
                                   TextBase*,
                                   Text*
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
    void layoutItem(T item)
    {
#ifndef NDEBUG
        check_supported_static(item);
#endif
        doLayoutItem(static_cast<EngravingItem*>(item));
    }

    // Layout Elements on Edit
    virtual void layoutOnEdit(Arpeggio* item) = 0;

private:
    // Layout Single Item
    virtual void doLayoutItem(EngravingItem* item) = 0;
};
}

#endif // MU_ENGRAVING_ILAYOUT_H
