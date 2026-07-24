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

// Text-bearing measure elements: chord symbols, lyrics and ties, each with the format
// layout fields (anchor bytes, gaps, arc endpoints) their read() needs.

#pragma once

#include <QString>

#include "elem-note.h"

namespace mu::iex::enc {
struct EncChordSym : EncMeasureElem {
    quint8 toniko { 0 };
    quint8 tipo   { 0 };
    quint8 radiko { 0 };
    quint8 baso   { 0 };
    bool hasFretDiagram { false };   // tipo bit 2 (0x04): Encore draws a guitar frame
    QString teksto;

    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
    QString chordName() const;
};

struct EncLyric : EncMeasureElem {
    QString text;
    quint8 kie { 0 };                // location/anchor byte (similar to xoffset)
    // Lyric layout, set from the EncFormatReader so read() stays format-agnostic.
    quint8 preKieSkip { 5 };        // bytes from the post-header cursor to the kie byte
    quint8 textGapAfterKie { 9 };   // bytes to skip after kie before text
    quint8 spacingFactor { 1 };     // element slot = size * spacingFactor

    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
};

// TIE element: dir byte (+5) and startFlag (+6) encode arc direction. See ENCORE_FORMAT.md §TIE element.
struct EncTie : EncMeasureElem {
    bool isTieStart { false };      // true when dir byte has bit 7 or bit 1 set, or startFlag has bit 7 set
    quint8 arcX1         { 0 };     // arc start x (element offset +10); only valid for size >= 18
    quint8 arcX2         { 0 };     // arc end   x (element offset +12); only valid for size >= 18
    qint8 sourcePosition { -1 };    // staff position of source note (+14); -1 = all notes in chord

    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
};
} // namespace mu::iex::enc
