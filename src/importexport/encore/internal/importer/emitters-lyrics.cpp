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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Queue Encore LYRIC elements and attach syllables to the nearest chords in a measure.

#include <algorithm>

#include "emitters-internal.h"

#include "../parser/ticks.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

namespace mu::iex::enc {
// Queue a LYRIC element. Hyphen separators ("-") update the hyphen flags on the
// preceding syllable and set a carry-forward flag for the next one.
void enqueueLyric(BuildCtx& ctx, const EncLyric* el, track_idx_t track)
{
    const String text(el->text);
    auto& queue = ctx.scratch.pendingLyrics[track];
    if (text == u"-") {
        if (!queue.empty()) {
            queue.back().hyphenAfter = true;
        } else {
            // The preceding syllable was attached in an earlier measure; promote it so the
            // hyphen renders across the bar (SINGLE -> BEGIN, END -> MIDDLE).
            auto it = ctx.scratch.lastAttachedLyric.find(track);
            if (it != ctx.scratch.lastAttachedLyric.end() && it->second) {
                mu::engraving::Lyrics* prev = it->second;
                if (prev->syllabic() == mu::engraving::LyricsSyllabic::SINGLE) {
                    prev->setSyllabic(mu::engraving::LyricsSyllabic::BEGIN);
                } else if (prev->syllabic() == mu::engraving::LyricsSyllabic::END) {
                    prev->setSyllabic(mu::engraving::LyricsSyllabic::MIDDLE);
                }
            }
        }
        ctx.scratch.nextLyricHyphenBefore[track] = true;
    } else if (text.isEmpty()) {
        ctx.scratch.nextLyricHyphenBefore[track] = false;
    } else {
        PendingLyric pl;
        pl.encTick = static_cast<int>(el->tick);
        pl.xoffset = static_cast<int>(el->kie);   // horizontal anchor; see attach pass
        pl.text = text;
        auto it = ctx.scratch.nextLyricHyphenBefore.find(track);
        pl.hyphenBefore = (it != ctx.scratch.nextLyricHyphenBefore.end()) && it->second;
        pl.hyphenAfter = false;
        ctx.scratch.nextLyricHyphenBefore[track] = false;
        queue.push_back(std::move(pl));
    }
}

// The real Encore tick and column of each voice-0 note, since the note loop accumulates durations
// rather than Encore ticks. Routed exactly as the note loop routes, or grand-staff notes land on the
// wrong staff.
static std::map<int, std::vector<std::pair<int, int> > > buildEncNoteTicksByStaff(
    BuildCtx& ctx, const MeasEmitCtx& mc, const EncMeasure& encMeas)
{
    std::map<int, std::vector<std::pair<int, int> > > encNoteTicksByStaff;   // staff -> [(tick, column)]
    for (const auto& elem : encMeas.elements) {
        const EncMeasureElem* e = elem.get();
        if (e->type != static_cast<quint8>(EncElemType::NOTE)) {
            continue;
        }
        if (!mc.lineSlotByRawByte) {
            continue;
        }
        std::optional<RoutedTrack> routed
            = routeElementStaffVoice(e, /*isNoteOrRest*/ true, *mc.lineSlotByRawByte, mc, ctx);
        if (!routed || routed->msVoice != 0) {
            continue;   // skip unroutable; lyrics anchor to voice-0 chords
        }
        auto& tickList = encNoteTicksByStaff[routed->staffIdx];
        const int t = static_cast<int>(e->tick);
        if (tickList.empty() || tickList.back().first != t) {
            tickList.emplace_back(t, static_cast<int>(static_cast<const EncNote*>(e)->xoffset));
        }
    }
    return encNoteTicksByStaff;
}

// Pair each ChordRest on chordTrack with an Encore tick. The kth chord takes the kth Encore note
// tick (positional assignment), which is more accurate than a cumTick conversion (see
// buildEncNoteTicksByStaff); rests do not consume a note tick.
struct CrAnchor {
    int encTick = 0;
    int column = 0;         // where the note is written; zero when unknown, as in format 2.50
    ChordRest* cr = nullptr;
};

static std::vector<CrAnchor> buildCrTickPairs(
    Measure* measure, const Fraction& measTick, const EncMeasure& encMeas,
    track_idx_t chordTrack, const std::vector<std::pair<int, int> >* noteTickList)
{
    std::vector<CrAnchor> crTickPairs;
    size_t noteTickIdx = 0;
    for (Segment* s = measure->first(SegmentType::ChordRest);
         s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(chordTrack);
        if (!el || !el->isChordRest()) {
            continue;
        }
        int segEncTick;
        int segColumn = 0;
        ChordRest* cr = toChordRest(el);
        if (cr->isChord() && noteTickList && noteTickIdx < noteTickList->size()) {
            segEncTick = (*noteTickList)[noteTickIdx].first;
            segColumn = (*noteTickList)[noteTickIdx].second;
            ++noteTickIdx;
        } else {
            // Rest, or note list exhausted: estimate from the measure's beat grid.
            const Fraction relTick = s->tick() - measTick;
            const int durTicks = encMeas.durTicks ? static_cast<int>(encMeas.durTicks) : kEncWholeTicks;
            segEncTick = (relTick.numerator() * durTicks)
                         / std::max(1, relTick.denominator());
        }
        crTickPairs.push_back({ segEncTick, segColumn, cr });
    }
    return crTickPairs;
}

// Best unconsumed ChordRest for a lyric at encTick. preferNotAfter makes a note at or before the tick
// win over a later one whatever the distance, which is the threshold pass; otherwise nearest wins.
static int findBestCr(const std::vector<CrAnchor>& pairs,
                      const std::vector<bool>& consumed, int encTick,
                      bool wantChord, int maxDelta, bool preferNotAfter)
{
    int bestIdx = -1;
    int bestDelta = INT_MAX;
    bool bestIsAfter = false;  // true if best match has note_tick > encTick
    for (size_t ni = 0; ni < pairs.size(); ++ni) {
        if (consumed[ni]) {
            continue;
        }
        const bool ok = wantChord ? pairs[ni].cr->isChord() : pairs[ni].cr->isRest();
        if (!ok) {
            continue;
        }
        const int delta = std::abs(pairs[ni].encTick - encTick);
        if (delta > maxDelta) {
            continue;
        }
        const bool isAfter = (pairs[ni].encTick > encTick);
        if (bestIdx < 0
            || (preferNotAfter && !isAfter && bestIsAfter)
            || ((!preferNotAfter || isAfter == bestIsAfter) && delta < bestDelta)) {
            bestDelta = delta;
            bestIdx = static_cast<int>(ni);
            bestIsAfter = isAfter;
        }
    }
    return bestIdx;
}

// The column a syllable is written at is the column of the note that sings it, and it holds where
// the stored tick does not. See ENCORE_FORMAT.md 6.9.
static int findBestCrByColumn(const std::vector<CrAnchor>& pairs, const std::vector<bool>& consumed,
                              int column, int maxDelta)
{
    int bestIdx = -1;
    int bestDelta = INT_MAX;
    for (size_t ni = 0; ni < pairs.size(); ++ni) {
        if (consumed[ni] || pairs[ni].column <= 0 || !pairs[ni].cr->isChord()) {
            continue;
        }
        const int delta = std::abs(pairs[ni].column - column);
        if (delta <= maxDelta && delta < bestDelta) {
            bestDelta = delta;
            bestIdx = static_cast<int>(ni);
        }
    }
    return bestIdx;
}

// The first chord after a column, still without a syllable.
static int findNextFreeCrRightOf(const std::vector<CrAnchor>& pairs, const std::vector<bool>& consumed, int column)
{
    for (size_t ni = 0; ni < pairs.size(); ++ni) {
        if (!consumed[ni] && pairs[ni].column > column && pairs[ni].cr->isChord()) {
            return static_cast<int>(ni);
        }
    }
    return -1;
}

// Attach queued lyrics to the nearest chord in the measure. Greedy "lyrics-first" assignment:
// each syllable in tick order claims the nearest available note within the threshold, so later
// syllables cannot steal a note from an earlier one.
void attachPendingLyrics(BuildCtx& ctx, const MeasEmitCtx& mc)
{
    Measure* measure = mc.measure;
    const EncMeasure& encMeas = *mc.encMeas;
    const Fraction measTick = mc.measTick;

    std::map<int, std::vector<std::pair<int, int> > > encNoteTicksByStaff
        = buildEncNoteTicksByStaff(ctx, mc, encMeas);

    // matchThreshold: half a beat in Encore ticks.
    const int beatTicksVal = encMeas.beatTicks ? static_cast<int>(encMeas.beatTicks) : 240;
    const int matchThreshold = beatTicksVal / 2;

    // Later verses store tick 0 on every syllable and keep the position only in the anchor, so build a
    // column to tick reference from the verses whose ticks span more than one value.
    std::map<int, std::vector<std::pair<int, int> > > xoffTickRefByStaff;   // staff -> [(xoffset, encTick)]
    for (const auto& [refTrack, refEntries] : ctx.scratch.pendingLyrics) {
        if (refEntries.size() < 2) {
            continue;
        }
        bool spans = false;
        for (const auto& e : refEntries) {
            if (e.encTick != refEntries.front().encTick) {
                spans = true;
                break;
            }
        }
        if (!spans) {
            continue;
        }
        auto& ref = xoffTickRefByStaff[static_cast<int>(refTrack) / VOICES];
        for (const auto& e : refEntries) {
            ref.emplace_back(e.xoffset, e.encTick);
        }
    }

    // Where no verse has spanning ticks, position by column alone: a held word can be stored at its end
    // note in one verse and at tick 0 in another. Only staves absent from the reference are touched.
    {
        std::map<int, std::vector<std::pair<int, int> > > noSpanByStaff;   // staff -> [(xoffset, encTick)]
        for (const auto& [t, es] : ctx.scratch.pendingLyrics) {
            const int staff = static_cast<int>(t) / VOICES;
            if (xoffTickRefByStaff.count(staff)) {
                continue;   // has a reliable spanning reference; handled below
            }
            auto& v = noSpanByStaff[staff];
            for (const auto& e : es) {
                v.emplace_back(e.xoffset, e.encTick);
            }
        }
        // xoffset units; adjacent notes differ by far more, cross-verse syllables by much less.
        const int kieThreshold = 25;
        for (auto& [t, es] : ctx.scratch.pendingLyrics) {
            auto sit = noSpanByStaff.find(static_cast<int>(t) / VOICES);
            if (sit == noSpanByStaff.end()) {
                continue;
            }
            for (auto& e : es) {
                for (const auto& [kx, kt] : sit->second) {
                    if (std::abs(kx - e.xoffset) <= kieThreshold && kt < e.encTick) {
                        e.encTick = kt;
                    }
                }
            }
        }
    }

    for (auto& [lyTrack, entries] : ctx.scratch.pendingLyrics) {
        if (entries.empty()) {
            continue;
        }
        // Encore multi-verse maps to voice: verse 1=voice 0, verse 2=voice 1. MuseScore anchors
        // all verses to the voice-0 chord via setVerse().
        const int lyStaffIdx = static_cast<int>(lyTrack) / VOICES;
        const int lyVerseNo = static_cast<int>(lyTrack) % VOICES;
        const track_idx_t chordTrack = static_cast<track_idx_t>(lyStaffIdx) * VOICES;

        // A collapsed verse (every syllable at the same tick) but with distinct xoffsets is the
        // tick=0 verse-2 case: remap each syllable's tick from its xoffset via the staff reference.
        {
            bool collapsed = entries.size() > 1;
            bool xoffDistinct = false;
            for (const auto& e : entries) {
                if (e.encTick != entries.front().encTick) {
                    collapsed = false;
                }
                if (e.xoffset != entries.front().xoffset) {
                    xoffDistinct = true;
                }
            }
            auto rit = xoffTickRefByStaff.find(lyStaffIdx);
            if (collapsed && xoffDistinct && rit != xoffTickRefByStaff.end()) {
                for (auto& e : entries) {
                    int bestTick = -1;
                    int bestDelta = INT_MAX;
                    for (const auto& [rx, rt] : rit->second) {
                        const int d = std::abs(rx - e.xoffset);
                        if (d < bestDelta) {
                            bestDelta = d;
                            bestTick = rt;
                        }
                    }
                    if (bestTick >= 0) {
                        e.encTick = bestTick;
                    }
                }
            }
        }

        const std::vector<std::pair<int, int> >* noteTickList = nullptr;
        {
            auto it = encNoteTicksByStaff.find(lyStaffIdx);
            if (it != encNoteTicksByStaff.end() && !it->second.empty()) {
                noteTickList = &it->second;
            }
        }
        std::vector<CrAnchor> crTickPairs
            = buildCrTickPairs(measure, measTick, encMeas, chordTrack, noteTickList);

        // Syllables are sung left to right, and the column is where each one is written; the order
        // they happen to be stored in is not. See ENCORE_FORMAT.md 6.9.
        std::stable_sort(entries.begin(), entries.end(),
                         [](const PendingLyric& a, const PendingLyric& b) {
            if (a.xoffset != b.xoffset) {
                return a.xoffset < b.xoffset;
            }
            if (a.encTick != b.encTick) {
                return a.encTick < b.encTick;
            }
            // A pair written at one place is a correction and the syllable it replaced, and which of
            // them is stored first does not survive a re-save, so it cannot decide. The fuller text
            // is taken as the one sung, and goes on the first verse.
            if (a.text.size() != b.text.size()) {
                return a.text.size() > b.text.size();
            }
            return a.text < b.text;
        });

        std::vector<bool> crConsumed(crTickPairs.size(), false);
        // Where the syllable before this one went, and from what: a pair that shares both belongs to
        // one note. See ENCORE_FORMAT.md 6.9.
        int lastIdx = -1, lastTick = -1, lastColumn = -1;
        for (const auto& pl : entries) {
            // Pass 0: the chord written in the same column, which is what the syllable sits under.
            const int kColumnSlack = 6;
            int bestIdx = pl.xoffset > 0
                          ? findBestCrByColumn(crTickPairs, crConsumed, pl.xoffset, kColumnSlack)
                          : -1;
            // Pass 0b: a syllable drawn in a column already taken was pushed left to fit, which Encore
            // does when the words are wider than their notes. It sings the next note along, and there
            // is nothing for it when no note is left. See ENCORE_FORMAT.md 6.9.
            bool crowded = false;
            if (bestIdx < 0 && lastIdx >= 0 && pl.encTick == lastTick && pl.xoffset == lastColumn) {
                bestIdx = findNextFreeCrRightOf(crTickPairs, crConsumed, pl.xoffset);
                crowded = true;
            }
            if (crowded && bestIdx < 0) {
                continue;
            }
            // Pass 1: nearest chord within the threshold, preferring notes at/before the lyric
            // tick so a slightly-misaligned lyric does not grab a later note just for proximity.
            if (bestIdx < 0) {
                bestIdx = findBestCr(crTickPairs, crConsumed, pl.encTick,
                                     /*wantChord*/ true, matchThreshold, /*preferNotAfter*/ true);
            }
            // Pass 2: nearest rest.
            if (bestIdx < 0) {
                bestIdx = findBestCr(crTickPairs, crConsumed, pl.encTick,
                                     /*wantChord*/ false, INT_MAX, /*preferNotAfter*/ false);
            }
            // Pass 3: a sung syllable always belongs to a note, so rather than drop one whose
            // stored tick sits far between notes, attach to the nearest chord at any distance.
            if (bestIdx < 0) {
                bestIdx = findBestCr(crTickPairs, crConsumed, pl.encTick,
                                     /*wantChord*/ true, INT_MAX, /*preferNotAfter*/ false);
            }
            if (bestIdx < 0) {
                continue;
            }
            crConsumed[bestIdx] = true;
            ChordRest* c = crTickPairs[bestIdx].cr;
            Lyrics* ly = Factory::createLyrics(c);
            ly->setTrack(chordTrack);
            ly->setVerse(lyVerseNo);
            ly->setXmlText(pl.text);
            LyricsSyllabic syll = LyricsSyllabic::SINGLE;
            if (pl.hyphenBefore && pl.hyphenAfter) {
                syll = LyricsSyllabic::MIDDLE;
            } else if (pl.hyphenBefore) {
                syll = LyricsSyllabic::END;
            } else if (pl.hyphenAfter) {
                syll = LyricsSyllabic::BEGIN;
            }
            ly->setSyllabic(syll);
            c->add(ly);
            ctx.scratch.lastAttachedLyric[lyTrack] = ly;
            lastIdx = bestIdx;
            lastTick = pl.encTick;
            lastColumn = pl.xoffset;
        }
        // Lyric ticks are measure-relative; unmatched leftovers cannot anchor in a
        // later measure, so discard them.
        entries.clear();
    }
    // ctx.scratch.nextLyricHyphenBefore survives barlines so a trailing hyphen (e.g. "RO -")
    // carries into the next measure's first syllable.
}
} // namespace mu::iex::enc
