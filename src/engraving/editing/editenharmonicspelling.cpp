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

#include "editenharmonicspelling.h"

#include <algorithm>
#include <vector>

#include "transpose.h"

#include "../dom/chord.h"
#include "../dom/key.h"
#include "../dom/note.h"
#include "../dom/part.h"
#include "../dom/pitchspelling.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/staff.h"

using namespace mu::engraving;

// table of alternative spellings for one octave
// each entry is the TPC of the note
//    tab1 does not contain double sharps
//    tab2 does not contain double flats

static constexpr int tab1[24] = {
    14,  2,    // 60  C   Dbb
    21,  9,    // 61  C#  Db
    16,  4,    // 62  D   Ebb
    23, 11,    // 63  D#  Eb
    18,  6,    // 64  E   Fb
    13,  1,    // 65  F   Gbb
    20,  8,    // 66  F#  Gb
    15,  3,    // 67  G   Abb
    22, 10,    // 68  G#  Ab
    17,  5,    // 69  A   Bbb
    24, 12,    // 70  A#  Bb
    19,  7,    // 71  B   Cb
};

static constexpr int tab2[24] = {
    26, 14,    // 60  B#  C
    21,  9,    // 61  C#  Db
    28, 16,    // 62  C## D
    23, 11,    // 63  D#  Eb
    30, 18,    // 64  D## E
    25, 13,    // 65  E#  F
    20,  8,    // 66  F#  Gb
    27, 15,    // 67  F## G
    22, 10,    // 68  G#  Ab
    29, 17,    // 69  G## A
    24, 12,    // 70  A#  Bb
    31, 19,    // 71  A## B
};

static constexpr int intervalPenalty[13] = {
    0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 3, 3
};

static constexpr int enharmonicSpelling[15][34] = {
    {
//Ces f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        0, 0, 0, 0, 0, 0, 0, // b
        1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Ges f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 0, 0, 0, 0, 0, 0, // b
        0, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Des f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//As  f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 0, 1, 1, 1, 1,
        0, 1, 1, 1, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Es  f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 0, 0, 1, 1, 1,
        0, 0, 1, 1, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Bb  f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 0, 0, 0, 1, 1,
        1, 0, 0, 1, 1, 1, 1, // #     // (ws) penalty for f#
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//F   f  c  g  d  a  e  b           // extra penalty for a# b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 1, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//C   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//G   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 0, 0, 0, 0, // b
        1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//D   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 1, 0, 0, 0, // b
        1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//A   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 1, 1, 0, 0, // b
        1, 1, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//E   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 1, 1, 1, 0, // b
        1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        0, 0, 1, 1, 1, 1, 1 // ##
    },
    {
//H   f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 1, 1, 1, 1, // b
        1, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 1, 1, // #
        1, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Fis f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 1, 1, 1, 1, 1, // b
        100, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, // #
        0, 1, 1, 1, 1, 1, 1 // ##
    },
    {
//Cis f  c  g  d  a  e  b
        1, 1, 1, 1, 1, 1,  // bb
        1, 1, 0, 0, 0, 0, 0, // b  //Fis
        100, 1, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, // #
        0, 0, 1, 1, 1, 1, 1 // ##
    }
};

static constexpr int penalty(int lof1, int lof2, int k)
{
    IF_ASSERT_FAILED((k >= 0 && k < 15)) {
        return 0;
    }
    assert(lof1 >= 0 && lof1 < 34);
    assert(lof2 >= 0 && lof2 < 34);
    int penalty  = enharmonicSpelling[k][lof1] * 4 + enharmonicSpelling[k][lof2] * 4;
    int distance = lof2 > lof1 ? lof2 - lof1 : lof1 - lof2;
    if (distance > 12) {
        penalty += 3;
    } else {
        penalty += intervalPenalty[distance];
    }
    return penalty;
}

static int computeWindow(const std::vector<Note*>& notes, int start, int end)
{
    int p   = 10000;
    int idx = -1;
    int pitch[10];
    int key[10];

    int i = start;
    int k = 0;
    while (i < end) {
        pitch[k] = notes[i]->pitch() % 12;
        Fraction tick = notes[i]->chord()->tick();
        key[k]   = int(notes[i]->staff()->key(tick)) + 7;
        if (key[k] < 0 || key[k] > 14) {
            LOGD("illegal key at tick %d: %d, window %d-%d",
                 tick.ticks(), key[k] - 7, start, end);
            return 0;
            // abort();
        }
        ++k;
        ++i;
    }

    for (; k < 10; ++k) {
        pitch[k] = pitch[k - 1];
        key[k]   = key[k - 1];
    }

    for (i = 0; i < 512; ++i) {
        int pa    = 0;
        int pb    = 0;
        int l     = pitch[0] * 2 + (i & 1);
        assert(l >= 0 && l <= static_cast<int>(sizeof(tab1) / sizeof(*tab1)));
        int lof1a = tab1[l];
        int lof1b = tab2[l];

        for (k = 1; k < 10; ++k) {
            int l1 = pitch[k] * 2 + ((i & (1 << k)) >> k);
            assert(l1 >= 0 && l1 <= static_cast<int>(sizeof(tab1) / sizeof(*tab1)));
            int lof2a = tab1[l1];
            int lof2b = tab2[l1];
            pa += penalty(lof1a, lof2a, key[k]);
            pb += penalty(lof1b, lof2b, key[k]);
            lof1a = lof2a;
            lof1b = lof2b;
        }
        if (pa < pb) {
            if (pa < p) {
                p   = pa;
                idx = i;
            }
        } else {
            if (pb < p) {
                p   = pb;
                idx = i * -1;
            }
        }
    }

    return idx;
}

