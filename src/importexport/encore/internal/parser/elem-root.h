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

// Top-level document structs: EncRoot and its parts (instruments, system lines, title,
// header, text, page/print setup) plus the free functions that parse and stitch them.

#pragma once

#include <memory>
#include <vector>

#include <QString>

#include "elem-enums.h"
#include "elem-note.h"     // MeasureElemVec, EncMeasureElem
#include "elem-measure.h"  // EncMeasure (used in EncRoot::measures)
#include "elem-text.h"     // EncLyric, EncTie, EncChordSym

namespace mu::iex::enc {
struct EncFormatReader;   // defined in reader.h

// ---------------------------------------------------------------------------
// Instrument / part
// ---------------------------------------------------------------------------

// Tablature tuning: open-string MIDI pitches (low -> high). See ENCORE_FORMAT.md §Tab tuning.
struct EncTabTuning {
    bool hasData { false };
    std::vector<int> openStringPitches;   // low -> high, as stored by Encore

    int strings() const { return static_cast<int>(openStringPitches.size()); }
};

struct EncInstrument {
    QString name;
    quint32 offset    { 0 };
    qint64 contentFilePos { -1 };   // byte offset of TK content start (after 8-byte header); -1 for compact
    int nstaves   { 0 };
    int midiProgram { 0 };   // 1-indexed GM program (0 = not configured)
    // Signed chromatic offset from Encore's Staff Sheet "Key" field.
    // 0=written, -12=octave lower, +12=octave higher. v0xC4 only.
    qint8 keyTransposeSemitones { 0 };
    // Per-instrument tab tuning, from the last 8 bytes of this TK block (each track carries its own).
    EncTabTuning tabTuning;

    EncCharSize charSize() const { return (offset > 250) ? EncCharSize::TWO_BYTES : EncCharSize::ONE_BYTE; }

    bool read(QDataStream& ds, quint32 vs, bool probeEncoding = false);
};

// ---------------------------------------------------------------------------
// Staff data within a system line
// ---------------------------------------------------------------------------

struct EncLineStaffData {
    EncClefType clef       { EncClefType::G };
    quint8 key        { 0 };
    quint8 pageIdx    { 0 };
    EncStaffType staffType  { EncStaffType::MELODY };
    quint8 instrStaffIdx { 0 };
    bool showStaff { true }; // byte +19 of LINE staff entry: 0x01 = visible, 0x00 = hidden.
    // Staff display size: byte +13 of LINE staff entry, 0-indexed (0=Size1/60%, 1=Size2/70%, 2=Size3/75%, 3=Size4/100%).
    quint8 staffSizeHint { 3 };

    unsigned int instrumentIndex() const { return instrStaffIdx & 0x3F; }
    unsigned int staffIndex() const { return instrStaffIdx >> 6; }

    bool read(QDataStream& ds);
};

struct EncLine {
    quint32 offset       { 0 };
    quint16 start        { 0 };
    quint8 measureCount { 0 };
    std::vector<EncLineStaffData> staffData;
    // v0xA6 only: per-staff written key index (Encore key index 0-14), parsed directly
    // from the 22-byte staff entries because v0xA6's header staffPerSystem and LINE staff
    // layout differ from v0xC2/C4, which leaves staffData empty. See parsers-root.cpp.
    std::vector<quint8> staffKeys;

    bool read(QDataStream& ds, quint32 vs, int staffPerSystem);
};

// ---------------------------------------------------------------------------
// Title block
// ---------------------------------------------------------------------------

QString readTextItem(QDataStream& ds, EncCharSize cs, qint64 blockEnd);

struct EncHeaderFooter {
    QString text;
    EncTextAlign align { EncTextAlign::LEFT };
};

struct EncTitle {
    QString title;
    std::vector<QString> subtitle;
    std::vector<QString> instruction;
    std::vector<QString> author;
    std::vector<EncHeaderFooter> header;
    std::vector<EncHeaderFooter> footer;
    std::vector<QString> copyright;

    bool read(QDataStream& ds, quint32 vs, EncCharSize cs);

