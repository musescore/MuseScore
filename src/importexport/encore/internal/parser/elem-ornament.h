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

// EncOrnament: articulations, dynamics, spanners (slur/wedge/ottava), trills, tempo and
// staff text, sharing one element struct whose fields vary by ornament type and format.

#pragma once

#include "elem-note.h"

namespace mu::iex::enc {
struct EncOrnament : EncMeasureElem {
    // Field names follow the Encore binary format notation used throughout the spec
    quint8 tipo      { 0 };
    qint16 yoffset   { 0 };  // signed 16-bit Cartesian y (positive = upward in Encore)
    quint8 altMezuro    { 0 };  // v0xC2 spanning measure-count lives at element +16 (not +18)
    quint8 alMezuro     { 0 };
    bool alMezuroValid  { true };  // false when format cannot guarantee measure-count semantics (v0xC2)
    quint8 xoffset2  { 0 };
    quint8 speguleco { 0 };
    quint8 noto      { 0 };
    quint8 tempo     { 0 };
    quint8 tind      { 0 };
    // Element-relative offsets for formats whose ornament does not follow the v0xC4 field order;
    // -1 means read the field inline. All three are set from EncFormatReader before read().
    // v0xA6's compact ornament uses all of them. See ENCORE_FORMAT.md §6.8 Ornament.
    int tindOffset      { -1 };   // staff-text TEXT index
    int yByteOffset     { -1 };   // y as a signed byte, every subtype
    int measCountOffset { -1 };   // forward measure count, every subtype

    using EncMeasureElem::EncMeasureElem;

    EncOrnamentType ornType() const { return static_cast<EncOrnamentType>(tipo); }
    void setOrnType(EncOrnamentType t) { tipo = static_cast<quint8>(t); }

    bool read(QDataStream& ds) override;
};
} // namespace mu::iex::enc