static void changeAllTpcs(Note* n, int tpc1)
{
    if (!n) {
        return;
    }
    Interval v;
    Fraction tick = n->chord() ? n->chord()->tick() : Fraction(-1, 1);
    if (n->part() && n->part()->instrument(tick)) {
        v = n->staff()->transpose(tick);
        v.flip();
    }
    int tpc2 = Transpose::transposeTpc(tpc1, v, true);
    n->undoChangeProperty(Pid::TPC1, tpc1);
    n->undoChangeProperty(Pid::TPC2, tpc2);
    for (Note* tied : n->tiedNotes()) {
        tied->undoChangeProperty(Pid::TPC1, tpc1);
        tied->undoChangeProperty(Pid::TPC2, tpc2);
    }
}

static void spellNotelist(std::vector<Note*>& notes)
{
    static constexpr int WINDOW = 9;

    int n = int(notes.size());

    int start = 0;
    while (start < n) {
        int end = start + WINDOW;
        if (end > n) {
            end = n;
        }
        int opt = computeWindow(notes, start, end);
        const int* tab;
        if (opt < 0) {
            tab = tab2;
            opt *= -1;
        } else {
            tab = tab1;
        }

        if (start == 0) {
            changeAllTpcs(notes[0], tab[(notes[0]->pitch() % 12) * 2 + (opt & 1)]);
            if (n > 1) {
                changeAllTpcs(notes[1], tab[(notes[1]->pitch() % 12) * 2 + ((opt & 2) >> 1)]);
            }
            if (n > 2) {
                changeAllTpcs(notes[2], tab[(notes[2]->pitch() % 12) * 2 + ((opt & 4) >> 2)]);
            }
        }
        if ((end - start) >= 6) {
            changeAllTpcs(notes[start + 3], tab[(notes[start + 3]->pitch() % 12) * 2 + ((opt & 8) >> 3)]);
            changeAllTpcs(notes[start + 4], tab[(notes[start + 4]->pitch() % 12) * 2 + ((opt & 16) >> 4)]);
            changeAllTpcs(notes[start + 5], tab[(notes[start + 5]->pitch() % 12) * 2 + ((opt & 32) >> 5)]);
        }
        if (end == n) {
            int n1 = end - start;
            int k;
            switch (n1 - 6) {
            case 3:
                k = end - start - 3;
                changeAllTpcs(notes[end - 3], tab[(notes[end - 3]->pitch() % 12) * 2 + ((opt & (1 << k)) >> k)]);
                [[fallthrough]];
            case 2:
                k = end - start - 2;
                changeAllTpcs(notes[end - 2], tab[(notes[end - 2]->pitch() % 12) * 2 + ((opt & (1 << k)) >> k)]);
                [[fallthrough]];
            case 1:
                k = end - start - 1;
                changeAllTpcs(notes[end - 1], tab[(notes[end - 1]->pitch() % 12) * 2 + ((opt & (1 << k)) >> k)]);
            }
            break;
        }
        // advance to next window
        start += 3;
    }
}

void EditEnharmonicSpelling::spell(Score* score)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    for (staff_idx_t i = 0; i < score->nstaves(); ++i) {
        std::vector<Note*> notes;
        for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            track_idx_t strack = i * VOICES;
            track_idx_t etrack = strack + VOICES;
            for (track_idx_t track = strack; track < etrack; ++track) {
                EngravingItem* e = s->element(track);
                if (!e || !e->isChordRest()) {
                    continue;
                }

                if (e->isChord()) {
                    Chord* chord = toChord(e);
                    std::copy_if(chord->notes().begin(), chord->notes().end(), std::back_inserter(notes),
                                 [score](EngravingItem* chordElement) {
                        return score->selection().isNone() || chordElement->selected();
                    });
                }

                // grace notes are always chords, but their host may be a chord or a rest
                for (Chord* graceChord : toChordRest(e)->graceNotes()) {
                    std::copy_if(graceChord->notes().begin(), graceChord->notes().end(), std::back_inserter(notes),
                                 [score](EngravingItem* chordElement) {
                        return score->selection().isNone() || chordElement->selected();
                    });
                }
            }
        }

        spellNotelist(notes);
    }
}

