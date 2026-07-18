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

// Measure-element structs: the EncMeasureElem base plus note, rest, key/clef change,
// MIDI CC and generic elements, with the flags parsing derives (grace, tie, tuplet).

#pragma once

#include <memory>
#include <vector>

#include <QDataStream>

#include "elem-enums.h"

namespace mu::iex::enc {
// Notes within this many Encore ticks treated as simultaneous (MIDI timing drift).
inline constexpr int CHORD_CLUSTER_THRESHOLD = 4;   // Encore ticks (~8ms at 120bpm)

// faceValue byte accessors: low nibble = duration (1=whole..8=128th), high nibble = notehead type.
inline quint8 fvLow(quint8 fv) { return fv & 0x0F; }
inline quint8 fvHigh(quint8 fv) { return static_cast<quint8>((fv >> 4) & 0x0F); }

// Base class for all measure elements.
struct EncMeasureElem {
    quint16 tick  { 0 };
    quint8 type  { 0 };
    quint8 voice { 0 };
    quint8 size  { 0 };
    quint8 staffIdx    { 0 };   // low 6 bits of raw staff byte: staff index in system
    quint8 staffWithin { 0 };   // high 2 bits (>> 6): staff index within instrument (0=first, 1=second, ...)
    quint8 xoffset  { 0 };
    qint16 realDuration { -1 };

    // Raw staff byte (staffWithin<<6)|staffIdx, identical to instrStaffIdx in the LINE block.
    // Importers reverse-map it to a LINE slot (see buildLineSlotByRawByte).
    quint8 rawStaffByte() const
    {
        return static_cast<quint8>((static_cast<quint8>(staffWithin) << 6) | static_cast<quint8>(staffIdx));
    }

    // Nonzero = tuplet member; sort tuplet notes first at their tick so they create the chord.
    virtual quint8 tupletByte() const { return 0; }
    // Raw faceValue byte; 0 for elements without one.
    virtual quint8 faceValueByte() const { return 0; }
    virtual bool impliedTupletMember() const { return false; }

    EncMeasureElem() = default;
    EncMeasureElem(quint16 t, quint8 tp, quint8 v)
        : tick(t), type(tp), voice(v) {}
    virtual ~EncMeasureElem() = default;

    virtual bool read(QDataStream& ds);
};

// A pitched note: face value (duration + notehead), MIDI pitch, articulations, tuplet ratio,
// and grace/tie flags derived during parsing.
struct EncNote : EncMeasureElem {
    quint8 faceValue       { 0 };
    quint8 grace1          { 0 };
    quint8 grace2          { 0 };
    qint8 position        { 0 };
    quint8 tuplet          { 0 };
    quint8 dotControl      { 0 };
    quint8 semiTonePitch   { 0 };
    // Parsed for format completeness and as a read-order cursor, not consumed by any emitter: Encore's
    // playback (as-performed) duration; the importer takes rhythm from faceValue. Removing it shifts
    // every subsequent field read in EncNote::read, so keep it (or an equivalent byte skip).
    quint16 playbackDurTicks{ 0 };
    // Parsed for format completeness and as read-order cursors, not consumed by any emitter: velocity
    // is Encore's per-note MIDI velocity and alterationGlyph its explicit-accidental glyph selector;
    // MuseScore derives both from the score model. Removing either shifts subsequent field reads.
    quint8 velocity        { 0 };
    quint8 options         { 0 };
    quint8 alterationGlyph { 0 };
    quint8 articulationUp  { 0 };
    quint8 articulationDown{ 0 };
    // Set by calculateRealDurations() for v0xA6: note is a non-leading grace
    // within a grace group (shorter duration than the leading grace).
    bool isInnerGrace           { false };
    // Set by postProcessElement() for formats where grace1 low nibble encodes tie-sender (v0xC2).
    bool isTieSender            { false };
    // Set by calculateRealDurations() Phase 4 for v0xC2: note belongs to an implied tuplet group
    // (rdur/faceValue mismatch gives the ratio). Explicit flag so incidental MIDI timing drift
    // in other formats is never misread as a tuplet.
    bool isImpliedTupletMember  { false };
    // Set by fixDottedEighthPattern() (v0xC2): forces dots=1 for the dotted-eighth in the
    // dotted-eighth+sixteenth anomaly, bypassing the unreliable dotControl bit-0 fallback.
    bool forceDotted            { false };
    // Note materialized from a tab-only staff's pitch-bearing REST element (rest byte layout, so
    // faceValue is derived from realDuration later; see parsers-measure.cpp / EncRoot::read).
    bool fromTabFingering       { false };

    using EncMeasureElem::EncMeasureElem;

    quint8 tupletByte() const override { return tuplet; }
    quint8 faceValueByte() const override { return faceValue; }
    bool impliedTupletMember() const override { return isImpliedTupletMember; }
    int actualNotes() const { return tuplet >> 4; }
    int normalNotes() const { return tuplet & 0x0F; }

    EncGraceType graceType() const;
    // grace1 bit 0x20 = small note (a grace or a cue). grace2 bit 0x01 = muted (playback off), a
    // per-note Encore flag independent of size; a cue is small and muted by default.
    bool isSmall() const { return grace1 & 0x20; }
    bool isMuted() const { return grace2 & 0x01; }

    bool read(QDataStream& ds) override;
};

// A rest; mrestCount > 1 marks an Encore multi-measure rest (v0xC4) shown as one symbol.
struct EncRest : EncMeasureElem {
    quint8 faceValue  { 0 };
    quint8 tuplet     { 0 };
    quint8 dotControl { 0 };
    // Multi-measure rest display count (v0xC4): > 1 means this one MEAS block spans that
    // many empty measures. Only meaningful when it is the block's sole REST. See ENCORE_FORMAT.md §REST element.
    quint8 mrestCount { 1 };
    // Set by calculateRealDurations() Phase 4 for v0xC2 (same semantics as EncNote::isImpliedTupletMember).
    bool isImpliedTupletMember { false };

    using EncMeasureElem::EncMeasureElem;

    quint8 tupletByte() const override { return tuplet; }
    quint8 faceValueByte() const override { return faceValue; }
    bool impliedTupletMember() const override { return isImpliedTupletMember; }
    int actualNotes() const { return tuplet >> 4; }
    int normalNotes() const { return tuplet & 0x0F; }

    bool read(QDataStream& ds) override;
};

// Mid-measure key-signature change (tipo = Encore key index).
struct EncKeyChange : EncMeasureElem {
    quint8 tipo { 0 };

    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
};

// Mid-measure clef change.
struct EncClefChange : EncMeasureElem {
    EncClefType clefType { EncClefType::G };

    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
};

// Placeholder for element types the importer does not model; carried through but not emitted.
struct EncGenericElem : EncMeasureElem {
    using EncMeasureElem::EncMeasureElem;

    bool read(QDataStream& ds) override;
};

// Inline MIDI Control Change (EncElemType::MIDI_CC). Playback only, no notation: the importer
// logs controller/value and drops it. See ENCORE_FORMAT.md §MIDI control change (type 11).
struct EncMidiCc : EncMeasureElem {
    using EncMeasureElem::EncMeasureElem;

    quint8 controller { 0 };   // 64=sustain pedal, 7=volume, 1=modulation
    quint8 value      { 0 };   // 127=max/on, 0=off

    bool read(QDataStream& ds) override;
};

using MeasureElemVec    = std::vector<std::unique_ptr<EncMeasureElem> >;
using MeasureElemRefVec = std::vector<const EncMeasureElem*>;
} // namespace mu::iex::enc
