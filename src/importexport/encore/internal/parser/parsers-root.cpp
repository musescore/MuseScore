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
// in ENCORE_FORMAT.md §WINI block.
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
// See ENCORE_FORMAT.md §PREC block.
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

bool EncRoot::read(QDataStream& ds)
{
    if (!header.readMagicAndVersion(ds)) {
        return false;
    }
    fmt = EncFormatReader::create(header.chuMagio, header.magic);
    if (!header.read(ds, *fmt)) {
        return false;
    }
    EncCharSize charsize = EncCharSize::ONE_BYTE;

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
            // Some formats (v0xA6) store per-staff key indices in the LINE block rather than in
            // staffData; let the format reader extract them. No-op for v0xC2/C4.
            fmt->readLineStaffKeys(line, ds, lineContentStart);
            lines.push_back(std::move(line));
        } else if (nextId == "MEAS") {
            EncMeasure meas;
            meas.read(ds, varSize, *fmt);
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
            // v0xA6: Key transposition is at content+42; read before EncInstrument::read.
            // v0xC4 reads Key from outside the TK block in readInstrumentMeta instead.
            fmt->readKeyFromTKBlock(instr, ds, ds.device()->pos());
            // v0xC4: Encore 5.0.2 may use UTF-16 LE names; probe determines the encoding.
            instr.read(ds, varSize, fmt->probeInstrumentEncoding());
            charsize = instr.charSize();
            instruments.push_back(std::move(instr));
        } else {
            skipBlock(ds, varSize);
        }
    }

    if (instruments.empty()) {
        // No TK blocks found; seed empty entries so readInstrumentMeta can recover names.
        for (int i = 0; i < header.instrumentCount; ++i) {
            instruments.emplace_back();
        }
    }

    // Pad to instrumentCount: some v0xC4 files have fewer TK blocks than declared.
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