void EditEnharmonicSpelling::spellWithSharpsOrFlats(Score* score, Prefer prefer)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    std::vector<Note*> notes = score->selection().noteList();
    for (Note* note : notes) {
        Interval interval = note->part()->instrument(note->chord()->tick())->transpose();
        int tpc1 = TPC_INVALID;
        int tpc2 = TPC_INVALID;
        if (note->staff()->concertPitch() || interval.isZero()) {
            // Note: using Key::C always and ignoring actual key signature. Otherwise, "respell with flats" would still use sharps
            // sometimes if they're in the key, and vice versa, which seems contrary to the intent of the command.
            tpc1 = pitch2tpc(note->pitch(), Key::C, prefer);
            interval.flip();
            tpc2 = Transpose::transposeTpc(tpc1, interval, true);
        } else {
            // Spell the transposed pitch first, then convert to concert pitch
            int writtenPitch = note->pitch() - interval.chromatic;
            tpc2 = pitch2tpc(writtenPitch, Key::C, prefer);
            tpc1 = Transpose::transposeTpc(tpc2, interval, true);
        }

        note->undoChangeProperty(Pid::TPC1, tpc1);
        note->undoChangeProperty(Pid::TPC2, tpc2);
        for (Note* tiedNote : note->tiedNotes()) {
            tiedNote->undoChangeProperty(Pid::TPC1, tpc1);
            tiedNote->undoChangeProperty(Pid::TPC2, tpc2);
        }
    }
}

void EditEnharmonicSpelling::changeEnharmonicSpelling(Score* score, bool both)
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    static constexpr int tpcTable[36] = {
        26, 14,  2,        // 60  B#   C   Dbb
        21, 21,  9,        // 61  C#   C#  Db
        28, 16,  4,        // 62  C##  D   Ebb
        23, 23, 11,        // 63  D#   D#  Eb
        30, 18,  6,        // 64  D##  E   Fb
        25, 13,  1,        // 65  E#   F   Gbb
        20, 20,  8,        // 66  F#   F#  Gb
        27, 15,  3,        // 67  F##  G   Abb
        22, 22, 10,        // 68  G#   G#  Ab
        29, 17,  5,        // 69  G##  A   Bbb
        24, 24, 12,        // 70  A#   A#  Bb
        31, 19,  7,        // 71  A##  B   Cb
    };

    std::vector<Note*> notes = score->selection().uniqueNotes();
    for (Note* note : notes) {
        Staff* staff = note->staff();
        if (staff->part()->instrument(note->tick())->useDrumset()) {
            continue;
        }

        if (staff->isTabStaff(note->tick())) {
            int string = note->line() + (both ? 1 : -1);
            int fret = staff->part()->stringData(note->tick(), staff->idx())->fret(note->pitch(), string, staff);
            if (fret != -1) {
                note->undoChangeProperty(Pid::FRET, fret);
                note->undoChangeProperty(Pid::STRING, string);
            }
            continue;
        }

        int tpc = note->tpc();
        for (int i = 0; i < 36; ++i) {
            if (tpcTable[i] != tpc) {
                continue;
            }

            if ((i % 3) < 2) {
                tpc = tpcTable[i] == tpcTable[i + 1] ? tpcTable[i + 2] : tpcTable[i + 1];
            } else {
                tpc = tpcTable[i - 2];
            }
            break;
        }

        note->undoSetTpc(tpc);
        if (both || staff->part()->instrument(note->chord()->tick())->transpose().isZero()) {
            int transposedTpc = note->transposeTpc(tpc);
            if (note->concertPitch()) {
                note->undoChangeProperty(Pid::TPC2, transposedTpc);
            } else {
                note->undoChangeProperty(Pid::TPC1, transposedTpc);
            }
        }
    }
}

int EditEnharmonicSpelling::bestEnharmonicFit(const std::vector<int> tpcs, Key key)
{
    int keyIndex = int(key) - int(Key::MIN);
    if (keyIndex < 0 || keyIndex >= int(Key::NUM_OF)) {
        return tpcs.front();
    }

    // Highest penalty in enharmonicSpelling available (100) + 1
    int bestPenalty = 101;
    int closestTpc = Tpc::TPC_INVALID;

    for (int tpc : tpcs) {
        if (tpc == Tpc::TPC_INVALID) {
            continue;
        }

        int lof = tpc - Tpc::TPC_MIN;
        if (lof < 0 || lof >= 34) {
            continue;
        }

        int penalty = enharmonicSpelling[keyIndex][lof];

        if (penalty < bestPenalty) {
            bestPenalty = penalty;
            closestTpc = tpc;
        }
    }

    return closestTpc;
}
