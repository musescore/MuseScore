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

void recoverMissingNames(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    // NAME_BASE=202 is the name position of instrument 0 in every compact-table layout; the step
    // differs (2158 for large-TK/~~~~-block files, 112 for no-~~~~-block compact files). The
    // COMPACT_NAME_BASE fallback fills names left unresolved by the primary probe.
    // See ENCORE_FORMAT.md §Instrument block.
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
        const bool useLargeTk = (firstBlockOff > 2278);
        const qint64 step = useLargeTk ? NAME_STEP : COMPACT_NAME_STEP;
        for (size_t n = 0; n < instruments.size(); ++n) {
            if (!instruments[n].name.isEmpty()) {
                continue;
            }
            const qint64 off = NAME_BASE + static_cast<qint64>(n) * step;
            instruments[n].name = tryReadName(off);
        }
        return;
    }

    // An instrument that has its own TK block (contentFilePos >= 0) carries an authoritative
    // name: if that name came back empty, the instrument is genuinely unnamed and must stay
    // empty (the "Part N" fallback applies later). Positional recovery from the formula/compact
    // offsets is only for instruments WITHOUT a TK block, and for ~~~~-block files whose compact
    // table legitimately names even the TK-block instruments. Without a ~~~~ block, probing the
    // formula offset for a real-TK instrument reads unrelated music/structure bytes as garbage.
    const bool hasTilde = findTildeBlockOffset(ds) >= 0;
    auto resolvedByTkBlock = [&](size_t n) {
        return !hasTilde && instruments[n].contentFilePos >= 0;
    };

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
}

