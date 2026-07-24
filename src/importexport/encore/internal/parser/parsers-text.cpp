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

// Read the file header (version/counts/score size) and the indexed TEXT block for staff text.

#include "elem.h"
#include "readers.h"

namespace mu::iex::enc {
// ---------------------------------------------------------------------------
// EncHeader
// ---------------------------------------------------------------------------

bool EncHeader::readMagicAndVersion(QDataStream& ds)
{
    for (int i = 0; i < 4; ++i) {
        quint8 ch;
        ds >> ch;
        magic.append(QChar(ch));
    }
    if (magic == "SCOW") {
        ds.setByteOrder(QDataStream::LittleEndian);
    } else if (magic == "SCO5") {
        ds.setByteOrder(QDataStream::BigEndian);
    } else {
        return false;
    }
    ds >> chuMagio;
    return true;
}

bool EncHeader::read(QDataStream& ds, const EncFormatReader& fmt)
{
    ds.skipRawData(0x28 - 5);   // from +5 (just past the 5-byte magic) to the header fields at 0x28
    ds >> chuVersio >> nekon1 >> fiksa1 >> lineCount >> pageCount;
    ds >> instrumentCount >> staffPerSystem >> measureCount;   // cursor now at 0x36
    // Format-revision byte at 0x3E distinguishes releases that share an app version:
    // 1 = Encore 4.5, 4 = Encore 5.0 (both v0xC4 with chuVersio 1056). Meaningless for v0xA6.
    ds.skipRawData(0x3E - 0x36);
    ds >> formatRev;                                           // cursor now at 0x3F
    // Global staff-size selector (1=small … 4=default). v0xC2/C4/C5 store it at 0x52;
    // v0xA6 stores it at 0x8D (byte 0x52 is an unrelated field there). Offset from fmt.
    const qint64 szOff = fmt.scoreSizeOffset();
    if (fmt.headerEnd() > szOff) {
        ds.skipRawData(static_cast<int>(szOff - 0x3F));   // skip from 0x3F to the size byte
        quint8 sz;
        ds >> sz;
        if (sz >= 1 && sz <= 4) {
            scoreSize = sz;
        }
        ds.skipRawData(static_cast<int>(fmt.headerEnd() - szOff - 1)); // skip to header end
    } else {
        ds.skipRawData(static_cast<int>(fmt.headerEnd() - 0x3F));
    }
    return true;
}

// ---------------------------------------------------------------------------
// EncTextBlock - indexed text payload for STAFFTEXT 0x1E ornaments
// ---------------------------------------------------------------------------

bool EncTextBlock::read(QDataStream& ds, quint32 varSize, int textOffset, bool hasRunHeader)
{
    // See ENCORE_FORMAT.md §TEXT block for layout. Entry N referenced by ORN tind byte (+32).
    // varSize is untrusted; route every skip through skipBlock/skipToBlockEnd so a value above
    // INT_MAX cannot wrap negative in skipRawData(quint32) and desync the following magic scan.
    const qint64 startPos = ds.device()->pos();
    if (varSize < 8) {
        skipBlock(ds, varSize);
        return true;
    }
    quint16 sync = 0;
    quint16 count = 0;
    quint32 contentSize = 0;
    ds >> sync >> count >> contentSize;
    (void)sync;
    // contentSize is the declared payload length; varSize (the block's own size field) is the
    // authoritative bound, so the per-entry checks below clamp to varSize and contentSize is
    // not relied upon.
    (void)contentSize;
    quint32 consumed = 8;
    entries.clear();
    entries.reserve(count);
    for (quint16 i = 0; i < count && consumed + 2 <= varSize; ++i) {
        // A read past EOF on a truncated block leaves the stream non-Ok; stop instead of
        // fabricating zero-filled entries from the past-EOF zero fill.
        if (ds.status() != QDataStream::Ok) {
            break;
        }
        quint16 entrySize = 0;
        ds >> entrySize;
        consumed += 2;
        if (entrySize == 0 || consumed + entrySize > varSize) {
            break;
        }
        QByteArray payload(entrySize, 0);
        int rd = ds.readRawData(payload.data(), entrySize);
        if (rd != entrySize) {
            break;
        }
        consumed += entrySize;
        // Payload text starts at a format-supplied offset; the rich-text run header pushes it
        // further and its length must be derived, not fixed. See ENCORE_FORMAT.md §TEXT block.
        int effTextOffset = textOffset;
        if (hasRunHeader && entrySize >= 4) {
            // The header carries two independent counts: a run-offset table count at +0 (uint32
            // entries) and a formatting-descriptor count at +2 (6-byte descriptors); text starts
            // after both, at 4 + tableCount*4 + descCount*6. Assuming a single descriptor reads too
            // early on multi-descriptor entries and misdecodes the text as byte-swapped UTF-16.
            const int tableCount = static_cast<quint8>(payload[0])
                                   | (static_cast<quint8>(payload[1]) << 8);
            const int descCount  = static_cast<quint8>(payload[2])
                                   | (static_cast<quint8>(payload[3]) << 8);
            if (tableCount >= 1 && descCount >= 1) {
                const int computed = 4 + tableCount * 4 + descCount * 6;
                if (computed + 2 <= entrySize) {
                    effTextOffset = computed;
                }
            }
        }
        // Payload text at effTextOffset, probe picks UTF-16 LE or Latin-1;
        // see ENCORE_FORMAT.md §Encoding probe.
        QString text;
        if (entrySize >= effTextOffset + 2) {
            const quint8 b0 = static_cast<quint8>(payload[effTextOffset]);
            const quint8 b1 = static_cast<quint8>(payload[effTextOffset + 1]);
            const bool isUtf16 = (b0 >= 0x20 && b0 < 0x7F && b1 == 0x00);
            // Decode the whole text region, then post-process: multi-line comments separate lines
            // with U+0004 and terminate with a U+0000 null. See ENCORE_FORMAT.md §TEXT block.
            const int textBytes = entrySize - effTextOffset;
            if (textBytes > 0) {
                if (isUtf16) {
                    text = QString::fromUtf16(
                        reinterpret_cast<const char16_t*>(payload.constData() + effTextOffset),
                        textBytes / 2);
                } else {
                    text = QString::fromLatin1(payload.constData() + effTextOffset, textBytes);
                }
                // Truncate at the U+0000 null terminator.
                int nullIdx = text.indexOf(QChar(QChar::Null));
                if (nullIdx >= 0) {
                    text = text.left(nullIdx);
                }
                // U+0004 separates lines within a comment; convert to newline.
                text.replace(QChar(0x0004), QChar(u'\n'));
                // Each line (including the last) is followed by U+0004, leaving a
                // trailing newline after conversion; drop trailing newlines.
                while (text.endsWith(QChar(u'\n'))) {
                    text.chop(1);
                }
            }
        }
        entries.push_back(text);
    }
    // Skip any remaining bytes inside the block (padding or unparsed tail), clamped to the device.
    skipToBlockEnd(ds, startPos, varSize);
    return true;
}
} // namespace mu::iex::enc
