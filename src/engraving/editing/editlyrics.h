/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <optional>

#include "../dom/property.h"
#include "../types/types.h"

namespace mu::engraving {
class ChordRest;
class Lyrics;
class PartialLyricsLine;
class Score;
class Segment;
class Transaction;

class EditLyrics
{
public:
    // Captured before ending the text edit, which loses the cursor's font style
    struct InitialState {
        Lyrics* lyrics = nullptr;
        ChordRest* chordRest = nullptr;
        Segment* segment = nullptr;
        track_idx_t track = 0;
        int verse = 0;
        PlacementV placement = PlacementV::BELOW;
        PropertyFlags placementFlags = PropertyFlags::STYLED;
        FontStyle fontStyle = FontStyle::Normal;
        PropertyFlags fontStyleFlags = PropertyFlags::STYLED;
        AutoOnOff centering = AutoOnOff::AUTO;
        TextStyleType styleType = TextStyleType::LYRICS_ODD;
        double yOffset = 0.0;
        bool hasPrecedingRepeat = false;
        bool hasFollowingRepeat = false;
        PartialLyricsLine* prevPartialDash = nullptr;
    };

    static InitialState initialState(Lyrics* lyrics);

    static bool canNavigateToNextSyllable(const InitialState& state);

    // nullptr when the dash only extends past a following repeat
    static Lyrics* navigateToNextSyllable(Transaction& tx, Score* score, const InitialState& state);

    struct MelismaEnds {
        Lyrics* nextLyrics = nullptr;
        bool isNewNextLyrics = false;
        Lyrics* fromLyrics = nullptr;
    };

    // New next lyrics are not added: add them in a separate transaction, as ending the edit of
    // empty new lyrics rolls back the transaction that added them
    static MelismaEnds addMelisma(Transaction& tx, Score* score, const InitialState& state);

    static Lyrics* addVerse(Transaction& tx, Score* score, const InitialState& state);

    static bool canNavigateToLyrics(const InitialState& state, bool back);

    struct NavigatedLyrics {
        Lyrics* lyrics = nullptr;
        bool isNew = false;
    };

    // Unless moveOnly, a forward move ends the word
    static NavigatedLyrics navigateToLyrics(Transaction& tx, Score* score, const InitialState& state, bool back, bool moveOnly);

    static Lyrics* navigateToVerse(Transaction& tx, Score* score, const InitialState& state, int verse);

private:
    static Segment* findNextSyllableSegment(const InitialState& state, track_idx_t& toLyricTrack);

    static Segment* findNextLyricsSegment(const InitialState& state, bool back);

    // Without a style type, uses the odd/even verse style
    static Lyrics* createLyrics(ChordRest* cr, const InitialState& state, int verse, std::optional<TextStyleType> styleType);
    static Lyrics* createSyllable(ChordRest* cr, const InitialState& state, std::optional<TextStyleType> styleType,
                                  LyricsSyllabic syllabic);
    static PartialLyricsLine* createPartialLyricsLine(Score* score, const InitialState& state, bool isEndMelisma, const Fraction& ticks);

    static PartialLyricsLine* findPartialMelisma(const Score* score, const ChordRest* cr, int verse, PlacementV placement);

    // e.g. 1st volta -> 2nd volta
    static bool isInNonAdjacentRepeatSection(const ChordRest* toCr, const Lyrics* fromLyrics, track_idx_t track);

    static Fraction partialMelismaEndTick(const Score* score, Segment* segment, track_idx_t track, const Fraction& endTick);
};
}