    bool hasContent() const
    {
        if (!title.isEmpty()) {
            return true;
        }
        auto anyNonEmpty = [](const std::vector<QString>& v) {
            for (const auto& s : v) {
                if (!s.isEmpty()) {
                    return true;
                }
            }
            return false;
        };
        auto anyHFNonEmpty = [](const std::vector<EncHeaderFooter>& v) {
            for (const auto& hf : v) {
                if (!hf.text.isEmpty()) {
                    return true;
                }
            }
            return false;
        };
        return anyNonEmpty(subtitle) || anyNonEmpty(instruction) || anyNonEmpty(author)
               || anyHFNonEmpty(header) || anyHFNonEmpty(footer) || anyNonEmpty(copyright);
    }
};

// ---------------------------------------------------------------------------
// File header
// ---------------------------------------------------------------------------

struct EncHeader {
    QString magic;
    quint8 chuMagio       { 0 };
    quint16 chuVersio      { 0 };
    // Parsed for format completeness and as read-order cursors, not consumed by the importer:
    // reserved header words after the version. Removing them shifts every subsequent header read.
    quint16 nekon1         { 0 };
    quint16 fiksa1         { 0 };
    qint16 lineCount      { 0 };
    qint16 pageCount      { 0 };
    qint8 instrumentCount{ 0 };
    qint8 staffPerSystem { 0 };
    qint16 measureCount   { 0 };
    quint8 formatRev      { 0 };  // format-revision byte at 0x3E: 1 = Encore 4.5, 4 = Encore 5.0 (v0xC4)
    quint8 scoreSize      { 4 };  // staff-size selector 1-4 at header offset 0x52; 4 = default

    bool readMagicAndVersion(QDataStream& ds);
    bool read(QDataStream& ds, const EncFormatReader& fmt);
};

// ---------------------------------------------------------------------------
// EncRoot: top-level container
// ---------------------------------------------------------------------------

bool isInstrumentMagic(const QString& magic);
bool isKnownMagic(const QString& magic);
QString findNextKnownMagic(QDataStream& ds);
void addSpannerEnds(std::vector<EncMeasure>& measures);

// TEXT block: N-th entry referenced by ORN tind byte. textOffset (from EncFormatReader) is the
// per-entry text offset (14 for v0xC4/v0xC2, 0 for v0xA6). See ENCORE_FORMAT.md §TEXT block.
struct EncTextBlock {
    std::vector<QString> entries;

    bool read(QDataStream& ds, quint32 varSize, int textOffset = 14, bool hasRunHeader = true);
};

// WINI block: margins in points (1/72 inch). See ENCORE_FORMAT.md §WINI block.
struct EncPageSetup {
    bool hasData      { false };
    qint32 top        { 0 };   // top margin in pts
    qint32 left       { 0 };   // left margin in pts
    qint32 bottomEdge { 0 };   // pageHeight_pts - bottomMargin_pts
    qint32 rightEdge  { 0 };   // pageWidth_pts  - rightMargin_pts

    // Decode the WINI block margins in place; consumes the whole block (clamped to the stream).
    void read(QDataStream& ds, quint32 varSize);
};

// PREC block: a Windows DEVMODE. Page size, orientation and notation scale.
// See ENCORE_FORMAT.md §PREC block.
struct EncPrintSetup {
    bool hasData     { false };
    int orientation  { 0 };   // dmOrientation: 1=portrait, 2=landscape
    int paperSize    { 0 };   // dmPaperSize (DMPAPER_*): 1=Letter, 5=Legal, 8=A3, 9=A4, 11=A5, ...
    int paperLength  { 0 };   // dmPaperLength: tenths of a millimetre (custom sizes only)
    int paperWidth   { 0 };   // dmPaperWidth:  tenths of a millimetre (custom sizes only)
    int scale        { 0 };   // dmScale: notation/print scale percent (100 = default)
};

// Parse the SCO5 (macOS Encore 5) NSPrintInfo XML plist found in the PREC block into
// orientation / paper size / scale. Returns false when the buffer is not a usable plist.
bool parsePrecPlist(const QByteArray& buf, EncPrintSetup& out);

struct EncRoot {
    EncHeader header;
    std::vector<EncInstrument> instruments;
    std::vector<EncLine> lines;
    EncTabTuning tabTuning;
    std::vector<EncMeasure> measures;
    EncTitle titleBlock;
    EncTextBlock textBlock;
    EncPageSetup pageSetup;
    EncPrintSetup printSetup;
    std::unique_ptr<struct EncFormatReader> fmt;  // set during read()

    bool read(QDataStream& ds);
};
} // namespace mu::iex::enc
