/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include <variant>

#include "modularity/imoduleinterface.h"
#include "draw/types/geometry.h"

#include "../types/fraction.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class Score;
class EngravingItem;
class Page;

class Accidental;
class ActionIcon;
class Arpeggio;

class BarLine;
class Beam;
class Box;
class Bracket;

class Chord;
class Clef;

class Dynamic;

class EngravingItem;

class FiguredBassItem;

class Harmony;

class Image;

enum class KerningType : unsigned char;
class KeySig;

class LedgerLine;
class SLine;
class LineSegment;
class Lyrics;

class NoteDot;

class Parenthesis;

class Rest;

class ShadowNote;
class Spanner;
class Slur;
class SlurSegment;
class SlurTie;
class StaffText;
class Stem;
class SystemLockIndicator;

class TextBase;
class Text;
class TextLineBaseSegment;
class TieSegment;
class TimeSig;
}

namespace mu::engraving::rendering {
class IScoreRenderer : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IScoreRenderer)

public:
    virtual ~IScoreRenderer() = default;

    // Main interface

    virtual void layoutScore(Score* score, const Fraction& st, const Fraction& et) const = 0;

    struct PaintOptions
    {
        bool isSetViewport = true;
        bool isPrinting = false;
        bool isMultiPage = false;
        bool printPageBackground = true;
        RectF frameRect;
        int fromPage = -1; // 0 is first
        int toPage = -1;
        int copyCount = 1;
        int trimMarginPixelSize = -1;
        int deviceDpi = -1;

        std::function<void(muse::draw::Painter* painter, const Page* page, const RectF& pageRect)> onPaintPageSheet;
        std::function<void()> onNewPage;
    };

    virtual SizeF pageSizeInch(const Score* score) const = 0;
    virtual SizeF pageSizeInch(const Score* score, const PaintOptions& opt) const = 0;
    virtual void paintScore(muse::draw::Painter* painter, Score* score, const IScoreRenderer::PaintOptions& opt) const = 0;
    virtual void paintItem(muse::draw::Painter& painter, const EngravingItem* item) const = 0;

    // Temporary compatibility interface
    using Supported = std::variant<std::monostate,
                                   Accidental*,
                                   ActionIcon*,
                                   Arpeggio*,
                                   BarLine*,
                                   Box*,
                                   Bracket*,
                                   Clef*,
                                   Dynamic*,
                                   FiguredBassItem*,
                                   Harmony*,
                                   Image*,
                                   KeySig*,
                                   LedgerLine*,
                                   SLine*,
                                   LineSegment*,
                                   Lyrics*,
                                   NoteDot*,
                                   Parenthesis*,
                                   Rest*,
                                   ShadowNote*,
                                   Spanner*,
                                   Slur*,
                                   SlurTie*,
                                   StaffText*,
                                   Stem*,
                                   SystemLockIndicator*,
                                   TextBase*,
                                   Text*,
                                   TimeSig*
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

    //! TODO Investigation is required, probably these functions or their calls should not be.
    // Other
    virtual void layoutTextLineBaseSegment(TextLineBaseSegment* item) = 0;
    virtual void layoutBeam1(Beam* item) = 0;
    virtual void layoutStem(Chord* item) = 0;

    // Layout Text 1
    virtual void layoutText1(TextBase* item, bool base = false) = 0;

    void drawItem(const EngravingItem* item, muse::draw::Painter* p)
    {
        doDrawItem(item, p);
    }

    virtual void computeBezier(TieSegment* tieSeg, PointF shoulderOffset = PointF()) = 0;
    virtual void computeBezier(SlurSegment* slurSeg, PointF shoulderOffser = PointF()) = 0;

    virtual void computeMasks(Score* score) = 0;

private:
    // Layout Single Item
    virtual void doLayoutItem(EngravingItem* item) = 0;

    virtual void doDrawItem(const EngravingItem* item, muse::draw::Painter* p) = 0;
};
}
