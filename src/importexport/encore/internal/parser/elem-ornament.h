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
    // Byte offset of tind from the type/voice byte, or -1 to read it inline by size. Set from
    // EncFormatReader::staffTextTindOffset(); v0xA6's compact ornament stores tind at +26.
    int tindOffset  { -1 };
    // Byte offset of yoffset from the type/voice byte, or -1 to read it inline. Set from
    // EncFormatReader::staffTextYoffsetOffset(); v0xA6's compact ornament stores it at +6.
    int yoffOffset  { -1 };

    using EncMeasureElem::EncMeasureElem;

    EncOrnamentType ornType() const { return static_cast<EncOrnamentType>(tipo); }
    void setOrnType(EncOrnamentType t) { tipo = static_cast<quint8>(t); }

    bool read(QDataStream& ds) override;
};
} // namespace mu::iex::enc
