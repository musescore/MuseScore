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

// Read the metadata blocks: TK instrument names, LINE per-staff layout/clef/key, and TITL text.

#include "elem.h"

#include <algorithm>

#include "parsers-encoding.h"
#include "readers.h"

namespace mu::iex::enc {
// ---------------------------------------------------------------------------
// EncInstrument
// ---------------------------------------------------------------------------

bool EncInstrument::read(QDataStream& ds, quint32 vs, bool probeEncoding)
{
    // The block-size field is little-endian even in SCO5 files, whose payload is otherwise
    // big-endian; undo the big-endian read so the low 16 bits hold the size in both byte orders.
    // A big-endian read would mask to 0, skip the name-scan loop, and lose every instrument name.
    const quint32 sizeField = (ds.byteOrder() == QDataStream::BigEndian) ? qbswap(vs) : vs;
    offset = sizeField & 0xFFFF;
    // Encoding probe overrides charSize(); see ENCORE_FORMAT.md §Encoding probe.
    EncCharSize cs = charSize();
    if (probeEncoding) {
        const qint64 savedPos = ds.device()->pos();
        quint8 b0 = 0, b1 = 0;
        ds >> b0 >> b1;
        ds.device()->seek(savedPos);
        if (probeUtf16LE(b0, b1)) {
            cs = EncCharSize::TWO_BYTES;
        } else if (b0 >= 0x20 && b0 < 0x7F && b1 != 0x00 && b1 >= 0x20 && b1 < 0xFF) {
            cs = EncCharSize::ONE_BYTE;
        }
    }
    int nread = 8;
    QChar ch;
    bool done = false;
    // Bound the name scan by the block's declared content length and stream status: without it an
    // unterminated name field would spin forever and a truncated one would fabricate zeroed chars.
    const int nameLimit = static_cast<int>(offset);
    while (!done && nread < nameLimit && ds.status() == QDataStream::Ok) {
        if (cs == EncCharSize::ONE_BYTE) {
            quint8 b;
            ds >> b;
            ch = QChar(char16_t(b));
            nread += 1;
        } else {
            quint8 lo, hi;
            ds >> lo >> hi;
            ch = QChar(char16_t((hi << 8) + lo));
            nread += 2;
        }
        if (ch == '\0') {
            done = true;
        } else {
            name.append(ch);
        }
    }
    // Skip whatever remains of the block after the name, clamped to the device (offset is
    // an untrusted file value). nread counts bytes consumed from the block's 8-byte header.
    skipBlock(ds, static_cast<qint64>(offset) - nread);
    return true;
}

// ---------------------------------------------------------------------------
// EncLineStaffData / EncLine
// ---------------------------------------------------------------------------

bool EncLineStaffData::read(QDataStream& ds)
{
    // 30-byte staff entry; byte offsets and field meanings in ENCORE_FORMAT.md §LINE staff entry (30 bytes).
    ds.skipRawData(13);                         // bytes 0-12: visual layout
    ds >> staffSizeHint;                        // byte 13: display size (0=60% .. 3=100%)
    qint8 ct;
    ds >> ct;                                   // byte 14: clef type
    clef = static_cast<EncClefType>(ct);
    ds >> key >> pageIdx;                       // bytes 15-16
    quint8 skip0, skip1, showByte;
    ds >> skip0 >> skip1 >> showByte;           // bytes 17-19
    showStaff = (showByte != 0);
    (void)skip0;
    (void)skip1;
    quint8 st;
    ds >> st;                                   // byte 20: staff type
    staffType = static_cast<EncStaffType>(st);
    ds >> instrStaffIdx;                        // byte 21
    ds.skipRawData(8);                          // bytes 22-29
    return true;
}

bool EncLine::read(QDataStream& ds, quint32 vs, int staffPerSystem)
{
    // LINE block layout constants. kStaffEntryBytes is one EncLineStaffData staff entry;
    // kBlockHeaderBytes is the magic + size already consumed on entry; kLinePrefixBytes is the
    // fixed prefix before the staff entries (the header plus the 13 bytes of skip + start +
    // measureCount read below).
    static constexpr int kBlockHeaderBytes = 8;
    static constexpr int kLinePrefixBytes  = 21;
    static constexpr int kStaffEntryBytes  = 30;

    offset = vs;
    ds.skipRawData(10);
    ds >> start >> measureCount;
    // staffPerSystem comes from the header; clamp it to the staff entries the block can
    // actually hold so a corrupt count cannot spin the loop fabricating zeroed staves, and
    // stop early if the stream runs out.
    const int maxStaves = static_cast<int>(offset) / kStaffEntryBytes;
    if (staffPerSystem > maxStaves) {
        staffPerSystem = maxStaves;
    }
    for (int i = 0; i < staffPerSystem && ds.status() == QDataStream::Ok; ++i) {
        EncLineStaffData lsd;
        lsd.read(ds);
        staffData.push_back(lsd);
    }
    // Skip the block's tail after the parsed staff entries, clamped to the device.
    const qint64 toSkip = static_cast<qint64>(offset) + kBlockHeaderBytes
                          - kLinePrefixBytes - static_cast<qint64>(kStaffEntryBytes) * staffPerSystem;
    skipBlock(ds, toSkip);
    return true;
}

// ---------------------------------------------------------------------------
// Title block
// ---------------------------------------------------------------------------

// Read 30-byte prefix + text payload of one TITL line; layout in ENCORE_FORMAT.md §TITL block.
// Alignment lives at prefix+14; the text field is a fixed width per encoding.
static constexpr int kTitlTextBytesOneByte = 66;
static constexpr int kTitlTextBytesTwoByte = 1026;

// blockEnd bounds every read to the TITL block's declared end (startPos + varSize). A truncated
// block would otherwise pull zero-fill past EOF and, worse, a block shorter than the fixed line
// structure would read into the following block and desync the top-level magic scan. No read here
// crosses blockEnd, so EncTitle::read can realign exactly with skipToBlockEnd afterwards.
static EncHeaderFooter readTitleLine(QDataStream& ds, EncCharSize cs, qint64 blockEnd)
{
    const qint64 prefixAvail = std::max<qint64>(0, blockEnd - ds.device()->pos());
    const int prefixWant = static_cast<int>(std::min<qint64>(30, prefixAvail));
    QByteArray prefix(30, 0);
    int got = (prefixWant > 0) ? ds.readRawData(prefix.data(), prefixWant) : 0;
    quint8 alignByte = (got >= 15) ? static_cast<quint8>(prefix[14]) : 0;

    QString item;
    bool done = false;
    if (cs == EncCharSize::ONE_BYTE) {
        for (int j = 0; j < kTitlTextBytesOneByte; ++j) {
            if (ds.device()->pos() >= blockEnd || ds.status() != QDataStream::Ok) {
                break;
            }
            quint8 b;
            ds >> b;
            if (b == 0) {
                done = true;
            }
            if (!done) {
                item.append(QChar(char16_t(b)));
            }
        }
    } else {
        for (int j = 0; j < kTitlTextBytesTwoByte;) {
            if (ds.device()->pos() >= blockEnd - 1 || ds.status() != QDataStream::Ok) {
                break;
            }
            quint8 lo, hi;
            ds >> lo;
            ++j;
            ds >> hi;
            ++j;
            QChar ch = QChar(char16_t((hi << 8) + lo));
            if (ch == '\0') {
                done = true;
            }
            if (!done) {
                item.append(ch);
            }
        }
    }

    EncHeaderFooter out;
    out.text = item;
    switch (alignByte) {
    case static_cast<quint8>(EncTextAlign::CENTER): out.align = EncTextAlign::CENTER;
        break;
    case static_cast<quint8>(EncTextAlign::RIGHT):  out.align = EncTextAlign::RIGHT;
        break;
    default:                                        out.align = EncTextAlign::LEFT;
        break;
    }
    return out;
}

QString readTextItem(QDataStream& ds, EncCharSize cs, qint64 blockEnd)
{
    return readTitleLine(ds, cs, blockEnd).text;
}

bool EncTitle::read(QDataStream& ds, quint32 vs, EncCharSize cs)
{
    // Detect encoding from varsize alone, not from TK-derived cs: the ONE_BYTE and TWO_BYTES
    // block layouts differ by ~10x, so varsize resolves it unambiguously. Needed because Encore
    // 5.0.2 can write UTF-16 in TITL even when the TK offset says one-byte.
    if (vs >= 10000) {
        cs = EncCharSize::TWO_BYTES;
    } else if (vs > 0 && vs < 5000) {
        cs = EncCharSize::ONE_BYTE;
    }
    // Some files save two identical TITL blocks. Clear vectors so the
    // second read replaces the first instead of doubling all lines.
    subtitle.clear();
    instruction.clear();
    author.clear();
    header.clear();
    footer.clear();
    copyright.clear();

    // Bound every line read to the block's declared end so a truncated or short TITL block cannot
    // read past EOF or into the following block. varSize excludes the 8-byte block header, so the
    // end lies at startPos + varSize.
    const qint64 startPos = ds.device()->pos();
    const qint64 blockEnd = startPos + static_cast<qint64>(vs);

    ds.skipRawData(2);
    title = readTextItem(ds, cs, blockEnd);
    for (int i = 0; i < 2; ++i) {
        subtitle.push_back(readTextItem(ds, cs, blockEnd));
    }
    for (int i = 0; i < 3; ++i) {
        instruction.push_back(readTextItem(ds, cs, blockEnd));
    }
    for (int i = 0; i < 4; ++i) {
        author.push_back(readTextItem(ds, cs, blockEnd));
    }
    for (int i = 0; i < 2; ++i) {
        header.push_back(readTitleLine(ds, cs, blockEnd));
    }
    for (int i = 0; i < 2; ++i) {
        footer.push_back(readTitleLine(ds, cs, blockEnd));
    }
    for (int i = 0; i < 6; ++i) {
        copyright.push_back(readTextItem(ds, cs, blockEnd));
    }
    // Realign to the block end; the fixed trailing pad (504 / 120 bytes) is whatever is left.
    skipToBlockEnd(ds, startPos, static_cast<qint64>(vs));
    return true;
}
} // namespace mu::iex::enc
