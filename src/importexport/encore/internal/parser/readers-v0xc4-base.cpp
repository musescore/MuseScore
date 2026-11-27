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

// Instrument metadata recovery for v0xC2/v0xC4: names, MIDI programs and key transpositions across
// the large-TK, small-TK, compact and no-TK-block table layouts.

#include "readers-v0xc4-base.h"

#include <QDataStream>

#include <algorithm>
#include <initializer_list>

#include "elem.h"
#include "parsers-encoding.h"
#include "log.h"

namespace mu::iex::enc {
namespace {
// Read the 1-byte MIDI program at absolute offset off. Returns the program number when it is
// a valid 1..128 value, or 0 otherwise (including a seek failure). Callers keep their own
// off-in-bounds check; this just collapses the repeated seek + read + range-validate idiom.
static int readMidiByteAt(QDataStream& ds, qint64 off)
{
    if (!ds.device()->seek(off)) {
        return 0;
    }
    quint8 prg = 0;
    ds >> prg;
    return (prg >= 1 && prg <= 128) ? static_cast<int>(prg) : 0;
}

static constexpr qint64 VOICES_PER_STAFF = 8;

// Read whether or not a program is there: percussion often carries none. ENCORE_FORMAT.md §5.1.
static void readChannelBeforeMidi(QDataStream& ds, qint64 midiOff, EncInstrument& instr)
{
    const qint64 off = midiOff - VOICES_PER_STAFF;
    if (off < 0 || !ds.device()->seek(off)) {
        return;
    }
    quint8 ch = 0;
    ds >> ch;
    if (ch < 16) {
        instr.midiChannel = static_cast<int>(ch) + 1;
    }
}

// Read the 1-byte signed key-transpose value at absolute offset off. Returns true and sets
// out (-33..24) when valid; returns false otherwise (including a seek failure). Key 0 is a
// valid value, so a bool/out-param is used rather than a sentinel return.
static bool readKeyByteAt(QDataStream& ds, qint64 off, qint8& out)
{
    if (!ds.device()->seek(off)) {
        return false;
    }
    quint8 raw = 0;
    ds >> raw;
    const qint8 sv = static_cast<qint8>(raw);
    if (sv >= -33 && sv <= 24) {
        out = sv;
        return true;
    }
    return false;
}

// Scans the first 4096 bytes for PAGE/LINE/MEAS block magic; returns ds.device()->size() if not found.
// includePageBlock=false skips PAGE (used by the compact-layout MIDI scan).
static qint64 findFirstBlockOffset(QDataStream& ds, bool includePageBlock = true)
{
    if (!ds.device()->seek(0)) {
        return ds.device()->size();
    }
    static constexpr int PROBE = 4096;
    const QByteArray buf = ds.device()->read(PROBE);
    for (int i = 0; i <= buf.size() - 4; ++i) {
        const char* p = buf.constData() + i;
        if (includePageBlock && p[0] == 'P' && p[1] == 'A' && p[2] == 'G' && p[3] == 'E') {
            return static_cast<qint64>(i);
        }
        if ((p[0] == 'L' && p[1] == 'I' && p[2] == 'N' && p[3] == 'E')
            || (p[0] == 'M' && p[1] == 'E' && p[2] == 'A' && p[3] == 'S')) {
            return static_cast<qint64>(i);
        }
    }
    return ds.device()->size();
}

// Find the offset of the first ~~~~ block (v0xC2 compact instrument table).
// Returns -1 if not found.  Reads the first 1 KiB in one shot to avoid
// QDataStream buffering issues with interleaved seeks.
static qint64 findTildeBlockOffset(QDataStream& ds)
{
    const qint64 fileSize = static_cast<qint64>(ds.device()->size());
    static constexpr qint64 kScanLimit = 1024;
    const qint64 readSize = qMin(fileSize, kScanLimit + 8);
    if (!ds.device()->seek(0)) {
        return -1;
    }
    const QByteArray chunk = ds.device()->read(static_cast<qint64>(readSize));
    const int n = chunk.size();
    for (int off = 0; off + 7 < n; ++off) {
        if ((quint8)chunk[off] != 0x7e || (quint8)chunk[off + 1] != 0x7e
            || (quint8)chunk[off + 2] != 0x7e || (quint8)chunk[off + 3] != 0x7e) {
            continue;
        }
        const quint32 vs = (quint8)chunk[off + 4]
                           | ((quint8)chunk[off + 5] << 8)
                           | ((quint8)chunk[off + 6] << 16)
                           | ((quint8)chunk[off + 7] << 24);
        if (vs > 0 && static_cast<qint64>(vs) < fileSize / 2) {
            return static_cast<qint64>(off);
        }
    }
    return -1;
}

// Byte offset of the first instrument entry; its name starts 8 bytes in, at NAME_BASE.
static constexpr qint64 ENTRY_TABLE_BASE = 194;

// Entry size derived from the file, since the TK size field lies; three sources, most trustworthy
// first, per ENCORE_FORMAT.md §5.1. evidenceOnly withholds the last one, which is a guess.
qint64 instrumentEntryStride(const std::vector<EncInstrument>& instruments, QDataStream& ds,
                             bool evidenceOnly = false)
{
    static constexpr qint64 kMinStride = 64;
    qint64 firstPos = -1, firstIdx = -1;
    for (size_t n = 0; n < instruments.size(); ++n) {
        if (instruments[n].contentFilePos < 0) {
            continue;
        }
        const qint64 pos = instruments[n].contentFilePos - 8;
        if (firstPos < 0) {
            firstPos = pos;
            firstIdx = static_cast<qint64>(n);
            continue;
        }
        const qint64 span = pos - firstPos;
        const qint64 steps = static_cast<qint64>(n) - firstIdx;
        if (steps > 0 && span > 0 && span % steps == 0 && span / steps >= kMinStride) {
            return span / steps;
        }
    }
    if (firstIdx > 0 && firstPos > ENTRY_TABLE_BASE) {
        const qint64 span = firstPos - ENTRY_TABLE_BASE;
        if (span % firstIdx == 0 && span / firstIdx >= kMinStride) {
            return span / firstIdx;
        }
    }
    if (evidenceOnly) {
        return 0;
    }
    const qint64 count = static_cast<qint64>(instruments.size());
    const qint64 tableEnd = findFirstBlockOffset(ds);
    if (count > 0 && tableEnd > ENTRY_TABLE_BASE && tableEnd < ds.device()->size()) {
        const qint64 span = tableEnd - ENTRY_TABLE_BASE;
        if (span % count == 0 && span / count >= kMinStride) {
            return span / count;
        }
    }
    return 0;
}

void recoverMissingNames(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    // Name positions of the two formula layouts; see ENCORE_FORMAT.md §5.1.
    static constexpr qint64 NAME_BASE = 202;
    static constexpr qint64 NAME_STEP = 2158;
    static constexpr qint64 COMPACT_NAME_BASE = 314;
    static constexpr qint64 COMPACT_NAME_STEP = 112;

    auto tryReadName = [&](qint64 off) -> QString {
        if (off + 2 >= static_cast<qint64>(ds.device()->size())) {
            return {};
        }
        if (!ds.device()->seek(off)) {
            return {};
        }
        quint8 b0 = 0, b1 = 0;
        ds >> b0 >> b1;
        if (b0 < 0x20 || b0 >= 0x7F) {
            return {};
        }
        const bool isLatin1 = (b1 != 0x00 && b1 >= 0x20 && b1 < 0xFF);
        if (!probeUtf16LE(b0, b1) && !isLatin1) {
            return {};
        }
        if (!ds.device()->seek(off)) {
            return {};
        }
        int remaining = static_cast<int>(ds.device()->size() - off);
        return readEncodedStringRemaining(ds, remaining);
    };

    // No-~~~~-block format: all instruments sit in a linear table. Detect large-TK (step 2158) vs
    // compact (step 112) by comparing firstBlockOff against the large-TK MIDI base (2278).
    const bool noTkBlocks = instruments.empty()
                            || std::all_of(instruments.begin(), instruments.end(),
                                           [](const EncInstrument& i){ return i.contentFilePos < 0; });
    if (noTkBlocks && findTildeBlockOffset(ds) < 0) {
        const qint64 firstBlockOff = findFirstBlockOffset(ds, /*includePageBlock=*/ true);
        // A span that divides by the instrument count says what the entries measure, and is taken only
        // when it also lands on a name for the second entry. See ENCORE_FORMAT.md §5.1.
        const qint64 spanStride = instrumentEntryStride(instruments, ds);
        const bool spanFits = spanStride > 0 && instruments.size() > 1
                              && !tryReadName(NAME_BASE + spanStride).trimmed().isEmpty();
        const bool useLargeTk = (firstBlockOff > 2278);
        const qint64 step = spanFits ? spanStride : (useLargeTk ? NAME_STEP : COMPACT_NAME_STEP);
        for (size_t n = 0; n < instruments.size(); ++n) {
            if (!instruments[n].name.isEmpty()) {
                continue;
            }
            const qint64 off = NAME_BASE + static_cast<qint64>(n) * step;
            instruments[n].name = tryReadName(off);
        }
        return;
    }

    // A name from the block itself is the last word, empty included, so positional recovery is only for
    // entries without one, and for ~~~~ files whose table names them all. ENCORE_FORMAT.md §5.1.
    const bool hasTilde = findTildeBlockOffset(ds) >= 0;
    auto resolvedByTkBlock = [&](size_t n) {
        return !hasTilde && instruments[n].contentFilePos >= 0;
    };

    // Both formulas below belong to other layouts, so a stride the file shows comes first: theirs walks
    // out of the table and reads page bytes, which spell plausible words. ENCORE_FORMAT.md §5.1.
    if (const qint64 proven = instrumentEntryStride(instruments, ds, /*evidenceOnly*/ true)) {
        for (size_t n = 0; n < instruments.size(); ++n) {
            if (!instruments[n].name.trimmed().isEmpty() || resolvedByTkBlock(n)) {
                continue;
            }
            instruments[n].name = tryReadName(ENTRY_TABLE_BASE + 8 + static_cast<qint64>(n) * proven);
        }
        // Nothing else is tried: an entry whose name field is empty has no name, and both formulas
        // would answer with whatever their own layout's positions hold in this file.
        return;
    }

    for (size_t n = 0; n < instruments.size(); ++n) {
        if (!instruments[n].name.isEmpty() || resolvedByTkBlock(n)) {
            continue;
        }
        // Primary probe.
        const qint64 off = NAME_BASE + static_cast<qint64>(n) * NAME_STEP;
        instruments[n].name = tryReadName(off);
    }

    // Compact-layout fallback: entries are stored in FORWARD instrument order
    // (entry 0 = first instrument without a name, entry 1 = second, ...).
    // Assign names to instruments that are still missing a name, in order.
    {
        size_t nextTarget = 0;
        for (size_t k = 0; k < instruments.size(); ++k) {
            while (nextTarget < instruments.size()
                   && (!instruments[nextTarget].name.trimmed().isEmpty()
                       || resolvedByTkBlock(nextTarget))) {
                ++nextTarget;
            }
            if (nextTarget >= instruments.size()) {
                break;
            }
            const qint64 cOff = COMPACT_NAME_BASE + static_cast<qint64>(k) * COMPACT_NAME_STEP;
            const QString candidate = tryReadName(cOff);
            if (!candidate.trimmed().isEmpty()) {
                instruments[nextTarget].name = candidate;
                ++nextTarget;
            }
        }
    }

    // Last resort: the entry's own slot, at the stride the file implies.
    if (const qint64 stride = instrumentEntryStride(instruments, ds)) {
        for (size_t n = 0; n < instruments.size(); ++n) {
            if (!instruments[n].name.trimmed().isEmpty() || resolvedByTkBlock(n)) {
                continue;
            }
            instruments[n].name = tryReadName(ENTRY_TABLE_BASE + 8 + static_cast<qint64>(n) * stride);
        }
    }
}

static qint64 programTableOffset(QDataStream& ds, std::initializer_list<qint64> candidates);

// No-TK layout (instruments[0].contentFilePos < 0).
static void readMidiProgramsNoTk(
    std::vector<EncInstrument>& instruments,
    QDataStream& ds,
    qint64 firstBlockOff,
    qint64 midiFromEntryEnd)
{
    // Encore 4 writes a fixed-stride table and puts a TK magic on only some of its entries, so a
    // file can show what its entries measure while neither absolute layout below describes it. Then
    // every entry keeps its tables at its own end, exactly as an entry with a block does.
    if (const qint64 stride = instrumentEntryStride(instruments, ds)) {
        const qint64 firstTable = ENTRY_TABLE_BASE + stride - midiFromEntryEnd;
        if (programTableOffset(ds, { firstTable }) >= 0) {
            for (size_t n = 0; n < instruments.size(); ++n) {
                const qint64 table = ENTRY_TABLE_BASE + static_cast<qint64>(n + 1) * stride - midiFromEntryEnd;
                readChannelBeforeMidi(ds, table, instruments[n]);
                if (const int prg = readMidiByteAt(ds, table)) {
                    instruments[n].midiProgram = prg;
                }
            }
            return;
        }
    }

    // For v0xC2 files with a ~~~~ block, findFirstBlockOffset may return a large offset
    // (past the ~~~~ block) because ~~~~ is not a recognized block type.  Cap it so
    // the compact layout is correctly detected.
    const qint64 tildeOff = findTildeBlockOffset(ds);
    const qint64 effectiveFirstBlock = (tildeOff >= 0 && tildeOff < firstBlockOff)
                                       ? tildeOff : firstBlockOff;

    static constexpr qint64 LT_BASE = 2278, LT_STEP = 2158;
    // Compact v0xC2: MIDI is at byte +93 within each 112-byte instrument entry.
    // COMPACT_NAME_BASE=314 = entry_table_start(281) + name_field_offset(33).
    // COMPACT_MIDI_BASE = entry_table_start + midi_field_offset = 281 + 93 = 374.
    static constexpr qint64 COMPACT_MIDI_BASE = 374;
    static constexpr qint64 COMPACT_MIDI_STEP = 112;

    const bool useLargeTk = (effectiveFirstBlock > LT_BASE);
    if (useLargeTk) {
        for (size_t n = 0; n < instruments.size(); ++n) {
            const qint64 off = LT_BASE + static_cast<qint64>(n) * LT_STEP;
            if (off >= effectiveFirstBlock || off >= static_cast<qint64>(ds.device()->size())) {
                break;
            }
            readChannelBeforeMidi(ds, off, instruments[n]);
            if (int prg = readMidiByteAt(ds, off)) {
                instruments[n].midiProgram = prg;
            }
        }
    } else {
        // The two compact sub-layouts of ENCORE_FORMAT.md 5.1. In the second, entries map only to
        // instruments that lack both a name and a program.
        static constexpr qint64 CMP_BASE = 390, CMP_STEP = 276;

        // First try sub-layout (a): check if CMP_BASE has valid MIDI.
        quint8 probeA = 0;
        if (CMP_BASE < static_cast<qint64>(ds.device()->size()) && ds.device()->seek(CMP_BASE)) {
            ds >> probeA;
        }
        // probeA is only valid if CMP_BASE is before the first data block.
        const bool aLayoutValid = (probeA >= 1 && probeA <= 128 && CMP_BASE < effectiveFirstBlock);
        if (aLayoutValid) {
            // Sub-layout (a): sequential table (guarded by firstBlockOff).
            for (size_t n = 0; n < instruments.size(); ++n) {
                const qint64 off = CMP_BASE + static_cast<qint64>(n) * CMP_STEP;
                if (off >= effectiveFirstBlock || off >= static_cast<qint64>(ds.device()->size())) {
                    break;
                }
                readChannelBeforeMidi(ds, off, instruments[n]);
                if (int prg = readMidiByteAt(ds, off)) {
                    instruments[n].midiProgram = prg;
                }
            }
        } else if (tildeOff < 0) {
            // The no-tilde compact layout: entries at 176, step 112, program at entry+86.
            static constexpr qint64 NC_MIDI_BASE = 262;
            static constexpr qint64 NC_MIDI_STEP = 112;
            for (size_t n = 0; n < instruments.size(); ++n) {
                if (instruments[n].midiProgram != 0) {
                    continue;
                }
                const qint64 off = NC_MIDI_BASE + static_cast<qint64>(n) * NC_MIDI_STEP;
                if (off >= static_cast<qint64>(ds.device()->size())) {
                    break;
                }
                readChannelBeforeMidi(ds, off, instruments[n]);
                if (int prg = readMidiByteAt(ds, off)) {
                    instruments[n].midiProgram = prg;
                }
            }
        } else {
            // ~~~~-block compact format: MIDI at byte +93 of each 112-byte entry.
            // Skip instruments that have their own named block (e.g. "Voz " in
            // some v0xC2 files), their MIDI is read in the first pass below.
            static constexpr qint64 PNB = 202, PNS = 2158;  // primary name base/step
            auto hasPrimaryBlock = [&](size_t n) -> bool {
                const qint64 off = PNB + static_cast<qint64>(n) * PNS;
                if (off + 1 >= static_cast<qint64>(ds.device()->size())) {
                    return false;
                }
                if (!ds.device()->seek(off)) {
                    return false;
                }
                quint8 u = 0;
                ds >> u;
                return u >= 0x20 && u < 0x7F;
            };

            // First pass: MIDI for instruments with an explicit primary block
            // (their MIDI is at block_start + 60).
            static constexpr qint64 MIDI_AT_BLOCK_START = 60;
            for (size_t n = 0; n < instruments.size(); ++n) {
                if (!hasPrimaryBlock(n) || instruments[n].midiProgram != 0) {
                    continue;
                }
                const qint64 mOff = PNB + static_cast<qint64>(n) * PNS + MIDI_AT_BLOCK_START;
                if (mOff >= static_cast<qint64>(ds.device()->size())) {
                    continue;
                }
                readChannelBeforeMidi(ds, mOff, instruments[n]);
                if (int prm = readMidiByteAt(ds, mOff)) {
                    instruments[n].midiProgram = prm;
                }
            }

            size_t nextTarget = 0;
            for (size_t k = 0; k < instruments.size(); ++k) {
                while (nextTarget < instruments.size()
                       && (instruments[nextTarget].midiProgram != 0
                           || instruments[nextTarget].contentFilePos >= 0
                           || hasPrimaryBlock(nextTarget))) {
                    ++nextTarget;
                }
                if (nextTarget >= instruments.size()) {
                    break;
                }
                const qint64 off = COMPACT_MIDI_BASE + static_cast<qint64>(k) * COMPACT_MIDI_STEP;
                if (off >= static_cast<qint64>(ds.device()->size())) {
                    break;
                }
                readChannelBeforeMidi(ds, off, instruments[nextTarget]);
                if (int prg = readMidiByteAt(ds, off)) {
                    instruments[nextTarget].midiProgram = prg;
                }
                ++nextTarget;
            }
        }
    }
    // Recover names even for no-TK files.
    recoverMissingNames(instruments, ds);
}

// Files whose TK size is the total block size: consecutive entries then sit exactly `offset` apart
// instead of offset plus eight. See ENCORE_FORMAT.md 5.1.
static bool isTotalBlockSizeTkFmt(const std::vector<EncInstrument>& instruments)
{
    if (instruments.size() < 2) {
        return false;
    }
    const qint64 stride = instruments[1].contentFilePos - instruments[0].contentFilePos;
    return stride > 0 && stride == static_cast<qint64>(instruments[0].offset);
}

// Entry size at or above which an instrument entry uses the Encore 5 layout, whose per-staff
// program table sits LARGE_ENTRY_MIDI bytes into the entry.
static constexpr qint64 LARGE_ENTRY_MIN = 2000;
static constexpr qint64 LARGE_ENTRY_MIDI = 2084;

// Read the program table of a large entry. Encore 5 files are recognised by their entry stride
// alone: the TK size field is unreliable (4.x-era saves declare 112 whatever the entry really is),
// so a large-entry file could otherwise be mistaken for a small-entry one and read an empty byte.
static void readMidiProgramsLargeEntry(std::vector<EncInstrument>& instruments, QDataStream& ds,
                                       qint64 entryStride)
{
    for (size_t n = 0; n < instruments.size(); ++n) {
        const qint64 entryStart = instruments[n].contentFilePos >= 0
                                  ? instruments[n].contentFilePos - 8
                                  : ENTRY_TABLE_BASE + static_cast<qint64>(n) * entryStride;
        const qint64 off = entryStart + LARGE_ENTRY_MIDI;
        if (off >= static_cast<qint64>(ds.device()->size())) {
            break;
        }
        readChannelBeforeMidi(ds, off, instruments[n]);
        if (int prg = readMidiByteAt(ds, off)) {
            instruments[n].midiProgram = prg;
        }
    }
}

// The distances the two generations keep between the program table and the entry end; a file can
// carry either, whatever its version byte says.
static constexpr qint64 MIDI_FROM_ENTRY_END_305 = 46;
static constexpr qint64 MIDI_FROM_ENTRY_END_300 = 44;

// A per-staff table is proved by its run shape, one byte per voice. Three proofs, ranked because the
// weaker ones also match more; see ENCORE_FORMAT.md §5.1.
enum class TableProof {
    Program,        // a program names an instrument and the channels differ from it
    ValueRun,       // the program repeats once per voice, whatever the channels hold
    ChannelRun,     // no program named, and the channel run is all there is
};

static bool programTableStartsAt(QDataStream& ds, qint64 off, TableProof proof)
{
    static constexpr int MIDI_CHANNELS = 16;
    static constexpr int MIDI_PROGRAMS = 128;
    const qint64 span = VOICES_PER_STAFF + (proof == TableProof::ValueRun ? VOICES_PER_STAFF : 1);
    if (off < VOICES_PER_STAFF || off - VOICES_PER_STAFF + span > ds.device()->size()
        || !ds.device()->seek(off - VOICES_PER_STAFF)) {
        return false;
    }
    quint8 ch = 0;
    for (qint64 v = 0; v < VOICES_PER_STAFF; ++v) {
        ds >> ch;
        if (ch >= MIDI_CHANNELS) {
            return false;
        }
    }
    quint8 prg = 0;
    ds >> prg;
    if (prg > MIDI_PROGRAMS) {
        return false;
    }
    if (proof == TableProof::ValueRun) {
        if (prg == 0 && ch == 0) {
            return false;       // a stretch of zeros says nothing either way
        }
        for (qint64 v = 1; v < VOICES_PER_STAFF; ++v) {
            quint8 next = 0;
            ds >> next;
            if (next != prg) {
                return false;
            }
        }
        return true;
    }
    return proof == TableProof::Program ? (prg != 0 && ch != prg) : (prg == 0 && ch != prg);
}

// Absolute offset of the per-staff program table: the first of the positions the layouts put it at
// that a channel run confirms, or -1. See ENCORE_FORMAT.md §5.1 Instrument block.
static qint64 programTableOffset(QDataStream& ds, std::initializer_list<qint64> candidates)
{
    for (const TableProof proof : { TableProof::Program, TableProof::ValueRun, TableProof::ChannelRun }) {
        for (const qint64 off : candidates) {
            if (programTableStartsAt(ds, off, proof)) {
                return off;
            }
        }
    }
    return -1;
}

// Where the tables can sit in the small-TK layouts, most specific first. Only a measured stride
// bounds the entry; an assumed one rules nothing out.
static constexpr qint64 MIDI_AFTER_CONTENT = 76;
static constexpr qint64 MIDI_IN_CONTENT = 60;

static qint64 smallTkProgramTable(const EncInstrument& instr, QDataStream& ds, qint64 entryStride,
                                  qint64 midiFromEntryEnd, bool strideMeasured, bool largeEntry)
{
    const qint64 entryStart = instr.contentFilePos - 8;
    const qint64 entryEnd = entryStride > 0 ? entryStart + entryStride : -1;
    const qint64 afterContent = instr.contentFilePos + static_cast<qint64>(instr.offset);
    const qint64 table = programTableOffset(ds, {
                largeEntry ? entryStart + LARGE_ENTRY_MIDI : -1,
                entryEnd >= 0 ? entryEnd - midiFromEntryEnd : -1,
                entryEnd >= 0 ? entryEnd - MIDI_FROM_ENTRY_END_305 : -1,
                entryEnd >= 0 ? entryEnd - MIDI_FROM_ENTRY_END_300 : -1,
                afterContent + MIDI_AFTER_CONTENT,
                instr.contentFilePos + MIDI_IN_CONTENT,
            });
    const bool outsideEntry = entryEnd >= 0 && (table < entryStart || table >= entryEnd);
    return (strideMeasured && outsideEntry) ? -1 : table;
}

// SmallTK layout (0 < offset <= 250).
static void readMidiProgramsSmallTk(
    std::vector<EncInstrument>& instruments,
    QDataStream& ds,
    qint64 entryStride,
    qint64 midiFromEntryEnd)
{
    // The two fixed positions of ENCORE_FORMAT.md §5.1; a confirmed one beats both.
    const bool totalSizeFmt = isTotalBlockSizeTkFmt(instruments);
    const bool strideMeasured = std::count_if(instruments.begin(), instruments.end(),
                                              [](const EncInstrument& i) { return i.contentFilePos >= 0; }) >= 2;
    const bool largeEntry = strideMeasured && entryStride >= LARGE_ENTRY_MIN;
    if (totalSizeFmt) {
        LOGD() << "enc: small-TK total-block-size format detected (Encore 4.x): reading MIDI at content+60";
    }
    for (auto& instr : instruments) {
        if (instr.contentFilePos < 0 || instr.midiProgram != 0) {
            continue;
        }
        const qint64 table = smallTkProgramTable(instr, ds, entryStride, midiFromEntryEnd,
                                                 strideMeasured, largeEntry);
        if (table >= 0) {
            readChannelBeforeMidi(ds, table, instr);
            // A table naming no program leaves the search below to run as it always did.
            if (const int prg = readMidiByteAt(ds, table)) {
                instr.midiProgram = prg;
                continue;
            }
        }
        const qint64 afterContent = instr.contentFilePos + static_cast<qint64>(instr.offset) + MIDI_AFTER_CONTENT;
        // An overstated varSize puts this past the entry, so keep it only while it stays inside.
        const qint64 entryEnd = entryStride > 0 ? instr.contentFilePos - 8 + entryStride : -1;
        const bool afterContentInEntry = (entryEnd < 0) || (afterContent < entryEnd);
        const bool contentSizeMisdeclared = totalSizeFmt || !afterContentInEntry;
        qint64 off = afterContent;
        if (contentSizeMisdeclared) {
            off = entryEnd >= 0 ? entryEnd - midiFromEntryEnd : instr.contentFilePos + MIDI_IN_CONTENT;
        }
        if (off >= static_cast<qint64>(ds.device()->size())) {
            continue;
        }
        readChannelBeforeMidi(ds, off, instr);
        int prg = readMidiByteAt(ds, off);
        if (!prg && contentSizeMisdeclared) {
            // Entries short enough that the tables reach back into the content itself keep the
            // program 60 bytes in; for those the two offsets coincide, for longer ones they do not.
            prg = readMidiByteAt(ds, instr.contentFilePos + MIDI_IN_CONTENT);
        }
        if (prg) {
            instr.midiProgram = prg;
        }
    }
}

void readMidiPrograms(std::vector<EncInstrument>& instruments, QDataStream& ds, qint64 midiFromEntryEnd)
{
    // MIDI table offsets vary by layout. See ENCORE_FORMAT.md §5.1 Instrument block.
    if (instruments.empty()) {
        return;
    }
    const bool noTkBlocks = (instruments[0].contentFilePos < 0);
    if (noTkBlocks) {
        const qint64 firstBlockOff = findFirstBlockOffset(ds, /*includePageBlock=*/ true);
        readMidiProgramsNoTk(instruments, ds, firstBlockOff, midiFromEntryEnd);
        return;
    }
    const bool compact = (instruments[0].offset == 0);
    const bool smallTK = (!compact && instruments[0].offset <= 250);

    // The entry stride the file implies, not the declared TK size, tells the layouts apart.
    const qint64 entryStride = instrumentEntryStride(instruments, ds);
    if (entryStride >= LARGE_ENTRY_MIN) {
        readMidiProgramsLargeEntry(instruments, ds, entryStride);
    }

    if (smallTK) {
        readMidiProgramsSmallTk(instruments, ds, entryStride, midiFromEntryEnd);
        // Mixed-TK files: the compact byte-93 program for entries with neither a block nor a program.
        {
            static constexpr qint64 CMPX_MIDI_BASE = 374;
            static constexpr qint64 CMPX_MIDI_STEP = 112;
            size_t nextTarget = 0;
            for (size_t k = 0; k < instruments.size(); ++k) {
                while (nextTarget < instruments.size()
                       && (instruments[nextTarget].midiProgram != 0
                           || instruments[nextTarget].contentFilePos >= 0)) {
                    ++nextTarget;
                }
                if (nextTarget >= instruments.size()) {
                    break;
                }
                const qint64 off = CMPX_MIDI_BASE + static_cast<qint64>(k) * CMPX_MIDI_STEP;
                if (off >= static_cast<qint64>(ds.device()->size())) {
                    break;
                }
                readChannelBeforeMidi(ds, off, instruments[nextTarget]);
                if (int prg = readMidiByteAt(ds, off)) {
                    instruments[nextTarget].midiProgram = prg;
                }
                ++nextTarget;
            }
        }
        return;
    }

    // Large-TK or compact layout.
    qint64 firstBlockOff = ds.device()->size();
    if (compact) {
        firstBlockOff = findFirstBlockOffset(ds, /*includePageBlock=*/ false);
    }

    const qint64 base = compact ? 390 : 2278;
    const qint64 step = compact ? 276 : 2158;
    for (size_t n = 0; n < instruments.size(); ++n) {
        const qint64 off = base + static_cast<qint64>(n) * step;
        if (off >= firstBlockOff) {
            break;
        }
        if (off >= static_cast<qint64>(ds.device()->size())) {
            break;
        }
        if (instruments[n].midiProgram != 0) {
            continue;
        }
        readChannelBeforeMidi(ds, off, instruments[n]);
        if (int prg = readMidiByteAt(ds, off)) {
            instruments[n].midiProgram = prg;
        }
    }
}

// The key sits KEY_BEFORE_MIDI ahead of the program table, so it follows wherever that table is.
// A staff with no program leaves nothing to confirm; fallback is then the layout's fixed position,
// or -1 to read nothing.
static void readKeyFromTable(EncInstrument& instr, QDataStream& ds, qint64 table, qint64 fallback)
{
    static constexpr qint64 KEY_BEFORE_MIDI = 23;
    const qint64 off = table >= 0 ? table - KEY_BEFORE_MIDI : fallback;
    if (off < 0 || off >= static_cast<qint64>(ds.device()->size())) {
        return;
    }
    qint8 sv = 0;
    if (readKeyByteAt(ds, off, sv)) {
        instr.keyTransposeSemitones = sv;
    }
}

// No-TK layout: one linear table, either at the large-entry positions or at the compact ones.
static void readKeyTranspositionsNoTk(std::vector<EncInstrument>& instruments, QDataStream& ds,
                                      qint64 firstBlockOff, qint64 midiFromEntryEnd)
{
    // A table the file measures itself comes before either absolute layout; see readMidiProgramsNoTk.
    if (const qint64 stride = instrumentEntryStride(instruments, ds)) {
        const qint64 firstTable = ENTRY_TABLE_BASE + stride - midiFromEntryEnd;
        if (programTableOffset(ds, { firstTable }) >= 0) {
            for (size_t n = 0; n < instruments.size(); ++n) {
                const qint64 table = ENTRY_TABLE_BASE + static_cast<qint64>(n + 1) * stride - midiFromEntryEnd;
                readKeyFromTable(instruments[n], ds, table, /*fallback*/ -1);
            }
            return;
        }
    }
    static constexpr qint64 LT_BASE = 2278, LT_STEP = 2158;
    static constexpr qint64 CMP_BASE = 390, CMP_STEP = 112;
    // Which of the two tables the file uses follows from where its first block starts; an absolute
    // offset tried against the other layout can land in measure data and pass for a table there.
    const bool useLargeTk = (firstBlockOff > LT_BASE);
    for (size_t n = 0; n < instruments.size(); ++n) {
        const qint64 idx = static_cast<qint64>(n);
        const qint64 table = useLargeTk ? LT_BASE + idx * LT_STEP : CMP_BASE + idx * CMP_STEP;
        const qint64 fallback = useLargeTk || instruments.size() == 1 ? table - 23 : -1;
        readKeyFromTable(instruments[n], ds, programTableOffset(ds, { table }), fallback);
    }
}

// SmallTK layout (0 < offset <= 250). The distance from the entry end is not settled by the version
// byte: a file can carry either generation's, and a misdeclared size moves the content end.
static void readKeyTranspositionsSmallTk(std::vector<EncInstrument>& instruments, QDataStream& ds,
                                         qint64 entryStride, qint64 midiFromEntryEnd)
{
    static constexpr qint64 KEY_AFTER_CONTENT = MIDI_AFTER_CONTENT - 23;
    static constexpr qint64 KEY_IN_CONTENT = 42;
    const bool totalSizeFmt = isTotalBlockSizeTkFmt(instruments);
    const bool strideMeasured = std::count_if(instruments.begin(), instruments.end(),
                                              [](const EncInstrument& i) { return i.contentFilePos >= 0; }) >= 2;
    // A size of 112 on entries that really measure 2158 leaves the entry large; the stride says so
    // only when it came from the spacing between two blocks.
    const bool largeEntry = strideMeasured && entryStride >= LARGE_ENTRY_MIN;
    // The tables sit the same distance into every entry of a file, so one entry that proves where
    // they are speaks for the ones whose own region proves nothing: a staff with neither channel nor
    // program is a stretch of zeros, and its key is still where its siblings keep theirs.
    std::vector<qint64> tables(instruments.size(), -1);
    qint64 provedDistance = -1;
    for (size_t n = 0; n < instruments.size(); ++n) {
        if (instruments[n].contentFilePos < 0) {
            continue;
        }
        tables[n] = smallTkProgramTable(instruments[n], ds, entryStride, midiFromEntryEnd,
                                        strideMeasured, largeEntry);
        if (tables[n] >= 0 && provedDistance < 0) {
            provedDistance = tables[n] - instruments[n].contentFilePos;
        }
    }
    for (size_t n = 0; n < instruments.size(); ++n) {
        EncInstrument& instr = instruments[n];
        if (instr.contentFilePos < 0) {
            continue;
        }
        const qint64 fallback = totalSizeFmt
                                ? instr.contentFilePos + KEY_IN_CONTENT
                                : instr.contentFilePos + static_cast<qint64>(instr.offset) + KEY_AFTER_CONTENT;
        // With nothing assigned anywhere, no run proves and no sibling measures: fall back to this layout's
        // distance from the entry end, which only a measured stride bounds. ENCORE_FORMAT.md §5.1.
        const qint64 entryEnd = strideMeasured && entryStride > 0
                                ? instr.contentFilePos - 8 + entryStride : -1;
        qint64 table = tables[n];
        if (table < 0) {
            table = provedDistance >= 0 ? instr.contentFilePos + provedDistance
                    : (entryEnd >= 0 ? entryEnd - midiFromEntryEnd : -1);
        }
        readKeyFromTable(instr, ds, table, fallback);
    }
}

void readKeyTranspositions(std::vector<EncInstrument>& instruments, QDataStream& ds,
                           qint64 midiFromEntryEnd)
{
    if (instruments.empty()) {
        return;
    }
    if (instruments[0].contentFilePos < 0) {
        readKeyTranspositionsNoTk(instruments, ds, findFirstBlockOffset(ds, /*includePageBlock=*/ true),
                                  midiFromEntryEnd);
        return;
    }
    // A size of zero is Encore 4's 0x70000000 masked to 16 bits: it says nothing, and with blocks
    // present the entry decides, as it does for every small layout.
    if (instruments[0].offset <= 250) {
        readKeyTranspositionsSmallTk(instruments, ds, instrumentEntryStride(instruments, ds), midiFromEntryEnd);
        return;
    }
    static constexpr qint64 PRG_BASE = 2278, PRG_STEP = 2158;
    for (size_t n = 0; n < instruments.size(); ++n) {
        const qint64 fixed = PRG_BASE + static_cast<qint64>(n) * PRG_STEP;
        readKeyFromTable(instruments[n], ds, programTableOffset(ds, {
                    instruments[n].contentFilePos - 8 + LARGE_ENTRY_MIDI, fixed,
                }), fixed - 23);
    }
}
} // namespace

bool EncFormatReader_V0xC4Base::readInstrumentMeta(std::vector<EncInstrument>& instruments,
                                                   QDataStream& ds,
                                                   const EncRoot& /*file*/) const
{
    recoverMissingNames(instruments, ds);
    readMidiPrograms(instruments, ds, midiProgramFromEntryEnd());
    readKeyTranspositions(instruments, ds, midiProgramFromEntryEnd());
    return true;
}
} // namespace mu::iex::enc
