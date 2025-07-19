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
#ifndef MU_ENGRAVING_LYRICSLAYOUT_DEV_H
#define MU_ENGRAVING_LYRICSLAYOUT_DEV_H

#include "layoutcontext.h"

namespace mu::engraving {
class Score;
class System;
class Lyrics;
class LyricsLine;
class LyricsLineSegment;
class SkylineLine;
}

namespace mu::engraving::rendering::score {
class LyricsLayout
{
    struct LyricsVerse {
    private:
        std::vector<Lyrics*> m_lyrics;
        std::vector<LyricsLineSegment*> m_lines;
    public:
        void addLyrics(Lyrics* l) { m_lyrics.push_back(l); }
        void addLine(LyricsLineSegment* lls) { m_lines.push_back(lls); }

        const std::vector<Lyrics*>& lyrics() const { return m_lyrics; }
        const std::vector<LyricsLineSegment*>& lines() const { return m_lines; }
    };

    using LyricsVersesMap = std::map<int, LyricsVerse>;

public:
    LyricsLayout() = default;

    static void layout(Lyrics* item, LayoutContext& ctx);
    static void layout(LyricsLine* item, LayoutContext& ctx);
    static void layout(LyricsLineSegment* item, LayoutContext& ctx);

    static void computeVerticalPositions(System* system, LayoutContext& ctx);

private:
    static void createOrRemoveLyricsLine(Lyrics* item, LayoutContext& ctx);

    static void layoutMelismaLine(LyricsLineSegment* item);
    static void layoutDashes(LyricsLineSegment* item);

    static Lyrics* findNextLyrics(const ChordRest* endChordRest, int verseNumber);

    static void computeVerticalPositions(staff_idx_t staffIdx, System* system, LayoutContext& ctx);
    static void collectLyricsVerses(staff_idx_t staffIdx, System* system, LyricsVersesMap& lyricsVersesAbove,
                                    LyricsVersesMap& lyricsVersesBelow);

    static void setDefaultPositions(staff_idx_t staffIdx, const LyricsVersesMap& lyricsVersesAbove,
                                    const LyricsVersesMap& lyricsVersesBelow, LayoutContext& ctx);

    static void checkCollisionsWithStaffElements(System* system, staff_idx_t staffIdx,  LayoutContext& ctx,
                                                 const LyricsVersesMap& lyricsVersesAbove, const LyricsVersesMap& lyricsVersesBelow);
    static SkylineLine createSkylineForVerse(int verse, bool north, const LyricsVersesMap& lyricsVerses, System* system);
    static void moveThisVerseAndOuterOnes(int verse, int lastVerse, bool above, double diff, const LyricsVersesMap& lyricsVerses);

    static void addToSkyline(System* system, staff_idx_t staffIdx, LayoutContext& ctx, const LyricsVersesMap& lyricsVersesAbove,
                             const LyricsVersesMap& lyricsVersesBelow);

    static double lyricsLineStartX(const LyricsLineSegment* item);
    static double lyricsLineEndX(const LyricsLineSegment* item, const Lyrics* endLyrics = nullptr);
    static void adjustLyricsLineYOffset(LyricsLineSegment* item, const Lyrics* endLyrics = nullptr);
};
}
#endif // MU_ENGRAVING_LYRICSLAYOUT_DEV_H
