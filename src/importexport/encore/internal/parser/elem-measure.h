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

// EncMeasure: one parsed measure (time signature, tempo, barlines, repeats) and its
// element list, with the API to read it and resolve real note durations.

#pragma once

#include <QString>

#include "elem-note.h"   // MeasureElemVec, EncMeasureElem
#include "elem-enums.h"  // EncBarlineType, EncRepeatType

namespace mu::iex::enc {
struct EncFormatReader;   // defined in reader.h

struct EncMeasure {
    quint32 varsize           { 0 };
    quint16 bpm               { 0 };
    quint8 timeSigGlyph      { 0 };
    quint16 beatTicks         { 0 };
    quint16 durTicks          { 0 };
    quint8 timeSigNum        { 0 };
    quint8 timeSigDen        { 0 };
    quint8 barTypeStart      { 0 };
    quint8 barTypeEnd        { 0 };
    quint8 repeatAlternative { 0 };
    quint32 coda              { 0 };
    MeasureElemVec elements;

    EncMeasure() = default;
    EncMeasure(const EncMeasure&) = delete;
    EncMeasure& operator=(const EncMeasure&) = delete;
    EncMeasure(EncMeasure&&) noexcept = default;
    EncMeasure& operator=(EncMeasure&&) noexcept = default;

    ~EncMeasure() = default;

    EncBarlineType startBarline() const { return static_cast<EncBarlineType>(barTypeStart); }
    EncBarlineType endBarline() const { return static_cast<EncBarlineType>(barTypeEnd); }
    EncRepeatType repeatMark() const { return static_cast<EncRepeatType>(coda & 0xFF); }

    bool read(QDataStream& ds, const quint32 vs, const struct EncFormatReader& fmt);
    void calculateRealDurations(bool hasGraceTimeBorrowing, const struct EncFormatReader& fmt);
    // Snap a note whose MIDI tick drifted back to the tick of its xoffset column.
    void reconcileStaleNoteTicksByColumn();
};
} // namespace mu::iex::enc
