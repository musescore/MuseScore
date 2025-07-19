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

#include "layoutcontext.h"

namespace mu::engraving {
class Chord;
class Stem;
class StaffType;
class Rest;
}

namespace mu::engraving::rendering::score {
class StemLayout
{
public:
    // all in spatium units
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMLEN_UP = 3.0;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMDIST_UP = 1.0;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMPOSY_UP = -STAFFTYPE_TAB_DEFAULTSTEMDIST_UP;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMLEN_DN = 3.0;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMDIST_DN = 1.0;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMPOSY_DN = STAFFTYPE_TAB_DEFAULTSTEMDIST_DN;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMLEN_THRU = 3.5;
    static constexpr double STAFFTYPE_TAB_DEFAULTSTEMPOSX = 0.75;
    // TAB STEM NOTATION
    // the ratio between the length of a full stem and the length of a short stem
    // (used for half note stems, in some TAB styles)
    static constexpr double STAFFTYPE_TAB_SHORTSTEMRATIO = 0.5;
    // metrics of slashes through half note stems
    static constexpr double STAFFTYPE_TAB_SLASH_WIDTH = 1.2; /* X width of half note slash */
    static constexpr double STAFFTYPE_TAB_SLASH_SLANTY = 0.8; /* the Y coord of the slash slant */
    static constexpr double STAFFTYPE_TAB_SLASH_THICK = 0.4; /* slash thickness */
    static constexpr double STAFFTYPE_TAB_SLASH_DISPL = 0.8; /* the total displacement between one slash and the next:
                                                      includes slash thickness and empty space between slashes*/
    // the total height of a double slash
    static constexpr double STAFFTYPE_TAB_SLASH_2TOTHEIGHT
        = (STAFFTYPE_TAB_SLASH_THICK + STAFFTYPE_TAB_SLASH_DISPL + STAFFTYPE_TAB_SLASH_SLANTY);
    // the initial Y coord for a double shash on an UP stem = topmost corner of topmost slash
    static constexpr double STAFFTYPE_TAB_SLASH_2STARTY_UP = ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP - STAFFTYPE_TAB_SLASH_2TOTHEIGHT) * 0.5);
    // the initial Y coord for a double shash on an DN stem = topmost corner of topmost slash
    static constexpr double STAFFTYPE_TAB_SLASH_2STARTY_DN = ((STAFFTYPE_TAB_DEFAULTSTEMLEN_UP + STAFFTYPE_TAB_SLASH_2TOTHEIGHT) * 0.5);

    static double calcDefaultStemLength(Chord* item, const LayoutContext& ctx);
    static int minStaffOverlap(bool up, int staffLines, int beamCount, bool hasHook, double beamSpacing, bool useWideBeams,
                               bool isFullSize);

    static double stemPosX(const ChordRest* item);
    static double stemPosX(const Chord* item);
    static double stemPosX(const Rest* item);

    static PointF stemPos(const ChordRest* item);
    static PointF stemPos(const Chord* item);
    static PointF stemPos(const Rest* item);

    static double tabStemLength(const Chord* item, const StaffType* st);
    static double tabStemPosX() { return STAFFTYPE_TAB_DEFAULTSTEMPOSX; }
    static double tabRestStemPosY(const ChordRest* item, const StaffType* st);
    static PointF tabStemPos(const Chord* item, const StaffType* st);

private:
    static int stemLengthBeamAddition(const Chord* item, const LayoutContext& ctx);
    static int maxReduction(const Chord* item, const LayoutContext& ctx, int extensionOutsideStaff);
    static int stemOpticalAdjustment(const Chord* item, int stemEndPosition);
    static int calcMinStemLength(Chord* item, const LayoutContext& ctx);
    static int calc4BeamsException(const Chord* item, int stemLength);

    static inline int calcBeamCount(const Chord* item);
};
} // namespace mu::engraving::rendering::score
