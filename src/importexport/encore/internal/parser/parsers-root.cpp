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

// Top-level file walk: scan block magics and dispatch to each block reader; page/print setup.

#include "elem.h"

#include <algorithm>

#include <QRegularExpression>

#include "readers.h"

namespace mu::iex::enc {
// ---------------------------------------------------------------------------
// EncRoot - top-level container
// ---------------------------------------------------------------------------

// WINI page-setup block: page margins in typographic points (1/72 inch). Layout and field offsets
// in ENCORE_FORMAT.md §5.8 Margins block (WINI).
void EncPageSetup::read(QDataStream& ds, quint32 varSize)
{
    if (varSize < 40) {
        skipBlock(ds, varSize);
        return;
    }
    qint32 t = 0, l = 0, b = 0, r = 0;
    ds.skipRawData(24);   // skip fields 0..11 (window/screen data)
    ds >> t >> l >> b >> r;
    // The four margins come from an attacker-controlled block at a magic offset;
    // only trust them when the reads stayed within the stream.
    const bool ok = (ds.status() == QDataStream::Ok);
    skipBlock(ds, static_cast<qint64>(varSize) - 40);
    if (ok && b > 0 && r > 0 && b > t && r > l) {
        hasData    = true;
        top        = t;
        left       = l;
        bottomEdge = b;
        rightEdge  = r;
    }
}

bool isInstrumentMagic(const QString& magic)
{
    return magic.length() == 4
           && magic.at(0) == 'T' && magic.at(1) == 'K'
           && magic.at(2).isDigit() && magic.at(3).isDigit();
}

// The two digits of a TKnn magic. Only a lone block is placed by this; see placeInstrumentsInSlots.
static int instrumentMagicIndex(const QString& magic)
{
    return (magic.at(2).digitValue() * 10) + magic.at(3).digitValue();
}

// Instrument entries sit in a fixed-stride table (see ENCORE_FORMAT.md §5.1 Instrument block), so
// which instrument a TK block describes is decided by WHERE it sits, not by the digits in its magic:
// files exist whose seven entries are labelled TK00 TK01 TK02 TK04 TK04 TK05 TK06. Reorder the
// blocks, discovered in file order, into their table slots, leaving a gap for any entry whose block
// header was zeroed out (readInstrumentMeta then names it from its position). A lone block gives no
// stride to measure, so there the magic is the only thing to go on. Anything that does not divide
// evenly into slots is left in discovery order.
static void placeInstrumentsInSlots(std::vector<EncInstrument>& instruments,
                                    const std::vector<int>& magicIndex, int instrumentCount)
{
    static constexpr qint64 kEntryTableBase = 194;
    const int n = static_cast<int>(instruments.size());
    if (n == 0 || instrumentCount <= 0) {
        return;
    }
    auto entryStart = [&](int i) { return instruments[i].contentFilePos - 8; };

    std::vector<int> slot(n, -1);
    if (n == 1) {
        slot[0] = magicIndex[0];
    } else {
        const qint64 stride = entryStart(1) - entryStart(0);
        if (stride <= 0) {
            return;
        }
        for (int i = 0; i < n; ++i) {
            const qint64 delta = entryStart(i) - kEntryTableBase;
            if (delta < 0 || delta % stride != 0) {
                return;
            }
            slot[i] = static_cast<int>(delta / stride);
        }
    }

    int highest = -1;
    for (int i = 0; i < n; ++i) {
        if (slot[i] < 0 || slot[i] >= instrumentCount || slot[i] <= highest) {
            return;   // out of range or not strictly increasing: keep discovery order
        }
        highest = slot[i];
    }
    if (highest == n - 1) {
        return;   // already one block per slot in order
    }

    std::vector<EncInstrument> placed(static_cast<size_t>(highest) + 1);
    for (int i = 0; i < n; ++i) {
        placed[static_cast<size_t>(slot[i])] = std::move(instruments[i]);
    }
    instruments = std::move(placed);
}

bool isKnownMagic(const QString& magic)
{
    return magic == "LINE" || magic == "MEAS" || magic == "TITL" || magic == "TEXT"
           || magic == "WINI" || magic == "PREC" || isInstrumentMagic(magic);
}

QString findNextKnownMagic(QDataStream& ds)
{
    QString magic;
    for (int i = 0; i < 4 && !ds.atEnd(); ++i) {
        quint8 ch;
        ds >> ch;
        magic.append(QChar(ch));
    }
    // Limit scan to 1 MiB; TK blocks max ~2 KiB, more gap means corrupt file.
    constexpr int kMaxScanBytes = 1 << 20;
    int scanned = 0;
    while (!isKnownMagic(magic) && !ds.atEnd() && scanned < kMaxScanBytes) {
        magic.remove(0, 1);
        quint8 ch;
        ds >> ch;
        magic.append(QChar(ch));
        ++scanned;
    }
    if (!isKnownMagic(magic)) {
        magic.clear();
    }
    return magic;
}

// Parse a Windows DEVMODE (the PREC block) into page orientation, paper size and notation
// scale. The device-name prefix is 32 bytes for an ANSI DEVMODE and 64 bytes (UTF-16) for a
// Unicode one; the fixed fields follow at the same relative offsets. Detect the variant by
// trying both bases and keeping the one whose dmOrientation is a valid 1 (portrait) or 2
// (landscape); range-check the rest so a wrong base or an unusual driver blob is ignored.
// See ENCORE_FORMAT.md §5.7 Printer block (PREC).
static void parsePrecDevmode(const QByteArray& buf, EncPrintSetup& out)
{
    auto s16 = [&](int off) -> int {
        if (off < 0 || off + 2 > buf.size()) {
            return -1;
        }
        return static_cast<qint16>(static_cast<quint8>(buf[off])
                                   | (static_cast<quint8>(buf[off + 1]) << 8));
    };
    for (int base : { 32, 64 }) {
        const int orient = s16(base + 12);
        const int paper  = s16(base + 14);
        const int scale  = s16(base + 20);
        if ((orient != 1 && orient != 2) || paper < 0) {
            continue;
        }
        out.hasData     = true;
        out.orientation = orient;
        out.paperSize   = paper;
        out.paperLength = s16(base + 16);
        out.paperWidth  = s16(base + 18);
        out.scale       = (scale > 0 && scale <= 400) ? scale : 0;
        return;
    }
}

// SCO5 (macOS Encore 5) stores the PREC page setup as an NSPrintInfo XML plist
// rather than a Windows DEVMODE. Pull orientation, paper size and notation scale
// from it; the page margins are NOT in this block (the plist only carries the
// printer's imageable rects, not Encore's document margins).
bool parsePrecPlist(const QByteArray& buf, EncPrintSetup& out)
{
    const QString s = QString::fromUtf8(buf);
    if (!s.contains("PMOrientation") && !s.contains("PaperName")) {
        return false;
    }
    auto firstMatch = [&](const QString& pattern) -> QString {
        QRegularExpression re(pattern);
        QRegularExpressionMatch m = re.match(s);
        return m.hasMatch() ? m.captured(1) : QString();
    };

    // Value entries put the data tag immediately after the key (the dict wrappers do not).
    const int orient = firstMatch(QStringLiteral("PMOrientation</key>\\s*<integer>(-?\\d+)</integer>")).toInt();
    const double scaling = firstMatch(QStringLiteral("PMScaling</key>\\s*<real>([-0-9.]+)</real>")).toDouble();
    QString paper = firstMatch(QStringLiteral("PMTiogaPaperName</key>\\s*<string>([^<]+)</string>"));
    if (paper.isEmpty()) {
        paper = firstMatch(QStringLiteral("PMPaperName</key>\\s*<string>([^<]+)</string>"));
    }

    const QString p = paper.toLower();
    int paperCode = 0;
    if (p.contains("a4")) {
        paperCode = 9;
    } else if (p.contains("a3")) {
        paperCode = 8;
    } else if (p.contains("a5")) {
        paperCode = 11;
    } else if (p.contains("legal")) {
        paperCode = 5;
    } else if (p.contains("letter")) {
        paperCode = 1;
    }

    if (paperCode == 0 && orient != 1 && orient != 2) {
        return false;
    }
    out.hasData     = true;
    out.orientation = (orient == 1 || orient == 2) ? orient : 1;
    out.paperSize   = paperCode;
    out.paperLength = 0;
    out.paperWidth  = 0;
    // PMScaling is a fraction (1.2 = 120%); store as a percent like dmScale.
    out.scale       = (scaling > 0.0 && scaling <= 4.0)
                      ? static_cast<int>(scaling * 100.0 + 0.5) : 0;
    return true;
}

// PREC carries page orientation, paper size and notation scale. SCOW stores it as a Windows
// DEVMODE; SCO5 (macOS Encore 5) stores it as an NSPrintInfo XML plist. Detect by content.
static void readPrintSetup(QDataStream& ds, quint32 varSize, EncPrintSetup& out)
{
    // varSize is untrusted: a huge value (e.g. 0x40000000) would request ~1 GB and a value above
    // INT_MAX casts to a negative int (UB/abort in QByteArray). A PREC block is a Windows DEVMODE
    // (~220 bytes) or a small NSPrintInfo plist; clamp to a few KB and to the bytes actually left.
    constexpr qint64 kMaxPrecBytes = 64 * 1024;
    const qint64 startPos = ds.device()->pos();
    const qint64 remaining = ds.device()->size() - startPos;
    qint64 want = static_cast<qint64>(varSize);
    if (want < 0) {
        want = 0;
    }
    want = std::min({ want, remaining, kMaxPrecBytes });
    QByteArray buf(static_cast<int>(want), '\0');
    const int n = (want > 0) ? ds.readRawData(buf.data(), static_cast<int>(want)) : 0;
    // Always advance to the declared block end so the next magic scan stays aligned even when we
    // only read (or trusted) a clamped prefix.
    skipToBlockEnd(ds, startPos, static_cast<qint64>(varSize));
    if (n <= 0) {
        return;
    }
    buf.resize(n);
    if (buf.startsWith("<?xml") || buf.contains("<plist")) {
        parsePrecPlist(buf, out);
    } else {
        parsePrecDevmode(buf, out);
    }
}

void addSpannerEnds(std::vector<EncMeasure>& measures)
{
    std::vector<MeasureElemVec> extra(measures.size());

    for (size_t i = 0; i < measures.size(); ++i) {
        for (const auto& elem : measures[i].elements) {
            EncMeasureElem* e = elem.get();
            if (auto* orna = dynamic_cast<EncOrnament*>(e)) {
                EncOrnamentType ot = orna->ornType();
                if (ot == EncOrnamentType::SLURSTART || ot == EncOrnamentType::WEDGESTART) {
                    EncOrnamentType endType = (ot == EncOrnamentType::SLURSTART)
                                              ? EncOrnamentType::SLURSTOP
                                              : EncOrnamentType::WEDGESTOP;
                    auto endOrna = std::make_unique<EncOrnament>(*orna);
                    endOrna->setOrnType(endType);
                    endOrna->xoffset = orna->xoffset2;
                    int endMeas = static_cast<int>(i) + orna->alMezuro;
                    if (endMeas >= 0 && static_cast<size_t>(endMeas) < extra.size()) {
                        extra[endMeas].push_back(std::move(endOrna));
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < measures.size(); ++i) {
        for (auto& e : extra[i]) {
            measures[i].elements.push_back(std::move(e));
        }
    }
}

// Parse the 8 tuning slots ending at file offset `blockEnd`: open-string MIDI pitches (low -> high)
// then pad bytes (0x7F/0x58); the count is the leading non-pad slots. See ENCORE_FORMAT.md.
static void parseTabTuningBefore(QIODevice* dev, qint64 blockEnd, EncTabTuning& out)
{
    if (!dev || blockEnd < 8) {
        return;
    }
    const qint64 saved = dev->pos();
    if (!dev->seek(blockEnd - 8)) {
        return;
    }
    const QByteArray tuningBytes = dev->read(8);
    dev->seek(saved);
    if (tuningBytes.size() < 8) {
        return;
    }
    auto isPad = [](quint8 b) { return b == 0x7F || b == 0x58; };
    std::vector<int> pitches;
    for (int i = 0; i < 8; ++i) {
        const quint8 b = static_cast<quint8>(tuningBytes.at(i));
        if (isPad(b)) {
            break;                       // pad marks the end of the tuning
        }
        if (b < 20 || b > 108) {
            pitches.clear();             // implausible open-string pitch: not a tuning array
            break;
        }
        pitches.push_back(b);
    }
    if (!pitches.empty()) {
        out.hasData = true;
        out.openStringPitches = std::move(pitches);
    }
}

// Score-level fallback tuning: the 8 slots before the first PAGE block (the last TK block's tail).
static void readTabTuning(QDataStream& ds, EncTabTuning& out)
{
    QIODevice* dev = ds.device();
    if (!dev) {
        return;
    }
    const qint64 saved = dev->pos();
    if (!dev->seek(0)) {
        return;
    }
    const int page = dev->readAll().indexOf("PAGE");
    dev->seek(saved);
    if (page >= 9) {
        parseTabTuningBefore(dev, page, out);
    }
}

// True when the score has a tab staff but no notation staff; such files store the tab's notes as
// pitch-bearing REST elements that must be read as notes (mixed files keep the tab as a note-less view).
static bool isTabOnlyScore(const std::vector<EncLine>& lines)
{
    if (lines.empty()) {
        return false;
    }
    bool hasTab = false;
    for (const auto& sd : lines[0].staffData) {
        const bool isNotation = sd.staffType == EncStaffType::MELODY
                                && sd.clef != EncClefType::TAB && sd.clef != EncClefType::PERC;
        if (isNotation) {
            return false;
        }
        if (sd.clef == EncClefType::TAB || sd.staffType == EncStaffType::TAB) {
            hasTab = true;
        }
    }
    return hasTab;
}

bool EncRoot::read(QDataStream& ds)
{
    if (!header.readMagicAndVersion(ds)) {
        return false;
    }
    // The format version at 0x28 selects the element body layout, and the reader has to know it
    // before the header is read (reading the header needs the reader). Peek it and restore the
    // cursor; 0x28 is the same offset in every format.
    // See ENCORE_FORMAT.md §1.7 Choosing a reader.
    quint16 formatVersion = 0;
    if (QIODevice* dev = ds.device()) {
        const qint64 saved = dev->pos();
        if (dev->seek(0x28)) {
            ds >> formatVersion;
        }
        dev->seek(saved);
    }
    fmt = EncFormatReader::create(header.chuMagio, header.magic, formatVersion);
    if (!header.read(ds, *fmt)) {
        return false;
    }
    readTabTuning(ds, tabTuning);
    EncCharSize charsize = EncCharSize::ONE_BYTE;
    std::vector<int> instrumentMagicIndices;

    while (!ds.atEnd()) {
        // Truncated/corrupt input leaves the stream in a non-Ok state; stop rather than
        // fabricate zero-filled "valid-looking" blocks from the past-EOF zero fill.
        if (ds.status() != QDataStream::Ok) {
            break;
        }
        QString nextId = findNextKnownMagic(ds);
        if (nextId.isEmpty()) {
            break;
        }
        quint32 varSize;
        ds >> varSize;

        if (nextId == "LINE") {
            const qint64 lineContentStart = ds.device()->pos();
            EncLine line;
            line.read(ds, varSize, header.staffPerSystem);
            // Where the per-staff key, clef and size live in a LINE staff entry this parse
            // cannot walk, the reader extracts them: EncFormatReader::readLineStaffEntries.
            fmt->readLineStaffEntries(line, ds, lineContentStart);
            lines.push_back(std::move(line));
        } else if (nextId == "MEAS") {
            EncMeasure meas;
            meas.read(ds, varSize, *fmt, isTabOnlyScore(lines));
            meas.calculateRealDurations(fmt->hasGraceTimeBorrowing(), *fmt);
            // Skip extra "ghost" MEAS blocks beyond the declared measureCount.
            if (header.measureCount > 0
                && static_cast<int>(measures.size()) >= header.measureCount) {
                continue;
            }
            measures.push_back(std::move(meas));
        } else if (nextId == "TITL") {
            EncTitle tmp;
            tmp.read(ds, varSize, charsize);
            // Encore writes one TITL block per page; page 2+ blocks are often
            // blank. Keep the first block that has non-empty content and ignore
            // subsequent empty ones.
            if (tmp.hasContent() || !titleBlock.hasContent()) {
                titleBlock = std::move(tmp);
            }
        } else if (nextId == "TEXT") {
            // Multi-part files write one TEXT block per part view, with the same
            // strings reordered. ORN tind indices match only the first (score)
            // block, so keep the first non-empty block and skip later ones
            // (mirrors the TITL handling above).
            EncTextBlock tmp;
            tmp.read(ds, varSize, fmt->textBlockEntryTextOffset(), fmt->textBlockEntryHasRunHeader());
            if (textBlock.entries.empty()) {
                textBlock = std::move(tmp);
            }
        } else if (nextId == "WINI") {
            pageSetup.read(ds, varSize);
        } else if (nextId == "PREC") {
            readPrintSetup(ds, varSize, printSetup);
        } else if (isInstrumentMagic(nextId)) {
            EncInstrument instr;
            instr.contentFilePos = ds.device()->pos();
            // Formats that keep the Key transposition inside the TK block read it here:
            // EncFormatReader::readKeyFromTKBlock. The others take it in readInstrumentMeta.
            fmt->readKeyFromTKBlock(instr, ds, ds.device()->pos());
            // Some files use UTF-16 LE names; the probe decides. See ENCORE_FORMAT.md §7.8 Text encoding.
            instr.read(ds, varSize, fmt->probeInstrumentEncoding());
            charsize = instr.charSize();
            // Each TK block carries its own 8-slot tab tuning just before the trailing 8-byte header
            // of the next block; read the one for this track.
            parseTabTuningBefore(ds.device(), instr.contentFilePos + varSize - 8, instr.tabTuning);
            instrumentMagicIndices.push_back(instrumentMagicIndex(nextId));
            instruments.push_back(std::move(instr));
        } else {
            skipBlock(ds, varSize);
        }
    }

    // Tab-only notes come from REST-layout elements with no face value; derive it from the
    // realDuration now known from tick gaps (the pitch was already read).
    for (auto& meas : measures) {
        for (auto& elem : meas.elements) {
            auto* note = dynamic_cast<EncNote*>(elem.get());
            if (note && note->fromTabFingering && note->faceValue == 0) {
                note->faceValue = ticks2faceValue(note->realDuration > 0 ? note->realDuration : 240);
            }
        }
    }

    placeInstrumentsInSlots(instruments, instrumentMagicIndices, header.instrumentCount);

    if (instruments.empty()) {
        // No TK blocks found; seed empty entries so readInstrumentMeta can recover names.
        for (int i = 0; i < header.instrumentCount; ++i) {
            instruments.emplace_back();
        }
    }

    // Pad to instrumentCount: some files have fewer TK blocks than declared.
    while (static_cast<int>(instruments.size()) < header.instrumentCount) {
        instruments.emplace_back();
    }

    fmt->readInstrumentMeta(instruments, ds, *this);

    // "Part N" fallback for any instrument whose name is still empty after recovery.
    for (int i = 0; i < static_cast<int>(instruments.size()); ++i) {
        if (instruments[i].name.isEmpty()) {
            instruments[i].name = QString("Part %1").arg(i + 1);
        }
    }

    // Grand-staff instruments have two LINE entries with same instrumentIndex(), staffIndex() 0 and 1.
    if (!lines.empty()) {
        for (const auto& lsd : lines[0].staffData) {
            const int ii = static_cast<int>(lsd.instrumentIndex());
            const int si = static_cast<int>(lsd.staffIndex());
            if (ii >= 0 && ii < static_cast<int>(instruments.size())) {
                instruments[ii].nstaves = std::max(instruments[ii].nstaves, si + 1);
            }
        }
    }

    addSpannerEnds(measures);
    return true;
}
} // namespace mu::iex::enc
