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

// Read CHORD symbols and decode the numeric root/quality/bass encoding into a chord name.

#include "elem-text.h"
#include "parsers-encoding.h"

namespace mu::iex::enc {
// Chord quality suffixes indexed by toniko value (0-63): Encore's own chord palette in palette
// order. Encore typography is mapped to what the MuseScore chord parser expects: augmented-fifth
// "+5" becomes "#5", and "sus2,sus4" drops the comma. See ENCORE_FORMAT.md §CHORD symbol element.
static const char* const kChordQuality[] = {
    "",            //  0: major (no suffix)
    "m",           //  1: minor
    "+",           //  2: augmented
    "dim",         //  3: diminished
    "dim7",        //  4: diminished 7
    "5",           //  5: power chord
    "6",           //  6: major 6
    "6/9",         //  7
    "(add2)",      //  8
    "(add9)",      //  9
    "(omit3)",     // 10
    "(omit5)",     // 11
    "maj7",        // 12
    "maj7(b5)",    // 13
    "maj7(6/9)",   // 14
    "maj7(#5)",    // 15 (Encore "maj7(+5)")
    "maj7(#11)",   // 16
    "maj9",        // 17
    "maj9(b5)",    // 18
    "maj9(#5)",    // 19 (Encore "maj9(+5)")
    "maj9(#11)",   // 20
    "maj13",       // 21
    "maj13(b5)",   // 22
    "maj13(#11)",  // 23
    "7",           // 24: dominant 7
    "7(b5)",       // 25
    "7(b9)",       // 26
    "7(#9)",       // 27
    "7(#11)",      // 28
    "7(b5,b9)",    // 29
    "7(b5,#9)",    // 30
    "7(b9,#9)",    // 31
    "9",           // 32
    "9(b5)",       // 33
    "9(#11)",      // 34
    "11",          // 35
    "13",          // 36
    "13(b5)",      // 37
    "13(b9)",      // 38
    "13(#9)",      // 39
    "13(#11)",     // 40
    "+7",          // 41: augmented 7
    "+7(b9)",      // 42
    "+7(#9)",      // 43
    "+9",          // 44
    "sus2",        // 45
    "sus2sus4",    // 46 (Encore "sus2,sus4"; comma removed for MuseScore parser)
    "sus4",        // 47
    "7sus4",       // 48
    "9sus4",       // 49
    "13sus4",      // 50
    "m(add2)",     // 51
    "m(add9)",     // 52
    "m6",          // 53
    "m6/9",        // 54
    "m7",          // 55
    "m(maj7)",     // 56
    "m7(b5)",      // 57
    "m7(add4)",    // 58
    "m7(add11)",   // 59
    "m9",          // 60
    "m9(maj7)",    // 61
    "m11",         // 62
    "m13",         // 63
};
static constexpr int kChordQualityCount = static_cast<int>(sizeof(kChordQuality) / sizeof(kChordQuality[0]));

// Note names for the lower nibble of radiko/baso (0=C, 1=D, 2=E, 3=F, 4=G, 5=A, 6=B).
static const char* const kNoteNames[] = { "C", "D", "E", "F", "G", "A", "B" };
static constexpr int kNoteNameCount   = static_cast<int>(sizeof(kNoteNames) / sizeof(kNoteNames[0]));

static QString encRootToString(quint8 field)
{
    const int noteIdx = field & 0x0F;
    if (noteIdx >= kNoteNameCount) {
        return {};
    }
    QString root = QString::fromLatin1(kNoteNames[noteIdx]);
    const int acc = (field & 0xF0) >> 4;
    if (acc == 1) {
        root += u'#';
    } else if (acc == 2) {
        root += u'b';
    }
    return root;
}

bool EncChordSym::read(QDataStream& ds)
{
    EncMeasureElem::read(ds);
    ds >> toniko >> tipo;
    ds.skipRawData(3);
    ds >> xoffset;
    ds.skipRawData(1);
    ds >> radiko >> baso;
    // tipo bit 2 = draw a fretboard diagram above the symbol. See ENCORE_FORMAT.md §CHORD symbol element.
    hasFretDiagram = (tipo & 0x04) != 0;
    const bool hasText = (tipo & 1);
    if (hasText) {
        teksto = readEncodedStringFixed(ds, 36);   // fixed 36-byte text slot
    }
    // No trailing skip: the element loop reseeks to the element end after read().
    return true;
}

QString EncChordSym::chordName() const
{
    if (!teksto.isEmpty()) {
        return teksto;
    }

    const QString root = encRootToString(radiko);
    if (root.isEmpty()) {
        return {};
    }

    const QString quality = (toniko < kChordQualityCount)
                            ? QString::fromLatin1(kChordQuality[toniko])
                            : QString{};

    QString name = root + quality;

    if (tipo & 0x02) {
        const QString bass = encRootToString(baso);
        if (!bass.isEmpty()) {
            name += QChar(u'/') + bass;
        }
    }

    return name;
}
} // namespace mu::iex::enc