// No-TK layout (instruments[0].contentFilePos < 0).
static void readMidiProgramsNoTk(
    std::vector<EncInstrument>& instruments,
    QDataStream& ds,
    qint64 firstBlockOff)
{
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
            if (int prg = readMidiByteAt(ds, off)) {
                instruments[n].midiProgram = prg;
            }
        }
    } else {
        // Compact layout, two sub-layouts:
        //   a) Encore 3.x compact (older synthetic files): MIDI at CMP_BASE=390, step=276.
        //   b) Encore 4.x v0xC2 with ~~~~ block: MIDI at byte +93 of each 112-byte entry,
        //      base=374 (= entry_table_start(281) + midi_field_offset(93)), step=112.
        //      Instrument entries map to instruments that lack both a name AND a MIDI
        //      program (instruments with their own named TK block are skipped).
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
                if (int prg = readMidiByteAt(ds, off)) {
                    instruments[n].midiProgram = prg;
                }
            }
        } else if (tildeOff < 0) {
            // No-~~~~-block compact format: instrument entries at
            // entry_base=176, step=112, MIDI at entry+86.
            // NAME_BASE(202) = entry_base(176) + name_off(26), so
            // MIDI_BASE = 176 + 86 = 262 = NAME_BASE - 26 + 86.
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

// Detect Encore 4.x v0xC4 files where TK varSize encodes the TOTAL block size
// (including the 8-byte magic+size header) rather than just the content length.
// Symptom: consecutive contentFilePos values are spaced exactly `offset` bytes apart
// (not `offset+8` as in the standard Encore 5.x layout).
static bool isTotalBlockSizeTkFmt(const std::vector<EncInstrument>& instruments)
{
    if (instruments.size() < 2) {
        return false;
    }
    const qint64 stride = instruments[1].contentFilePos - instruments[0].contentFilePos;
    return stride > 0 && stride == static_cast<qint64>(instruments[0].offset);
}

// SmallTK layout (0 < offset <= 250).
static void readMidiProgramsSmallTk(
    std::vector<EncInstrument>& instruments,
    QDataStream& ds)
{
    // Standard Encore 5.x: MIDI is 76 bytes past the content end
    //   (content is `instr.offset` bytes; absolute = contentFilePos + offset + 76).
    // Encore 4.x total-size variant: varSize is the TOTAL block size including header,
    //   so actual content = varSize-8 = 104 bytes, and MIDI is at content[60].
    static constexpr qint64 MIDI_AFTER_CONTENT = 76;
    static constexpr qint64 MIDI_IN_CONTENT    = 60;
    const bool totalSizeFmt = isTotalBlockSizeTkFmt(instruments);
    if (totalSizeFmt) {
        LOGD() << "enc: small-TK total-block-size format detected (Encore 4.x): reading MIDI at content+60";
    }
    for (auto& instr : instruments) {
        if (instr.contentFilePos < 0) {
            continue;
        }
        const qint64 off = totalSizeFmt
                           ? instr.contentFilePos + MIDI_IN_CONTENT
                           : instr.contentFilePos + static_cast<qint64>(instr.offset) + MIDI_AFTER_CONTENT;
        if (off >= static_cast<qint64>(ds.device()->size())) {
            continue;
        }
        if (int prg = readMidiByteAt(ds, off)) {
            instr.midiProgram = prg;
        }
    }
}

void readMidiPrograms(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    // MIDI table offsets vary by layout. See ENCORE_FORMAT.md §Instrument block.
    if (instruments.empty()) {
        return;
    }
    const bool noTkBlocks = (instruments[0].contentFilePos < 0);
    if (noTkBlocks) {
        const qint64 firstBlockOff = findFirstBlockOffset(ds, /*includePageBlock=*/ true);
        readMidiProgramsNoTk(instruments, ds, firstBlockOff);
        return;
    }
    const bool compact = (instruments[0].offset == 0);
    const bool smallTK = (!compact && instruments[0].offset <= 250);

    if (smallTK) {
        readMidiProgramsSmallTk(instruments, ds);
        // Fallback for mixed-TK files (e.g. v0xC2 with one named TK block and the
        // remaining instruments in a compact ~~~~ block at a fixed layout).
        // Apply compact byte-93 MIDI for any instrument that still lacks both a TK
        // block (contentFilePos<0) and a MIDI program (midiProgram==0).
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
        if (int prg = readMidiByteAt(ds, off)) {
            instruments[n].midiProgram = prg;
        }
    }
}

static void readKeyTranspositionsNoTk(std::vector<EncInstrument>& instruments, QDataStream& ds, qint64 firstBlockOff)
{
    static constexpr qint64 LT_BASE = 2278, LT_STEP = 2158, KEY_OFF = -23;
    static constexpr qint64 CMP_BASE = 390;
    const bool useLargeTk = (firstBlockOff > LT_BASE);
    if (useLargeTk) {
        for (size_t n = 0; n < instruments.size(); ++n) {
            const qint64 off = LT_BASE + KEY_OFF + static_cast<qint64>(n) * LT_STEP;
            if (off < 0 || off >= static_cast<qint64>(ds.device()->size())) {
                continue;
            }
            qint8 sv;
            if (readKeyByteAt(ds, off, sv)) {
                instruments[n].keyTransposeSemitones = sv;
            }
        }
    } else {
        if (instruments.size() != 1) {
            return;
        }
        const qint64 off = CMP_BASE + KEY_OFF;
        if (off >= static_cast<qint64>(ds.device()->size())) {
            return;
        }
        qint8 sv;
        if (readKeyByteAt(ds, off, sv)) {
            instruments[0].keyTransposeSemitones = sv;
        }
    }
}

static void readKeyTranspositionsSmallTk(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    // Standard Encore 5.x: key is 23 bytes before the MIDI position
    //   (KEY_OFF = -23 from MIDI base = 76-23 = 53 bytes past content end).
    // Encore 4.x total-size variant: key at content[42] (matches v0xA6 TK layout).
    static constexpr qint64 KEY_AFTER_CONTENT = 76 - 23;   // = 53 (standard 5.x)
    static constexpr qint64 KEY_IN_CONTENT    = 42;         // Encore 4.x total-size variant
    const bool totalSizeFmt = isTotalBlockSizeTkFmt(instruments);
    for (auto& instr : instruments) {
        if (instr.contentFilePos < 0) {
            continue;
        }
        const qint64 off = totalSizeFmt
                           ? instr.contentFilePos + KEY_IN_CONTENT
                           : instr.contentFilePos + static_cast<qint64>(instr.offset) + KEY_AFTER_CONTENT;
        if (off < 0 || off >= static_cast<qint64>(ds.device()->size())) {
            continue;
        }
        qint8 sv;
        if (readKeyByteAt(ds, off, sv)) {
            instr.keyTransposeSemitones = sv;
        }
    }
}

static void readKeyTranspositionsCompact(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    if (instruments.size() != 1) {
        return;
    }
    static constexpr qint64 MIDI_BASE = 390, KEY_OFF = -23;
    const qint64 off = MIDI_BASE + KEY_OFF;
    if (off >= static_cast<qint64>(ds.device()->size())) {
        return;
    }
    qint8 sv;
    if (readKeyByteAt(ds, off, sv)) {
        instruments[0].keyTransposeSemitones = sv;
    }
}

void readKeyTranspositions(std::vector<EncInstrument>& instruments, QDataStream& ds)
{
    if (instruments.empty()) {
        return;
    }
    const bool noTkBlocks = (instruments[0].contentFilePos < 0);
    if (noTkBlocks) {
        const qint64 firstBlockOff = findFirstBlockOffset(ds, /*includePageBlock=*/ true);
        readKeyTranspositionsNoTk(instruments, ds, firstBlockOff);
        return;
    }
    const bool compact = (instruments[0].offset == 0);
    const bool tkBased = (instruments[0].offset > 250);

    if (compact) {
        readKeyTranspositionsCompact(instruments, ds);
        return;
    }

    if (!tkBased) {
        readKeyTranspositionsSmallTk(instruments, ds);
        return;
    }

    static constexpr qint64 PRG_BASE = 2278, PRG_STEP = 2158, KEY_OFF = -23;
    for (size_t n = 0; n < instruments.size(); ++n) {
        const qint64 off = PRG_BASE + KEY_OFF + static_cast<qint64>(n) * PRG_STEP;
        if (off < 0 || off >= static_cast<qint64>(ds.device()->size())) {
            continue;
        }
        qint8 sv;
        if (readKeyByteAt(ds, off, sv)) {
            instruments[n].keyTransposeSemitones = sv;
        }
    }
}
} // namespace

bool EncFormatReader_V0xC4Base::readInstrumentMeta(std::vector<EncInstrument>& instruments,
                                                   QDataStream& ds,
                                                   const EncRoot& /*file*/) const
{
    recoverMissingNames(instruments, ds);
    readMidiPrograms(instruments, ds);
    readKeyTranspositions(instruments, ds);
    return true;
}
} // namespace mu::iex::enc
